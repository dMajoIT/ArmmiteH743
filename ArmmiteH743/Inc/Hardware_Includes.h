/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

Hardware_Includes.h

Defines the hardware aspects for PIC32-Generic MMBasic.

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


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
#ifdef STM32H743xx
	#include "stm32h7xx.h"
	#include "stm32h7xx_hal.h"
#else
	#ifdef STM32F767xx
		#include "stm32f7xx.h"
		#include "stm32f7xx_hal.h"
		#include "stm32f7xx_nucleo_144.h"
		#include "stm32f7xx_ll_pwr.h"
		#include "stm32f7xx_ll_usart.h"
		#include "stm32f7xx_ll_cortex.h"
		#include "stm32f7xx_ll_rcc.h"
		#include "stm32f7xx_ll_gpio.h"
		#include "stm32f7xx_ll_spi.h"
	#else
		#include "stm32f4xx.h"
		#include "stm32f4xx_hal.h"
		#include "stm32f4xx_ll_gpio.h"
		#include "stm32f4xx_ll_spi.h"
		#include "stm32f4xx_ll_usb.h"
	#endif
#endif
#include "IOPorts.h"
#include "configuration.h"
#include "Timers.h"
#include "SPI-LCD.h"
#include "ff.h"
#ifdef STM32H743xx
	#include "usbh_core.h"
	#include "usbh_hid.h"
	#include "usbh_hid_parser.h"
	#include "usb_host.h"
#endif
    // global variables
    extern int MMCharPos;
    extern char *StartEditPoint;
    extern int StartEditChar;
    extern char *InterruptReturn;
    extern char IgnorePIN;
    extern char WatchdogSet;
    extern char oc1, oc2, oc3, oc4, oc5, oc6, oc7, oc8, oc9;
    extern char canopen,canmode;
    extern volatile MMFLOAT VCC;
    extern int PromptFont, PromptFC, PromptBC;                          // the font and colours selected at the prompt;
    extern void TM_USART2_ReceiveHandler(uint8_t c);
    typedef struct {
    	uint8_t *Buffer;
    	uint16_t Size;
    	uint16_t Num;
    	uint16_t In;
    	uint16_t Out;
    	uint8_t Initialized;
    	uint8_t StringDelimiter;
    } TM_USART_t;
    extern volatile unsigned int WDTimer;                               // used for the watchdog timer
    extern TM_USART_t TM_USART2;
    extern TM_USART_t TM_USART3;
    extern int BasicRunning;
    extern uint32_t PROG_FLASH_SIZE;
    extern uint32_t PROG_FLASH_SIZEMAX;
    extern uint32_t LIBRARY_FLASH_OFFSET;
    // console related I/O
    int MMInkey(void);
    int MMgetchar(void);
    char MMputchar(char c);
    extern void CheckAbort(void) ;
    extern void TM_USART_INT_InsertToBuffer(TM_USART_t* u, uint8_t c);
    int kbhitConsole(void);
    void putConsole(int c);
    void SoftReset(void);
    extern void SerUSBPutS(char *s);
    extern void SerUSBPutC(char c);
    void SaveProgramToFlash(char *pm, int msg);
    int getConsole(void);
    void initSerialConsole(void);
    extern int pattern_matching (	/* 0:not matched, 1:matched */
    	const TCHAR* pat,	/* Matching pattern */
    	const TCHAR* nam,	/* String to be tested */
    	int skip,			/* Number of pre-skip chars (number of ?s) */
    	int inf				/* Infinite search (* specified) */
    );



    // Use the core timer.  The maximum delay is 4 seconds
    void uSec(unsigned int us);
    void shortpause(unsigned int ticks);
    // used to control the processor reset
    //extern unsigned int _excep_dummy;//  __attribute__ ((persistent)); // for some reason persistent does not work on the first variable
    extern unsigned int _excep_code;//  __attribute__ ((persistent));  // if there was an exception this is the exception code
    //extern unsigned int _excep_addr;//  __attribute__ ((persistent));  // and this is the address
    extern unsigned int _restart_reason;//  __attribute__ ((persistent));  // and this is the address
    extern void PRet(void);
    extern void PInt(int64_t n);
    extern void PO(char *s,int m);
    extern void PIntComma(int64_t n);
    extern void PO2Str(char *s1, const char *s2, int m);
    extern void PO2Int(char *s1, int64_t n);
    extern void PO3Int(char *s1, int64_t n1, int64_t n2);
    extern void PIntH(unsigned long long int n);
    extern void PIntHC(unsigned long long int n);
    extern void PFlt(MMFLOAT flt);
    extern void PFltComma(MMFLOAT n);
    extern void SRet(void);
    extern void SInt(int64_t n);
    extern void SIntComma(int64_t n);
    extern void SIntH(unsigned long long int n);
    extern void SIntHC(unsigned long long int n);
    extern void SFlt(MMFLOAT flt);
    extern void SFltComma(MMFLOAT n);
    extern void PPinName(int n);
    extern void PPinNameComma(int n);
    extern void varnamecopy(char *out, char *in);
    extern void mycpy(void *out, const void *in, int n);
    extern void myset(void *out, uint32_t in, int n);
    extern void mymemset(void *out, uint32_t in, int n);
    extern void mycopy(void *out, const void *in, int n);
    extern void zcopy(void *out, const void *in, int n);
    extern void _Z10copy_wordsPKmPmm(uint32_t *s, uint32_t *d, int n);
    extern uint64_t *clearpage(void *out);
    extern void cleardims(void *vardim);
    extern void clearvar(void *var);

    extern void  setterminal(int height,int width);

    #ifdef __DEBUG
        void dump(char *p, int nbr);
    #endif

