################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Audio.c \
../Src/BmpDecoder.c \
../Src/CFunctions.c \
../Src/Commands.c \
../Src/Custom.c \
../Src/Draw.c \
../Src/Editor.c \
../Src/External.c \
../Src/FileIO.c \
../Src/Flash.c \
../Src/Functions.c \
../Src/GPS.c \
../Src/GUI.c \
../Src/I2C.c \
../Src/MATHS.c \
../Src/MMBasic.c \
../Src/MM_Custom.c \
../Src/MM_Misc.c \
../Src/Memory.c \
../Src/MiscSTM32.c \
../Src/Onewire.c \
../Src/Operators.c \
../Src/OtherDisplays.c \
../Src/PWM.c \
../Src/SPI-LCD.c \
../Src/SPI.c \
../Src/SSD1963.c \
../Src/Serial.c \
../Src/SerialFileIO.c \
../Src/Timers.c \
../Src/Touch.c \
../Src/XModem.c \
../Src/cJSON.c \
../Src/debug.c \
../Src/ff.c \
../Src/ffunicode.c \
../Src/hxcmod.c \
../Src/keyboard.c \
../Src/main.c \
../Src/mmc_stm32.c \
../Src/picojpeg.c \
../Src/reciter.c \
../Src/regex.c \
../Src/render.c \
../Src/sam.c \
../Src/sprites.c \
../Src/stm32h7xx_hal_msp.c \
../Src/stm32h7xx_it.c \
../Src/syscalls.c \
../Src/system_stm32h7xx.c \
../Src/turtle.c \
../Src/upng.c \
../Src/usb_host.c \
../Src/usbh_conf.c \
../Src/usbh_platform.c 

S_UPPER_SRCS += \
../Src/assember.S 

OBJS += \
./Src/Audio.o \
./Src/BmpDecoder.o \
./Src/CFunctions.o \
./Src/Commands.o \
./Src/Custom.o \
./Src/Draw.o \
./Src/Editor.o \
./Src/External.o \
./Src/FileIO.o \
./Src/Flash.o \
./Src/Functions.o \
./Src/GPS.o \
./Src/GUI.o \
./Src/I2C.o \
./Src/MATHS.o \
./Src/MMBasic.o \
./Src/MM_Custom.o \
./Src/MM_Misc.o \
./Src/Memory.o \
./Src/MiscSTM32.o \
./Src/Onewire.o \
./Src/Operators.o \
./Src/OtherDisplays.o \
./Src/PWM.o \
./Src/SPI-LCD.o \
./Src/SPI.o \
./Src/SSD1963.o \
./Src/Serial.o \
./Src/SerialFileIO.o \
./Src/Timers.o \
./Src/Touch.o \
./Src/XModem.o \
./Src/assember.o \
./Src/cJSON.o \
./Src/debug.o \
./Src/ff.o \
./Src/ffunicode.o \
./Src/hxcmod.o \
./Src/keyboard.o \
./Src/main.o \
./Src/mmc_stm32.o \
./Src/picojpeg.o \
./Src/reciter.o \
./Src/regex.o \
./Src/render.o \
./Src/sam.o \
./Src/sprites.o \
./Src/stm32h7xx_hal_msp.o \
./Src/stm32h7xx_it.o \
./Src/syscalls.o \
./Src/system_stm32h7xx.o \
./Src/turtle.o \
./Src/upng.o \
./Src/usb_host.o \
./Src/usbh_conf.o \
./Src/usbh_platform.o 

S_UPPER_DEPS += \
./Src/assember.d 

