################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AXI/AXI_DMA.c \
../src/AXI/AXI_IO.c 

OBJS += \
./src/AXI/AXI_DMA.o \
./src/AXI/AXI_IO.o 

C_DEPS += \
./src/AXI/AXI_DMA.d \
./src/AXI/AXI_IO.d 


# Each subdirectory must supply rules for building sources it contributes
src/AXI/%.o: ../src/AXI/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 Linux gcc compiler'
	arm-linux-gnueabihf-gcc -Wall -O0 -g3 -I"D:\XlinxSDKWorkspace\Arm_Bridge1\src\AXI" -I"D:\XlinxSDKWorkspace\Arm_Bridge1\src\DMA" -I"D:\XlinxSDKWorkspace\Arm_Bridge1\src\fc_hal" -I"D:\XlinxSDKWorkspace\Arm_Bridge1\src\include" -c -fmessage-length=0 -MT"$@" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


