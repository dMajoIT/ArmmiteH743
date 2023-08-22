################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
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
Src/%.o: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Inc -I"C:/workspace/ArmmiteH7VIT6/FATFSOLD" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O3 -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/%.o: ../Src/%.S Src/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/Audio.d ./Src/Audio.o ./Src/BmpDecoder.d ./Src/BmpDecoder.o ./Src/CFunctions.d ./Src/CFunctions.o ./Src/Commands.d ./Src/Commands.o ./Src/Custom.d ./Src/Custom.o ./Src/Draw.d ./Src/Draw.o ./Src/Editor.d ./Src/Editor.o ./Src/External.d ./Src/External.o ./Src/FileIO.d ./Src/FileIO.o ./Src/Flash.d ./Src/Flash.o ./Src/Functions.d ./Src/Functions.o ./Src/GPS.d ./Src/GPS.o ./Src/GUI.d ./Src/GUI.o ./Src/I2C.d ./Src/I2C.o ./Src/MATHS.d ./Src/MATHS.o ./Src/MMBasic.d ./Src/MMBasic.o ./Src/MM_Custom.d ./Src/MM_Custom.o ./Src/MM_Misc.d ./Src/MM_Misc.o ./Src/Memory.d ./Src/Memory.o ./Src/MiscSTM32.d ./Src/MiscSTM32.o ./Src/Onewire.d ./Src/Onewire.o ./Src/Operators.d ./Src/Operators.o ./Src/OtherDisplays.d ./Src/OtherDisplays.o ./Src/PWM.d ./Src/PWM.o ./Src/SPI-LCD.d ./Src/SPI-LCD.o ./Src/SPI.d ./Src/SPI.o ./Src/SSD1963.d ./Src/SSD1963.o ./Src/Serial.d ./Src/Serial.o ./Src/SerialFileIO.d ./Src/SerialFileIO.o ./Src/Timers.d ./Src/Timers.o ./Src/Touch.d ./Src/Touch.o ./Src/XModem.d ./Src/XModem.o ./Src/assember.d ./Src/assember.o ./Src/cJSON.d ./Src/cJSON.o ./Src/debug.d ./Src/debug.o ./Src/ff.d ./Src/ff.o ./Src/ffunicode.d ./Src/ffunicode.o ./Src/hxcmod.d ./Src/hxcmod.o ./Src/keyboard.d ./Src/keyboard.o ./Src/main.d ./Src/main.o ./Src/mmc_stm32.d ./Src/mmc_stm32.o ./Src/picojpeg.d ./Src/picojpeg.o ./Src/reciter.d ./Src/reciter.o ./Src/render.d ./Src/render.o ./Src/sam.d ./Src/sam.o ./Src/sprites.d ./Src/sprites.o ./Src/stm32h7xx_hal_msp.d ./Src/stm32h7xx_hal_msp.o ./Src/stm32h7xx_it.d ./Src/stm32h7xx_it.o ./Src/syscalls.d ./Src/syscalls.o ./Src/system_stm32h7xx.d ./Src/system_stm32h7xx.o ./Src/turtle.d ./Src/turtle.o ./Src/upng.d ./Src/upng.o ./Src/usb_host.d ./Src/usb_host.o ./Src/usbh_conf.d ./Src/usbh_conf.o ./Src/usbh_platform.d ./Src/usbh_platform.o

.PHONY: clean-Src

