/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

MiscSTM32.c

Handles the a few miscellaneous functions for the STM32 version.


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


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
extern void SetBacklight(int intensity);
extern char LCDAttrib;
extern void CallCFuncT5(void);                                      // this is implemented in CFunction.c
extern unsigned int CFuncT5;                                        // we should call the CFunction T5 interrupt function if this is non zero
extern void ConfigSDCard(char *p);
volatile MMFLOAT VCC=3.3;
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;
#ifdef STM32F4version
	#define GenSPI hspi2
	extern SPI_HandleTypeDef GenSPI;
	extern void initKeyboard(void);
	extern void MIPS16 InitTouch(void);
	extern int CurrentSPISpeed;
#endif
extern RTC_HandleTypeDef hrtc;
extern void setterminal(void);

///////////////////////////////////////////////////////////////////////////////////////////////
// constants and functions used in the OPTION LIST command
char *LCDList[] = {"","VGA", "SSD1963_4", "SSD1963_5", "SSD1963_5A", "SSD1963_5ER","SSD1963_7", "SSD1963_7A","SSD1963_8", "","" ,     //0-10
		"SSD1963_4_16", "SSD1963_5_16", "SSD1963_5A_16", "SSD1963_5ER_16","SSD1963_7_16", "SSD1963_7A_16", "SSD1963_8_16", "IPS_4_16","","",  //11-20
		"ILI9163", "ST7735", "","",                                                                                                   //21-24
		"USER", "ILI9481", "ILI9341", "ILI9481IPS","ILI9488","",                                                                      //25-30
		"ILI9341_16", "ILI9341_8", "SSD1963_5_BUFF","SSD1963_7_BUFF","SSD1963_8_BUFF",                                                //31-35
		"SSD1963_5_640","SSD1963_7_640", "SSD1963_8_640", "SSD1963_5_8BIT", "SSD1963_7_8BIT",                                         //36-40
		"SSD1963_8_8BIT","","","", "HDMI"};                                                                                           //41-45
const char *OrientList[] = {"", "LANDSCAPE", "PORTRAIT", "RLANDSCAPE", "RPORTRAIT"};
const char *CaseList[] = {"", "LOWER", "UPPER"};
const char *KBrdList[] = {"", "UK", "US", "DE", "FR", "ES", "IT", "BE" };
void SRet(void){
    SerUSBPutS("\r\n");
}

void SInt(int64_t n) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    SerUSBPutS(s);
}

void SIntComma(int64_t n) {
	SerUSBPutS(", "); SInt(n);
}

void SIntH(unsigned long long int n) {
    char s[20];
    IntToStr(s, (int64_t)n, 16);
    SerUSBPutS(s);
}
void SIntHC(unsigned long long int n) {
	SerUSBPutS(", "); SIntH(n);
}

void SFlt(MMFLOAT flt){
	   char s[20];
	   FloatToStr(s, flt, 4,4, ' ');
	   SerUSBPutS(s);
}
void SFltComma(MMFLOAT n) {
	SerUSBPutS(", "); SFlt(n);
}

void PRet(void){
    MMPrintString("\r\n");
}

void PO(char *s,int m) {
	if(m==1)MMPrintString("OPTION ");
	else if(m==2)MMPrintString("DEFAULT ");
	else if(m==3)MMPrintString("CURRENT ");
	MMPrintString(s); MMPrintString(" ");
}

void PInt(int64_t n) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    MMPrintString(s);
}

void PIntComma(int64_t n) {
    MMPrintString(", "); PInt(n);
}

void PO2Str(char *s1, const char *s2, int m) {
    PO(s1,m); MMPrintString((char *)s2); MMPrintString("\r\n");
}


void PO2Int(char *s1, int64_t n) {
    PO(s1,1); PInt(n); MMPrintString("\r\n");
}

void PO2IntH(char *s1, int64_t n) {
    PO(s1,1); PIntH(n); MMPrintString("\r\n");
}

void PO3Int(char *s1, int64_t n1, int64_t n2) {
    PO(s1,1); PInt(n1); MMPrintString(",");PInt(n2); MMPrintString("\r\n");
}
void PIntH(unsigned long long int n) {
    char s[20];
    IntToStr(s, (int64_t)n, 16);
    MMPrintString(s);
}
void PIntHC(unsigned long long int n) {
    MMPrintString(", "); PIntH(n);
}

void PFlt(MMFLOAT flt){
	   char s[20];
	   FloatToStr(s, flt, 4,4, ' ');
	    MMPrintString(s);
}
void PFltComma(MMFLOAT n) {
    MMPrintString(", "); PFlt(n);
}

