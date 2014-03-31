################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/pybindings/PyXCDF.cc 

OBJS += \
./src/pybindings/PyXCDF.o 

CC_DEPS += \
./src/pybindings/PyXCDF.d 


# Each subdirectory must supply rules for building sources it contributes
src/pybindings/%.o: ../src/pybindings/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


