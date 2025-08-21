/*-*****************************************************************************

MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

main.c

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
  * @file           : main.c
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_gpio.h"  //GERRY
#include "usb_host.h"

/* USER CODE BEGIN Includes */
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include "memory.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

DAC_HandleTypeDef hdac1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

RNG_HandleTypeDef hrng;

RTC_HandleTypeDef hrtc;

FDCAN_HandleTypeDef hfdcan;   //CAN added

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;
SPI_HandleTypeDef GenSPI;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim16;

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart3;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
extern FATFS FatFs;  /* File system object for SD card logical drive */
extern char SDPath[4]; /* SD card logical drive path */
extern void InitFileIO(void);
FIL fil;
struct s_PinDef *PinDef;
int PromptFont, PromptFC, PromptBC;                             // the font and colours selected at the prompt
char BreakKey = BREAK_KEY;                                          // defaults to CTRL-C.  Set to zero to disable the break function
char IgnorePIN = false;
char WatchdogSet = false;
uint8_t RxBuffer, TxBuffer;
#define progress  "\rprogress\r\n"
int MMCharPos;
int MMPromptPos;
char LCDAttrib;


volatile int MMAbort = false;
int use_uart;
//unsigned int __attribute__((section(".my_section"))) _excep_dummy; // for some reason persistent does not work on the first variable
unsigned int __attribute__((section(".my_section"))) _excep_code;  //  __attribute__ ((persistent));  // if there was an exception this is the exception code
//unsigned int __attribute__((section(".my_section"))) _excep_addr;  //  __attribute__ ((persistent));  // and this is the address
unsigned int __attribute__((section(".my_section"))) _restart_reason;  //  __attribute__ ((persistent));  // and this is the address
unsigned int __attribute__((section(".my_section"))) _excep_cause;  //  __attribute__ ((persistent));  // and this is the address
unsigned int __attribute__((section(".my_section"))) _excep_keys;  //  __attribute__ ((persistent));  // and this is the address
char *InterruptReturn = NULL;
int BasicRunning = false;
int BasicReset = 0;
volatile int keyboardseen=0;
char ConsoleRxBuf[CONSOLE_RX_BUF_SIZE];
volatile int ConsoleRxBufHead = 0;
volatile int ConsoleRxBufTail = 0;
char ConsoleTxBuf[CONSOLE_TX_BUF_SIZE];
volatile int ConsoleTxBufHead = 0;
volatile int ConsoleTxBufTail = 0;
uint8_t BlinkSpeed = 0, str[20];
extern void printoptions(void);
uint32_t PROG_FLASH_SIZE = 0x80000;
uint32_t PROG_FLASH_SIZEMAX = 0x80000;
uint32_t LIBRARY_FLASH_OFFSET = 0x60000;
void initConsole(void);
volatile uint64_t Count5High = 0;
uint32_t ticks_per_microsecond;
uint32_t pll1,pll2;
uint32_t hse_value;
int executerun=0;   //cmm2
char canopen=0;	  //CAN has no pins assigned
char errstring[256]={0};
int errpos=0;
int SystemError=0;
int myDummy=0;  //Used in fix for intermittent USBKeyboard connection

//extern void JumpToBootloader(void);

#define DISPLAY_CLS             1
#define REVERSE_VIDEO           3
#define CLEAR_TO_EOL            4
#define CLEAR_TO_EOS            5
#define SCROLL_DOWN             6
#define DRAW_LINE               7
//extern void MX470PutS(char *s, int fc, int bc);
//extern void MX470Cursor(int x, int y);
extern void MX470Display(int fn);

extern BYTE (*xchg_byte) (BYTE data_out);
extern BYTE xchg_spi (BYTE data_out);
extern BYTE xchg_bitbang (BYTE data_out);
extern void (*xmit_byte_multi) (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
);
extern void (*rcvr_byte_multi) (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
);
extern void xmit_spi_multi (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
);
extern void rcvr_spi_multi (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
);
extern void xmit_bitbang_multi (
	const BYTE* buff,	// Data to be sent
	UINT cnt			// Number of bytes to send
);
extern void rcvr_bitbang_multi (
	BYTE* buff,		// Buffer to store received data
	UINT cnt		// Number of bytes to receive
);

/* USER CODE END PV */
/******* BLINKLED Macro********************************************************/
/************************************************************************/
//#define PE3_Pin GPIO_PIN_3
//#define PE3_GPIO_Port GPIOE
//#define BLINKLED {while (1){HAL_GPIO_TogglePin(PE3_GPIO_Port,PE3_Pin);HAL_Delay(500);}}  //WEACT
//#define BLINKLED {while (1){HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_3);HAL_Delay(500);}}  //WEACT

//#define PA1_Pin GPIO_PIN_1
//#define PA1_GPIO_Port GPIOA
//#define BLINKLED2 {while (1){HAL_GPIO_TogglePin(PA1_GPIO_Port,PA1_Pin);HAL_Delay(500);}} //DEVEBOX
/************************************************************************/



/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void SystemClock_Config25M(void);
//void SystemClock_Config25MDEVEBOX(void);

static void MX_GPIO_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_RNG_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void MX_RTC_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI3_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_UART5_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
void MX_TIM8_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI5_Init(void);
void MX_USB_HOST_Process(void);
static void MX_TIM16_Init(int prescale);
//backlight
static void MX_TIM1_Init(void);
static void MX_FDCAN1_Init(void);   //CAN added

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                
#define MAXKEYLEN           64

#define CMD_BUFFER_SIZE STRINGSIZE*4
char lastcmd[CMD_BUFFER_SIZE];            // used to store the last command in case it is needed by the EDIT command
void InsertLastcmd( char *s);


/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void putConsole(int c);
void executelocal(char *p);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void cleanend(void){
	int i;//, adjust=0;
	//int maxH=PageTable[WritePage].ymax;
    memset(inpbuf,0,STRINGSIZE);
   // int lastgui=gui_font_height;
	//SetFont(Option.DefaultFont);
	//adjust=gui_font_height-lastgui;
   // if(mouse1)i2c_disable();                                                  // close I2C
   // if(mouse2)i2c2_disable();                                                  // close I2C
   // if(mouse3)i2c3_disable();                                                  // close I2C
	//ShareNeeded=RoundUptoPage512((uint32_t)PageTable[0].address);
	//MPU_Config_nCacheable(0);
	//HAL_LTDC_SetAddress(&hltdc,  (uint32_t)PageTable[0].address, LTDC_LAYER_1);
   // mouse0close();
   // mouse0=0; mouse1=0; mouse2=0; mouse3=0;
    OnKeyGOSUB=NULL;							            // set the next stmt to the interrupt location
    com1_interrupt=NULL;									// set the next stmt to the interrupt location
    com1_TX_interrupt=NULL;
    com2_interrupt=NULL;									// set the next stmt to the interrupt location
    com2_TX_interrupt=NULL;
    com3_interrupt=NULL;									// set the next stmt to the interrupt location
    com3_TX_interrupt=NULL;
    com4_interrupt=NULL;									// set the next stmt to the interrupt location
    com4_TX_interrupt=NULL;
    KeyInterrupt=NULL;									    // set the next stmt to the interrupt location
    WAVInterrupt=NULL;									    // set the next stmt to the interrupt location
    COLLISIONInterrupt=NULL;							    // set the next stmt to the interrupt location
    ADCInterrupt=NULL;									    // set the next stmt to the interrupt location
    DACInterrupt=NULL;									    // set the next stmt to the interrupt location
    IrInterrupt=NULL;									    // set the next stmt to the interrupt location
   // FrameInterrupt=NULL;									    // set the next stmt to the interrupt location
  // CountInterrupt=NULL;


    for(i = 0; i < NBRINTERRUPTS; i++) {                            // scan through the interrupt table
    	inttbl[i].intp=NULL;							// set the next stmt to the interrupt location
    }
    for(i = 0; i < NBRSETTICKS; i++) {
    	TickInt[i]=NULL;
    }
	keyselect=0;
	//closecursor();
	//ScrollLCD(adjust,1);
	//CurrentY-=adjust;
    //if(CurrentY + gui_font_height >= maxH-(ShortScroll?gui_font_height:0)-1) {
	//	ShortScroll=Option.showstatus;
    //    if(!ShortScroll){
    //        ScrollLCD(CurrentY + gui_font_height- maxH , 1);
    //    	CurrentY -= (CurrentY + gui_font_height * 2 - maxH );
    //    } else {
    //    	ScrollLCD(lastgui,1);
    //    	CurrentY -= (gui_font_height * 2 - adjust);
    //    }
//	//	CurrentY= maxH-(gui_font_height*2);
//   // 		CurrentY=CurrentY-(gui_font_height);
	//}
	//sendCRLF = 3;
	//SCB_CleanInvalidateDCache();
	CloseAudio(1);
	CloseAllFiles();
    longjmp(mark, 1);
}

#ifndef CMD_FLASH

// dump a memory area to the console
// for debugging
void dump(char *p, int nbr,int page) {
    char buf1[80], buf2[80], *b1, *b2, *pt;
    b1 = buf1; b2 = buf2;
    MMPrintString("\r\n");
    MMPrintString("---------------------Dump Page ");
    PInt(page);
    MMPrintString("----------------  \r\n");
    MMPrintString("   addr    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF\r\n");
    b1 += sprintf(b1, "%8x: ", (unsigned int)p);
    for(pt = p; (unsigned int)pt % 16 != 0; pt--) {
        b1 += sprintf(b1, "   ");
        b2 += sprintf(b2, " ");
    }
    while(nbr > 0) {
        b1 += sprintf(b1, "%02x ", *p);
        b2 += sprintf(b2, "%c", (*p >= ' ' && *p < 0x7f) ? *p : '.');
        p++;
        nbr--;
        if((unsigned int)p % 16 == 0) {
            MMPrintString(buf1);
            MMPrintString("   ");
            MMPrintString(buf2);
            b1 = buf1; b2 = buf2;
            b1 += sprintf(b1, "\r\n%8x: ", (unsigned int)p);
        }
    }
    if(b2 != buf2) {
        MMPrintString(buf1);
        MMPrintString("   ");
        for(pt = p; (unsigned int)pt % 16 != 0; pt++) {
            MMPrintString("   ");
        }
      MMPrintString(buf2);
    }
    MMPrintString("\r\n");
    uSec(50000);
}
#endif



//void flashled(int);
void flashled(int i, int d){
	 //FLASH RED LED for Diagnostics
	 if(HAS_144PINS){//Flash LED on pin 75 PB14
		      PinDef=(struct s_PinDef *)PinDef144;
		     __HAL_RCC_GPIOB_CLK_ENABLE();
			 ExtCfg(75, EXT_DIG_OUT, 0);
			 LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_14);
			 while(i>0){
			    LL_GPIO_TogglePin(GPIOB,GPIO_PIN_14);
				HAL_Delay( d);
				LL_GPIO_TogglePin(GPIOB,GPIO_PIN_14);
				HAL_Delay( d);
			    i--;
			 }
			 LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_14);
			 HAL_Delay( 1500);
	 }else{ //Flash LED on pin2 ,PE3
		    PinDef=(struct s_PinDef *)PinDef100;
		    __HAL_RCC_GPIOE_CLK_ENABLE();
		  	 ExtCfg(2, EXT_DIG_OUT, 0);
			 LL_GPIO_SetOutputPin(GPIOE,GPIO_PIN_3);
			 while(i>0){
			    LL_GPIO_TogglePin(GPIOE,GPIO_PIN_3);
				HAL_Delay( d);
				LL_GPIO_TogglePin(GPIOE,GPIO_PIN_3);
				HAL_Delay( d);
			    i--;
			 }
			 LL_GPIO_ResetOutputPin(GPIOE,GPIO_PIN_3);
			 HAL_Delay( 1500);
	 }


}