void PPinName(int n){
	char s[3]="Px";
	int pn=0,pp=1;
	if(PinDef[n].sfr==GPIOA)s[1]='A';
	else if(PinDef[n].sfr==GPIOB)s[1]='B';
	else if(PinDef[n].sfr==GPIOC)s[1]='C';
	else if(PinDef[n].sfr==GPIOD)s[1]='D';
	else if(PinDef[n].sfr==GPIOE)s[1]='E';
	while(PinDef[n].bitnbr!=pp){pp<<=1;pn++;}
	if(s[1]!='x'){
		MMPrintString(s);
		PInt(pn);
	} else PInt(n);
}
void PPinNameComma(int n) {
    MMPrintString(", "); PPinName(n);

}
///////////////////////////////////////////////////////////////////////////////////////////////

void printoptions(void){
//	LoadOptions();
	MMPrintString("\rARMmite H7 MMBasic Version " VERSION "\r\n");

	if(HAL_GetREVID()==0x1003 && (Option.CPUspeed != 400))PO2Int("CPU SPEED", Option.CPUspeed);
	if(HAL_GetREVID()!=0x1003 && (Option.CPUspeed != 480))PO2Int("CPU SPEED", Option.CPUspeed);
	//if(Option.CPUspeed != 400)PO2Int("CPU SPEED", Option.CPUspeed);
    if(Option.Autorun == true) PO2Str("AUTORUN", "ON",1);
    if(Option.Baudrate != CONSOLE_BAUDRATE) PO2Int("BAUDRATE", Option.Baudrate);
    if(Option.Invert == true) PO2Str("CONSOLE", "INVERT",1);
    if(Option.Invert == 2) PO2Str("CONSOLE", "AUTO",1);
    if(Option.ColourCode == true) PO2Str("COLOURCODE", "ON",1);
    if(Option.Listcase != CONFIG_TITLE) PO2Str("CASE", CaseList[(int)Option.Listcase],1);
    if(Option.Tab != 2) PO2Int("TAB", Option.Tab);
    if(Option.Height != 24 || Option.Width != 80) PO3Int("DISPLAY", Option.Height, Option.Width);

    if((Option.DISPLAY_TYPE > SSD_PANEL && Option.DISPLAY_TYPE<= SPI_PANEL) || (Option.DISPLAY_TYPE > USER && Option.DISPLAY_TYPE <= SPI_PANEL_END)) {
        PO("LCDPANEL",1); MMPrintString((char *)LCDList[(uint8_t)Option.DISPLAY_TYPE]); MMPrintString(", "); MMPrintString((char *)OrientList[(int)Option.DISPLAY_ORIENTATION]);
        PIntComma(Option.LCD_CD); PIntComma(Option.LCD_Reset); PIntComma(Option.LCD_CS); MMPrintString("\r\n");
    }
    if(Option.DISPLAY_TYPE > VGA && Option.DISPLAY_TYPE <= SSD_PANEL) {
        PO("LCDPANEL",1); MMPrintString((char *)LCDList[(uint8_t)Option.DISPLAY_TYPE]); MMPrintString(", "); MMPrintString((char *)OrientList[(int)Option.DISPLAY_ORIENTATION]);
        if(Option.SSDspeed)MMPrintString(",1");
        MMPrintString("\r\n");
    }
    if(Option.DISPLAY_TYPE>= ILI9341_16 && Option.DISPLAY_TYPE!=HDMI){
        PO("LCDPANEL",1); MMPrintString(LCDList[(uint8_t)Option.DISPLAY_TYPE]);
        MMPrintString(", "); MMPrintString((char *)OrientList[(int)Option.DISPLAY_ORIENTATION]);
        MMPrintString("\r\n");
    }
    if(Option.DISPLAY_TYPE == USER || Option.DISPLAY_TYPE == HDMI){
    	PO("LCDPANEL",1); MMPrintString(LCDList[(uint8_t)Option.DISPLAY_TYPE]);
        PIntComma(HRes);
        PIntComma(VRes);
        MMPrintString("\r\n");
    }
    if(Option.TOUCH_CS) {
        PO("TOUCH",1); PInt(Option.TOUCH_CS); PIntComma(Option.TOUCH_IRQ);
        if(Option.TOUCH_Click) PIntComma(Option.TOUCH_Click);
        MMPrintString("\r\n");
        if(Option.TOUCH_XZERO != 0 || Option.TOUCH_YZERO != 0) {
            MMPrintString("GUI CALIBRATE "); PInt(Option.TOUCH_SWAPXY); PIntComma(Option.TOUCH_XZERO); PIntComma(Option.TOUCH_YZERO);
            PIntComma(Option.TOUCH_XSCALE * 10000); PIntComma(Option.TOUCH_YSCALE * 10000); MMPrintString("\r\n");
        }
    }
    // if(Option.DefaultBrightness!=100){
    if(HRes != 0){
     	MMPrintString("BACKLIGHT ");
     	if(Option.DefaultBrightness>100){
     		PInt(Option.DefaultBrightness-101);MMPrintString(",REVERSE \r\n");
     	}else{
     		PInt(Option.DefaultBrightness);;MMPrintString(",DEFAULT \r\n");
     	}
     }
	if(Option.FlashPages !=1 ) PO2Int("FLASHPAGES", Option.FlashPages);
	if(Option.ProgFlashSize != PROG_FLASH_SIZEMAX) PO2IntH("ProgramFlashSize", Option.ProgFlashSize);
    if(Option.SDCARD_CS) {
        PO("SDCARD",1);
        PInt(Option.SDCARD_CS);
        if(Option.SD_CD) PIntComma(Option.SD_CD);else if(Option.SD_WP) putConsole(',');
        if(Option.SD_WP) PIntComma(Option.SD_WP);
        MMPrintString("\r\n");
    }
    if(Option.DISPLAY_CONSOLE == true) PO2Str("LCDPANEL", "CONSOLE",1);
    if(Option.MaxCtrls != 201) PO2Int("CONTROLS", Option.MaxCtrls - 1);
    if(!Option.SerialPullup) PO2Str("SERIAL PULLUPS", "OFF",1);
    if(Option.USBKeyboard != NO_KEYBOARD){
    	PO("USBKEYBOARD",1);
    	MMPrintString((char *)KBrdList[(int)Option.USBKeyboard]);
    	if(Option.USBpower)PIntComma(Option.USBpower);
        MMPrintString("\r\n");

    }
    if(Option.RTC_Calibrate)PO2Int("RTC CALIBRATE", Option.RTC_Calibrate);
    if(!(Option.RepeatStart==600 && Option.RepeatRate==150)){
    	char buff[40]={0};
    	sprintf(buff,"OPTION KEYBOARD REPEAT %d,%d\r\n",Option.RepeatStart, Option.RepeatRate);
    	MMPrintString(buff);
    }
    if(*Option.F1key)PO2Str("F1",(char *)Option.F1key,1);
    if(*Option.F5key)PO2Str("F5",(char *)Option.F5key,1);
    if(*Option.F6key)PO2Str("F6",(char *)Option.F6key,1);
    if(*Option.F7key)PO2Str("F7",(char *)Option.F7key,1);
    if(*Option.F8key)PO2Str("F8",(char *)Option.F8key,1);
    if(*Option.F9key)PO2Str("F9",(char *)Option.F9key,1);
    return;

}