C_DEPS += \
./Src/Audio.d \
./Src/BmpDecoder.d \
./Src/CFunctions.d \
./Src/Commands.d \
./Src/Custom.d \
./Src/Draw.d \
./Src/Editor.d \
./Src/External.d \
./Src/FileIO.d \
./Src/Flash.d \
./Src/Functions.d \
./Src/GPS.d \
./Src/GUI.d \
./Src/I2C.d \
./Src/MATHS.d \
./Src/MMBasic.d \
./Src/MM_Custom.d \
./Src/MM_Misc.d \
./Src/Memory.d \
./Src/MiscSTM32.d \
./Src/Onewire.d \
./Src/Operators.d \
./Src/OtherDisplays.d \
./Src/PWM.d \
./Src/SPI-LCD.d \
./Src/SPI.d \
./Src/SSD1963.d \
./Src/Serial.d \
./Src/SerialFileIO.d \
./Src/Timers.d \
./Src/Touch.d \
./Src/XModem.d \
./Src/cJSON.d \
./Src/debug.d \
./Src/ff.d \
./Src/ffunicode.d \
./Src/hxcmod.d \
./Src/keyboard.d \
./Src/main.d \
./Src/mmc_stm32.d \
./Src/picojpeg.d \
./Src/reciter.d \
./Src/regex.d \
./Src/render.d \
./Src/sam.d \
./Src/sprites.d \
./Src/stm32h7xx_hal_msp.d \
./Src/stm32h7xx_it.d \
./Src/syscalls.d \
./Src/system_stm32h7xx.d \
./Src/turtle.d \
./Src/upng.d \
./Src/usb_host.d \
./Src/usbh_conf.d \
./Src/usbh_platform.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DSTM32H743xx '-D__weak=__attribute__((weak))' -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -c -I../Inc -I../FATFS -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O2 -ffunction-sections -Wall -fipa-pta -ffast-math -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/%.o: ../Src/%.S Src/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m7 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/Audio.d ./Src/Audio.o ./Src/Audio.su ./Src/BmpDecoder.d ./Src/BmpDecoder.o ./Src/BmpDecoder.su ./Src/CFunctions.d ./Src/CFunctions.o ./Src/CFunctions.su ./Src/Commands.d ./Src/Commands.o ./Src/Commands.su ./Src/Custom.d ./Src/Custom.o ./Src/Custom.su ./Src/Draw.d ./Src/Draw.o ./Src/Draw.su ./Src/Editor.d ./Src/Editor.o ./Src/Editor.su ./Src/External.d ./Src/External.o ./Src/External.su ./Src/FileIO.d ./Src/FileIO.o ./Src/FileIO.su ./Src/Flash.d ./Src/Flash.o ./Src/Flash.su ./Src/Functions.d ./Src/Functions.o ./Src/Functions.su ./Src/GPS.d ./Src/GPS.o ./Src/GPS.su ./Src/GUI.d ./Src/GUI.o ./Src/GUI.su ./Src/I2C.d ./Src/I2C.o ./Src/I2C.su ./Src/MATHS.d ./Src/MATHS.o ./Src/MATHS.su ./Src/MMBasic.d ./Src/MMBasic.o ./Src/MMBasic.su ./Src/MM_Custom.d ./Src/MM_Custom.o ./Src/MM_Custom.su ./Src/MM_Misc.d ./Src/MM_Misc.o ./Src/MM_Misc.su ./Src/Memory.d ./Src/Memory.o ./Src/Memory.su ./Src/MiscSTM32.d ./Src/MiscSTM32.o ./Src/MiscSTM32.su ./Src/Onewire.d ./Src/Onewire.o ./Src/Onewire.su ./Src/Operators.d ./Src/Operators.o ./Src/Operators.su ./Src/OtherDisplays.d ./Src/OtherDisplays.o ./Src/OtherDisplays.su ./Src/PWM.d ./Src/PWM.o ./Src/PWM.su ./Src/SPI-LCD.d ./Src/SPI-LCD.o ./Src/SPI-LCD.su ./Src/SPI.d ./Src/SPI.o ./Src/SPI.su ./Src/SSD1963.d ./Src/SSD1963.o ./Src/SSD1963.su ./Src/Serial.d ./Src/Serial.o ./Src/Serial.su ./Src/SerialFileIO.d ./Src/SerialFileIO.o ./Src/SerialFileIO.su ./Src/Timers.d ./Src/Timers.o ./Src/Timers.su ./Src/Touch.d ./Src/Touch.o ./Src/Touch.su ./Src/XModem.d ./Src/XModem.o ./Src/XModem.su ./Src/assember.d ./Src/assember.o ./Src/cJSON.d ./Src/cJSON.o ./Src/cJSON.su ./Src/debug.d ./Src/debug.o ./Src/debug.su ./Src/ff.d ./Src/ff.o ./Src/ff.su ./Src/ffunicode.d ./Src/ffunicode.o ./Src/ffunicode.su ./Src/hxcmod.d ./Src/hxcmod.o ./Src/hxcmod.su ./Src/keyboard.d ./Src/keyboard.o ./Src/keyboard.su ./Src/main.d ./Src/main.o ./Src/main.su ./Src/mmc_stm32.d ./Src/mmc_stm32.o ./Src/mmc_stm32.su ./Src/picojpeg.d ./Src/picojpeg.o ./Src/picojpeg.su ./Src/reciter.d ./Src/reciter.o ./Src/reciter.su ./Src/regex.d ./Src/regex.o ./Src/regex.su ./Src/render.d ./Src/render.o ./Src/render.su ./Src/sam.d ./Src/sam.o ./Src/sam.su ./Src/sprites.d ./Src/sprites.o ./Src/sprites.su ./Src/stm32h7xx_hal_msp.d ./Src/stm32h7xx_hal_msp.o ./Src/stm32h7xx_hal_msp.su ./Src/stm32h7xx_it.d ./Src/stm32h7xx_it.o ./Src/stm32h7xx_it.su ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/system_stm32h7xx.d ./Src/system_stm32h7xx.o ./Src/system_stm32h7xx.su ./Src/turtle.d ./Src/turtle.o ./Src/turtle.su ./Src/upng.d ./Src/upng.o ./Src/upng.su ./Src/usb_host.d ./Src/usb_host.o ./Src/usb_host.su ./Src/usbh_conf.d ./Src/usbh_conf.o ./Src/usbh_conf.su ./Src/usbh_platform.d ./Src/usbh_platform.o ./Src/usbh_platform.su

.PHONY: clean-Src