/* USER CODE END 0 */


/**
 * @brief Transforms input beginning with * into a corresponding RUN command.
 *
 * e.g.
 *   *foo              =>  RUN "foo"
 *   *"foo bar"        =>  RUN "foo bar"
 *   *foo --wombat     =>  RUN "foo", "--wombat"
 *   *foo "wom"        =>  RUN "foo", Chr$(34) + "wom" + Chr$(34)
 *   *foo "wom" "bat"  =>  RUN "foo", Chr$(34) + "wom" + Chr$(34) + " " + Chr$(34) + "bat" + Chr$(34)
 *   *foo --wom="bat"  =>  RUN "foo", "--wom=" + Chr$(34) + "bat" + Chr$(34)
 */
static void transform_star_command(char *input) {
    char *src =  input;
    while (isspace((unsigned char)*src)) src++; // Skip leading whitespace.

    if (*src != '*') error("Internal fault");
    src++;

    // Trim any trailing whitespace from the input.
    char *end = input + strlen(input) - 1;
    while (isspace((unsigned char)*end)) *end-- = '\0';

    // Allocate extra space to avoid string overrun.
    char *tmp = (char *) GetTempMemory(STRINGSIZE + 32);
    strcpy(tmp, "RUN");
    char *dst = tmp + 3;

    if (*src == '"') {
        // Everything before the second quote is the name of the file to RUN.
        *dst++ = ' ';
        *dst++ = *src++; // Leading quote.
        while (*src && *src != '"') *dst++ = *src++;
        if (*src == '"') *dst++ = *src++; // Trailing quote.
    } else {
        // Everything before the first space is the name of the file to RUN.
        int count = 0;
        while (*src && !isspace((unsigned char)*src)) {
            if (++count == 1) {
                *dst++ = ' ';
                *dst++ = '\"';
            }
            *dst++ = *src++;
        }
        if (count) *dst++ = '\"';
    }

    while (isspace((unsigned char)*src)) src++; // Skip whitespace.

    // Anything else is arguments.
    if (*src) {
        *dst++ = ',';
        *dst++ = ' ';

        // If 'src' starts with double-quote then replace with: Chr$(34) +
        if (*src == '"') {
            memcpy(dst, "Chr$(34) + ", 11);
            dst += 11;
            src++;
        }

        *dst++ = '\"';

        // Copy from 'src' to 'dst'.
        while (*src) {
            if (*src == '"') {
                // Close current set of quotes to insert a Chr$(34)
                memcpy(dst, "\" + Chr$(34)", 12);
                dst += 12;

                // Open another set of quotes unless this was the last character.
                if (*(src + 1)) {
                    memcpy(dst, " + \"", 4);
                    dst += 4;
                }
                src++;
            } else {
                *dst++ = *src++;
            }
            if (dst - tmp >= STRINGSIZE) error("String too long");
        }

        // End with a double quote unless 'src' ended with one.
        if (*(src - 1) != '"') *dst++ = '\"';

        *dst = '\0';
    }

    if (dst - tmp >= STRINGSIZE) error("String too long");

    // Copy transformed string back into the input buffer.
    memcpy(input, tmp, STRINGSIZE);
    input[STRINGSIZE - 1] = '\0';

    ClearSpecificTempMemory(tmp);
}
//#define DIAG
/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//	__set_MSP(0x2407F000);
	ProgMemory=(char*)ADDR_FLASH_SECTOR_0_BANK2;
	int i;
	static int ErrorInPrompt;
	SOption=(volatile struct option_s *)FLASH_SAVED_OPTION_ADDR;
    SavedMemoryBufferSize=0;
  /* USER CODE END 1 */

  /* Enable I-Cache-------------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache-------------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  HAL_Delay(200);
  if(HAS_144PINS)PinDef=(struct s_PinDef *)PinDef144;
  if(HAS_100PINS)PinDef=(struct s_PinDef *)PinDef100;
#ifdef DIAG
   flashled(1,300);
#endif
  //flashled(2,100);
  /* USER CODE BEGIN Init  */

//  if(_excep_code == RESTART_BOOT0){
//  	_excep_code = 0;
//  	JumpToBootloader();
//  }



//flashled(2,300);
#ifndef DIAG
  LoadOptions();
#endif
//flashled(3,300);

  if(HAS_144PINS){
	  hse_value=8000000;
    if(Option.CPUspeed<10 || Option.CPUspeed>600){
	  pll1=(HAL_GetREVID()==0x1003 ? 100 : 120);
	  pll2=2;
    } else {
	  pll1=Option.CPUspeed/4;
	  pll2=2;
	  while(pll1<25){
		  pll1*=2;
		  pll2*=2;
	  }
    }

  }else{
#ifdef DIAG
	 flashled(2,300);
	 pll1=(HAL_GetREVID()==0x1003 ? 180 : 192);
	 pll2=2;
#endif
	 hse_value=25000000;
	 if(Option.CPUspeed<10 || Option.CPUspeed>600){
	  	  pll1=(HAL_GetREVID()==0x1003 ? 180 : 192);
	  	  pll2=2;
	 } else {
#ifndef	DIAG
		 // minimum pll1 is 30
		if(Option.CPUspeed<15){
			 pll1=40;
			 pll2=20;
		}else if(Option.CPUspeed<20){
			pll1=30;
		    pll2=10;
		}else if(Option.CPUspeed<25){
			pll1=40;
			pll2=10;
		}else if(Option.CPUspeed<30){
			pll1=40;
			pll2=8;
		}else if(Option.CPUspeed<35){
			pll1=60;
		    pll2=10;
		}else if(Option.CPUspeed<40){
			pll1=70;
			pll2=10;
		}else if(Option.CPUspeed<45){
			pll1=32;
			pll2=4;
		}else if(Option.CPUspeed<50){
			pll1=90;
			pll2=10;
		}else{
		   pll1=2*Option.CPUspeed/5;
	  	  pll2=2;
	  	  while(pll1<65){
	  		  pll1*=2;
	 		  pll2*=2;
	  	  }
		}
#endif
	 }

  }
  /* USER CODE END Init */





  /* Configure the system clock */
  if(HAS_144PINS){
	//flashled(2,300);
    SystemClock_Config();
  }else{
	 // flashled(2,700);
    SystemClock_Config25M();
#ifdef DIAG
    flashled(3,300);
#endif

  }

  //HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);  //clock on PA8

  //flashled(5,300);
  //SystemCoreClockUpdate();

  /* USER CODE BEGIN SysInit */
  HAL_FLASH_Unlock();
  PeripheralBusSpeed=SystemCoreClock/2;
  ticks_per_microsecond=PeripheralBusSpeed/1000000;

  #ifdef DIAG
         ResetAllFlash(0);              // init the options if this is the very first startup
     	//  _excep_code=0;
       flashled(4,300);
  #endif

  if(Option.Magic != MagicKey) {
	   ResetAllFlash(0);              // init the options if this is the very first startup
	  _excep_code=0;
  }

  LoadOptions();
#ifdef DIAG
    flashled(5,300);
#endif
  PROG_FLASH_SIZE = Option.FlashPages*0x20000;
  //fix Option.ProgFlashSize if from previous version software
  if(Option.ProgFlashSize < LIBRARY_FLASH_OFFSET){
	  Option.ProgFlashSize = PROG_FLASH_SIZEMAX;
	  SaveOptions(1);
  }
  //Option.ProgFlashSize = PROG_FLASH_SIZE;
  if(Option.DISPLAY_TYPE == SSD1963_5_640 || Option.DISPLAY_TYPE == SSD1963_7_640  || Option.DISPLAY_TYPE == SSD1963_8_640)SavedMemoryBufferSize=640*2*200+256;
  if(Option.DISPLAY_TYPE == SSD1963_5_BUFF || Option.DISPLAY_TYPE == SSD1963_7_BUFF  || Option.DISPLAY_TYPE == SSD1963_8_BUFF )SavedMemoryBufferSize=800*2*256+256;
  if(Option.DISPLAY_TYPE >= SSD1963_5_8BIT && Option.DISPLAY_TYPE < HDMI) SavedMemoryBufferSize=800*32+256;
  RAMBase = (void *)((unsigned int)RAMBASE + (Option.MaxCtrls * sizeof(struct s_ctrl))+ SavedMemoryBufferSize);
  RAMBase = (void *)MRoundUp((unsigned int)RAMBase);
  //PinDef = (struct s_PinDef *)PinDef144;   //VET6
  //if(HAS_144PINS)PinDef=(struct s_PinDef *)PinDef144;
  //if(HAS_100PINS)PinDef=(struct s_PinDef *)PinDef100;
  // setup a pointer to the base of the GUI controls table
  Ctrl = (struct s_ctrl *)RAMBASE;
  for(i = 1; i < Option.MaxCtrls; i++) {
      Ctrl[i].state = Ctrl[i].type = 0;
      Ctrl[i].s = NULL;
  }

  xchg_byte=(Option.SDCARD_CS==79 ? xchg_bitbang : xchg_spi);
  xmit_byte_multi=(Option.SDCARD_CS==79 ? xmit_bitbang_multi : xmit_spi_multi);
  rcvr_byte_multi=(Option.SDCARD_CS==79 ? rcvr_bitbang_multi : rcvr_spi_multi);


  goto skip_init;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DAC1_Init();
  MX_TIM3_Init();
  MX_RNG_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_RTC_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  MX_UART5_Init();
  MX_USART6_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_USB_HOST_Init();
  MX_TIM8_Init();
  MX_I2C1_Init();
  MX_SPI5_Init();
  /* USER CODE BEGIN 2 */
skip_init:

if(HAS_144PINS){   //Original NUCLEO STM32H743TI 144 pins and  8MHz external Clock CPU 400MHz and TI2 480MHz
	MX_GPIO_Init();
	MX_DAC1_Init();
	MX_TIM3_Init();
	MX_RNG_Init();
	MX_SPI2_Init();
	MX_TIM2_Init();
	MX_TIM16_Init((Option.CPUspeed>>1)-1);
	MX_TIM1_Init();  //TIM1 CH2N is the backlight
	MX_RTC_Init();
	MX_USART3_UART_Init();
	MX_SPI1_Init();
	MX_SPI3_Init();
	MX_I2C2_Init();
	MX_SPI5_Init();
	MX_I2C1_Init();
	//MX_FDCAN1_Init();
	if(HAL_GPIO_ReadPin(GPIOC,  GPIO_PIN_13)){
	 //Its an MMBasic Reset
		ResetAllFlash(0);
		//_restart_reason=2;
		BasicReset=13;
//#ifdef DIAG
     //   flashled(4,200);
//#endif
	}
  initExtIO();
  HAL_TIM_Base_Start_IT(&htim16);
  HAL_UART_DeInit(&huart3);
  huart3.Init.BaudRate = Option.Baudrate;

  HAL_UART_Init(&huart3);
  HAL_TIM_Base_Start(&htim2);
  InitDisplayOther(1);
  InitDisplaySPI(1);
  InitDisplaySSD(1);
  InitBasic();
  InitTouch();
  InitHeap();
  InitFileIO();
  BasicRunning = true;
}else{	      //WeAct or DevEBox STM32H743VIT6 100 pins with  25MHz external Clock and CPU 480MHz
	MX_GPIO_Init();
	MX_DAC1_Init();
	MX_TIM3_Init();
	MX_RNG_Init();

	MX_TIM2_Init();
	MX_TIM16_Init((Option.CPUspeed>>1)-1);
	MX_TIM1_Init(); //TIM1 CH2N is PWM for the backlight

	MX_RTC_Init();
	MX_USART3_UART_Init();
	MX_SPI1_Init();
	//MX_SPI3_Init();  //NOT USED VIT6
	MX_I2C2_Init();
	MX_SPI5_Init();  // VIT6 uses SPI2 as the GenSPI for Touch,SDCARD
	MX_I2C1_Init();
	MX_FDCAN1_Init();
	//Test for an MMBasic reset by PC13 being high
	//The K1 PC13 pin needs a PULLDOWN as its floating
	//PC13 is part of RTC backup domain. Setting as
	// an input with PULLDOWN seems to need doing twice
	// for some reason before it works.
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_Delay(200);
    //Again
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_Delay(200);
    //test if K1 is pressed
	if(HAL_GPIO_ReadPin(GPIOC,  GPIO_PIN_13)){
	  //Its an MMBasic Reset
	   ResetAllFlash(0);
	   //_restart_reason=2;
	   BasicReset=13;
//#ifdef DIAG
       // flashled(4,200);
//#endif
	}
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

   initExtIO();
   HAL_TIM_Base_Start_IT(&htim16);
   HAL_UART_DeInit(&huart3);
   huart3.Init.BaudRate = Option.Baudrate;
   HAL_UART_Init(&huart3);
   HAL_TIM_Base_Start(&htim2);
   InitDisplayOther(1);
   InitDisplaySPI(1);
   InitDisplaySSD(1);
   //VET6 uses hardware SPI4 as MBMasic SPI2 if no 16 bit parallel displays in use.
   //so only initiate if the spi pin is not used EXT_BOOT_RESERVED by the parallel display
   if(ExtCurrentConfig[SPI2_OUT_PIN] != EXT_BOOT_RESERVED) MX_SPI2_Init();

   InitBasic();
   InitTouch();  //
   InitHeap();
   InitFileIO();
   BasicRunning = true;
}
//if( !BasicReset && _restart_reason <= 7)_restart_reason=1;          //Button Reset by default
if( _restart_reason <= 7)_restart_reason=1;          //Button Reset by default
if( _restart_reason > 7)_restart_reason=0;           //Power Reset
if( BasicReset)_restart_reason=2;                    //MMBasic reset via PC13 high
if (_excep_code==RESET_COMMAND)_restart_reason=3;    //Command Restart
if (_excep_code==WATCHDOG_TIMEOUT)_restart_reason=4; //watchdog Restart
if (_excep_code==SCREWUP_TIMEOUT)_restart_reason=5;  //command timeout Restart
if (_excep_code==RESTART_HEAP)_restart_reason=6;     //Heap Restart


if(Option.USBKeyboard != NO_KEYBOARD){
	  if(abs(Option.USBpower)) {
	    if(HAS_144PINS)SetAndReserve(abs(Option.USBpower), P_OUTPUT, (Option.USBpower>0 ? 1: 0), EXT_BOOT_RESERVED);
	  }
	  HAL_Delay(200);
	  clearrepeat();
	  MX_USB_HOST_Init();
	  HID_MenuInit();
	  i=20;
	  while(i--){
			HID_MenuProcess();
			MX_USB_HOST_Process();
			HAL_Delay(Option.USBPolling);
	  }
}
  _excep_keys=10;
  initConsole();
  HAL_Delay(200);
  ErrorInPrompt = false;
  if(!(_excep_code == RESTART_NOAUTORUN || _excep_code == RESET_COMMAND || _excep_code == SCREWUP_TIMEOUT || _excep_code == WATCHDOG_TIMEOUT )){
	  if(Option.Autorun==0 ){
#ifdef DIAG
       flashled(10,100);
#endif
		  //htim1.Instance->CCR2=1000-50*10;
		  MMPrintString(MES_SIGNON); //MMPrintString(b);                                 // print sign on message
		  if(HAS_144PINS) MMPrintString(" (144 pins) Rev ");
		  if(HAS_100PINS) MMPrintString(" (100 pins) Rev ");
		  if (HAL_GetREVID()==0x1003){
			  MMPrintString("Y");
		  }else{
			  MMPrintString("V");
		  }
		 // MMPrintString(" ");
		 // PInt(pll1);
		 // MMPrintString(" ");
		 // PInt(pll2);
		  MMPrintString(COPYRIGHT);                                   // print copyright message
		  MMPrintString("\r\n");


	  }
  }

  if (BasicReset){
    MMPrintString("!!! MMBasic Reset !!!\r\n" );
    BasicReset=0;
  }

  if(_excep_code == RESTART_HEAP) {
 	  MMPrintString("Error: Heap overrun\r\n");
  }

  if(_excep_code == WATCHDOG_TIMEOUT) {
      WatchdogSet = true;                                 // remember if it was a watchdog timeout
      MMPrintString("\r\n\nWatchdog timeout\r\n");
  }
  if(_excep_code == SCREWUP_TIMEOUT) {
      MMPrintString("\r\n\nCommand timeout\r\n");
  }
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
  HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2047);
  HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2047);
  MX_TIM3_Init();
  *tknbuf = 0;
  HAL_UART_Receive_IT(&huart3, &RxBuffer, 1);
  HAL_NVIC_DisableIRQ(USART1_IRQn);
  HAL_NVIC_DisableIRQ(USART2_IRQn);
  HAL_NVIC_DisableIRQ(USART6_IRQn);
  HAL_NVIC_DisableIRQ(UART5_IRQn);
  //Keyboard was here now at start
  if(setjmp(mark) != 0) {
	  if(CurrentlyPlaying != P_NOTHING)CloseAudio(1);
      // we got here via a long jump which means an error or CTRL-C or the program wants to exit to the command prompt
	  OptionConsole=3;
      ContinuePoint = nextstmt;                                   // in case the user wants to use the continue command
      *tknbuf = 0;                                                // we do not want to run whatever is in the token buffer
      //Options are reloaded on an error, so non permanent Options are set to default we need to reset any we really need.
      if((Option.DISPLAY_TYPE > SPI_PANEL && Option.DISPLAY_TYPE != USER) || (Option.DISPLAY_TYPE==SSD1963_4_16)|| (Option.DISPLAY_TYPE==SSD1963_4)){Option.Refresh=1;}
      memset(inpbuf,0,STRINGSIZE);

  } else {
            ClearProgram();
            PrepareProgram(true);
            _excep_cause = CAUSE_MMSTARTUP;
            if(FindSubFun("MM.STARTUP", 0) >= 0) { 	ExecuteProgram("MM.STARTUP\0"); }
            _excep_cause = CAUSE_NOTHING;
            if(Option.Autorun && *ProgMemory == 0x01 && _excep_code != RESTART_NOAUTORUN) {

            	//Fix from picomite for lockup withe execute command and OPTION AUTORUN ON
            	//if(Option.ProgFlashSize != PROG_FLASH_SIZEMAX) ExecuteProgram(ProgMemory + Option.ProgFlashSize);       // run anything that might be in the library
               // ExecuteProgram(ProgMemory);     	// then run the program if autorun is on
            	// memset(tknbuf,0,STRINGSIZE);
            	 *tknbuf=GetCommandValue((char *)"RUN");
            	 goto autorun;

            }
  }
//  SerUSBPutS("\033[?1000l");                         // Tera Term turn off mouse click report in vt200 mode


// Diagnostic use
  /******* BLINK LED and wait if you get here******************************/
 		 // BLINKLED
  /************************************************************************/

//USE_FULL_ASSERT
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 executerun=0;    //cmm2
//goto skip2;
  /* USER CODE END WHILE */
//    MX_USB_HOST_Process();

  /* USER CODE BEGIN 3 */
//skip2:
	if(Option.DISPLAY_CONSOLE) {
		SetFont(PromptFont);
		gui_fcolour = PromptFC;
		gui_bcolour = PromptBC;
		if(CurrentX != 0) MMPrintString("\r\n");                   // prompt should be on a new line
	}
	  MMAbort = false;
      BreakKey = BREAK_KEY;
      EchoOption = true;
      LocalIndex = 0;                                             // this should not be needed but it ensures that all space will be cleared
      ClearTempMemory();                                           // clear temp string space (might have been used by the prompt)
      CurrentLinePtr = NULL;                                      // do not use the line number in error reporting
      if(MMCharPos > 1) MMPrintString("\r\n");                    // prompt should be on a new line
        while(Option.PIN && !IgnorePIN) {
          _excep_code = PIN_RESTART;
          if(Option.PIN == 99999999)                              // 99999999 is permanent lockdown
              MMPrintString("Console locked, press enter to restart: ");
          else
              MMPrintString("Enter PIN or 0 to restart: ");
          MMgetline(0, inpbuf);
          if(Option.PIN == 99999999) SoftReset();
          if(*inpbuf != 0) {
              HAL_Delay(3000);
              i = atoi(inpbuf);
              if(i == 0) SoftReset();
              if(i == Option.PIN) {
                  IgnorePIN = true;
                  break;
              }
          }
      }
      _excep_code = 0;
      PrepareProgram(false);
      if(!ErrorInPrompt && FindSubFun("MM.PROMPT", 0) >= 0) {
          ErrorInPrompt = true;
          ExecuteProgram("MM.PROMPT\0");
          MMPromptPos=MMCharPos-1;    //Save length of prompt
      } else {
          MMPrintString("> ");                                    // print the prompt
          MMPromptPos=2;    //Save length of prompt
      }
      ErrorInPrompt = false;
      EditInputLine();          //Enter|Recall|Edit the command line. Save to command history


      if(!*inpbuf) continue;                                      // ignore an empty line
/*	 Old *RUN stuff
      char *p=inpbuf;
	  skipspace(p);
	  if(*p=='*'){ //shortform RUN command so convert to a normal version
		  memmove(&p[4],&p[0],strlen(p)+1);
		  p[0]='R';p[1]='U';p[2]='N';p[3]='$';p[4]=34;
		  char  *q;
		  //executerun=1;  //CMM2
		  if((q=strchr(p,' ')) != 0){ //command line after the filename
			  *q=','; //chop the command at the first space character
			  memmove(&q[1],&q[0],strlen(q)+1);
			  q[0]=34;
		  } else strcat(p,"\"");
		  p[3]=' ';
//		  PRet();MMPrintString(inpbuf);PRet();
	  }
*/

	  char *p=inpbuf;
	  skipspace(p);
	  executelocal(p);
	  if(*p=='*'){ //shortform RUN command so convert to a normal version
	      transform_star_command(inpbuf);
	      p = inpbuf;
	  }
	  tokenise(true);                                             // turn into executable code
autorun:
      i=0;
      if(*tknbuf==GetCommandValue((char *)"RUN"))i=1;
	  if(setjmp(run) != 0) {
		  //executerun=1;  //CMM2
		//  PrepareProgram(false);     //Fix to execute subs/functions and CSUBS with Hashing.
		//  CurrentLinePtr=0;
	  }
	  PrepareProgram(false);     //Fix to execute subs/functions and CSUBS with Hashing.
	  CurrentLinePtr=0;         // Copied from CMM2 code
      ExecuteProgram(tknbuf);                                     // execute the line straight away
      if(i){
          //cmdline=GetTempMemory(STRINGSIZE);
    	 // cmdline[0]='\0';   //Clear the cmdline so that cmd_end() does not complain.
    	  cmdline = "";      //Clear the cmdline so that cmd_end() does not complain.
       	  cmd_end();
      }else {
          memset(inpbuf,0,STRINGSIZE);
	      longjmp(mark, 1);												// jump back to the input prompt
      }

  }
  /* USER CODE END 3 */

}

