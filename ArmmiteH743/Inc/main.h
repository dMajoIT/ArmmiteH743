/*-*****************************************************************************

MMBasic  for STM32H743 [ZI2 and VIT6] (Armmite H7)

main.h

Copyright 2011-2023 Geoff Graham and  Peter Mather.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
  be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham and Peter Mather.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

/* USER CODE BEGIN Header */
/**
  * In addition the software components from STMicroelectronics are provided
  * subject to the license as detailed below:
  *
  ******************************************************************************
  * @file           : main.h
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define PACKAGE_BASE            (0x58000524UL)      // from web search SYSCFG_PKGR G.A.
#define package (*(volatile unsigned int *)(PACKAGE_BASE) & 0b1111)
/*
  Bits 31:4           Reserved, must be kept at reset value.
  Bits 3:0         PKG[3:0]: Package
  These bits indicate the device package.
  0000: LQFP100
  0010: TQFP144
  0101: TQFP176/UFBGA176
  1000: LQFP208/TFBGA240
  Other configurations: all pads enabled
*/
#define HAS_100PINS          (package==0x0)
#define HAS_144PINS          (package==0x2)
#define HAS_176PINS          (package==0x5)
#define HAS_208PINS          (package==0x8)

//Pins for Bitbanged SPI on the SDCARD on 100Pin DEVEBox and WeAct cards
#define SD_CLK_Pin GPIO_PIN_12  //pc12  SD_CK
#define SD_CLK_GPIO_Port GPIOC
#define SD_MISO_Pin GPIO_PIN_8  //pc8   SD_D0
#define SD_MISO_GPIO_Port GPIOC
#define SD_MOSI_Pin GPIO_PIN_2 //pd2  SD_CMD
#define SD_MOSI_GPIO_Port GPIOD

#define SWITCH_Pin GPIO_PIN_13  //PC13  Pin 7 on  both 100 pins and 144 pins
#define SWITCH_GPIO_Port GPIOC  //Pull to VCC for MMBasic Reset

#define COUNT1_Pin (HAS_144PINS ? GPIO_PIN_0 :   GPIO_PIN_0   )
//#define COUNT1_GPIO_Port (HAS_144PINS ? GPIOF : GPIOD)
#define COUNT2_Pin (HAS_144PINS ? GPIO_PIN_1 :  GPIO_PIN_1   )
#define COUNT2_GPIO_Port GPIOF
#define COUNT3_Pin (HAS_144PINS ? GPIO_PIN_2 :  GPIO_PIN_10   )
#define COUNT3_GPIO_Port GPIOF
#define COUNT4_Pin (HAS_144PINS ? GPIO_PIN_3 :  GPIO_PIN_3 )
#define COUNT4_GPIO_Port GPIOF
#define IR_Pin (HAS_144PINS ? GPIO_PIN_4 : GPIO_PIN_11   )
//#define IR_GPIO_Port GPIOF

#define PWM_1A_Pin GPIO_PIN_12
#define PWM_1A_GPIO_Port GPIOD
#define PWM_1B_Pin GPIO_PIN_13
#define PWM_1B_GPIO_Port GPIOD
#define PWM_1C_Pin GPIO_PIN_14
#define PWM_1C_GPIO_Port GPIOD
#define PWM_1D_Pin GPIO_PIN_15
#define PWM_1D_GPIO_Port GPIOD

#define PWM_2A_Pin GPIO_PIN_0
#define PWM_2A_GPIO_Port GPIOA
#define PWM_2B_Pin GPIO_PIN_1
#define PWM_2B_GPIO_Port GPIOA
#define PWM_2C_Pin GPIO_PIN_2
#define PWM_2C_GPIO_Port GPIOA
#define PWM_2D_Pin GPIO_PIN_3
#define PWM_2D_GPIO_Port GPIOA

#define DAC1_Pin GPIO_PIN_4
#define DAC1_GPIO_Port GPIOA
#define DAC2_Pin GPIO_PIN_5
#define DAC2_GPIO_Port GPIOA


