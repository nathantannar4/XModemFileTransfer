################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Ensc351Part2.cpp \
../src/Medium.cpp \
../src/PeerX.cpp \
../src/ReceiverSS.cpp \
../src/ReceiverX.cpp \
../src/SenderSS.cpp \
../src/SenderX.cpp \
../src/myIO.cpp 

OBJS += \
./src/Ensc351Part2.o \
./src/Medium.o \
./src/PeerX.o \
./src/ReceiverSS.o \
./src/ReceiverX.o \
./src/SenderSS.o \
./src/SenderX.o \
./src/myIO.o 

CPP_DEPS += \
./src/Ensc351Part2.d \
./src/Medium.d \
./src/PeerX.d \
./src/ReceiverSS.d \
./src/ReceiverX.d \
./src/SenderSS.d \
./src/SenderX.d \
./src/myIO.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I"/home/osboxes/XModemFileTransfer/Ensc351" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


