################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/DMA/dma_init.c 

OBJS += \
./src/DMA/dma_init.o 

C_DEPS += \
./src/DMA/dma_init.d 


# Each subdirectory must supply rules for building sources it contributes
src/DMA/%.o: ../src/DMA/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 Linux gcc compiler'
	arm-linux-gnueabihf-gcc -Wall -O0 -g3 -I"D:\XlinxSDKWorkspace\Arm_Bridge1\src\AXI" -I"D:\XlinxSDKWorkspace\Arm_Bridge1\src\DMA" -I"D:\XlinxSDKWorkspace\Arm_Bridge1\src\fc_hal" -I"D:\XlinxSDKWorkspace\Arm_Bridge1\src\include" -c -fmessage-length=0 -MT"$@" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


