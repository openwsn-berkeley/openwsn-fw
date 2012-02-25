################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board.c \
../cr_startup_lpc17.c \
../leds.c \
../main.c \
../timer.c 

OBJS += \
./board.o \
./cr_startup_lpc17.o \
./leds.o \
./main.o \
./timer.o 

C_DEPS += \
./board.d \
./cr_startup_lpc17.d \
./leds.d \
./main.d \
./timer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv2p00_LPC17xx -D__CODE_RED -D__REDLIB__ -I"/home/xvilajosana/Storage/Development/wsndev/workspaces/workspace_lpc1769/SimpleDemo" -I"/home/xvilajosana/Storage/Development/wsndev/workspaces/workspace_lpc1769/FreeRTOS_Library/include" -I"/home/xvilajosana/Storage/Development/wsndev/workspaces/workspace_lpc1769/FreeRTOS_Library/portable" -I"/home/xvilajosana/Storage/Development/wsndev/workspaces/workspace_lpc1769/CMSISv2p00_LPC17xx/inc" -O0 -g3 -fsigned-char -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


