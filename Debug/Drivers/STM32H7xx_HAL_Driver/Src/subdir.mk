################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.c \
../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c 

OBJS += \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.o \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.o 

C_DEPS += \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.d \
./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32H7xx_HAL_Driver/Src/%.o: ../Drivers/STM32H7xx_HAL_Driver/Src/%.c Drivers/STM32H7xx_HAL_Driver/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Inc -I"C:/workspace/ArmmiteH7VIT6/FATFSOLD" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O3 -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-STM32H7xx_HAL_Driver-2f-Src

clean-Drivers-2f-STM32H7xx_HAL_Driver-2f-Src:
	-$(RM) ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.o ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.d ./Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.o

.PHONY: clean-Drivers-2f-STM32H7xx_HAL_Driver-2f-Src

