################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/w5500/W5500/w5500.c 

OBJS += \
./Src/w5500/W5500/w5500.o 

C_DEPS += \
./Src/w5500/W5500/w5500.d 


# Each subdirectory must supply rules for building sources it contributes
Src/w5500/W5500/%.o Src/w5500/W5500/%.su Src/w5500/W5500/%.cyclo: ../Src/w5500/W5500/%.c Src/w5500/W5500/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-w5500-2f-W5500

clean-Src-2f-w5500-2f-W5500:
	-$(RM) ./Src/w5500/W5500/w5500.cyclo ./Src/w5500/W5500/w5500.d ./Src/w5500/W5500/w5500.o ./Src/w5500/W5500/w5500.su

.PHONY: clean-Src-2f-w5500-2f-W5500

