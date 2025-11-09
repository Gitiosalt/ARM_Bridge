################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/fc_hal/fc_hal.c 

OBJS += \
./src/fc_hal/fc_hal.o 

C_DEPS += \
./src/fc_hal/fc_hal.d 


# Each subdirectory must supply rules for building sources it contributes
src/fc_hal/%.o: ../src/fc_hal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 Linux gcc compiler'
	arm-linux-gnueabihf-gcc -Wall -O0 -g3 -I"F:\github\ARM_Bridge\ARM_Bridge\src\AXI" -I"F:\github\ARM_Bridge\ARM_Bridge\src\DMA" -I"F:\github\ARM_Bridge\ARM_Bridge\src\fc_hal" -I"F:\github\ARM_Bridge\ARM_Bridge\src\include" -c -fmessage-length=0 -MT"$@" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


