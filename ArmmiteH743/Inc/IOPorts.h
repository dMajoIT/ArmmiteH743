/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

IOPorts.h

Include file that defines the IOPins for the chip in MMBasic.

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



#ifndef IOPORTS_H
#define IOPORTS_H

// these are the valid peek/poke memory ranges for the STM32H743  SRAMEND-SRAMBASE used by vartab
#define RAM(a)  ((a >= SRAMBASE && a < SRAMEND) ||(a >= RAMBASE && a < RAMEND) || (a >= 0xd0000000 && a < 0xd0800000) || (a >= 0x24000000 && a < 0x24080000) || (a >= 0x38000000 && a<  0x38010000) || (a >= 0x38800000 && a<  0x38801000) || (a >= 0x40000000 && a<  0x60000000))
#define ROM(a)  ((a >= 0x08000000 && a < 0x08200000))
#define PEEKRANGE(a) (RAM(a) || ROM(a))
#define POKERANGE(a) (RAM(a))

// General defines
#define P_INPUT				1						// for setting the TRIS on I/O bits
#define P_OUTPUT			0
#define P_ON				1
#define P_OFF				0
// Structure that defines the SFR, bit number and mode for each I/O pin
struct s_PinDef {
	GPIO_TypeDef *sfr;
    unsigned int bitnbr;
    unsigned char mode;
#ifdef STM32H743xx
    ADC_TypeDef *  ADC;
#else
    ADC_TypeDef *  ADCn;
#endif
    uint32_t ADCchannel;
};
typedef struct s_PinDef PinDefAlias;

// Defines for the various modes that an I/O pin can be set to
#define PUNUSED       1
#define ANALOG_IN    2
#define DIGITAL_IN   4
#define COUNTING     8
#define INTERRUPT    16
#define DIGITAL_OUT  32
#define OC_OUT       64
#define DO_NOT_RESET 128
//#define HAS_64PINS 0
#define NBR_PINS_144CHIP    144					    // number of pins for external i/o on a 144 pin chip
#define NBR_PINS_100CHIP    100					    // number of pins for external i/o on a 100 pin chip
#define NBR_PINS_64CHIP    64					    // number of pins for external i/o on a 64 pin chip
#define NBR_PINS_MAXCHIP    144					    // max number of pins for supported packages on chip
#define MAX_ANALOGUE_PIN_PACKAGE    55		       // max analogue pin no for supported packages on chip
#ifndef STM32F4version
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

//#define HAS_100PINS  1
//#define HAS_144PINS  0
//#define NBRPINS             144
//#define NBRPINS             100
#define NBRPINS             (HAS_144PINS ? 144 : 100)

#else
#define package (*(volatile unsigned int *)(PACKAGE_BASE) & 0b11111)
#define flashsize *(volatile unsigned int *)(FLASHSIZE_BASE)
#define chipID (DBGMCU->IDCODE & 0x00000FFF)
#define HAS_32PINS          (package==0x8)
#define HAS_48PINS          (package==0xB)
#define HAS_64PINS          (package==0x9 || package==0)
#define HAS_100PINS          (package==0x10)
#define HAS_144PINS          (package==0x03)
#define NBRPINS             (HAS_144PINS ? 144 : 100)
#endif