//    #define dp(...) {char s[140];sprintf(s,  __VA_ARGS__); MMPrintString(s); MMPrintString("\r\n");}

    #define db(i) {IntToStr(inpbuf, i, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
    #define db2(i1, i2) {IntToStr(inpbuf, i1, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i2, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
    #define db3(i1, i2, i3) {IntToStr(inpbuf, i1, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i2, 10); MMPrintString(inpbuf); MMPrintString("  "); IntToStr(inpbuf, i3, 10); MMPrintString(inpbuf); MMPrintString("\r\n");}
    #define ds(s) {MMPrintString(s); MMPrintString("\r\n");}
    #define ds2(s1, s2) {MMPrintString(s1); MMPrintString(s2); MMPrintString("\r\n");}
    #define ds3(s1, s2, s3) {MMPrintString(s1); MMPrintString(s2); MMPrintString(s3); MMPrintString("\r\n");}
    #define pp(i) { PinSetBit(i, TRISCLR); PinSetBit(i, ODCCLR); PinSetBit(i, LATCLR); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); }
    #define pp2(i) { PinSetBit(i, TRISCLR); PinSetBit(i, ODCCLR); PinSetBit(i, LATCLR); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); uSec(30); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); }
    #define pp3(i) { PinSetBit(i, TRISCLR); PinSetBit(i, ODCCLR); PinSetBit(i, LATCLR); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); uSec(30); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); uSec(30); PinSetBit(i, LATSET); uSec(30); PinSetBit(i, LATCLR); }
#endif
#define VGA             1
#define SSD1963_4       2
#define SSD1963_5       3
#define SSD1963_5A      4
#define SSD1963_5ER     5
#define SSD1963_7       6
#define SSD1963_7A      7
#define SSD1963_8       8
#define SSD_PANEL_8     SSD1963_8    // anything less than or equal to SSD_PANEL_8 is handled by the SSD 8-bit driver, anything more by the 16-bit or SPI driver

#define SSD1963_4_16    11
#define SSD1963_5_16    12
#define SSD1963_5A_16   13
#define SSD1963_5ER_16  14
#define SSD1963_7_16    15
#define SSD1963_7A_16   16
#define SSD1963_8_16    17

#define IPS_4_16        18          //OTM8009A 16bit display 800*480
#define SSD_PANEL       20    // anything less than or equal to SSD_PANEL is handled by the SSD1963.c driver, anything more by the SPI-LCD.c driver

#define ILI9163         21       //SPI-LCD.c
#define ST7735          22       //SPI-LCD.c
#define SPI_PANEL       ST7735   // anything greater than SPI_PANEL is handled by otherdisplays

#define USER            25
#define ILI9481         26
#define ILI9341		    27
#define ILI9481IPS      28
#define ILI9488         29
#define ST7796S         30
#define SPI_PANEL_END   30

#define ILI9341_16      31
#define ILI9341_8       32
#define SSD1963_5_BUFF  33
#define SSD1963_7_BUFF  34
#define SSD1963_8_BUFF  35
#define SSD1963_5_640   36
#define SSD1963_7_640   37
#define SSD1963_8_640   38
#define SSD1963_5_8BIT  39
#define SSD1963_7_8BIT  40
#define SSD1963_8_8BIT  41
#define HDMI			45

#define LANDSCAPE       1
#define PORTRAIT        2
#define RLANDSCAPE      3
#define RPORTRAIT       4
#define DISPLAY_LANDSCAPE   (Option.DISPLAY_ORIENTATION & 1)
#define TOUCH_NOT_CALIBRATED    -999999
#define RESET_COMMAND       9999                                // indicates that the reset was caused by the RESET command
#define WATCHDOG_TIMEOUT    9998                                // reset caused by the watchdog timer
#define PIN_RESTART         9997                                // reset caused by entering 0 at the PIN prompt
#define RESTART_NOAUTORUN   9996                                // reset required after changing the LCD or touch config
#define RESTART_HEAP		9995
#define SCREWUP_TIMEOUT    	9994                                // reset caused by the watchdog timer
#define RESTART_DOAUTORUN	9993
//#define RESTART_BOOT0   	9992
#define SD_SLOW_SPI_SPEED 0
#define SD_FAST_SPI_SPEED 1
#define LCD_SPI_SPEED    2                                   // the speed of the SPI bus when talking to an SPI LCD display controller
#define TOUCH_SPI_SPEED 3
#define NONE_SPI_SPEED 4
#define IsAlpha(a) isalpha((uint8_t)a)
#define IsNamesart(a) isnamestart((uint8_t)a)
#define IsDigit(a) isdigit((uint8_t)a)
#define IsxDigit(a) isxdigit((uint8_t)a)
#define IsPrint(a) isprint((uint8_t)a)
#define IsAlnum(a) isalnum((uint8_t)a)
#include "Serial.h"
#include "FileIO.h"
#include "Memory.h"
#include "External.h"
#include "MM_Misc.h"
#include "MM_Custom.h"
#include "Onewire.h"
#include "I2C.h"
#include "SerialFileIO.h"
#include "PWM.h"
#include "SPI.h"
#include "CAN.h"
#include "Flash.h"
#include "Xmodem.h"
#include "Draw.h"
#include "editor.h"
#include "ff.h"
#include "diskio.h"
#include "touch.h"
#include "SSD1963.h"
#include "GUI.h"
#include "audio.h"
#include "OtherDisplays.h"
#include "gps.h"
#include "keyboard.h"
#include "sprites.h"
#include "upng.h"
//#include "turtle.h"
#include "cJSON.h"
#include "Maths.h"