void MIPS16 OtherOptions(void) {
	char *tp, *ttp;
	int i;

	tp = checkstring(cmdline, "RESET");
	if(tp) {
        ResetAllOptions();
		goto saveandreset;
	}

	tp = checkstring(cmdline, "USBKEYBOARD");
	if(tp) {
	    getargs(&tp, 5, ",");
	    if(CurrentLinePtr) error("Invalid in a program");
        if(checkstring(argv[0], "DISABLE"))	{
        	Option.USBKeyboard = NO_KEYBOARD;
        	Option.noLED=0;
        }
        else {
            if(checkstring(argv[0], "US"))	Option.USBKeyboard = CONFIG_US;
            else if(checkstring(argv[0], "UK"))	Option.USBKeyboard = CONFIG_UK;
            else if(checkstring(argv[0], "DE"))	Option.USBKeyboard = CONFIG_DE;
            else if(checkstring(argv[0], "FR"))	Option.USBKeyboard = CONFIG_FR;
            else if(checkstring(argv[0], "ES"))	Option.USBKeyboard = CONFIG_ES;
            else if(checkstring(argv[0], "BE"))	Option.USBKeyboard = CONFIG_BE;
            else error("Layout not supported");
        }
        if(argc==3 && *argv[2]) {
        	CheckPin(abs(getinteger(argv[2])), CP_IGNORE_INUSE); // | CP_IGNORE_BOOTRES | CP_IGNORE_RESERVED);
        	Option.USBpower = (char)getinteger(argv[2]);
        }
        if(argc==5)Option.noLED=getint(argv[4],0,1);
        else Option.noLED=0;
        goto saveandreset;
	}
    tp = checkstring(cmdline, "KEYBOARD REPEAT");
	if(tp) {
		getargs(&tp,3,",");
		Option.RepeatStart=getint(argv[0],100,2000);
		Option.RepeatRate=getint(argv[2],25,2000);
		SaveOptions(1);
		return;
	}

    tp = checkstring(cmdline, "SERIAL PULLUP");
	if(tp) {
        if(checkstring(tp, "DISABLE"))
            Option.SerialPullup = 0;
        else if(checkstring(tp, "ENABLE"))
            Option.SerialPullup = 1;
        else error("Invalid Command");
        goto saveandreset;
	}

    tp = checkstring(cmdline, "SDCARD");
    if(tp) {
    	if(CurrentLinePtr) error("Invalid in a program");
        if(checkstring(tp, "DISABLE"))
            Option.SDCARD_CS = Option.SD_CD = Option.SD_WP = 0;
        else
            ConfigSDCard(tp);

        goto saveandreset;
    }

	tp=checkstring(cmdline, "RTC CALIBRATE");
	if(tp){
		Option.RTC_Calibrate=getint(tp,-511,512);
		int up=RTC_SMOOTHCALIB_PLUSPULSES_RESET;
		int calibrate= -Option.RTC_Calibrate;
		if(Option.RTC_Calibrate>0){
			up=RTC_SMOOTHCALIB_PLUSPULSES_SET;
			calibrate=512-Option.RTC_Calibrate;
		}
		HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, up, calibrate);
		SaveOptions(1);
		return;
	}

	tp = checkstring(cmdline, "CONTROLS");
    if(tp) {
    	Option.MaxCtrls = getint(tp, 0, 1000) + 1;
        goto saveandreset;
    }