#define NBRINTPINS          10
#if defined(DEFINE_PINDEF_TABLE)
const struct s_PinDef PinDef144[NBR_PINS_144CHIP + 1]={
        { NULL,  0, PUNUSED , NULL, 0},                                                             // pin 0
        { GPIOE,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 1 SSD_D2
        { GPIOE,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 2 SSD_D3
        { GPIOE,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 3 SSD_D4
        { GPIOE,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 4 SSD_D5
        { GPIOE,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 5 SSD_D6
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 6 VBAT
        { GPIOC,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 7 - Push Button
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 8 OSC32-IN
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 9 OSC32-OUT
        { GPIOF,  GPIO_PIN_0, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 10 COUNT1
        { GPIOF,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 11 COUNT2
        { GPIOF,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 12 COUNT3
        { GPIOF,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_5},     // pin 13 COUNT4
        { GPIOF,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_9},     // pin 14 IR
        { GPIOF,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_4},     // pin 15
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 16 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 17 VDD
        { GPIOF,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_8},     // pin 18
        { GPIOF,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_3},     // pin 19 SPI5-CLK
        { GPIOF,  GPIO_PIN_8,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_7},     // pin 20 SPI5-IN
        { GPIOF,  GPIO_PIN_9,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_2},     // pin 21 SPI5-OUT
        { GPIOF,  GPIO_PIN_10,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_6},    // pin 22
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 23 RCC-OSCIN
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 24 RCC-OSCOUT
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 25 NRST
        { GPIOC,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_10},    // pin 26 OV7670_DAT0
        { GPIOC,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_11},    // pin 27 SPI2-OUT, OV7670_DAT1
        { GPIOC,  GPIO_PIN_2, DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_0},      // pin 28 SPI2-IN, OV7670_DAT2
        { GPIOC,  GPIO_PIN_3, DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_1},      // pin 29 OV7670_DAT3
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 30 VDD
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 31 VSSA
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 32 VREF+
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 33 VDDA
        { GPIOA,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_16},    // pin 34 PWM-2A
        { GPIOA,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_17},    // pin 35 PWM-2B
        { GPIOA,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_14},    // pin 36 PWM-2C
    //
        { GPIOA,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_15},    // pin 37 PWM-2D
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 38 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 39 VDD
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 40 DAC1
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 41 DAC2
        { GPIOA,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_3},     // pin 42 SPI-IN
        { GPIOA,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_7},     // pin 43 SPI-OUT
        { GPIOC,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_4},     // pin 44 OV7670_DAT4
        { GPIOC,  GPIO_PIN_5, DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_8},      // pin 45 OV7670_DAT5
       // { GPIOB,  GPIO_PIN_0, DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_9},     // pin 46 Green-LED
		{ NULL,  0, PUNUSED, NULL, 0},                                                           // pin 46 LCD-BL TIM1 CH2N AF1
        { GPIOB,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_5},     // pin 47
        { GPIOB,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 48 SPI3-OUT
        { GPIOF,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_2},    // pin 49
        { GPIOF,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_6},    // pin 50
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 51 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 52 VDD
        { GPIOF,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN ,ADC2, ADC_CHANNEL_2},     // pin 53
        { GPIOF,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN ,ADC2, ADC_CHANNEL_6},     // pin 54
        { GPIOF,  GPIO_PIN_15,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 55
        { GPIOG,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 56
        { GPIOG,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 57 SSD_RS
        { GPIOE,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 58 SSD_D7
        { GPIOE,  GPIO_PIN_8,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 59 SSD_D8
        { GPIOE,  GPIO_PIN_9,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 60 SSD_D9
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 61 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 62 VDD
        { GPIOE,  GPIO_PIN_10,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 63 SSD_D10
        { GPIOE,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 64 SSD_D11
        { GPIOE,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 65 SSD_D12
        { GPIOE,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 66 SSD_D13
        { GPIOE,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                            // pin 67 SSD_D14
        { GPIOE,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 68 SSD_D15
        { GPIOB,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 69 I2C2-SCL
        { GPIOB,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 70 I2C2-SDA
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 71 VCAP
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 72 VDD
    ///
        { GPIOB,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 73 COM3-RX (UART5)
        { GPIOB,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 74 COM3-TX (UART5)
        { GPIOB,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 75 Red-LED
        { GPIOB,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 76 COM1-RX
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 77 CONSOLE-TX
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 78 CONSOLE-RX
        { GPIOD,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 79 USB-POWER V2 Nucleo
        { GPIOD,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 80
        { GPIOD,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 81 PWM-1A
        { GPIOD,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 82 PWM-1B
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 83 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 84 VDD
        { GPIOD,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 85 PWM-1C
        { GPIOD,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 86 PWM-1D
        { GPIOG,  GPIO_PIN_2, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 87 SD_CD
        { GPIOG,  GPIO_PIN_3, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 88 OV7670_PCLK
        { GPIOG,  GPIO_PIN_4, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 89 OV7670_VSYNC
        { GPIOG,  GPIO_PIN_5, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 90 OV7670_HREF
	    { GPIOG,  GPIO_PIN_6, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 91 USB-POWER V1 Nucleo
        { GPIOG,  GPIO_PIN_7, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 92
        { GPIOG,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 93 COUNT5-HIGHSPEED (H7 only)
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 94 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 95 VDDUSB
        { GPIOC,  GPIO_PIN_6, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 96 OV7670_DAT6
        { GPIOC,  GPIO_PIN_7, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 97 OV7670_DAT7
        { GPIOC,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 98
        { GPIOC,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 99
        { GPIOA,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 100 OV7670_XCLK
        { GPIOA,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 101
        { GPIOA, GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 102
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 103 USB-D+
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 104 USB-D-
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 105 SWDIO
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 106 VCAP
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 107 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 108 VDD
        //
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 109 SWCLK
        { GPIOA,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 110
        { GPIOC,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 111
        { GPIOC,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 112
        { GPIOC,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 113
        { GPIOD,  GPIO_PIN_0, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 114
        { GPIOD,  GPIO_PIN_1, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 115
        { GPIOD,  GPIO_PIN_2, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 116
        { GPIOD,  GPIO_PIN_3, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 117 SPI2-CLK
        { GPIOD,  GPIO_PIN_4, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 118 COM2-DE
        { GPIOD,  GPIO_PIN_5, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 119 COM2-TX
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 120 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 121 VDD
        { GPIOD,  GPIO_PIN_6, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 122 COM2-RX
        { GPIOD,  GPIO_PIN_7, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 123
        { GPIOG,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 124 COM4-RX (USART6)
        { GPIOG,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 125 SSD_WR
        { GPIOG,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 126 SPI-CLK
        { GPIOG,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 127 SSD_RESET
        { GPIOG,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 128 SSD_RD
        { GPIOG,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 129 COM4-TX (USART6)
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 130 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 131 VDD
        { GPIOG,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 132
        { GPIOB,  GPIO_PIN_3, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 133 SPI3-CLK
        { GPIOB,  GPIO_PIN_4, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 134 SPI3-IN
        { GPIOB,  GPIO_PIN_5, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 135
        { GPIOB,  GPIO_PIN_6, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 136 COM1-TX
        { GPIOB,  GPIO_PIN_7, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 137 Blue-LED
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 138 BOOT0
        { GPIOB,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 139 I2C-SCL
        { GPIOB,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 140 I2C-SDA
        { GPIOE,  GPIO_PIN_0, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 141 SSD_D0
        { GPIOE,  GPIO_PIN_1, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 142 SSD_D1
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 143 PDR-ON
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 144 VDD

};

const struct s_PinDef PinDef100[NBR_PINS_100CHIP + 1]={                                         //VIT6
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 0
        { GPIOE,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 1 SSD_D2
        { GPIOE,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                       // pin 2 SSD_D3 [LED E3]
        { GPIOE,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 3 SSD_D4
        { GPIOE,  GPIO_PIN_5,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 4 SSD_D5
        { GPIOE,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 5 SSD_D6
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 6 VBAT
        { GPIOC,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                      // pin 7 - Push Button K1
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 8 OSC32-IN
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 9 OSC32-OUT
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 10 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 11 VDD
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 12 RCC-OSCIN
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 13 RCC-OSCOUT
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 14 NRST
        { GPIOC,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_10},    // pin 15 ADC[C]
        { GPIOC,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_11},    // pin 16 ADC[C]
        { GPIOC,  GPIO_PIN_2, DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_0},      // pin 17 ADc[C]
        { GPIOC,  GPIO_PIN_3, DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC3, ADC_CHANNEL_1},      // pin 18 ADC[C]
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 19 VSSA
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 20 VREF+
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 21 VDDA
        { GPIOA,  GPIO_PIN_0,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_16},    // pin 22 PWM-2A
        { GPIOA,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_17},    // pin 23 PWM-2B
        { GPIOA,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_14},    // pin 24 PWM-2C
        { GPIOA,  GPIO_PIN_3,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_15},    // pin 25 PWM-2D & COM2-RX USART2
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 26 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 27 VDD
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 28 DAC1
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 29 DAC2
        { GPIOA,  GPIO_PIN_6,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_3},     // pin 30 ADC[B]
        { GPIOA,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC2, ADC_CHANNEL_7},     // pin 31 ADC[B]
        { GPIOC,  GPIO_PIN_4,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_4},     // pin 32 ADC[A]
        { GPIOC,  GPIO_PIN_5, DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_8},      // pin 33 ADC[A]
       // { GPIOB,  GPIO_PIN_0, DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_9},      // pin 34 ADC[A] LCD-BL TIM1 CH2N AF1
		{NULL,  0, PUNUSED , NULL, 0},                                                            // pin 34 ADC[A] LCD-BL TIM1 CH2N AF1
        { GPIOB,  GPIO_PIN_1,  DIGITAL_IN | DIGITAL_OUT | ANALOG_IN , ADC1, ADC_CHANNEL_5},     // pin 35 ADC[A]
        { GPIOB,  GPIO_PIN_2,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 36 [COUNT 1]
        { GPIOE,  GPIO_PIN_7,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 37 SSD_D7
        { GPIOE,  GPIO_PIN_8,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 38 SSD_D8
        { GPIOE,  GPIO_PIN_9,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 39 SSD_D9
        { GPIOE,  GPIO_PIN_10,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},            // pin 40 SSD_D10  LCD-BL*
        { GPIOE,  GPIO_PIN_11,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},            // pin 41 SSD_D11  LCD-CS
        { GPIOE,  GPIO_PIN_12,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},            // pin 42 SSD_D12  LCD-SCK SPI4*
        { GPIOE,  GPIO_PIN_13,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},            // pin 43 SSD_D13  LCD-RS
        { GPIOE,  GPIO_PIN_14,  DIGITAL_IN | DIGITAL_OUT , NULL, 0},            // pin 44 SSD_D14  LCD-MOSI SPI4*
        { GPIOE,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},             // pin 45 SSD_D15  LCD-RST
        { GPIOB,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 46 I2C2-SCL
        { GPIOB,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 47 I2C2-SDA
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 48 VCAP
		{ NULL,  0, PUNUSED , NULL, 0},                                                         // pin 49 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 50 VDD
        { GPIOB,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 51 COM3-RX (UART5)
        { GPIOB,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 52 SPI5[2]-CLK
        { GPIOB,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 53 SPI5[2]-IN
        { GPIOB,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 54 SPI5[2]-OUT
       	{  NULL,  0, PUNUSED , NULL, 0},                                                         // pin 55 CONSOLE-TX
		{  NULL,  0, PUNUSED , NULL, 0},                                                         // pin 56 CONSOLE-RX
        { GPIOD,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 57 SSD-WR
        { GPIOD,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 58 SSD-RDSYSTEM USE
        { GPIOD,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 59 PWM-1A
        { GPIOD,  GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 60 PWM-1B
        { GPIOD,  GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 61 PWM-1C
        { GPIOD,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 62 PWM-1D
		{ GPIOC,  GPIO_PIN_6, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                          // pin 63 COM4-TX (USART6)
		{ GPIOC,  GPIO_PIN_7, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                          // pin 64 COM4-RX (USART6)
        { GPIOC,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                          // pin 65 [SD-MISO -D0]
        { GPIOC,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                          // pin 66 [SD-D1]Unused
        { GPIOA,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 67 T-CS
        { GPIOA,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 68 COM1-TX (UART1)
        { GPIOA, GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 69 COM1-RX (UART1)
        { NULL,  0, PUNUSED , NULL, 0},                                                     // pin 70 USB-D+ KEYBOARD
        { NULL,  0, PUNUSED , NULL, 0},                                                     // pin 71 USB-D- KEYBOARD
        { GPIOA, GPIO_PIN_13, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                          // pin 72 SWDIO SW K1 PA13
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 73 VCAP
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 74 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 75 VDD
        { GPIOA, GPIO_PIN_14, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 76 SWCLK PA14
        { GPIOA,  GPIO_PIN_15, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 77 T-IRQ
        { GPIOC,  GPIO_PIN_10, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 78 [SD-D2] SD-CS (System SDCARD)
        { GPIOC,  GPIO_PIN_11, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 79 [SD-CS -D3]
        { GPIOC,  GPIO_PIN_12, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                             // pin 80 [SD-CLK]
        { GPIOD,  GPIO_PIN_0, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 81 [COUNT 2]
        { GPIOD,  GPIO_PIN_1, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 82 [COUNT 3]
        { GPIOD,  GPIO_PIN_2, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 83 [SD-MOSI -CMD]
        { GPIOD,  GPIO_PIN_3, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 84 [COUNT 5]
        { GPIOD,  GPIO_PIN_4, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                          // pin 85 COM2-DE [SD-SW *]
        { GPIOD,  GPIO_PIN_5, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                          // pin 86 COM2-TX USART2
        { GPIOD,  GPIO_PIN_6, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 87 F-CS
        { GPIOD,  GPIO_PIN_7, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 88 SPI-OUT
        { GPIOB,  GPIO_PIN_3, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 89 SPI-CLK
        { GPIOB,  GPIO_PIN_4, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 90 SPI-IN
        { GPIOB,  GPIO_PIN_5, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 91
        { GPIOB,  GPIO_PIN_6, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 92 COM3-TX (UART5)
        { GPIOB,  GPIO_PIN_7, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 93 [COUNT 4]
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 94 BOOT0
        { GPIOB,  GPIO_PIN_8, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 95 I2C-SCL
        { GPIOB,  GPIO_PIN_9, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 96 I2C-SDA
        { GPIOE,  GPIO_PIN_0, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 97 SSD_D0
        { GPIOE,  GPIO_PIN_1, DIGITAL_IN | DIGITAL_OUT , NULL, 0},                              // pin 98 SSD_D1
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 99 VSS
        { NULL,  0, PUNUSED , NULL, 0},                                                         // pin 100 VDD
};

#else
	const extern struct s_PinDef PinDef144[];
	const extern struct s_PinDef PinDef100[];
#endif      // DEFINE_PINDEF_TABLE
    extern struct s_PinDef *PinDef;
// Define the counting pin numbers
// INT1PIN refers to the PIC32 external interrupt #1, an so on for the others
#define INT1PIN              (HAS_144PINS ? 10  : 81)  //PF0   PD0  EXTI0
#define INT2PIN              (HAS_144PINS ? 11  : 82)  //PF1   PD1  EXTI1
#define INT3PIN              (HAS_144PINS ? 12  : 36)  //PF2   PB2  EXTI2
#define INT4PIN              (HAS_144PINS ? 13  : 84)  //PF3   PD3  EXTI4
#define IRPIN                (HAS_144PINS ? 14  : 93)  //PF4   PB7  EXTI9_5
#define COUNT5               (HAS_144PINS ? 93  : 0)  //PG8 TIM8_ETR  N/A

// I2C pin numbers
#define P_I2C_SCL           (HAS_144PINS ? 139  : 95)  //PB8
#define P_I2C_SDA           (HAS_144PINS ? 140  : 96)  //PB9
#define P_I2C2_SCL          (HAS_144PINS ? 69   : 46)  //PB10
#define P_I2C2_SDA          (HAS_144PINS ? 70   : 47)  //PB11

// The Console pin numbers are NOT defined here. See main.h where the PORT and PIN position are used.
// In the S_PinDef table above the CONSOLE pins must be defined as:  { NULL,  0, PUNUSED , NULL, 0},
//#define CONSOLE_TX_PIN		(HAS_144PINS ? 77  : 63) //55 //63  //PD8 USART3 PC6  USART6
//#define CONSOLE_RX_PIN		(HAS_144PINS ? 78  : 64) // 56 //64 //PD9 USART3 PC7  USART6


// COMx: port pin numbers used in serial.c
// Also defined as PORT and Pin Position in main.h
#define COM1_TX_PIN         (HAS_144PINS ? 136  : 68)  //136 PB6  68 A9 USART1
#define COM1_RX_PIN         (HAS_144PINS ? 76  : 69)   //76  PB15 69 A10

#define COM2_TX_PIN         (HAS_144PINS ? 119  : 86)  //119 PD5 86 PD5 USART2
#define COM2_RX_PIN         (HAS_144PINS ? 122  : 25)  //122 PD6 25 PA3
#define COM2_EN_PIN			(HAS_144PINS ? 118  : 85)  //118 PD4 85 PD4

#define COM3_TX_PIN         (HAS_144PINS ? 74  : 92)  //74  //PB13 52 PB6 UART5
#define COM3_RX_PIN         (HAS_144PINS ? 73  : 51)  //73  //PB12 51 PB12

//
#define COM4_TX_PIN         (HAS_144PINS ? 129  : 63)  //129 //PG14 PC6 USART6
#define COM4_RX_PIN         (HAS_144PINS ? 124  : 64)  //124 //PG9  PC7

// SPI pin numbers
#define SPI_INP_PIN         (HAS_144PINS ? 42   : 90)  //42  PA6  PB4
#define SPI_OUT_PIN         (HAS_144PINS ? 43   : 88)  //43  PA7  PD7
#define SPI_CLK_PIN         (HAS_144PINS ? 126  : 89) //126  PG11 PB3

// MMBASIC SPI2 Command
// Nucleo 144 Uses SPI2 Hardware and pin numbers     VIT6 uses SPI4 hardware and pins
#define SPI2_INP_PIN        (HAS_144PINS ? 28  : 43)  //28   PC2  43 PE13
#define SPI2_OUT_PIN        (HAS_144PINS ? 27  : 44)  //27   PC1  44 PE14 (also LCD RS on WeAct)
#define SPI2_CLK_PIN        (HAS_144PINS ? 117 : 42)  //117  PD3  42 PE12


//MMBasic SPI3
//Nucleo 144 Uses SPI3 Hardware and pin numbers VIT6 does not support SPI3
#define SPI3_INP_PIN        (HAS_144PINS ? 134  : 0)  //134  PB4
#define SPI3_OUT_PIN        (HAS_144PINS ? 48   : 0)  //48   PB2
#define SPI3_CLK_PIN        (HAS_144PINS ? 133  : 0)  //133  PB3

// SPI4 Onboard LCD Used VIT6
//#define LCD_LED_PIN        (HAS_144PINS ? 0  : 40)  // PE10
//#define LCD_CS_PIN         (HAS_144PINS ? 0  : 41)  // PE11
//#define LCD_RS_PIN         (HAS_144PINS ? 0  : 43)  // PE13

//#define LCD_INP_PIN        (HAS_144PINS ? 0  : 72)  // dummy reuse PA13 K1
//#define LCD_OUT_PIN        (HAS_144PINS ? 0  : 44)  // PE14
//#define LCD_CLK_PIN        (HAS_144PINS ? 0  : 42)  // PE12



// SYSTEM SPI used for SPI LCDPanels ,Touch and SDCARD
// Nucleo 144 uses SPI5 Harware and pins   VIT6  100 pins uses SPI2 Harware and Pins
#define SPISYS_INP_PIN        (HAS_144PINS ? 20  : 53 )  //20  //PF8  B14
#define SPISYS_OUT_PIN        (HAS_144PINS ? 21  : 54)  //21  //PF9   B15
#define SPISYS_CLK_PIN        (HAS_144PINS ? 19  : 52)  //19  //PF7   B13


// Bitbanged SPI for ONboard SDCARD socket on VIT6
#define SD_INP_PIN         65     //pc8
#define SD_OUT_PIN         83     //pd2
#define SD_CLK_PIN         80     //pc12
#define SD_CS              79     //pc11
    //SD_CD not used



    // DAC pin numbers
#define DAC_1_PIN           (HAS_144PINS ? 40  : 28)  //40  //PA4
#define DAC_2_PIN           (HAS_144PINS ? 41  : 29)  //41  //PA5
//
// PWM pin numbers
#define PWM_CH1_PIN         (HAS_144PINS ? 81  : 59)  //81 //PD12  - PWM1A
#define PWM_CH2_PIN         (HAS_144PINS ? 82  : 60)  //82 //PD13  - PWM1B
#define PWM_CH3_PIN         (HAS_144PINS ? 85  : 61)  //85 //PD14  - PWM1C
#define PWM_CH4_PIN         (HAS_144PINS ? 86  : 62)  //86 //PD15  - PWM1D
#define PWM_CH5_PIN         (HAS_144PINS ? 34  : 22)  //34 //PA0   - PWM2A
#define PWM_CH6_PIN         (HAS_144PINS ? 35  : 23)  //35 //PA1   - PWM2B
#define PWM_CH7_PIN         (HAS_144PINS ? 36  : 24)  //36 //PA2   - PWM2C
#define PWM_CH8_PIN         (HAS_144PINS ? 37  : 25)  //37 //PA3   - Now COM2_RX [PWM2D]

#define SSD1963_DAT1        (HAS_144PINS ? 141  : 97)  //141  	//E0
#define SSD1963_DAT2        (HAS_144PINS ? 142  : 98)  //142  	//E1
#define SSD1963_DAT3        (HAS_144PINS ? 1  : 1)  //1  		//E2
#define SSD1963_DAT4        (HAS_144PINS ? 2  : 2)  //2  		//E3
#define SSD1963_DAT5        (HAS_144PINS ? 3  : 3)  //3  		//E4
#define SSD1963_DAT6        (HAS_144PINS ? 4  : 4)  //4 		//E5
#define SSD1963_DAT7        (HAS_144PINS ? 5  : 5)  //5  		//E6
#define SSD1963_DAT8        (HAS_144PINS ? 58  : 37)  //58  	//E7
#define SSD1963_DAT9        (HAS_144PINS ? 59  : 38)  //59  	//E8
#define SSD1963_DAT10       (HAS_144PINS ? 60  : 39)  //60		//E9
#define SSD1963_DAT11       (HAS_144PINS ? 63  : 40)  //63  	//E10
#define SSD1963_DAT12       (HAS_144PINS ? 64  : 41)  //64  	//E11
#define SSD1963_DAT13       (HAS_144PINS ? 65  : 42)  //65  	//E12
#define SSD1963_DAT14       (HAS_144PINS ? 66  : 43)  //66  	//E13
#define SSD1963_DAT15       (HAS_144PINS ? 67  : 44)  //67  	//E14
#define SSD1963_DAT16       (HAS_144PINS ? 68  : 45)  //68  	//E15

// the SSD1963 write pin
#define SSD1963_WR_PIN      (HAS_144PINS ? 125  : 57)  //125		 //PG10    PD10

#define SSD1963_WR_TOGGLE_PIN        {GPIOG->ODR &= (~GPIO_PIN_10);GPIOG->ODR |= GPIO_PIN_10;}
#define SSD1963_WR_TOGGLE_PIN_VIT6   {GPIOD->ODR &= (~GPIO_PIN_10);GPIOD->ODR |= GPIO_PIN_10;}
//#define SSD1963_WR_TOGGLE_PIN_VIT6   {GPIOD->BSRR= GPIO_PIN_10<<16; shortpause(writepause1);GPIOD->BSRR= GPIO_PIN_10;}

#define SSD1963_WR_LOW   GPIOG->ODR &= (~GPIO_PIN_10);
#define SSD1963_WR_LOW_VIT6   GPIOD->ODR &= (~GPIO_PIN_10);

#define SSD1963_WR_HIGH  GPIOG->ODR |= GPIO_PIN_10;
#define SSD1963_WR_HIGH_VIT6  GPIOD->ODR |= GPIO_PIN_10;

#define SSD1963_WR_TOGGLE_PIN_FAST        {GPIOG->BSRR= GPIO_PIN_10<<16;GPIOG->BSRR= GPIO_PIN_10;}
#define SSD1963_WR_TOGGLE_PIN_FAST_VIT6   {GPIOD->BSRR= GPIO_PIN_10<<16;GPIOD->BSRR= GPIO_PIN_10;}
//#define SSD1963_WR_TOGGLE_PIN_FAST_VIT6   {shortpause(writepause1);GPIOD->BSRR= GPIO_PIN_10<<16;GPIOD->BSRR= GPIO_PIN_10;}


// the SSD1963 data/command pin.  On the LCD it is labeled RS.
//#define SSD1963_DC_PIN        (HAS_144PINS ? 57  : 64)  //57		//PG1 PC7
#define SSD1963_DC_PIN        (HAS_144PINS ? 57  : 66)  //57		//PG1 PC9

#define SSD1963_DC_LOW         GPIOG->ODR &= (~GPIO_PIN_1);
//#define SSD1963_DC_LOW_VIT6    GPIOC->ODR &= (~GPIO_PIN_7);
#define SSD1963_DC_LOW_VIT6    GPIOC->ODR &= (~GPIO_PIN_9);

#define SSD1963_DC_HIGH        GPIOG->ODR |= GPIO_PIN_1;
//#define SSD1963_DC_HIGH_VIT6   GPIOC->ODR |= GPIO_PIN_7;
#define SSD1963_DC_HIGH_VIT6   GPIOC->ODR |= GPIO_PIN_9;

// the SSD1963 reset pin
//#define SSD1963_RESET_PIN   (HAS_144PINS ? 127  : 63)  //127		//PG12  PC6
#define SSD1963_RESET_PIN   (HAS_144PINS ? 127  : 91)  //127		//PG12  PB5

// the SSD1963 read pin
#define SSD1963_RD_PIN    	(HAS_144PINS ? 128  : 58)  //128		//PG13 PD11
#define RDTOGGLE         GPIOG->ODR ^= GPIO_PIN_13;
#define RDTOGGLE_VIT6    GPIOD->ODR ^= GPIO_PIN_11;

// the OV7670 data pins
#define OV7670_DAT0        (HAS_144PINS ? 26  : 0)  //26  //PC0
#define OV7670_DAT1        (HAS_144PINS ? 27  : 0)  //27  //PC1
#define OV7670_DAT2        (HAS_144PINS ? 28  : 0)  //28  //PC2
#define OV7670_DAT3        (HAS_144PINS ? 29  : 0)  //29  //PC3
#define OV7670_DAT4        (HAS_144PINS ? 44  : 0)  //44  //PC4
#define OV7670_DAT5        (HAS_144PINS ? 45  : 0)  //45  //PC5
#define OV7670_DAT6        (HAS_144PINS ? 96  : 0)  //96  //PC6
#define OV7670_DAT7        (HAS_144PINS ? 97  : 0)  //97  //PC7
#define OV7670_PCLK        (HAS_144PINS ? 88  : 0)  //88  //PG3
#define OV7670_VSYNC       (HAS_144PINS ? 89  : 0)  //89  //PG4
#define OV7670_HREF        (HAS_144PINS ? 90  : 0)  //90  //PG5
#define OV7670_XCLK        (HAS_144PINS ? 100  : 0)  //100  //PA8
#define ST_HREF (GPIOG->IDR & GPIO_PIN_5)
#define ST_VSYNC (GPIOG->IDR & GPIO_PIN_4)
#define ST_PCLK (GPIOG->IDR & GPIO_PIN_3)

#endif