#define SSD_D0_Pin GPIO_PIN_0
#define SSD_D0_GPIO_Port GPIOE
#define SSD_D1_Pin GPIO_PIN_1
#define SSD_D1_GPIO_Port GPIOE
#define SSD_D2_Pin GPIO_PIN_2
#define SSD_D2_GPIO_Port GPIOE
#define SSD_D3_Pin GPIO_PIN_3
#define SSD_D3_GPIO_Port GPIOE
#define SSD_D4_Pin GPIO_PIN_4
#define SSD_D4_GPIO_Port GPIOE
#define SSD_D5_Pin GPIO_PIN_5
#define SSD_D5_GPIO_Port GPIOE
#define SSD_D6_Pin GPIO_PIN_6
#define SSD_D6_GPIO_Port GPIOE
#define SSD_D7_Pin GPIO_PIN_7
#define SSD_D7_GPIO_Port GPIOE
#define SSD_D8_Pin GPIO_PIN_8
#define SSD_D8_GPIO_Port GPIOE
#define SSD_D9_Pin GPIO_PIN_9
#define SSD_D9_GPIO_Port GPIOE
#define SSD_D10_Pin GPIO_PIN_10
#define SSD_D10_GPIO_Port GPIOE
#define SSD_D11_Pin GPIO_PIN_11
#define SSD_D11_GPIO_Port GPIOE
#define SSD_D12_Pin GPIO_PIN_12
#define SSD_D12_GPIO_Port GPIOE
#define SSD_D13_Pin GPIO_PIN_13
#define SSD_D13_GPIO_Port GPIOE
#define SSD_D14_Pin GPIO_PIN_14
#define SSD_D14_GPIO_Port GPIOE
#define SSD_D15_Pin GPIO_PIN_15
#define SSD_D15_GPIO_Port GPIOE



#define CONSOLE_TX_Pin GPIO_PIN_8
#define CONSOLE_TX_GPIO_Port GPIOD
#define CONSOLE_RX_Pin GPIO_PIN_9
#define CONSOLE_RX_GPIO_Port GPIOD


#define COM1_TX_Pin (HAS_144PINS ? GPIO_PIN_6 : GPIO_PIN_9)    //PB6  PA9
#define COM1_TX_GPIO_Port (HAS_144PINS ? GPIOB : GPIOA)
#define COM1_RX_Pin (HAS_144PINS ? GPIO_PIN_15 : GPIO_PIN_10)  //PB15 PA10
#define COM1_RX_GPIO_Port (HAS_144PINS ? GPIOB : GPIOA)

#define COM2_TX_Pin GPIO_PIN_5          //same
#define COM2_TX_GPIO_Port GPIOD
//#define COM2_RX_Pin GPIO_PIN_6          //fix in code
#define COM2_RX_Pin (HAS_144PINS ? GPIO_PIN_6 : GPIO_PIN_3)  //PD6  PA3
//#define COM2_RX_GPIO_Port GPIOD         //fix in code
#define COM2_RX_GPIO_Port (HAS_144PINS ? GPIOD : GPIOA)
#define COM2_DE_Pin GPIO_PIN_4          //same
#define COM2_DE_GPIO_Port GPIOD

#define COM3_TX_Pin (HAS_144PINS ? GPIO_PIN_13 : GPIO_PIN_6)  //PB13 PB6
#define COM3_TX_GPIO_Port GPIOB
#define COM3_RX_Pin GPIO_PIN_12
#define COM3_RX_GPIO_Port GPIOB

#define COM4_TX_Pin (HAS_144PINS ? GPIO_PIN_14 : GPIO_PIN_6)  //PG14 PC6
#define COM4_TX_GPIO_Port (HAS_144PINS ? GPIOG : GPIOC)
#define COM4_RX_Pin (HAS_144PINS ? GPIO_PIN_9 : GPIO_PIN_7)   //PG9  PC7
#define COM4_RX_GPIO_Port (HAS_144PINS ? GPIOG : GPIOC)

#define SSD_WR_Pin GPIO_PIN_10
#define SSD_WR_GPIO_Port GPIOG
#define SSD_RESET_Pin GPIO_PIN_12
#define SSD_RESET_GPIO_Port GPIOG
#define SSD_RD_Pin GPIO_PIN_13
#define SSD_RD_GPIO_Port GPIOG

#define USB_POWERON_Pin GPIO_PIN_6
#define USB_POWERON_GPIO_Port GPIOG


#define RED_LED_Pin GPIO_PIN_14
#define RED_LED_GPIO_Port GPIOB
#define BLUE_LED_Pin GPIO_PIN_7
#define BLUE_LED_GPIO_Port GPIOB
#define GREEN_LED_Pin GPIO_PIN_0
#define GREEN_LED_GPIO_Port GPIOB


/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */
#define CONSOLE_RX_BUF_SIZE 256
#define CONSOLE_TX_BUF_SIZE 2048                    // this is made a large size so that the serial console does not slow down the USB and LCD consoles
#define  MAX_BMP_FILES  25
#define  MAX_BMP_FILE_NAME 11
#define USARTx                           USART3
#define BREAK_KEY           3                       // the default value (CTRL-C) for the break key.  Reset at the command prompt.
#define forever 1
#define true	1
#define false	0
// used to determine if the exception occured during setup
#define CAUSE_NOTHING           0
#define CAUSE_DISPLAY           1
#define CAUSE_FILEIO            2
#define CAUSE_KEYBOARD          3
#define CAUSE_RTC               4
#define CAUSE_TOUCH             5
#define CAUSE_MMSTARTUP         6

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

void dump(char *p, int nbr,int page);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