#ifndef STM32F4version
    tp = checkstring(cmdline, "FLASHPAGES");
    if(tp) {
#ifdef STM32H743xx
    	i = getint(tp, 1, 4);
    	if(i==4 && Option.ProgFlashSize!=PROG_FLASH_SIZEMAX)error("FlashPage 4 used by Library");
    	Option.FlashPages=i;
#else
    	Option.FlashPages = getint(tp, 1, 2);
#endif
        goto saveandreset;
    }
#endif
	tp = checkstring(cmdline, "ERROR");
	if(tp) {
        if(!Option.SDCARD_CS) error("SD card not configured");
		if(checkstring(tp, "CONTINUE")) {
            OptionFileErrorAbort = false;
            return;
        }
		if(checkstring(tp, "ABORT")) {
            OptionFileErrorAbort = true;
            return;
        }
	}

    tp = checkstring(cmdline, "LCDPANEL");
    if(tp) {
        if(CurrentLinePtr) error("Invalid in a program");
    	if((ttp = checkstring(tp, "CONSOLE"))) {
            if(HRes == 0) error("LCD Panel not configured");
            if(ScrollLCD == (void (*)(int ))DisplayNotSet) error("SSD1963 display required");
            if(!DISPLAY_LANDSCAPE) error("Landscape only");
            Option.Height = VRes/gui_font_height; Option.Width = HRes/gui_font_width;
            skipspace(ttp);
            Option.DefaultFC = WHITE;
            Option.DefaultBC = BLACK;
           // Option.DefaultBrightness = 100;
            if(!(*ttp == 0 || *ttp == '\'')) {
                getargs(&ttp, 7, ",");                              // this is a macro and must be the first executable stmt in a block
                if(argc > 0) {
                    if(*argv[0] == '#') argv[0]++;                  // skip the hash if used
                    SetFont((((getint(argv[0], 1, FONT_BUILTIN_NBR) - 1) << 4) | 1));
                    //Option.DefaultFont = gui_font;
                    Option.DefaultFont = (gui_font>>4)+1;
                }
                if(argc > 2) Option.DefaultFC = getint(argv[2], BLACK, WHITE);
                if(argc > 4) Option.DefaultBC = getint(argv[4], BLACK, WHITE);
                if(Option.DefaultFC == Option.DefaultBC) error("Same colours");
                if(argc > 6){
                  if(Option.DefaultBrightness>100){
                     Option.DefaultBrightness = 101+getint(argv[6], 0, 100);
                  }else{
                     Option.DefaultBrightness = 101+getint(argv[6], 0, 100);
                  }
                  SetBacklight(Option.DefaultBrightness);
                  // SetBacklightSSD1963(Option.DefaultBrightness);
                }
            } else {
            	//SetFont((Option.DefaultFont-1)<<4 | 0x1);   //B7-B4 is fontno.-1 , B3-B0 is scale
                //SetFont((Option.DefaultFont = 0x11));
            	 SetFont((2-1)<<4 | 0x1);   //B7-B4 is fontno.-1 , B3-B0 is scale
            	 Option.DefaultFont = (gui_font>>4)+1;
            }
            Option.DISPLAY_CONSOLE = 1;
            // Option.ColourCode = true;  //On by default so done change it
            SaveOptions(1);
           // PromptFont = Option.DefaultFont;
            PromptFont = gui_font;
            PromptFC = Option.DefaultFC;
            PromptBC = Option.DefaultBC;
            setterminal();
            ClearScreen(Option.DefaultBC);
            return;
        }
        else if(checkstring(tp, "NOCONSOLE")) {
            Option.Height = SCREENHEIGHT; Option.Width = SCREENWIDTH;
            Option.DISPLAY_CONSOLE = 0;
            // Option.ColourCode = false;   //Don't change as on by default
            Option.DefaultFC = WHITE;
            Option.DefaultBC = BLACK;
            //SetFont((Option.DefaultFont = 0x01));
            Option.DefaultFont=0x01;
            SetFont(((Option.DefaultFont-1)<<4) | 1);
            //Option.DefaultBrightness = 100;
            setterminal();
            SaveOptions(1);
            ClearScreen(Option.DefaultBC);
            return;
        }
        else if(checkstring(tp, "DISABLE")) {
#ifdef STM32F4version
        	touchdisable();
#endif
            Option.Height = SCREENHEIGHT; Option.Width = SCREENWIDTH;
            if(Option.DISPLAY_CONSOLE){
               setterminal();
            }
            Option.DISPLAY_CONSOLE = Option.DISPLAY_TYPE = Option.DISPLAY_ORIENTATION = Option.SSDspeed = HRes = 0;
            Option.DefaultFC = WHITE; Option.DefaultBC = BLACK; Option.DefaultFont = 0x01;// Option.DefaultBrightness = 100;
            DrawRectangle = (void (*)(int , int , int , int , int )) DisplayNotSet;
            DrawBitmap =  (void (*)(int , int , int , int , int , int , int , unsigned char *)) DisplayNotSet;
            ScrollLCD = (void (*)(int ))DisplayNotSet;
            DrawBuffer = (void (*)(int , int , int , int , char * )) DisplayNotSet;
            ReadBuffer = (void (*)(int , int , int , int , char * )) DisplayNotSet;
#ifdef STM32F4version
#endif
        }
        else {
            if(Option.DISPLAY_TYPE) error("Display already configured");
#ifndef STM32F4version
            ConfigDisplayOther(tp);
#endif
            if(!Option.DISPLAY_TYPE)ConfigDisplaySSD(tp);
            if(!Option.DISPLAY_TYPE) ConfigDisplaySPI(tp);          // if it is not an SSD1963 then try for a SPI type
        }
#ifndef STM32F4version
        goto saveandreset;
#else
        SaveOptions(1);
        return;
#endif
    }
    tp = checkstring(cmdline, "TOUCH");
    if(tp) {
#ifdef STM32F4version
    	if(CurrentLinePtr) error("Invalid in a program");
		if(checkstring(tp, "DISABLE")) {
			touchdisable();
		} else {
            ConfigTouch(tp);
			InitTouch();
        }
        SaveOptions(1);
        return;
#else
    	if(CurrentLinePtr) error("Invalid in a program");
		if(checkstring(tp, "DISABLE")) {
            Option.TOUCH_Click = Option.TOUCH_CS = Option.TOUCH_IRQ = false;
        } else
            ConfigTouch(tp);
        goto saveandreset;
#endif
    }
#ifdef STM32F4version
    tp = checkstring(cmdline, "SERIAL CONSOLE");
	if(tp) {
		if(checkstring(tp, "OFF"))		    {
	        MMPrintString("Reset the processor to connect with the USB/CDC port\r\n");
	    	while(ConsoleTxBufTail != ConsoleTxBufHead){}
			Option.SerialConDisabled = true;
	        SaveOptions(1);
	        return;
		}
		if(checkstring(tp, "ON"))           {
	        MMPrintString("Reset the processor to connect with the serial port J6\r\n");
	    	while(ConsoleTxBufTail != ConsoleTxBufHead){}
			Option.SerialConDisabled = false;
	        SaveOptions(1);
	        return;
		}
	}
#endif
	tp = checkstring(cmdline, "LIST");
    if(tp) {
    	printoptions();
    	return;
    }


	error("Unrecognised option");


saveandreset:
   // used for options that require the cpu to be reset
    SaveOptions(1);
    _excep_code = RESTART_NOAUTORUN;                            // otherwise do an automatic reset
	while(ConsoleTxBufTail != ConsoleTxBufHead);
	uSec(10000);
    SoftReset();                                                // this will restart the processor

}





