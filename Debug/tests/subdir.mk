################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../tests/AppendTest.cc \
../tests/BufferFillTest.cc \
../tests/ConcatSeekTest.cc \
../tests/RandomTest.cc \
../tests/SeekTest.cc \
../tests/SimpleTest.cc \
../tests/SpeedTest.cc 

OBJS += \
./tests/AppendTest.o \
./tests/BufferFillTest.o \
./tests/ConcatSeekTest.o \
./tests/RandomTest.o \
./tests/SeekTest.o \
./tests/SimpleTest.o \
./tests/SpeedTest.o 

CC_DEPS += \
./tests/AppendTest.d \
./tests/BufferFillTest.d \
./tests/ConcatSeekTest.d \
./tests/RandomTest.d \
./tests/SeekTest.d \
./tests/SimpleTest.d \
./tests/SpeedTest.d 


# Each subdirectory must supply rules for building sources it contributes
tests/%.o: ../tests/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