void stripcomment(char *p){
    char *q=p;
    int toggle=0;
    while(*q){
        if(*q=='\'' && toggle==0){
            *q=0;
            break;
        }
        if(*q=='"')toggle^=1;
        q++;
    }
}
/*
void testlocal(char *p, char *command, void (*func)()){
    int len=strlen(command);
    if((strncasecmp(p,command,len)==0) && (strlen(p)==len || p[len]==' ' || p[len]=='\'')){
        p+=len;
        skipspace(p);
        cmdline=GetTempMemory(STRINGSIZE);
        stripcomment(p);
        strcpy(cmdline,p);
        (*func)();
        memset(inpbuf,0,STRINGSIZE);
        longjmp(mark, 1);												// jump back to the input prompt
    }

}
*/
void testlocal(char *p, char *command, void (*func)()){
    int len=strlen(command);
    if((strncasecmp(p,command,len)==0) && (strlen(p)==len || p[len]==' ' || p[len]=='\'')){
        p+=len;
        skipspace(p);
        cmdline=GetTempMemory(STRINGSIZE);
        stripcomment(p);
        strcpy(cmdline,p);
        char *q=cmdline;
        int toggle=0;
        while(*q){
            if(*q++ == '"') {
            toggle ^=1;
            }
        }
        if(toggle)cmdline[strlen(cmdline)]='"';
        (*func)();
        memset(inpbuf,0,STRINGSIZE);
        longjmp(mark, 1);                                                                                                                                                                 // jump back to the input prompt
    }
}



void executelocal(char *p){
    testlocal(p,"FILES",cmd_files);
   // testlocal(p,"UPDATE FIRMWARE",cmd_JumpToBootloader);
    testlocal(p,"NEW",cmd_new);
    testlocal(p,"AUTOSAVE",cmd_autosave);
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Supply configuration update enable 
    */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage 
  */
  if(HAL_GetREVID()==0x1003)__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  else __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
    /**Configure LSE Drive Capability 
    */
    /**
     * @brief  Macro to configure the External Low Speed oscillator (LSE) drive capability.
     * @note   As the LSE is in the Backup domain and write access is denied to
     *         this domain after reset, you have to enable write access using
     *         HAL_PWR_EnableBkUpAccess() function before to configure the LSE
     *         (to be done once after reset).
     * @note   On STM32H7 Rev.B and above devices this can't be updated while LSE is ON.
     * @param  __LSEDRIVE__: specifies the new state of the LSE drive capability.
     *          This parameter can be one of the following values:
     *            @arg RCC_LSEDRIVE_LOW: LSE oscillator low drive capability.
     *            @arg RCC_LSEDRIVE_MEDIUMLOW: LSE oscillator medium low drive capability.
     *            @arg RCC_LSEDRIVE_MEDIUMHIGH: LSE oscillator medium high drive capability.
     *            @arg RCC_LSEDRIVE_HIGH: LSE oscillator high drive capability.
     * @retval None
     */
     HAL_PWR_EnableBkUpAccess();   //Need to enable access to change LSEDRIVE
	 //__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
	 //__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);
	 //__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMLOW);
	 __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);


    /**Macro to configure the PLL clock source 
    */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
                              |RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = pll1;
  RCC_OscInitStruct.PLL.PLLP = pll2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
    SystemError=1;Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
    SystemError=1;Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_USART6
                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_RNG|RCC_PERIPHCLK_SPI5
                              |RCC_PERIPHCLK_SPI3|RCC_PERIPHCLK_SPI1
                              |RCC_PERIPHCLK_SPI2|RCC_PERIPHCLK_I2C2
                              |RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_USB|RCC_PERIPHCLK_FDCAN; //CAN added
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 100;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  //PeriphClkInitStruct.PLL2.PLL2Q = 8;  //50MHz
  PeriphClkInitStruct.PLL2.PLL2Q = 10;  //40MHz
  //PeriphClkInitStruct.PLL2.PLL2Q = 20;  //20MHz
  PeriphClkInitStruct.PLL2.PLL2R = 8;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.PLL3.PLL3M = 4;
  PeriphClkInitStruct.PLL3.PLL3N = 192;
  PeriphClkInitStruct.PLL3.PLL3P = 8;
  PeriphClkInitStruct.PLL3.PLL3Q = 8;
  PeriphClkInitStruct.PLL3.PLL3R = 4;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL3;
  PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;   //CAN Added
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_HSI;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
    SystemError=1;Error_Handler();
  }
  // enable MCO output (PA_8)
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI48, RCC_MCODIV_4);

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(SystemCoreClock/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 8, 0);

  __HAL_RCC_BKPRAM_CLK_ENABLE();
  HAL_PWREx_EnableBkUpReg();
  HAL_PWR_EnableBkUpAccess();
  HAL_PWREx_EnableUSBVoltageDetector();

}



void SystemClock_Config25M(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Supply configuration update enable
    */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  if(HAL_GetREVID()==0x1003)__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  else __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
  //__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
    /**Configure LSE Drive Capability
    */

     /**
      * @brief  Macro to configure the External Low Speed oscillator (LSE) drive capability.
      * @note   As the LSE is in the Backup domain and write access is denied to
      *         this domain after reset, you have to enable write access using
      *         HAL_PWR_EnableBkUpAccess() function before to configure the LSE
      *         (to be done once after reset).
      * @note   On STM32H7 Rev.B and above devices this can't be updated while LSE is ON.
      * @param  __LSEDRIVE__: specifies the new state of the LSE drive capability.
      *          This parameter can be one of the following values:
      *            @arg RCC_LSEDRIVE_LOW: LSE oscillator low drive capability.
      *            @arg RCC_LSEDRIVE_MEDIUMLOW: LSE oscillator medium low drive capability.
      *            @arg RCC_LSEDRIVE_MEDIUMHIGH: LSE oscillator medium high drive capability.
      *            @arg RCC_LSEDRIVE_HIGH: LSE oscillator high drive capability.
      * @retval None
      */
     HAL_PWR_EnableBkUpAccess();   //Need to enable access to change LSEDRIVE

	 //__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
	 //__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);
	 __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMLOW);
	 //__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Macro to configure the PLL clock source
    */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
                              |RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  //RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLM = 5;    //25MHz/5=5 192 /2=96  5x96 =480Mhz
 // RCC_OscInitStruct.PLL.PLLN = 96;
 // RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLN = pll1;

 // RCC_OscInitStruct.PLL.PLLP = 2;
 RCC_OscInitStruct.PLL.PLLP = pll2;

  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;

  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  //RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
    SystemError=1;Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) //Weact needs FLASh_LATENCY_4
  //if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) //Weact trying FLASh_LATENCY_5
 //if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)  //test
//  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
    SystemError=1;Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_USART6
                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_RNG|RCC_PERIPHCLK_SPI5
                              |RCC_PERIPHCLK_SPI3|RCC_PERIPHCLK_SPI1
                              |RCC_PERIPHCLK_SPI2|RCC_PERIPHCLK_I2C2
                              |RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_I2C1
							  |RCC_PERIPHCLK_USB|RCC_PERIPHCLK_FDCAN; //CAN added
  PeriphClkInitStruct.PLL2.PLL2M = 3;
  PeriphClkInitStruct.PLL2.PLL2N = 96;
  PeriphClkInitStruct.PLL2.PLL2P = 16;
  //PeriphClkInitStruct.PLL2.PLL2Q = 16;  //50MHz
  PeriphClkInitStruct.PLL2.PLL2Q = 20;  //40MHz
  //PeriphClkInitStruct.PLL2.PLL2Q = 40;  //20MHz
  PeriphClkInitStruct.PLL2.PLL2R = 8;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;  //_2
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.PLL3.PLL3M = 25;
  PeriphClkInitStruct.PLL3.PLL3N = 192;
  PeriphClkInitStruct.PLL3.PLL3P = 4;
  PeriphClkInitStruct.PLL3.PLL3Q = 4;
  PeriphClkInitStruct.PLL3.PLL3R = 2;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;  //_1
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL3;
  PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;   //CAN Added
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL3;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_PLL3;

 // PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
//  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL3;
 // PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
//  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;

  PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_HSI;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;  //PLL3
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	 SystemError=1;Error_Handler();
  }

  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI48, RCC_MCODIV_4);

  // enable MCO output (PA_8)
  // #ifdef RCC_MCO1SOURCE_LSE
  //    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);
  // #endif

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(SystemCoreClock/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 8, 0);

  __HAL_RCC_BKPRAM_CLK_ENABLE();
  HAL_PWREx_EnableBkUpReg();
  HAL_PWR_EnableBkUpAccess();
  HAL_PWREx_EnableUSBVoltageDetector();

}


/* ADC2 init function */
static void MX_ADC2_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV6;
  hadc2.Init.Resolution = ADC_RESOLUTION_16B;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_16;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* ADC3 init function */
static void MX_ADC3_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV6;
  hadc3.Init.Resolution = ADC_RESOLUTION_16B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_VBAT_DIV4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* DAC1 init function */
static void MX_DAC1_Init(void)
{

  DAC_ChannelConfTypeDef sConfig;

    /**DAC Initialization 
    */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**DAC channel OUT1 config 
    */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**DAC channel OUT2 config 
    */
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10707DBC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* I2C2 init function */
static void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00300B29;
  //hi2c2.Init.Timing = 0x10707DBC;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

    /**I2C Enable Fast Mode Plus 
    */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C2);

}

/* RNG init function */
static void MX_RNG_Init(void)
{

  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
	int up=RTC_SMOOTHCALIB_PLUSPULSES_RESET;
	int calibrate= -Option.RTC_Calibrate;

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 7;
  hrtc.Init.SynchPrediv = 4095;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
  //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
  //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
//  RtcGetTime();
  if(sDate.Year<18 || sTime.Hours>23 || sTime.Minutes>59){
  /* USER CODE END RTC_Init 2 */

    /**Initialize RTC and set the Time and Date 
    */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 3 */

  /* USER CODE END RTC_Init 3 */

  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 4 */
  }
  if(Option.RTC_Calibrate>0){
	  up=RTC_SMOOTHCALIB_PLUSPULSES_SET;
	  calibrate=512-Option.RTC_Calibrate;
  }
  HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, up, calibrate);
  /* USER CODE END RTC_Init 4 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan.Instance = FDCAN1;
  hfdcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan.Init.AutoRetransmission = DISABLE;
  hfdcan.Init.TransmitPause = DISABLE;
  hfdcan.Init.ProtocolException = DISABLE;
  hfdcan.Init.NominalPrescaler = 1;
  hfdcan.Init.NominalSyncJumpWidth = 1;
  hfdcan.Init.NominalTimeSeg1 = 2;
  hfdcan.Init.NominalTimeSeg2 = 2;
  hfdcan.Init.DataPrescaler = 1;
  hfdcan.Init.DataSyncJumpWidth = 1;
  hfdcan.Init.DataTimeSeg1 = 1;
  hfdcan.Init.DataTimeSeg2 = 1;
  hfdcan.Init.MessageRAMOffset = 0;
  hfdcan.Init.StdFiltersNbr = 0;
  hfdcan.Init.ExtFiltersNbr = 0;
  hfdcan.Init.RxFifo0ElmtsNbr = 0;
  hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan.Init.RxFifo1ElmtsNbr = 0;
  hfdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan.Init.RxBuffersNbr = 0;
  hfdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan.Init.TxEventsNbr = 0;
  hfdcan.Init.TxBuffersNbr = 0;
  hfdcan.Init.TxFifoQueueElmtsNbr = 0;
  hfdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

#ifdef OLDSTUFF
/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 1;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 2;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.RxFifo0ElmtsNbr = 1;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 1;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 1;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 1;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

#endif

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* SPI2 init function */
static void MX_SPI2_Init(void)
{

  /* SPI2 parameter configuration*/
  if(HAS_144PINS)hspi2.Instance = SPI2;
  if(HAS_100PINS)hspi2.Instance = SPI4;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* SPI3 init function */
static void MX_SPI3_Init(void)
{

  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;

  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi3.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi3.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi3.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi3.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi3.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi3.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* SPI5 init function */
static void MX_SPI5_Init(void)
{

  /* SPI5 parameter configuration*/
  if(HAS_144PINS)GenSPI.Instance = SPI5;
  if(HAS_100PINS)GenSPI.Instance = SPI2;

  GenSPI.Init.Mode = SPI_MODE_MASTER;
  GenSPI.Init.Direction = SPI_DIRECTION_2LINES;
  GenSPI.Init.DataSize = SPI_DATASIZE_8BIT;
  GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
  GenSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
  GenSPI.Init.NSS = SPI_NSS_SOFT;
  GenSPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  GenSPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
  GenSPI.Init.TIMode = SPI_TIMODE_DISABLE;
  GenSPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  GenSPI.Init.CRCPolynomial = 7;
  GenSPI.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  GenSPI.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  GenSPI.Init.FifoThreshold = SPI_FIFO_THRESHOLD_08DATA;
  GenSPI.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  GenSPI.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  GenSPI.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  GenSPI.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  GenSPI.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  GenSPI.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  GenSPI.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&GenSPI) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

//Used for LCD BACKLIGHT ?????????????????
static void MX_TIM1_Init(void)
{

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 167;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 999;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
	{
	Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
	Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
	{
	Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	{
	Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM2;
	sConfigOC.Pulse = 980;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
	Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
	{
	Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);
	if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
		/* PWM Generation Error */
		error("HAL_TIM_PWM_Start");
	}
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
}





/* TIM2 init function */

static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}


/* TIM3 init function -AUDIO Timer */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1249;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* TIM4 init function  Used for PWM 1*/
static void MX_TIM4_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 2;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 0xFFFF;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0x3000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sConfigOC.Pulse = 0x6000;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sConfigOC.Pulse = 0x9000;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sConfigOC.Pulse = 0xB000;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim4);

}

/* TIM5 init function Used for PWM 2*/
static void MX_TIM5_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 0;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim5);

}

