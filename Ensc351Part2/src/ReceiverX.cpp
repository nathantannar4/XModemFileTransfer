//============================================================================
//
//% Student Name 1: student1
//% Student 1 #: 123456781
//% Student 1 userid (email): stu1 (stu1@sfu.ca)
//
//% Student Name 2: student2
//% Student 2 #: 123456782
//% Student 2 userid (email): stu2 (stu2@sfu.ca)
//
//% Below, edit to list any people who helped you with the code in this file,
//%      or put 'None' if nobody helped (the two of) you.
//
// Helpers: _everybody helped us/me with the assignment (list names or put 'None')__
//
// Also, list any resources beyond the course textbooks and the course pages on Piazza
// that you used in making your submission.
//
// Resources:  ___________
//
//%% Instructions:
//% * Put your name(s), student number(s), userid(s) in the above section.
//% * Also enter the above information in other files to submit.
//% * Edit the "Helpers" line and, if necessary, the "Resources" line.
//% * Your group name should be "P2_<userid1>_<userid2>" (eg. P1_stu1_stu2)
//% * Form groups as described at:  https://courses.cs.sfu.ca/docs/students
//% * Submit files to courses.cs.sfu.ca
//
// File Name   : ReceiverX.cpp
// Version     : September 3rd, 2017
// Description : Starting point for ENSC 351 Project Part 2
// Original portions Copyright (c) 2017 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <string.h> // for memset()
#include <fcntl.h>
#include <stdint.h>
#include <iostream>
#include "myIO.h"
#include "ReceiverX.h"
#include "VNPE.h"

using namespace std;

ReceiverX::
ReceiverX(int d, const char *fname, bool useCrc)
:PeerX(d, fname, useCrc), goodBlk(false), goodBlk1st(false), numLastGoodBlk(0)
{
	NCGbyte = useCrc ? 'C' : NAK;
}

void ReceiverX::receiveFile()
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	transferringFileD = PE2(creat(fileName, mode), fileName);
	if(transferringFileD == -1) {
		cerr << "Error opening input file named: " << fileName << endl;
		result = "OpenError";
	}

	// ***** improve this member function *****

	// below is just an example template.  You can follow a
	// 	different structure if you want.

	// inform sender that the receiver is ready and that the
	//		sender can send the first block
	sendByte(NCGbyte);

	while(PE_NOT(myRead(mediumD, rcvBlk, 1), 1), (rcvBlk[0] != CAN), (rcvBlk[0] != EOT))
	{
		if (rcvBlk[0] == SOH) {
			getRestBlk();
			if (goodBlk1st)
			{
				sendByte(ACK);
				writeChunk();
			}
			else
				sendByte(NAK); // checksum or crc check failed
		}
		else
			// TODO: Wait for medium to go quiet
			sendByte(NAK);
	}

	char byteToReceive = rcvBlk[0];

	if (byteToReceive == CAN)
	{
		// TODO: Implement CTRL+Z to cancel transfer in Part3
		result = "SenderCancelled";
	}
	else if (byteToReceive == EOT)
	{
		sendByte(NAK); // NAK the first EOT
		PE_NOT(myRead(mediumD, rcvBlk, 1), 1);

		byteToReceive = rcvBlk[0];
		if (byteToReceive == EOT)
		{
			sendByte(ACK); // ACK the second EOT
			result = "Done";
		}
		else
		{
			sendByte(NAK); // NAK the non existing EOT
			result = "Done but only received 1 EOT";
		}
	}

	PE(close(transferringFileD));
}

/* Only called after an SOH character has been received.
The function tries
to receive the remaining characters to form a complete
block.  The member variable goodBlk will be made false if
the received block formed has something
wrong with it, like the checksum being incorrect.  The member
variable goodBlk1st will be made true if this is the first
time that the block was received in "good" condition.
*/
void ReceiverX::getRestBlk()
{
	// ********* this function must be improved ***********
	PE_NOT(myReadcond(mediumD, &rcvBlk[1], REST_BLK_SZ_CRC, REST_BLK_SZ_CRC, 0, 0), REST_BLK_SZ_CRC);

	if (Crcflg)
		crc(rcvBlk);
	else
		checksum(rcvBlk);

	goodBlk1st = goodBlk = true;
}

void ReceiverX::checksum(blkT blkBuf)
{
	uint8_t recievedCheckSum = rcvBlk[PAST_CHUNK]; // The checksum calculated by the sender
	uint8_t sum = 0;
	for(int i=DATA_POS + 1; i < (BLK_SZ_CRC - 1); i++ )
		sum += blkBuf[i];
	goodBlk = (sum == recievedCheckSum);
}

void ReceiverX::crc(blkT blkBuf)
{
	/* calculate and add CRC in network byte order */
	uint16_t crcCheck;
	crc16ns(&crcCheck, &blkBuf[DATA_POS]);
	goodBlk = ((uint8_t)(crcCheck >> 8) == blkBuf[BLK_SZ_CRC - 2]) && ((uint8_t)(crcCheck) == blkBuf[BLK_SZ_CRC - 1]);
}

//Write chunk (data) in a received block to disk
void ReceiverX::writeChunk()
{
	PE_NOT(write(transferringFileD, &rcvBlk[DATA_POS], CHUNK_SZ), CHUNK_SZ);
}

//Send 8 CAN characters in a row to the XMODEM sender, to inform it of
//	the cancelling of a file transfer
void ReceiverX::can8()
{
	// no need to space CAN chars coming from receiver
	const int canLen=8; // move to defines.h
    char buffer[canLen];
    memset( buffer, CAN, canLen);
    PE_NOT(myWrite(mediumD, buffer, canLen), canLen);
}

