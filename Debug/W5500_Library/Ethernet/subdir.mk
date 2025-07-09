################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../W5500_Library/Ethernet/socket.c \
../W5500_Library/Ethernet/wizchip_conf.c 

OBJS += \
./W5500_Library/Ethernet/socket.o \
./W5500_Library/Ethernet/wizchip_conf.o 

C_DEPS += \
./W5500_Library/Ethernet/socket.d \
./W5500_Library/Ethernet/wizchip_conf.d 


# Each subdirectory must supply rules for building sources it contributes
W5500_Library/Ethernet/%.o W5500_Library/Ethernet/%.su W5500_Library/Ethernet/%.cyclo: ../W5500_Library/Ethernet/%.c W5500_Library/Ethernet/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../W5500_Library/Ethernet -I../W5500_Library/Internet -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-W5500_Library-2f-Ethernet

clean-W5500_Library-2f-Ethernet:
	-$(RM) ./W5500_Library/Ethernet/socket.cyclo ./W5500_Library/Ethernet/socket.d ./W5500_Library/Ethernet/socket.o ./W5500_Library/Ethernet/socket.su ./W5500_Library/Ethernet/wizchip_conf.cyclo ./W5500_Library/Ethernet/wizchip_conf.d ./W5500_Library/Ethernet/wizchip_conf.o ./W5500_Library/Ethernet/wizchip_conf.su

.PHONY: clean-W5500_Library-2f-Ethernet