/* TIM8 init function */
void MX_TIM8_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 0;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 0xFFFF;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 0;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }

}

/* UART5 init function */
static void MX_UART5_Init(void)
{

  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  //huart5.Init.ClockPrescaler = UART_PRESCALER_DIV2;
  if(HAS_144PINS)huart5.Init.ClockPrescaler = UART_PRESCALER_DIV2;
  if(HAS_100PINS)huart5.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart5, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
  // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart5, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
   //_Error_Handler(__FILE__, __LINE__);
   SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_EnableFifoMode(&huart5) != HAL_OK)
  {
   //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN USART5_Init 2 */

  /* USER CODE END USART5_Init 2 */

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
 // huart1.Init.ClockPrescaler = UART_PRESCALER_DIV2;
  if(HAS_144PINS)huart1.Init.ClockPrescaler = UART_PRESCALER_DIV2;
  if(HAS_100PINS)huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
   //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
  // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_EnableFifoMode(&huart1) != HAL_OK)
  {
   //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart2, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
   //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
   //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
   //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  if(HAS_144PINS)huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  if(HAS_100PINS)huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  //if(HAS_100PINS)huart3.Init.ClockPrescaler = UART_PRESCALER_DIV6;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart3, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
   //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
 }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
  // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
  // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}



static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  //huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  if(HAS_144PINS)huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  if(HAS_100PINS)huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart6, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart6, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart6, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
   // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
 }
  if (HAL_UARTEx_DisableFifoMode(&huart6) != HAL_OK)
  {
  // _Error_Handler(__FILE__, __LINE__);
	  SystemError=1;Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 6 */

  /* USER CODE END USART2_Init 6 */

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
     PA8   ------> RCC_MCO_1
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();


 /* Pullup the F-CS pin on the WeAct 100 pin board. Will also be pulled up on the DevEBox as well.*/
 /* ensures the flash doesnt respond to the SPI bus unless F_Cs is pulled low. F-CS is pin 87 PD6 */
  //if(HAS_100PINS){
	 // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, GPIO_PIN_RESET);  //F-CS
     //MMPrintString("MX_GPIO fot PD6 F-CS \r\n");
	// GPIO_InitStruct.Pin = GPIO_PIN_6;
	// GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	// GPIO_InitStruct.Pull = GPIO_PULLUP;
	// GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	// HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	// HAL_Delay(200);
//  }

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, SSD_D2_Pin|SSD_D3_Pin|SSD_D4_Pin|SSD_D5_Pin 
                          |SSD_D6_Pin|SSD_D7_Pin|SSD_D8_Pin|SSD_D9_Pin 
                          |SSD_D10_Pin|SSD_D11_Pin|SSD_D12_Pin|SSD_D13_Pin 
                          |SSD_D14_Pin|SSD_D15_Pin|SSD_D0_Pin|SSD_D1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  if(HAS_144PINS){
  HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin|RED_LED_Pin|BLUE_LED_Pin, GPIO_PIN_RESET);
  }
  /*Configure GPIO pin Output Level */
  if(HAS_144PINS){
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_1|SSD_WR_Pin|SSD_RESET_Pin
                          |SSD_RD_Pin, GPIO_PIN_RESET);
  }
  if(HAS_100PINS){
   HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET); //WR and RD
   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);  //RST and  RS
   }
  /*Configure GPIO pins : SSD_D2_Pin SSD_D3_Pin SSD_D4_Pin SSD_D5_Pin 
                           SSD_D6_Pin SSD_D7_Pin SSD_D8_Pin SSD_D9_Pin 
                           SSD_D10_Pin SSD_D11_Pin SSD_D12_Pin SSD_D13_Pin 
                           SSD_D14_Pin SSD_D15_Pin SSD_D0_Pin SSD_D1_Pin */
  GPIO_InitStruct.Pin = SSD_D2_Pin|SSD_D3_Pin|SSD_D4_Pin|SSD_D5_Pin 
                          |SSD_D6_Pin|SSD_D7_Pin|SSD_D8_Pin|SSD_D9_Pin 
                          |SSD_D10_Pin|SSD_D11_Pin|SSD_D12_Pin|SSD_D13_Pin 
                          |SSD_D14_Pin|SSD_D15_Pin|SSD_D0_Pin|SSD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : SWITCH_Pin PC13  Pin 7 on  both 100 pins and 144 pins*/
  GPIO_InitStruct.Pin = SWITCH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
 // GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; //CPUSLEEP
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SWITCH_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WakeUp_Pin */
  /*  Pin 55 aternate wakeup
   GPIO_InitStruct.Pin = WakeUp_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
   GPIO_InitStruct.Pull = GPIO_PULLDOWN;
   HAL_GPIO_Init( WakeUp_GPIO_Port, &GPIO_InitStruct);
 */

  /*Configure GPIO pins : COUNT1_Pin COUNT2_Pin COUNT3_Pin COUNT4_Pin 
                           IR_Pin */
  if(HAS_144PINS){
  GPIO_InitStruct.Pin = COUNT1_Pin|COUNT2_Pin|COUNT3_Pin|COUNT4_Pin 
                          |IR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  }

  if(HAS_100PINS){
	     GPIO_InitStruct.Pin = COUNT1_Pin|COUNT2_Pin|COUNT4_Pin;
	  	 GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	     GPIO_InitStruct.Pull = GPIO_NOPULL;
	     HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	     GPIO_InitStruct.Pin = COUNT3_Pin |IR_Pin;
	     GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	     GPIO_InitStruct.Pull = GPIO_NOPULL;
	     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }

  /*Configure GPIO pins : GREEN_LED_Pin RED_LED_Pin BLUE_LED_Pin */
  if(HAS_144PINS){
  GPIO_InitStruct.Pin = GREEN_LED_Pin|RED_LED_Pin|BLUE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
  /*Configure GPIO pins : PG1 USB_POWERON_Pin SSD_WR_Pin SSD_RESET_Pin 
                           SSD_RD_Pin */
  if(HAS_144PINS){
  GPIO_InitStruct.Pin = GPIO_PIN_1|SSD_WR_Pin|SSD_RESET_Pin
                          |SSD_RD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  }
  /*Configure GPIO pin : PA8  OV7670_XCLK*/
  if(HAS_144PINS){
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
  /* EXTI interrupt init */
//  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 3, 0);
//  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 4 */
// send a character to the Console serial port
static void MX_TIM16_Init(int prescale)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = prescale;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 49999;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}
void SerialConsolePutC(int c) {
#ifdef CMDHISTORY
	if(c == '\b') {
		if (MMCharPos==1){

		}else{
			MMCharPos -= 1;
		}
	}
#endif
    int empty=(huart3.Instance->ICR & USART_ICR_TCCF) | !(huart3.Instance->CR1 & USART_CR1_TCIE) ;
	while(ConsoleTxBufTail == ((ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE)); //wait if buffer full
	ConsoleTxBuf[ConsoleTxBufHead] = c;							// add the char
	ConsoleTxBufHead = (ConsoleTxBufHead + 1) % CONSOLE_TX_BUF_SIZE;		   // advance the head of the queue
	if(empty){
        huart3.Instance->CR1 |= USART_CR1_TCIE;
	}
}
void putConsole(int c) {

    if(OptionConsole & 1)SerialConsolePutC(c);
    if(OptionConsole & 2){DisplayPutC(c);if(Option.Refresh)Display_Refresh();}
    // SerialConsolePutC(c);
    //DisplayPutC(c);if(Option.Refresh)Display_Refresh();
}
// get a char from the UART1 serial port (the console)
// will return immediately with -1 if there is no character waiting
int getConsole(void) {
    int c=-1;
    CheckAbort();
    huart3.Instance->CR1 &= ~USART_CR1_RXNEIE;
	if(ConsoleRxBufHead != ConsoleRxBufTail) {                            // if the queue has something in it
	    c = ConsoleRxBuf[ConsoleRxBufTail];
	    ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;   // advance the head of the queue
	}
    huart3.Instance->CR1 |= USART_CR1_RXNEIE;
    return c;
}
int kbhitConsole(void) {
    int i;
    i = ConsoleRxBufHead - ConsoleRxBufTail;
    if(i < 0) i += CONSOLE_RX_BUF_SIZE;
    return i;
}
// print a char on the Serial and USB consoles only (used in the EDIT command and dp() macro)
void SerUSBPutC(char c) {
    SerialConsolePutC(c);
}
// print a string on the Serial and USB consoles only (used in the EDIT command and dp() macro)
void SerUSBPutS(char *s) {
    while(*s) SerUSBPutC(*s++);
}
void initConsole(void) {
    ConsoleRxBufHead = ConsoleRxBufTail = 0;
}

void SoftReset(void){
	NVIC_SystemReset();
}

void CheckAbort(void) {
	CheckKeyboard();
    if(MMAbort) {
        WDTimer = 0;                                                // turn off the watchdog timer
    	ScrewUpTimer=0;
        memset(inpbuf,0,STRINGSIZE);
        longjmp(mark, 1);                                           // jump back to the input prompt
    }
}
/*****************************************************************************************
The vt100 escape code sequences
===============================
3 char codes            Arrow Up    esc [ A
                        Arrow Down  esc [ B
                        Arrow Right esc [ C
                        Arrow Left  esc [ D

4 char codes            Home        esc [ 1 ~
                        Insert      esc [ 2 ~
                        Del         esc [ 3 ~
                        End         esc [ 4 ~
                        Page Up     esc [ 5 ~
                        Page Down   esc [ 6 ~

5 char codes            F1          esc [ 1 1 ~
                        F2          esc [ 1 2 ~
                        F3          esc [ 1 3 ~
                        F4          esc [ 1 4 ~
                        F5          esc [ 1 5 ~         note the
                        F6          esc [ 1 7 ~         disconnect
                        F7          esc [ 1 8 ~
                        F8          esc [ 1 9 ~
                        F9          esc [ 2 0 ~
                        F10         esc [ 2 1 ~         note the
                        F11         esc [ 2 3 ~         disconnect
                        F12         esc [ 2 4 ~

                        SHIFT-F3    esc [ 2 5 ~         used in the editor
                        SHIFT-F4    esc [ 2 6 ~
                        SHIFT-F5    esc [ 2 8 ~
                        SHIFT-F6    esc [ 2 9 ~
                        SHIFT-F7    esc [ 3 1 ~
                        SHIFT-F8    esc [ 3 2 ~

*****************************************************************************************/

// check if there is a keystroke waiting in the buffer and, if so, return with the char
// returns -1 if no char waiting
// the main work is to check for vt100 escape code sequences and map to Maximite codes
// SHIFT F4-F12 Added as per piciomite 6.00.02B0
//NB: SHIFT F1, F2, F11, and F12 don't appear to generate anything
int MMInkey(void) {
    unsigned int c = -1;                                            // default no character
    unsigned int tc = -1;                                           // default no character
    unsigned int ttc = -1;                                          // default no character
    static unsigned int c1 = -1;
    static unsigned int c2 = -1;
    static unsigned int c3 = -1;
    static unsigned int c4 = -1;

    if(c1 != -1) {                                                  // check if there are discarded chars from a previous sequence
        c = c1; c1 = c2; c2 = c3; c3 = c4; c4 = -1;                 // shuffle the queue down
        return c;                                                   // and return the head of the queue
    }

    c = getConsole();                                               // do discarded chars so get the char
    if(c == 0x1b) {
        InkeyTimer = 0;                                             // start the timer
        while((c = getConsole()) == -1 && InkeyTimer < 30);         // get the second char with a delay of 30mS to allow the next char to arrive
        if(c == 'O'){   //support for many linux terminal emulators
            while((c = getConsole()) == -1 && InkeyTimer < 50);        // delay some more to allow the final chars to arrive, even at 1200 baud
            if(c == 'P') return F1;
            if(c == 'Q') return F2;
            if(c == 'R') return F3;
            if(c == 'S') return F4;
            if(c == '2'){
                while((tc = getConsole()) == -1 && InkeyTimer < 70);        // delay some more to allow the final chars to arrive, even at 1200 baud
                if(tc == 'R') return F3 + 0x20;
                c1 = 'O'; c2 = c; c3 = tc; return 0x1b;                 // not a valid 4 char code
            }
            c1 = 'O'; c2 = c; return 0x1b;                 // not a valid 4 char code
        }
        if(c != '[') { c1 = c; return 0x1b; }                       // must be a square bracket
        while((c = getConsole()) == -1 && InkeyTimer < 50);         // get the third char with delay
        if(c == 'A') return UP;                                     // the arrow keys are three chars
        if(c == 'B') return DOWN;
        if(c == 'C') return RIGHT;
        if(c == 'D') return LEFT;
        if(c < '1' && c > '6') { c1 = '['; c2 = c; return 0x1b; }   // the 3rd char must be in this range
        while((tc = getConsole()) == -1 && InkeyTimer < 70);        // delay some more to allow the final chars to arrive, even at 1200 baud
        if(tc == '~') {                                             // all 4 char codes must be terminated with ~
            if(c == '1') return HOME;
            if(c == '2') return INSERT;
            if(c == '3') return DEL;
            if(c == '4') return END;
            if(c == '5') return PUP;
            if(c == '6') return PDOWN;
            c1 = '['; c2 = c; c3 = tc; return 0x1b;                 // not a valid 4 char code
        }
        while((ttc = getConsole()) == -1 && InkeyTimer < 90);       // get the 5th char with delay
        if(ttc == '~') {                                            // must be a ~
            if(c == '1') {
                if(tc >='1' && tc <= '5') return F1 + (tc - '1');   // F1 to F5
                if(tc >='7' && tc <= '9') return F6 + (tc - '7');   // F6 to F8
            }
            if(c == '2') {
                if(tc =='0' || tc == '1') return F9 + (tc - '0');   // F9 and F10
                if(tc =='3' || tc == '4') return F11 + (tc - '3');  // F11 and F12
               // if(tc =='5') return F3 + 0x20;                      // SHIFT-F3
               if(tc =='5' || tc=='6') return F3 + 0x20 + tc-'5';   // SHIFT-F3 and F4
               if(tc =='8' || tc=='9') return F5 + 0x20 + tc-'8';   // SHIFT-F5 and F6
            }
            if(c == '3') {
                if(tc >='1' && tc <= '4') return F7 + 0x20 + (tc - '1');   // SHIFT-F7 to F10
            }
            //NB: SHIFT F1, F2,F9,F10, F11 and F12 don't appear to generate anything
        }
        // nothing worked so bomb out
        c1 = '['; c2 = c; c3 = tc; c4 = ttc;
        return 0x1b;
    }
    return c;
}

#define SetupTime 22
//void uSec(unsigned int us) {
//  us=us*ticks_per_microsecond-SetupTime;
//  WriteCoreTimer(0);
//  while (ReadCoreTimer()<us);
//}
#define tim5_tick() (int16_t)(TIM16->CNT)
void uSec(unsigned int ucnt)
{
	uint16_t start = TIM16->CNT;
//	ucnt=ucnt*ticks_per_microsecond-SetupTime;
	while((uint16_t)(tim5_tick() - start) < ucnt){}

}
void shortpause(unsigned int ticks){
	  WriteCoreTimer(0);
	  while (ReadCoreTimer()<ticks);
}



// takes a pointer to RAM containing a program (in clear text) and writes it to program flash in tokenised format
void SaveProgramToFlash(char *pm, int msg) {
    char *p, endtoken, fontnbr, prevchar = 0, buf[STRINGSIZE];
    int nbr, i, n, SaveSizeAddr;
    uint32_t storedupdates[MAXCFUNCTION], updatecount=0, realflashsave;
	memset(&last_k_info,0,sizeof(HID_KEYBD_Info_TypeDef));
	memset(k_pinfo,0,sizeof(HID_KEYBD_Info_TypeDef));

    memcpy(buf, tknbuf, STRINGSIZE);                                // save the token buffer because we are going to use it
    //FlashWriteInit(PROGRAM_FLASH,1);                                   // initialise for flash write but do not erase any pages
    i=3;
    while(FlashWriteInit(PROGRAM_FLASH) && i)i--;                     // erase flash
    if(i==0)error("Failed to erase flash memory");

    nbr = 0;                                                        // this is used to count the number of bytes written to flash
    while(*pm) {
        p = inpbuf;
        while(!(*pm == 0 || *pm == '\r' || (*pm == '\n' && prevchar != '\r'))) {
            if(*pm == TAB) {
                do {*p++ = ' ';
                    if((p - inpbuf) >= MAXSTRLEN) goto exiterror1;
                } while((p - inpbuf) % 2);
            } else {
                if(IsPrint(*pm)) {
                    *p++ = *pm;
                    if((p - inpbuf) >= MAXSTRLEN) goto exiterror1;
                }
            }
            prevchar = *pm++;
        }
        if(*pm) prevchar = *pm++;                                   // step over the end of line char but not the terminating zero
        *p = 0;                                                     // terminate the string in inpbuf

        if(*inpbuf == 0 && (*pm == 0 || (!IsPrint(*pm) && pm[1] == 0))) break; // don't save a trailing newline

        tokenise(false);                                            // turn into executable code
        p = tknbuf;
        while(!(p[0] == 0 && p[1] == 0)) {
            FlashWriteByte(*p++); nbr++;

            if((realflashpointer - FLASH_PROGRAM_ADDR) >= PROG_FLASH_SIZE - 5)
                goto exiterror1;
        }
        FlashWriteByte(0); nbr++;                              // terminate that line in flash
    }
    FlashWriteByte(0);
    FlashWriteAlign();                                            // this will flush the buffer and step the flash write pointer to the next word boundary
    // now we must scan the program looking for CFUNCTION/CSUB/DEFINEFONT statements, extract their data and program it into the flash used by  CFUNCTIONs
     // programs are terminated with two zero bytes and one or more bytes of 0xff.  The CFunction area starts immediately after that.
     // the format of a CFunction/CSub/Font in flash is:
     //   Unsigned Int - Address of the CFunction/CSub in program memory (points to the token representing the "CFunction" keyword) or NULL if it is a font
     //   Unsigned Int - The length of the CFunction/CSub/Font in bytes including the Offset (see below)
     //   Unsigned Int - The Offset (in words) to the main() function (ie, the entry point to the CFunction/CSub).  Omitted in a font.
     //   word1..wordN - The CFunction/CSub/Font code
     // The next CFunction/CSub/Font starts immediately following the last word of the previous CFunction/CSub/Font
    int firsthex=1;
    realflashsave= realflashpointer;
    p =(char *) ProgMemory;                                              // start scanning program memory
    while(*p != 0xff) {
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) break;                                          // end of the program
        if(*p == T_NEWLINE) {
            CurrentLinePtr = p;
            p++;                                                    // skip the newline token
        }
        if(*p == T_LINENBR) p += 3;                                 // step over the line number

        skipspace(p);
        if(*p == T_LABEL) {
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        if(*p == cmdCSUB || *p == cmdCFUN || *p == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
            if(*p == GetCommandValue("DefineFont")) {
             endtoken = GetCommandValue("End DefineFont");
             p++;                                                // step over the token
             skipspace(p);
             if(*p == '#') p++;
             fontnbr = getint(p, 1, FONT_TABLE_SIZE);
                                                 // font 6 has some special characters, some of which depend on font 1
             if(fontnbr == 1 || fontnbr == 6 || fontnbr == 7) error("Cannot redefine fonts 1, 6 or 7");
             realflashpointer+=4;
             skipelement(p);                                     // go to the end of the command
             p--;
//            } else if(*p == GetCommandValue("DefineData")) {
//                endtoken = GetCommandValue("End DefineData");
//                fontnbr = 0;
            } else {
            	if(*p == cmdCFUN)
            	   endtoken = GetCommandValue("End CFunction");
            	else
            	   endtoken = GetCommandValue("End CSub");

                realflashpointer+=4;
                fontnbr = 0;
                firsthex=0;
            }
             SaveSizeAddr = realflashpointer;                                // save where we are so that we can write the CFun size in here
             realflashpointer+=4;
             p++;
             skipspace(p);
             if(!fontnbr) {
                 if(!IsNamestart(*p))  error("Function name");
                 do { p++; } while(isnamechar(*p));
                 skipspace(p);
                 if(!(IsxDigit(p[0]) && IsxDigit(p[1]) && IsxDigit(p[2]))) {
                     skipelement(p);
                     p++;
                    if(*p == T_NEWLINE) {
                        CurrentLinePtr = p;
                        p++;                                        // skip the newline token
                    }
                    if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 }
             }
             do {
                 while(*p && *p != '\'') {
                     skipspace(p);
                     n = 0;
                     for(i = 0; i < 8; i++) {
                         if(!IsxDigit(*p)) error("Invalid hex word");
                         if((int)((char *)realflashpointer - ProgMemory) >= Option.ProgFlashSize - 5) error("Not enough memory");
                         n = n << 4;
                         if(*p <= '9')
                             n |= (*p - '0');
                         else
                             n |= (toupper(*p) - 'A' + 10);
                         p++;
                     }
                     realflashpointer+=4;
                     skipspace(p);
                     if(firsthex){
                    	 firsthex=0;
                    	 if(((n>>16) & 0xff) < 0x20)error("Can't define non-printing characters");
                     }
                 }
                 // we are at the end of a embedded code line
                 while(*p) p++;                                      // make sure that we move to the end of the line
                 p++;                                                // step to the start of the next line
                 if(*p == 0) error("Missing END declaration");
                 if(*p == T_NEWLINE) {
                     CurrentLinePtr = p;
                     p++;                                            // skip the newline token
                 }
                 if(*p == T_LINENBR) p += 3;                         // skip over the line number
                 skipspace(p);
             } while(*p != endtoken);
             storedupdates[updatecount++]=realflashpointer - SaveSizeAddr - 4;
         }
         while(*p) p++;                                              // look for the zero marking the start of the next element
     }
    realflashpointer = realflashsave ;
    updatecount=0;
    p = (char *)ProgMemory;                                              // start scanning program memory
     while(*p != 0xff) {
         if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
         if(*p == 0) break;                                          // end of the program
         if(*p == T_NEWLINE) {
             CurrentLinePtr = p;
             p++;                                                    // skip the newline token
         }
         if(*p == T_LINENBR) p += 3;                                 // step over the line number

         skipspace(p);
         if(*p == T_LABEL) {
             p += p[1] + 2;                                          // skip over the label
             skipspace(p);                                           // and any following spaces
         }
         if(*p == cmdCSUB || *p == cmdCFUN || *p == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
         if(*p == GetCommandValue("DefineFont")) {      // found a CFUNCTION, CSUB or DEFINEFONT token
             endtoken = GetCommandValue("End DefineFont");
             p++;                                                // step over the token
             skipspace(p);
             if(*p == '#') p++;
             fontnbr = getint(p, 1, FONT_TABLE_SIZE);
                                                 // font 6 has some special characters, some of which depend on font 1
             if(fontnbr == 1 || fontnbr == 6 || fontnbr == 7) error("Cannot redefine fonts 1, 6, or 7");

             FlashWriteWord(fontnbr - 1);             // a low number (< FONT_TABLE_SIZE) marks the entry as a font
             skipelement(p);                                     // go to the end of the command
             p--;
         } else {
        	 if(*p == cmdCFUN)
        	    endtoken = GetCommandValue("End CFunction");
        	 else
        	    endtoken = GetCommandValue("End CSub");

             FlashWriteWord((unsigned int)p);               // if a CFunction/CSub save a pointer to the declaration
             fontnbr = 0;
         }
             SaveSizeAddr = realflashpointer;                                // save where we are so that we can write the CFun size in here
             FlashWriteWord(storedupdates[updatecount++]);                        // leave this blank so that we can later do the write
             p++;
             skipspace(p);
             if(!fontnbr) {
                 if(!IsNamestart(*p))  error("Function name");
                 do { p++; } while(isnamechar(*p));
                 skipspace(p);
                 if(!(IsxDigit(p[0]) && IsxDigit(p[1]) && IsxDigit(p[2]))) {
                     skipelement(p);
                     p++;
                    if(*p == T_NEWLINE) {
                        CurrentLinePtr = p;
                        p++;                                        // skip the newline token
                    }
                    if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 }
             }
             do {
                 while(*p && *p != '\'') {
                     skipspace(p);
                     n = 0;
                     for(i = 0; i < 8; i++) {
                         if(!IsxDigit(*p)) error("Invalid hex word");
                         if((int)((char *)realflashpointer - ProgMemory) >= Option.ProgFlashSize - 5) error("Not enough memory");
                         n = n << 4;
                         if(*p <= '9')
                             n |= (*p - '0');
                         else
                             n |= (toupper(*p) - 'A' + 10);
                         p++;
                     }

                     FlashWriteWord(n);
                     skipspace(p);
                 }
                 // we are at the end of a embedded code line
                 while(*p) p++;                                      // make sure that we move to the end of the line
                 p++;                                                // step to the start of the next line
                 if(*p == 0) error("Missing END declaration");
                 if(*p == T_NEWLINE) {
                    CurrentLinePtr = p;
                    p++;                                        // skip the newline token
                 }
                 if(*p == T_LINENBR) p += 3;                     // skip over a line number
                 skipspace(p);
             } while(*p != endtoken);
         }
         while(*p) p++;                                              // look for the zero marking the start of the next element
     }
     FlashWriteWord(0xffffffff);                                // make sure that the end of the CFunctions is terminated with an erased word
     FlashWriteClose();                                              // this will flush the buffer and step the flash write pointer to the next word boundary

    if(msg) {                                                       // if requested by the caller, print an informative message
        if(MMCharPos > 1) MMPrintString("\r\n");                    // message should be on a new line
        MMPrintString("Saved ");
        IntToStr(tknbuf, nbr + 3, 10);
        MMPrintString(tknbuf);
        MMPrintString(" bytes\r\n");
    }
    memcpy(tknbuf, buf, STRINGSIZE);                                // restore the token buffer in case there are other commands in it
    return;

    // we only get here in an error situation while writing the program to flash
    exiterror1:
        FlashWriteByte(0); FlashWriteByte(0); FlashWriteByte(0);    // terminate the program in flash
        FlashWriteClose();
        error("Not enough memory");
}

// get a keystroke from the console.  Will wait forever for input
// if the char is a cr then replace it with a newline (lf)
int MMgetchar(void) {
    int c;
    static char prevchar = 0;

    loopback:
    do {
    	ShowCursor(true);
    	//uSec(50);    //This is in the F4
        CheckSDCard();
        processgps();  //This is in the F4
        c = MMInkey();
    } while(c == -1);
    if(c == '\n' && prevchar == '\r') {
        prevchar = 0;
        goto loopback;
    }
    ShowCursor(false);
    prevchar = c;
    if(c == '\n') c = '\r';
    return c;
}
// put a character out to the serial console
char MMputchar(char c) {
    putConsole(c);
    if(IsPrint(c)) MMCharPos++;
    if(c == '\r') {
        MMCharPos = 1;
    }
    return c;
}


// insert a string into the start of the lastcmd buffer.
// the buffer is a sequence of strings separated by a zero byte.
// using the up arrow usere can call up the last few commands executed.
void InsertLastcmd( char *s) {
int i, slen;
    if(strcmp(lastcmd, s) == 0) return;                             // don't duplicate
    slen = strlen(s);
    if(slen < 1 || slen > CMD_BUFFER_SIZE - 1) return;
    slen++;
    for(i = CMD_BUFFER_SIZE - 1; i >=  slen ; i--)
        lastcmd[i] = lastcmd[i - slen];                             // shift the contents of the buffer up
    strcpy(lastcmd, s);                                             // and insert the new string in the beginning
    for(i = CMD_BUFFER_SIZE - 1; lastcmd[i]; i--) lastcmd[i] = 0;             // zero the end of the buffer
}

// Debug display onto LCDPANEL
void show(int i,int pos,char *s){

	 char tt[20];
	 int x,y;
	 x=CurrentX;y=CurrentY;
	  IntToStr(tt, i, 10);
      strcat(tt,"      ");
      GUIPrintString(50*pos, 0, 0x11, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, WHITE, BLACK, s);
	  GUIPrintString(50*pos, 20, 0x11, JUSTIFY_LEFT, JUSTIFY_TOP, ORIENT_NORMAL, WHITE, BLACK, tt);
   CurrentX=x;CurrentY=y;
}

void EditInputLine(void) {
    char *p = NULL;
    char buf[MAXKEYLEN + 3];
    char goend[10];
   // char linelen[10];
    int lastcmd_idx, lastcmd_edit;
    int insert, /*startline,*/ maxchars;
    int CharIndex, BufEdited;
    int c, i, j;
    int l4,l3,l2;
    //maxchars = 2*Option.Width-1;
    maxchars=255; //i.e. 3 lines on Vt100 width 80 -2 characters for prompt.
    if(Option.DISPLAY_CONSOLE && Option.Width<=SCREENWIDTH){     //We will always assume the Vt100 is 80 colums if LCD is the console <=80.
       l2=SCREENWIDTH+1-MMPromptPos;
       l3=2*SCREENWIDTH+2-MMPromptPos;
       l4=3*SCREENWIDTH+3-MMPromptPos;
     }else{                         // otherwise assume the VT100 matches Option.Width
       l2=Option.Width +1-MMPromptPos;
       l3=2*Option.Width+2-MMPromptPos;
       l4=3*Option.Width+3-MMPromptPos;
     }

     // Build "\e[80C" equivalent string for the line length
     strcpy(goend,"\e[");IntToStr(&goend[strlen(goend)],l2+MMPromptPos, 10);strcat(goend, "C");


    MMPrintString(inpbuf);                                                              // display the contents of the input buffer (if any)
    CharIndex = strlen(inpbuf);                                                         // get the current cursor position in the line
    insert = false;

    lastcmd_edit = lastcmd_idx = 0;
    BufEdited = false; //(CharIndex != 0);

    while(1) {
    	//do {
    		  //if(BufEdited)ShowCursor(true);
    		  //ShowCursor(true);
    		  c=MMgetchar(); //need this as it checks SDCARD and GPS
    		  // c = MMInkey();
    	//	} while(c == -1);
    		//ShowCursor(false);

        if(c == TAB) {
            strcpy(buf, "        ");
            switch (Option.Tab) {
              case 2:
                buf[2 - (CharIndex % 2)] = 0; break;
              case 3:
                buf[3 - (CharIndex % 3)] = 0; break;
              case 4:
                buf[4 - (CharIndex % 4)] = 0; break;
              case 8:
                buf[8 - (CharIndex % 8)] = 0; break;
            }
        } else {
            buf[0] = c;
            buf[1] = 0;
        }
        do {
            switch(buf[0]) {
                case '\r':
                case '\n':  //if(autoOn && atoi(inpbuf) > 0) autoNext = atoi(inpbuf) + autoIncr;
                            //if(autoOn && !BufEdited) *inpbuf = 0;
                            goto saveline;
                            break;

                case '\b':
                	           if(CharIndex > 0) {
                	                 BufEdited = true;

                                     i = CharIndex - 1;
                                     j= CharIndex;
                                     for(p = inpbuf + i; *p; p++) *p = *(p + 1);                       // remove the char from inpbuf

                                 // Lets put the cursor at the beginning of where the command is displayed.

                                // backspace to the beginning of line
                                //if(j > 0) {
                                while(j)  {
                                  // MMputchar('\b');
                                  if (j==l4 || j==l3 ||j==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                                  j--;
                                }
                                fflush(stdout);
                                // MMCharPos=3;CurrentX=2*gui_font_width;
                                MX470Display(CLEAR_TO_EOS);SerUSBPutS("\033[0J");        //Clear to End Of Screen

                                 j=0;
                                 while(j < strlen(inpbuf)) {
                                       //BufEdited = true;
                                      // MMPrintString(inpbuf);
                                       MMputchar(inpbuf[j]);
                                       if((j==l4-1 || j==l3-1 || j==l2-1 ) && j == strlen(inpbuf)-1 ){SerUSBPutS(" ");SerUSBPutS("\b");}
                                       if((j==l4-1 || j==l3-1 || j==l2-1 ) && j < strlen(inpbuf)-1 ){SerUSBPutC(inpbuf[j+1]);SerUSBPutS("\b");}

                                      j++;
                                 }
                                 fflush(stdout);

                                 // return the cursor to the right position
                                 for(j = strlen(inpbuf); j > i; j--){
                                  //  MMputchar('\b');
                                    if (j==l4 || j==l3 || j==l2) {DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{MMputchar('\b');}
                                 }
                                 CharIndex--;

                                 fflush(stdout);
                                 if(strlen(inpbuf)==0)BufEdited = false;
                            }
                            break;


                /****************************************** <--- arrow *****************************************************/
                case CTRLKEY('S'):
                case LEFT:

                	    BufEdited = true;
                	    insert=false; //left at first char will turn OVR on
                	    if(CharIndex > 0) {
                                if(CharIndex == strlen(inpbuf)) {
                                    //insert = true;
                                }
                                //MMputchar('\b');
                                if (CharIndex==l4 || CharIndex==l3 || CharIndex==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{MMputchar('\b');}
                                //if(CharIndex==l2){SerUSBPutS("\e[1A");SerUSBPutS("\e[80C");}
                                insert=true; //Any left turns on INS
                                CharIndex--;
                         }
                     break;

                /****************************************** --->  arrow *****************************************************/
                case CTRLKEY('D'):
                case RIGHT:

                	  if(CharIndex < strlen(inpbuf)) {
                	   	BufEdited = true;
                	    MMputchar(inpbuf[CharIndex]);
                	    if((CharIndex==l4-1 || CharIndex==l3-1|| CharIndex==l2-1 ) && CharIndex == strlen(inpbuf)-1 ){SerUSBPutS(" ");SerUSBPutS("\b");}
                	    if((CharIndex==l4-1 || CharIndex==l3-1|| CharIndex==l2-1 ) && CharIndex < strlen(inpbuf)-1 ){SerUSBPutC(inpbuf[CharIndex+1]);SerUSBPutS("\b");}
                    	//if(CharIndex==l2-1 && CharIndex < strlen(inpbuf)-1){SerUSBPutC(inpbuf[CharIndex+1]);SerUSBPutS("\b");}
                        CharIndex++;
                      }

                      //insert=false; //right always switches to OVER
                     break;

                /*********************************************DEL ********************************************************/
                case CTRLKEY(']'):
                case DEL:

                	      if(CharIndex < strlen(inpbuf)) {
                	           BufEdited = true;
                	           i = CharIndex;

                	           for(p = inpbuf + i; *p; p++) *p = *(p + 1);                 // remove the char from inpbuf
                	           j = strlen(inpbuf);
                	           // Lets put the cursor at the beginning of where the command is displayed.

                                // backspace to the beginning of line
                                //if(j > 0) {
                	            j=CharIndex;
                                while(j)  {
                                  // MMputchar('\b');
                                  if (j==l4 || j==l3 ||j==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                                  j--;
                                }
                                fflush(stdout);
                                // MMCharPos=3;CurrentX=2*gui_font_width;
                                MX470Display(CLEAR_TO_EOS);SerUSBPutS("\033[0J");        //Clear to End Of Screen

                               j=0;
                               while(j < strlen(inpbuf)) {
                                    MMputchar(inpbuf[j]);
                                    if((j==l4-1 || j==l3-1 || j==l2-1 ) && j == strlen(inpbuf)-1 ){SerUSBPutS(" ");SerUSBPutS("\b");}
                                    if((j==l4-1 || j==l3-1 || j==l2-1 ) && j < strlen(inpbuf)-1 ){SerUSBPutC(inpbuf[j+1]);SerUSBPutS("\b");}
                                    j++;
                               }
                               fflush(stdout);
                               // return the cursor to the right position
                               for(j = strlen(inpbuf); j > i; j--){
                                     // MMputchar('\b');
                                      if (j==l4 || j==l3 || j==l2) {DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                               }
                               fflush(stdout);
                           }
                	       break;

                /*********************************************INS ********************************************************/
                case CTRLKEY('N'):
                case INSERT:insert = !insert;

                            break;

               /*********************************************HOME ********************************************************/
                case CTRLKEY('U'):
                case HOME:
                	         BufEdited = true;
                	         if(CharIndex > 0) {
                                if(CharIndex == strlen(inpbuf)) {
                                    insert = true;
                                }
                                // backspace to the beginning of line
                                while(CharIndex)  {
                                	// MMputchar('\b');

                                	 if (CharIndex==l4 || CharIndex==l3 || CharIndex==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{MMputchar('\b');}
                                   	 CharIndex--;
                                }
                                fflush(stdout);
                            }else{ //HOME @ home turns off edit mode
                            	BufEdited = false;
                            }
                            break;
              /*********************************************END ********************************************************/
                case CTRLKEY('K'):
                case END:
                	        BufEdited = true;
                	        while(CharIndex < strlen(inpbuf))
                                MMputchar(inpbuf[CharIndex++]);
                            fflush(stdout);
                            break;

/*
            if(c == F2)  tp = "RUN";
            if(c == F3)  tp = "LIST";
            if(c == F4)  tp = "EDIT";
            if(c == F10) tp = "AUTOSAVE";
            if(c == F11) tp = "XMODEM RECEIVE";
            if(c == F12) tp = "XMODEM SEND";
            if(c == F5) tp = Option.F5key;
            if(c == F6) tp = Option.F6key;
            if(c == F7) tp = Option.F7key;
            if(c == F8) tp = Option.F8key;
            if(c == F9) tp = Option.F9key;
*/
             /****************************************Function Keys ***************************************************/
                case 0x91:      //F1
                	if(*Option.F1key)strcpy(&buf[1],(char *)Option.F1key);
                    break;
                case 0x92:      //F2
                    strcpy(&buf[1],"RUN\r\n");
                    break;
                case 0x93:       //F3
                    strcpy(&buf[1],"LIST\r\n");
                    break;
                case 0x94:       //F4
                    strcpy(&buf[1],"EDIT\r\n");
                    break;
               case 0x95:
            	   if(*Option.F5key){
            		   strcpy(&buf[1],(char *)Option.F5key);
            		   break;
            	   }else{
            	    /*** F5 will clear LCDPANEL and the VT100  ***/
            	      SerUSBPutS("\e[2J\e[H");
            	      fflush(stdout);
            	      if(Option.DISPLAY_CONSOLE){MX470Display(DISPLAY_CLS);CurrentX=0;CurrentY=0;}
            	      MMPrintString("> ");
            	      fflush(stdout);
                      break;
            	   }
               case 0x96:
                   if(*Option.F6key)strcpy(&buf[1],(char *)Option.F6key);
                   break;
               case 0x97:
                   if(*Option.F7key)strcpy(&buf[1],(char *)Option.F7key);
                   break;
               case 0x98:
                   if(*Option.F8key)strcpy(&buf[1],(char *)Option.F8key);
                   break;
               case 0x99:
                   if(*Option.F9key)strcpy(&buf[1],(char *)Option.F9key);
                   break;
                case 0x9a:   //F10
                    strcpy(&buf[1],"AUTOSAVE\r\n");
                    break;
                case 0x9b:   //F11
                    strcpy(&buf[1],"XMODEM RECEIVE\r\n");
                    break;
                 case 0x9c:  //F12
                    strcpy(&buf[1],"XMODEM SEND\r\n");
                    break;

                                     /*  ^          */
                                     /* /|\         */
                case CTRLKEY('E'):   /*  | Up arrow */
                case UP:    if(!(BufEdited /*|| autoOn || CurrentLineNbr */)) {

                	            if(lastcmd_edit) {
                                    i = lastcmd_idx + strlen(&lastcmd[lastcmd_idx]) + 1;    // find the next command
                                    if(lastcmd[i] != 0 && i < CMD_BUFFER_SIZE - 1) lastcmd_idx = i;  // and point to it for the next time around
                                } else
                                    lastcmd_edit = true;
                                strcpy(inpbuf, &lastcmd[lastcmd_idx]);                      // get the command into the buffer for editing
                                goto insert_lastcmd;
                            }
                            break;

                                    /*  |               */
                case CTRLKEY('X'):  /* \|/  Down Arrow  */
                case DOWN:  if(!(BufEdited)) {

                                if(lastcmd_idx == 0)
                                    *inpbuf = lastcmd_edit = 0;
                                else {
                                    for(i = lastcmd_idx - 2; i > 0 && lastcmd[i - 1] != 0; i--);// find the start of the previous command
                                    lastcmd_idx = i;                                        // and point to it for the next time around
                                    strcpy(inpbuf, &lastcmd[i]);                            // get the command into the buffer for editing
                                }
                                goto insert_lastcmd;                                        // gotos are bad, I know, I know
                            }
                            break;

                insert_lastcmd:                                                             // goto here if we are just recalling a command from buffer


			                 // Lets put the cursor at the beginning of where the command is displayed.

                             // backspace to the beginning of line
                             //if(j > 0) {
				             j=CharIndex;  //????????????????????????????????
                             while(j)  {
                             // MMputchar('\b');
                             if (j==l4 || j==l3 ||j==l2 ){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                               j--;
                             }
                             fflush(stdout);
                             // MMCharPos=3;CurrentX=2*gui_font_width;
                             MX470Display(CLEAR_TO_EOS);SerUSBPutS("\033[0J");        //Clear to End Of Screen

				             CharIndex = strlen(inpbuf);
                             MMPrintString(inpbuf);                                          // display the line
                             if(CharIndex==l4 || CharIndex==l3 || CharIndex==l2){SerUSBPutS(" ");SerUSBPutS("\b");}
                             fflush(stdout);
                             CharIndex = strlen(inpbuf);                                     // get the current cursor position in the line
                             break;


                /********************************************* Other Keys ********************************************************/
                default:    if(buf[0] >= ' ' && buf[0] < 0x7f) {
                                //BufEdited = true;                                           // this means that something was typed
                                i = CharIndex;
                                j = strlen(inpbuf);
                                if(insert) {
                                    if(strlen(inpbuf) >= maxchars - 1) break;               // sorry, line fulljust ignore
                                    for(p = inpbuf + strlen(inpbuf); j >= CharIndex; p--, j--) *(p + 1) = *p;
                                    inpbuf[CharIndex] = buf[0];                             // insert the char
                                    MMPrintString(&inpbuf[CharIndex]);                      // display new part of the line
                                    CharIndex++;
                                    for(j = strlen(inpbuf); j > CharIndex; j--){
                                       // MMputchar('\b');
                                        if (j==l4 || j==l3 || j==l2){DisplayPutC('\b');SerUSBPutS("\e[1A");SerUSBPutS(goend);}else{ MMputchar('\b');}
                                    }
                                    fflush(stdout);                                   // return the cursor to the right position

                                } else {
                                	if(strlen(inpbuf) >= maxchars-1 ) break;               // sorry, line full  just ignore
                                    inpbuf[strlen(inpbuf) + 1] = 0;                         // incase we are adding to the end of the string
                                    inpbuf[CharIndex++] = buf[0];                           // overwrite the char
                                    MMputchar(buf[0]);                                      // display it
                                    if(j==l4-1 || j==l3-1 || j==l2-1){SerUSBPutS(" ");SerUSBPutS("\b");}
                                    fflush(stdout);
                                }

                              /**** Future NOSCROLL
                                i = CharIndex;
                                j = strlen((const char *)inpbuf);
                                // If its going to scroll then clear screen

                                if(Option.NoScroll && Option.DISPLAY_CONSOLE){
                                   if(CurrentY + 2*gui_font_height >= VRes) {
                                      ClearScreen(gui_bcolour);CurrentY=0;
                                      CurrentX = (MMPromptPos-2)*gui_font_width  ;
                                      DisplayPutC('>');
                                      DisplayPutC(' ');
                                      DisplayPutS((char *)inpbuf);                      // display the line

                                    }
                                }
                              ****/

                            }
                            break;
            }
            for(i = 0; i < MAXKEYLEN + 1; i++) buf[i] = buf[i + 1];                             // shuffle down the buffer to get the next char
        } while(*buf);
        if(CharIndex == strlen(inpbuf)) {insert = false;}
       // show(MMCharPos,0,"CharPOS");
       // show(MMCharPos,2,"inpbuf");
    }

    saveline:
    MMPrintString("\r\n");
    BufEdited = false;
    if(strlen(inpbuf) < maxchars)InsertLastcmd(inpbuf);
    ShowCursor(false);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
//void _Error_Handler(char *file, int line)
//{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
//  while(1)
//  {
//  }
  /* USER CODE END Error_Handler_Debug */
//}
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	MMErrorString("Error:");
    char buf[20];
    IntToStr(buf, SystemError, 10);
    MMErrorString(buf);
    MMErrorString("\r\n");
    MMPrintString("Error_Handler Called \r\n");


  /* USER CODE END Error_Handler_Debug */
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
