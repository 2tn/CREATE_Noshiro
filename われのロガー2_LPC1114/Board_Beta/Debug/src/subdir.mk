################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Board_Beta.c \
../src/SD.c \
../src/comm.c \
../src/cr_startup_lpc11xx.c \
../src/crp.c \
../src/format.c \
../src/i2c.c \
../src/interface.c \
../src/logger.c \
../src/sensor.c 

OBJS += \
./src/Board_Beta.o \
./src/SD.o \
./src/comm.o \
./src/cr_startup_lpc11xx.o \
./src/crp.o \
./src/format.o \
./src/i2c.o \
./src/interface.o \
./src/logger.o \
./src/sensor.o 

C_DEPS += \
./src/Board_Beta.d \
./src/SD.d \
./src/comm.d \
./src/cr_startup_lpc11xx.d \
./src/crp.d \
./src/format.d \
./src/i2c.d \
./src/interface.d \
./src/logger.d \
./src/sensor.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M0 -D__USE_CMSIS=CMSISv2p00_LPC11xx -D__LPC11XX__ -D__REDLIB__ -I"C:\Users\Yusuke\Documents\LPCXpresso_7.7.2_379\workspace\CMSISv2p00_LPC11xx\inc" -Og -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


