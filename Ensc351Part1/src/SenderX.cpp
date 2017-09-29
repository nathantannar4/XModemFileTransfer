//============================================================================
//
//% Student Name 1: Nathan Tannar
//% Student 1 #: 301258264
//% Student 1 userid (email): ntannar@sfu.ca
//
//% Student Name 2: Shahira Afrin
//% Student 2 #: 301236985
//% Student 2 userid (email): shahiraa@sfu.ca
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
//Used the following website to understand how to convert uint16t to two uint8t
//http://www.avrfreaks.net/forum/converting-uint16t-uint8t
//
//%% Instructions:
//% * Put your name(s), student number(s), userid(s) in the above section.
//% * Also enter the above information in other files to submit.
//% * Edit the "Helpers" line and, if necessary, the "Resources" line.
//% * Your group name should be "P1_<userid1>_<userid2>" (eg. P1_stu1_stu2)
//% * Form groups as described at:  https://courses.cs.sfu.ca/docs/students
//% * Submit files to courses.cs.sfu.ca
//
// File Name   : SenderX.cc
// Version     : September 3rd, 2017
// Description : Starting point for ENSC 351 Project
// Original portions Copyright (c) 2017 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <iostream>
#include <stdint.h> // for uint8_t
#include <string.h> // for memset()
#include <errno.h>
#include <fcntl.h>	// for O_RDWR

#include "myIO.h"
#include "SenderX.h"

using namespace std;

SenderX::SenderX(const char *fname, int d)
:PeerX(d, fname), bytesRd(-1), blkNum(255)
{
}

//-----------------------------------------------------------------------------

/* tries to generate a block.  Updates the
variable bytesRd with the number of bytes that were read
from the input file in order to create the block. Sets
bytesRd to 0 and does not actually generate a block if the end
of the input file had been reached when the previously generated block was
prepared or if the input file is empty (i.e. has 0 length).
*/
void SenderX::genBlk(blkT blkBuf)
{

//	Each block of the transfer looks like:
//	          <SOH><blk #><255-blk #><--128 data bytes--><CTRL-Z><cksum>
//	          <SOH><blk #><255-blk #><--128 data bytes--><CRC MSB ><CRC LSB>

	if (-1 == (bytesRd = myRead(transferringFileD, &blkBuf[3], CHUNK_SZ )))
		ErrorPrinter("myRead(transferringFileD, &blkBuf[3], CHUNK_SZ )", __FILE__, __LINE__, errno);

	else {

		for (ssize_t i = bytesRd; i < 128; i++)
			blkBuf[i + 3] = CTRL_Z;

		/*The checksum is based on the value of all the bytes in the chunk added together.  For
		 example, if the [last] five bytes in the chunk were 45, 12, 64, 236, 173 and the
		 other 123 bytes were zeroes, the sum would be 0+0+...+0+45+12+64+236+173 = 530.
		 However, one must repeatedly subtract 256 from the sum until the result, the
		 checksum, is between 0 and 255.  In this case, the checksum
		 would be 530 - 256 - 256 = 18.*/

		if (Crcflg) //do crc
		{
			uint16_t crc = 0;
			
			uint8_t* buf = &blkBuf[3];
			crc16ns(&crc,buf);
			blkBuf[BLK_SZ_CRC - 2] = (uint8_t)(crc >> 8); // Right shift right to get upper byte of crc
			blkBuf[BLK_SZ_CRC - 1] = (uint8_t)crc; //lower byte of crc
		}
		else //do checksum
		{
			uint8_t checksum = 0;

			for (ssize_t i = 0; i < 128; i++ )
				checksum += blkBuf[i + 3];

			while (checksum > 255)
				checksum -= 255;

			blkBuf[BLK_SZ_CRC - 2] = checksum;
		}

		/*The 1's complement of a byte (to make life easy) is simply 255 minus the
		 byte.  For example, if you had to take the 1's complement of 142, the answer
		 would be 255 - 142 = 133. */
		blkBuf[2] = 255 - blkNum;
		blkBuf[1] = blkNum;
		blkBuf[0] = SOH;

	}

}

void SenderX::sendFile()
{
	transferringFileD = myOpen(fileName, O_RDWR, 0);
	if(transferringFileD == -1) {
		cout /* cerr */ << "Error opening input file named: " << fileName << endl;
		result = "OpenError";
	}
	else {
		cout << "Sender will send " << fileName << endl;

		blkNum = 1;

		// do the protocol, and simulate a receiver that positively acknowledges every
		//	block that it receives.

		// assume 'C' or NAK received from receiver to enable sending with CRC or checksum, respectively
		genBlk(blkBuf); // prepare 1st block
		while (bytesRd)
		{

			// ********* fill in some code here to send a block ***********
			if (Crcflg) {
				myWrite(mediumD, blkBuf, BLK_SZ_CRC);
			} else {
				// checksum
				myWrite(mediumD, blkBuf, BLK_SZ_CRC - 1);
			}

			blkNum ++; // 1st block about to be sent or previous block was ACK'd

			// assume sent block will be ACK'd
			genBlk(blkBuf); // prepare next block
			// assume sent block was ACK'd
		};
		// finish up the protocol, assuming the receiver behaves normally
		// ********* fill in some code here **********
		sendByte(EOT);
		sendByte(EOT);



		//(myClose(transferringFileD));
		if (-1 == myClose(transferringFileD))
			ErrorPrinter("myClose(transferringFileD)", __FILE__, __LINE__, errno);
		result = "Done";
	}
}

