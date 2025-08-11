/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

Flash.h

Include file that contains the globals and defines for flash save/load in MMBasic.

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


#include "stdint.h"
#define FLASH_BASE_ADDR      (uint32_t)(FLASH_BASE)
#define FLASH_END_ADDR       (uint32_t)(0x08200000)

	/* Base address of the Flash sectors Bank 1 */
	#define ADDR_FLASH_SECTOR_0_BANK1     ((uint32_t)0x08000000) /* Base @ of Sector 0, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_1_BANK1     ((uint32_t)0x08020000) /* Base @ of Sector 1, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_2_BANK1     ((uint32_t)0x08040000) /* Base @ of Sector 2, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_3_BANK1     ((uint32_t)0x08060000) /* Base @ of Sector 3, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_4_BANK1     ((uint32_t)0x08080000) /* Base @ of Sector 4, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_5_BANK1     ((uint32_t)0x080A0000) /* Base @ of Sector 5, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_6_BANK1     ((uint32_t)0x080C0000) /* Base @ of Sector 6, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_7_BANK1     ((uint32_t)0x080E0000) /* Base @ of Sector 7, 128 Kbytes */
	/* Base address of the Flash sectors Bank 2 */
	#define ADDR_FLASH_SECTOR_0_BANK2     ((uint32_t)0x08100000) /* Base @ of Sector 0, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_1_BANK2     ((uint32_t)0x08120000) /* Base @ of Sector 1, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_2_BANK2     ((uint32_t)0x08140000) /* Base @ of Sector 2, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_3_BANK2     ((uint32_t)0x08160000) /* Base @ of Sector 3, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_4_BANK2     ((uint32_t)0x08180000) /* Base @ of Sector 4, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_5_BANK2     ((uint32_t)0x081A0000) /* Base @ of Sector 5, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_6_BANK2     ((uint32_t)0x081C0000) /* Base @ of Sector 6, 128 Kbytes */
	#define ADDR_FLASH_SECTOR_7_BANK2     ((uint32_t)0x081E0000) /* Base @ of Sector 7, 128 Kbytes */
	#define FLASH_PROGRAM_ADDR       ADDR_FLASH_SECTOR_0_BANK2   /* Start Basic Program flash area */
    #define FLASH_LIBRARY_ADDR       ADDR_FLASH_SECTOR_3_BANK2   /* Start Library Program flash area */
	#define FLASH_SAVED_OPTION_ADDR  ADDR_FLASH_SECTOR_5_BANK2   /* Start of Saved Options flash area */
    //#define FLASH_SAVED_OPTION_ADDR  ADDR_FLASH_SECTOR_1_BANK1   /* Start of Saved Options flash area */
	#define FLASH_SAVED_VAR_ADDR     ADDR_FLASH_SECTOR_4_BANK2   /* Start of Saved Variables flash area */
	#define SAVEDVARS_FLASH_SIZE 0x20000  // amount of flash reserved for saved variables
/* Base address of the Flash sectors */

/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/
#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE) && !defined(FLASH_INCLUDED)
#define FLASH_INCLUDED

    // IMPORTANT: Change the string constant in cmd_memory() if you change PROG_FLASH_SIZE
#define EDIT_BUFFER_SIZE  ((unsigned int)(RAMEND - (unsigned int)RAMBase - 1024))  // this is the maximum RAM that we can get
#define SAVED_OPTIONS_FLASH 4
#define SAVED_VARS_FLASH 2
#define PROGRAM_FLASH 1
#define LIBRARY_FLASH 3

struct option_s {
	uint32_t Magic;
    char Autorun;
    char Tab;
    char Invert;
    char Listcase;
    char Height;
    char Width;
    short dummy;
    uint32_t  PIN;
    uint32_t  Baudrate;
    int  ColourCode;

    // display related
    char DISPLAY_TYPE;
    char DISPLAY_ORIENTATION;

    // touch related
    unsigned char TOUCH_CS;
    unsigned char TOUCH_IRQ;
    char TOUCH_SWAPXY;
    unsigned char NoScroll;         //NoScroll from picomites added @beta3
    unsigned char BGR;              //Stores INVERT for LCDs
    char dummyc[1];
    int  TOUCH_XZERO;
    int  TOUCH_YZERO;
    MMFLOAT TOUCH_XSCALE;
    MMFLOAT TOUCH_YSCALE;

    // for the SPI LCDs
    char LCD_CD;
    char LCD_CS;
    char LCD_Reset;

    // these are only used in the MX470 version
    char SerialConDisabled;
    unsigned char SDCARD_CS;
    int SD_CD;
    int SD_WP;
    char SSDspeed;
    char DISPLAY_CONSOLE;
    char DefaultFont;
    char KeyboardConfig;
    unsigned char TOUCH_Click;
    unsigned int FlashPages;
    unsigned int ProgFlashSize;    // used to store the size of the program flash (also start of the LIBRARY code)

    int DefaultFC, DefaultBC;      // the default colours
    int DefaultBrightness;         // default backlight brightness
    short RTC_Calibrate;
	short RepeatStart;
	short RepeatRate;
    char SerialPullup;
    // To enable older CFunctions to run any new options *MUST* be added at the end of the list
    short MaxCtrls;                // maximum number of controls allowed
    char fulltime;
    char USBKeyboard;
    signed char USBpower;
    char Refresh;
    int DISPLAY_WIDTH;
    int DISPLAY_HEIGHT;
    int CPUspeed;
    uint8_t noLED;
    volatile uint8_t USBPolling;
    unsigned char F1key[MAXKEYLEN]; //268
    unsigned char F5key[MAXKEYLEN]; //268
    unsigned char F6key[MAXKEYLEN]; //332
    unsigned char F7key[MAXKEYLEN]; //396
    unsigned char F8key[MAXKEYLEN]; //460
    unsigned char F9key[MAXKEYLEN]; //524

};

extern volatile struct option_s Option, *SOption;
extern unsigned char *CFunctionFlash, *CFunctionLibrary;
extern volatile uint32_t  realflashpointer;
void ResetAllOptions(void);
void ResetAllFlash(int errabort);
void SaveOptions(int errabort);
void LoadOptions(void);
//void FlashWriteInit(int sector, int errabort);
int FlashWriteInit(int sector);
void FlashWriteByte(unsigned char b);
void FlashWriteWord(unsigned int i);
void FlashWriteAlign(void);
void FlashWriteClose(void);
void UpdateFlash(uint32_t address, uint32_t data);
int GetFlashOption(const unsigned int *w) ;
void SetFlashOption(const unsigned int *w, int x) ;
uint32_t GetSector(uint32_t Address);
void cmd_var(void);
void MIPS16 cmd_library(void);  //LIBRARY
long long int CallCFunction(char *CmdPtr, char *ArgList, char *DefP, char *CallersLinePtr);
extern char * ProgMemory;
extern void ClearSavedVars(int errabort);
#if !defined(MX170)
void RoundDoubleFloat(MMFLOAT *ff);
#endif


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


#endif


/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "VAR",	    	T_CMD,				0, cmd_var	},
	{ "Library",        T_CMD,              0, cmd_library  },                 //LIBRARY

#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

#endif
