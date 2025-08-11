/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

OtherDisplays.c

Does the basic LCD display commands and drawing in MMBasic.

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
extern SPI_HandleTypeDef GenSPI;
int low_y=480, high_y=0, low_x=800, high_x=0;
unsigned short colcount[256];
unsigned char *screenbuff, *screenbuff2, *screenbuff3;
int memswitch;
int memswitch2;
int displayoffset=0;


#define SPIqueue3(a) {HAL_SPI_Transmit(&GenSPI,a,3,500);}

/* transfer state */
__IO uint32_t wTransferState = TRANSFER_WAIT;

void DrawBitmapBuff(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
void DrawBufferBuff(int x1, int y1, int x2, int y2, char* p);
void ReadBufferBuff(int x1, int y1, int x2, int y2, char* p);
void DrawRectangleBuff(int x1, int y1, int x2, int y2, int c);
void DrawBitmapBuff8(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
void DrawBufferBuff8(int x1, int y1, int x2, int y2, char* p);
void ReadBufferBuff8(int x1, int y1, int x2, int y2, char* p);
void DrawRectangleBuff8(int x1, int y1, int x2, int y2, int c);
void ScrollBuff8(int lines);

//void P16_write(int cmd) {
//        GPIOE->ODR =  cmd;
//        SSD1963_WR_TOGGLE_PIN;
//}

//void P16_write_VIT6(int cmd) {
//        GPIOE->ODR =  cmd;
//        SSD1963_WR_TOGGLE_PIN_VIT6;
//}
// Write a command byte to the ILI9341
void P16_write_command(int cmd) {
	if(HAS_144PINS){
      PinSetBit(SSD1963_DC_PIN, LATCLR);
      GPIOE->ODR = cmd;
      SSD1963_WR_TOGGLE_PIN;
      PinSetBit(SSD1963_DC_PIN, LATSET);
	}else{
	  PinSetBit(SSD1963_DC_PIN, LATCLR);
	  GPIOE->ODR = cmd;
	  SSD1963_WR_TOGGLE_PIN_VIT6;
	  PinSetBit(SSD1963_DC_PIN, LATSET);
	}
}
void P16_write_command_ZI2(int cmd) {
    PinSetBit(SSD1963_DC_PIN, LATCLR);
    GPIOE->ODR = cmd;
    SSD1963_WR_TOGGLE_PIN;
    PinSetBit(SSD1963_DC_PIN, LATSET);
}

void P16_write_command_VIT6(int cmd) {
    PinSetBit(SSD1963_DC_PIN, LATCLR);
    GPIOE->ODR = cmd;
    SSD1963_WR_TOGGLE_PIN_VIT6;
    PinSetBit(SSD1963_DC_PIN, LATSET);
}

// Write an 8 bit data word to the ILI9341
void P16_write_data(int data) {
	if(HAS_144PINS){
      GPIOE->ODR = data;
      SSD1963_WR_TOGGLE_PIN;
	}else{
	  GPIOE->ODR = data;
	  SSD1963_WR_TOGGLE_PIN_VIT6;
	}
}
void P16_write_data_ZI2(int data) {
    GPIOE->ODR = data;
    SSD1963_WR_TOGGLE_PIN;
}
void P16_write_data_VIT6(int data) {
    GPIOE->ODR = data;
    SSD1963_WR_TOGGLE_PIN_VIT6;
}
//Called from CAMERA command
void SaveToBuffer(void) {
    int x, y;
    uint16_t *s, t;
    int mode=GPIOE->MODER;
    for(y=0;y<VRes;y++){
        if(y<memswitch){
            s=(uint16_t *)((y * HRes) * 2 + screenbuff);
        } else if(y<memswitch2){
             s=(uint16_t *)(((y-memswitch) * HRes) * 2 + screenbuff2);
        } else {
             s=(uint16_t *)(((y-memswitch2) * HRes) * 2 + screenbuff3);
        }
        for(x=0;x<HRes;x++){
            if(Option.DISPLAY_ORIENTATION & 1)SetAreaSSD1963(displayoffset+x, y, displayoffset+x, y);
            else SetAreaSSD1963(x, displayoffset+y, x, displayoffset+y);
            WriteSSD1963Command_ZI2(CMD_RD_MEMSTART);
            GPIOE->MODER=0;
            RDTOGGLE; shortpause(5); RDTOGGLE; shortpause(5);
            t=GPIOE->IDR;
            *s++=((t>>8) & 0xFF) | ((t<<8) & 0xFF00);
            GPIOE->MODER=mode;
        }
    }
    GPIOE->MODER=mode;
}
void fun_movement(void) {
	int threshold=getnumber(ep);
	int us=128000000;
    int r1,r2,g1,g2,b1,b2,tot1,tot2,count=0;;
	unsigned short a,c;
	int i,y;
	unsigned char *s;
    SetAreaSSD1963(80, 0, HRes+80, VRes-1);
    WriteSSD1963Command_ZI2(CMD_WR_MEMSTART);
	WriteCoreTimer(0);
    if(!cameraopen)error("Camera must be OPEN before use");
    while (!ST_PCLK  && ReadCoreTimer() < us); /* wait for clock to go high */
    while (ST_PCLK && ReadCoreTimer() < us); /* wait for clock to go back low */
    while (ST_HREF  && ReadCoreTimer() < us); /* wait for a line to end */
    while (!ST_HREF  && ReadCoreTimer() < us); /* wait for a line to end */
    if(!ST_VSYNC)while (!ST_VSYNC && ReadCoreTimer() < us);
    while (ST_VSYNC && ReadCoreTimer() < us); /* wait for a new frame to start */
    if(ReadCoreTimer()>=us)error("Timeout on camera data signals");
    __disable_irq();
    WriteCoreTimer(0);
    while (!ST_VSYNC); /* wait for a new frame to start */
    for(y=0;y<VRes;y++){
        if(y<memswitch){
        	s=(y * HRes) * 2 + screenbuff;
        } else if(y<memswitch2){
        	s=((y-memswitch) * HRes) * 2 + screenbuff2;
        } else {
        	s=((y-memswitch2) * HRes) * 2 + screenbuff3;
        }
       i=0;
       while (!ST_HREF ); /* wait for a line to start */
       while (i<=HRes) { /* wait for a line to end */
            c=*s;
            c=(((c>>8) & 0xFF) | ((c<<8) & 0xFF00));
            *s=GPIOC->IDR;
            /* first byte */
            while (!ST_PCLK); /* wait for clock to go high */
            r1=(c & 0xF800)>>13;
            g1=(c & 0x7C0)>>5;
            b1=(c & 0x1F)>>3;
            tot1=(r1+g1+b1)/3;
     	    a=((*s++) <<8);
            i++;
            while (ST_PCLK); /* wait for clock to go back low */
            /* second byte */
            *s=GPIOC->IDR & 0xFF;
            a|=*s;
            s++;
            r2=(a & 0xF800)>>13;
            g2=(a & 0x7E0)>>5;
            while (!ST_PCLK); /* wait for clock to go high */
            r2=(a & 0xF800)>>13;
            g2=(a & 0x7E0)>>5;
            b2=(a & 0x1F)>>3;
            tot2=(r2+g2+b2)/3;
            if(abs(tot1-tot2)>threshold && y<VRes-20 && y>=20)count++;
            GPIOE->ODR=a;
        	GPIOG->BSRR= GPIO_PIN_10<<16;
            while (ST_PCLK); /* wait for clock to go back low */
            GPIOG->BSRR= GPIO_PIN_10;
        }
    }
    __enable_irq();
    RtcGetTime();
    fret=(double)count;
    targ = T_NBR;
}
void ConfigDisplayOther(char *p){
	int p1, p2, p3;
	char code;
    getargs(&p, 11,",");
    if(Option.DISPLAY_TYPE)error("OPTION LCDPANEL already set");
    if(checkstring(argv[0], "HDMI")){
        if(argc != 5) error("Argument count");
        Option.DISPLAY_TYPE = HDMI;
        Option.DISPLAY_WIDTH = getint(argv[2], 1, 10000);
        Option.DISPLAY_HEIGHT = getint(argv[4], 1, 10000);
            // setup the pointers to the drawing primitives
        CheckPin(SSD1963_DC_PIN, CP_IGNORE_INUSE);
        CheckPin(SSD1963_RESET_PIN, CP_IGNORE_INUSE);
        CheckPin(SSD1963_WR_PIN, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT1, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT2, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT3, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT4, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT5, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT6, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT7, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT8, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT9, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT10, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT11, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT12, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT13, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT14, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT15, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT16, CP_IGNORE_INUSE);
        Option.DISPLAY_ORIENTATION=LANDSCAPE;
        Option.TOUCH_XZERO = TOUCH_NOT_CALIBRATED;                      // record the touch feature as not calibrated
    } else if(checkstring(argv[0], "ILI9341_16") || checkstring(argv[0], "ILI9341_8")
    		|| checkstring(argv[0], "SSD1963_5_640") || checkstring(argv[0], "SSD1963_5_BUFF") || checkstring(argv[0], "SSD1963_5_8BIT")
    		|| checkstring(argv[0], "SSD1963_7_640") || checkstring(argv[0], "SSD1963_7_BUFF") || checkstring(argv[0], "SSD1963_7_8BIT")
    		|| checkstring(argv[0], "SSD1963_8_640") || checkstring(argv[0], "SSD1963_8_BUFF") || checkstring(argv[0], "SSD1963_8_8BIT")
			) {
        if(!(argc==3 || argc==5))error("Argument count");
    	Option.DISPLAY_TYPE = ILI9341_16;
        if(checkstring(argv[0], "ILI9341_8"))Option.DISPLAY_TYPE = ILI9341_8;
        if(checkstring(argv[0], "SSD1963_5_640"))Option.DISPLAY_TYPE = SSD1963_5_640;
        if(checkstring(argv[0], "SSD1963_5_BUFF"))Option.DISPLAY_TYPE = SSD1963_5_BUFF;
        if(checkstring(argv[0], "SSD1963_7_640"))Option.DISPLAY_TYPE = SSD1963_7_640;
        if(checkstring(argv[0], "SSD1963_7_BUFF"))Option.DISPLAY_TYPE = SSD1963_7_BUFF;
        if(checkstring(argv[0], "SSD1963_8_640"))Option.DISPLAY_TYPE = SSD1963_8_640;
        if(checkstring(argv[0], "SSD1963_8_BUFF"))Option.DISPLAY_TYPE = SSD1963_8_BUFF;
        if(checkstring(argv[0], "SSD1963_5_8BIT"))Option.DISPLAY_TYPE = SSD1963_5_8BIT;
        if(checkstring(argv[0], "SSD1963_7_8BIT"))Option.DISPLAY_TYPE = SSD1963_7_8BIT;
        if(checkstring(argv[0], "SSD1963_8_8BIT"))Option.DISPLAY_TYPE = SSD1963_8_8BIT;
        if(checkstring(argv[2], "L") || checkstring(argv[2], "LANDSCAPE"))
            Option.DISPLAY_ORIENTATION = LANDSCAPE;
        else if(checkstring(argv[2], "P") || checkstring(argv[2], "PORTRAIT"))
            Option.DISPLAY_ORIENTATION = PORTRAIT;
        else if(checkstring(argv[2], "RL") || checkstring(argv[2], "RLANDSCAPE"))
            Option.DISPLAY_ORIENTATION = RLANDSCAPE;
        else if(checkstring(argv[2], "RP") || checkstring(argv[2], "RPORTRAIT"))
            Option.DISPLAY_ORIENTATION = RPORTRAIT;
        else
            error("Orientation");

        CheckPin(SSD1963_DC_PIN, CP_IGNORE_INUSE);
        CheckPin(SSD1963_RESET_PIN, CP_IGNORE_INUSE);
        CheckPin(SSD1963_WR_PIN, CP_IGNORE_INUSE);
        CheckPin(SSD1963_RD_PIN, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT1, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT2, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT3, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT4, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT5, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT6, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT7, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT8, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT9, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT10, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT11, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT12, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT13, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT14, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT15, CP_IGNORE_INUSE);
        CheckPin(SSD1963_DAT16, CP_IGNORE_INUSE);
        Option.TOUCH_XZERO = TOUCH_NOT_CALIBRATED;                      // record the touch feature as not calibrated
        if(argc > 3 && *argv[4]) {
            Option.SSDspeed = getint(argv[4],0,1);
         } else {
        	 Option.SSDspeed = 0;
         }
        Option.Refresh = 1;
    } else if(checkstring(argv[0], "ILI9341") || checkstring(argv[0], "ILI9481") || checkstring(argv[0], "ILI9488")|| checkstring(argv[0], "ST7796S")) {
        Option.DISPLAY_TYPE = ILI9341;
        if(checkstring(argv[0], "ILI9481"))Option.DISPLAY_TYPE = ILI9481;
        if(checkstring(argv[0], "ILI9488"))Option.DISPLAY_TYPE = ILI9488;
        if(checkstring(argv[0], "ST7796S"))Option.DISPLAY_TYPE = ST7796S;
        if(checkstring(argv[2], "L") || checkstring(argv[2], "LANDSCAPE"))
            Option.DISPLAY_ORIENTATION = LANDSCAPE;
        else if(checkstring(argv[2], "P") || checkstring(argv[2], "PORTRAIT"))
            Option.DISPLAY_ORIENTATION = PORTRAIT;
        else if(checkstring(argv[2], "RL") || checkstring(argv[2], "RLANDSCAPE"))
            Option.DISPLAY_ORIENTATION = RLANDSCAPE;
        else if(checkstring(argv[2], "RP") || checkstring(argv[2], "RPORTRAIT"))
            Option.DISPLAY_ORIENTATION = RPORTRAIT;
        else
            error("Orientation");
        /*
        CheckPin(getinteger(argv[4]), CP_IGNORE_INUSE);

        CheckPin(getinteger(argv[6]), CP_IGNORE_INUSE);
        if(argc>=9 && *argv[8]){
            CheckPin(getinteger(argv[8]), CP_IGNORE_INUSE);
            Option.LCD_CS = getinteger(argv[8]);
        } else {
            Option.LCD_CS = 0;
        }
        */
    	if((code=codecheck(argv[4])))argv[4]+=2;
    	p1 = getinteger(argv[4]);
    	if(code)p1=codemap(code, p1);
    	if((code=codecheck(argv[6])))argv[6]+=2;
    	p2 = getinteger(argv[6]);
    	if(code)p2=codemap(code, p2);
        CheckPin(p1, CP_IGNORE_INUSE);
        CheckPin(p2, CP_IGNORE_INUSE);
        //if(argc >= 9) {
       	if(argc>=9 && *argv[8]){
        	if((code=codecheck(argv[8])))argv[8]+=2;
        	p3 = getinteger(argv[8]);
        	if(code)p3=codemap(code, p3);
            CheckPin(p3, CP_IGNORE_INUSE);
            Option.LCD_CS = p3;
        }else{
            Option.LCD_CS = 0;
        }
       	if(argc == 11){
        	if(checkstring(argv[10],"INVERT"))Option.BGR=1;
        }else{
        	Option.BGR=0;
        }
        //Option.LCD_CD = getinteger(argv[4]);
        //Option.LCD_Reset = getinteger(argv[6]);
        Option.LCD_CD = p1;
        Option.LCD_Reset = p2;
        Option.TOUCH_XZERO = TOUCH_NOT_CALIBRATED;                      // record the touch feature as not calibrated
        Option.Refresh = 1;
    }
} 
void InitDisplayOther(int fullinit){
    if(!(Option.DISPLAY_TYPE>SPI_PANEL && Option.DISPLAY_TYPE!=USER))return;
    {
    	if(Option.DISPLAY_TYPE  < SSD1963_5_BUFF){
    		screenbuff=(void *)0x30000000;
    		screenbuff2=(void *)0x38000000;
    	} else {
    		screenbuff2=(void *)0x30000000;
    		screenbuff3=(void *)0x38000000;
    	}
    	if(Option.DISPLAY_TYPE  < SSD1963_5_8BIT){
    		DrawRectangle = DrawRectangleBuff;
    		DrawBitmap = DrawBitmapBuff;
    		DrawBuffer = DrawBufferBuff;
    		ReadBuffer = ReadBufferBuff;
    		ScrollLCD = ScrollBuff;
    	} else {
        	DrawRectangle = DrawRectangleBuff8;
        	DrawBitmap = DrawBitmapBuff8;
        	DrawBuffer = DrawBufferBuff8;
        	ReadBuffer = ReadBufferBuff8;
        	ScrollLCD = ScrollBuff8;
    	}
    	if(Option.DISPLAY_TYPE == ILI9341){
    		if(fullinit){
    			SetAndReserve(Option.LCD_CD, P_OUTPUT, 1, EXT_BOOT_RESERVED);                            // config data/command as an output
    			SetAndReserve(Option.LCD_Reset, P_OUTPUT, 0, EXT_BOOT_RESERVED);                         // config reset as an output
    			if(Option.LCD_CS) SetAndReserve(Option.LCD_CS, P_OUTPUT, 1, EXT_BOOT_RESERVED);          // config chip select as an output
    			OpenSpiChannel();
    		}
    		DisplayHRes = 320;
    		DisplayVRes = 240;
    		ResetController();
    		spi_write_command(ILI9341_DISPLAYOFF);
    		spi_write_cd(ILI9341_POWERCONTROL1,1,0x23);
    		spi_write_cd(ILI9341_POWERCONTROL2,1,0x10);
    		spi_write_cd(ILI9341_VCOMCONTROL1,2,0x2B,0x2B);
    		spi_write_cd(ILI9341_VCOMCONTROL2,1,0xC0);
    		spi_write_cd(ILI9341_PIXELFORMAT,1,0x55);
    		spi_write_cd(ILI9341_FRAMECONTROL,2,0x00,0x1B);
    		spi_write_cd(ILI9341_ENTRYMODE,1,0x07);
    		if(Option.BGR)spi_write_cd(ILI9341_INVERTON,1,0);  //INVERT
    		spi_write_cd(ILI9341_SLEEPOUT,1,0);
    		HAL_Delay(50);
    		spi_write_command(ILI9341_NORMALDISP);
    		spi_write_command(ILI9341_DISPLAYON);
    		HAL_Delay(100);
    		switch(Option.DISPLAY_ORIENTATION) {
            	case LANDSCAPE:     spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Landscape); break;
            	case PORTRAIT:      spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Portrait); break;
            	case RLANDSCAPE:    spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Landscape180); break;
            	case RPORTRAIT:     spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Portrait180); break;
    		}
    		memswitch=500;
    		memswitch2=500;
    	} else if (Option.DISPLAY_TYPE == ILI9481 || Option.DISPLAY_TYPE == ILI9488 || Option.DISPLAY_TYPE == ST7796S)   {
    		if(fullinit){
    			SetAndReserve(Option.LCD_CD, P_OUTPUT, 1, EXT_BOOT_RESERVED);                            // config data/command as an output
    			SetAndReserve(Option.LCD_Reset, P_OUTPUT, 1, EXT_BOOT_RESERVED);                         // config reset as an output
    			if(Option.LCD_CS) SetAndReserve(Option.LCD_CS, P_OUTPUT, 1, EXT_BOOT_RESERVED);          // config chip select as an output
    			OpenSpiChannel();
    		}
    		DisplayHRes = 480;
    		DisplayVRes = 320;
    		ResetController();
    		if (Option.DISPLAY_TYPE == ILI9481) {
    		  spi_write_command(0x11);
    		  HAL_Delay(20);
    		  spi_write_cd(0xD0,3,0x07,0x42,0x18);
    		  spi_write_cd(0xD1,3,0x00,0x07,0x10);
    		  spi_write_cd(0xD2,2,0x01,0x02);
    		  spi_write_cd(0xC0,5,0x10,0x3B,0x00,0x02,0x11);
//            spi_write_cd(0xC1, 3,0x10, 0x12, 0xC8);
//            spi_write_cd(0xC5,1,0x01);
    		  spi_write_cd(0xB3,4,0x00,0x00,0x00,0x10);
    		  spi_write_cd(0xC8,12,0x00,0x32,0x36,0x45,0x06,0x16,0x37,0x75,0x77,0x54,0x0C,0x00);
    		  spi_write_cd(0xE0,15,0x0f,0x24,0x1c,0x0a,0x0f,0x08,0x43,0x88,0x03,0x0f,0x10,0x06,0x0f,0x07,0x00);
    		  spi_write_cd(0xE1,15,0x0F,0x38,0x30,0x09,0x0f,0x0f,0x4e,0x77,0x3c,0x07,0x10,0x05,0x23,0x1b,0x00);
    		  spi_write_cd(0x36,0x0A);
    		  spi_write_cd(0x3A,1,0x55);
    		  spi_write_cd(0x2A,4,0x00,0x00,0x01,0x3F);
    		  spi_write_cd(0x2B,4,0x00,0x00,0x01,0xE0);
    		  if(Option.BGR)spi_write_cd(ILI9341_INVERTON,1,0);  //INVERT
    		  HAL_Delay(120);
    		  spi_write_command(0x29);
    		}
    		if (Option.DISPLAY_TYPE == ILI9488) {

				spi_write_cd(0xe0,15,0x00,0x03,0x09,0x08,0x16,0x0a,0x3f,0x78,0x4c,0x09,0x0a,0x08,0x16,0x1a,0x0f); // positive Gamma Control
				spi_write_cd(0xe1,15,0x00,0x16,0x19,0x03,0x0f,0x05,0x32,0x45,0x46,0x04,0x0e,0x0d,0x35,0x37,0x0f);   // Negative Gamma Control
				spi_write_cd(0XC0,2,0x17,0x15);                // Power Control 1
				spi_write_cd(0xC1,1,0x41);                     // Power Control 2
				spi_write_cd(0xC5,3,0x00,0x12,0x80);           // VCOM Control
				spi_write_cd(0x36,1,0x48);                     // Memory Access Control  MX, BGR
				spi_write_cd(0x3A,1,0x66);                     // Pixel Interface Format // 18 bit colour for SPI 66
				spi_write_cd(0xB0,1,0x00);                     // Interface Mode Control
				spi_write_cd(0xB1,1,0xa0);                     // Frame Rate Control
				spi_write_cd(0xB4,1,0x02);                     // Display Inversion Control
			    spi_write_cd(0xB6,3,0x02,0x02,0x3B);           // Display Function Control
			    spi_write_cd(0xB7,1,0xc6);                     // Entry Mode Set
				spi_write_cd(0xF7,4,0xa9,0x51,0x2c,0x82);      // Adjust Control 3
				spi_write_command(ILI9341_NORMALDISP);
				//spi_write_command(ILI9341_INVERTON);  //INVERT
				if(Option.BGR)spi_write_cd(ILI9341_INVERTON,1,0);  //INVERT
				spi_write_command (0x11);                     //Exit Sleep
    		    HAL_Delay(120);
    		    spi_write_command(ILI9341_DISPLAYON);             //Display on
    		    HAL_Delay(25);                   //uSec(25000);
    		}
    		if (Option.DISPLAY_TYPE == ST7796S) {

		        spi_write_cd(0xC5, 1, 0x1C);             //VCOM  Control 1 [1C]
		        spi_write_cd(0x3A, 1, 0x55);              //565
		        spi_write_command(0xB0);              //Interface     [00]
		        HAL_Delay(150);
	            //0xB1, 2, 0xB0, 0x11,        //Frame Rate Control [A0 10]
		        spi_write_cd(0xB4, 1, 0x01);              //Inversion Control [01]
		        spi_write_cd(0xB6, 3, 0x80, 0x02, 0x3B);  // Display Function Control [80 02 3B] .kbv SS=1, NL=480
		        spi_write_cd(0xB7, 1, 0xC6);             //Entry Mode      [06]
	            //    0xF7, 4, 0xA9, 0x51, 0x2C, 0x82,    //Adjustment Control 3 [A9 51 2C 82]
		        spi_write_cd(0xF0, 1, 0xC3);              //?? lock manufacturer commands
		        spi_write_cd(0xF0, 1, 0x96);              //
                //		spi_write_cd(0xFB, 1, 0x3C);              //
		        if(Option.BGR)spi_write_cd(ILI9341_INVERTON,1,0);  //INVERT
		        spi_write_command(0x11);
		        HAL_Delay(150);
	            spi_write_command(0x29); //Display on
	            HAL_Delay(150);
    		}
    		switch(Option.DISPLAY_ORIENTATION) {
            	case LANDSCAPE:     spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Landscape); break;
            	case PORTRAIT:      spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Portrait); break;
            	case RLANDSCAPE:    spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Landscape180); break;
            	case RPORTRAIT:     spi_write_cd(ILI9341_MEMCONTROL,1,ILI9341_Portrait180); break;
    		}


    		if(fullinit){
    			if(Option.DISPLAY_ORIENTATION & 1) {
    				VRes=DisplayVRes;
    				HRes=DisplayHRes;
    			} else {
    				VRes=DisplayHRes;
    				HRes=DisplayVRes;
    			}
    			if(VRes>HRes){
    				memswitch=400;
    				memswitch2=500;
    			} else {
    				memswitch=300;
    				memswitch2=400;
    			}
    		}
    		SetFont(Option.DefaultFont);
    		PromptFont = gui_font;
    		PromptFC = gui_fcolour = Option.DefaultFC;
    		PromptBC = gui_bcolour = Option.DefaultBC;
    		ResetDisplay();
    		ClearScreen(gui_bcolour);
    		//ClearScreen(0xff00);
    		return;
    	} else if((Option.DISPLAY_TYPE >= SSD1963_5_BUFF)){
    		screenbuff=RAMBase-SavedMemoryBufferSize;
			DisplayVRes = 480;
    		if(Option.DISPLAY_TYPE >= SSD1963_5_640 && Option.DISPLAY_TYPE < SSD1963_5_8BIT){
    			DisplayHRes = 640;                                  // this is the 5" glass
    			memswitch=200;
    			memswitch2=200+230;
    			displayoffset=80;
    		} else if (Option.DISPLAY_TYPE >= SSD1963_5_8BIT){
    			DisplayHRes = 800;                                  // this is the 5" glass
    			memswitch=31;		// 480-81-368 needed in main memory
    			memswitch2=31+368;  //368 lines in 288K memory
    								// 81 lines in 64K memory
    			displayoffset=0;
     		} else {
    			DisplayHRes = 800;                                  // this is the 5" glass
    			memswitch=256;
    			memswitch2=256+184;
    			displayoffset=0;
    		}
    		if(fullinit){
    			SetAndReserve(SSD1963_DC_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);          // config data/command as an output
    			SetAndReserve(SSD1963_RESET_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);       // config reset as an output
    			SetAndReserve(SSD1963_RD_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_WR_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);  // The WR pin is fixed but make it an output

    			SetAndReserve(SSD1963_DAT1, P_OUTPUT, 0, EXT_BOOT_RESERVED);    // config the eight pins used for the data
    			SetAndReserve(SSD1963_DAT2, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT3, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT4, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT5, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT6, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT7, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT8, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT9, P_OUTPUT, 0, EXT_BOOT_RESERVED);    // config the eight pins used for the data
    			SetAndReserve(SSD1963_DAT10, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT11, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT12, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT13, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT14, P_OUTPUT, 0, EXT_BOOT_RESERVED);
            	SetAndReserve(SSD1963_DAT15, P_OUTPUT, 0, EXT_BOOT_RESERVED);
            	SetAndReserve(SSD1963_DAT16, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    		}
    		if(Option.DISPLAY_TYPE == SSD1963_5_640
    				|| Option.DISPLAY_TYPE == SSD1963_5_BUFF
    				|| Option.DISPLAY_TYPE == SSD1963_5_8BIT
					){
    			SSD1963HorizPulseWidth = 128;
    			SSD1963HorizBackPorch = 88;
    			SSD1963HorizFrontPorch = 40;
    			SSD1963VertPulseWidth = 2;
    			SSD1963VertBackPorch = 25;
    			SSD1963VertFrontPorch = 18;
    			//Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
    			//Typical DCLK is 33MHz.  30MHz = 120MHz*(LCDC_FPR+1)/2^20.  LCDC_FPR = 262143 (0x3FFFF)
    			SSD1963PClock1 = 0x03;
    			SSD1963PClock2 = 0xff;
    			SSD1963PClock3 = 0xff;
    			SSD1963Mode1 = 0x24;                                // 24-bit for 5" panel, data latch in falling edge for LSHIFT
    			SSD1963Mode2 = 0;                                   // Hsync+Vsync mode
    			SSD1963PixelInterface=3; //PIXEL data interface - 16-bit RGB565
    			SSD1963PixelFormat=0b01010000; //PIXEL data interface RGB565
    		} else if(Option.DISPLAY_TYPE == SSD1963_8_640
    				|| Option.DISPLAY_TYPE == SSD1963_8_BUFF
    				|| Option.DISPLAY_TYPE == SSD1963_8_8BIT
					){
                SSD1963HorizPulseWidth = 1;
                SSD1963HorizBackPorch = 210;
                SSD1963HorizFrontPorch = 45;
                SSD1963VertPulseWidth = 1;
                SSD1963VertBackPorch = 34;
                SSD1963VertFrontPorch = 10;
                //Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
                //Typical DCLK is 33.3MHz(datasheet), experiment shows 30MHz gives a stable result
                //30MHz = 120MHz*(LCDC_FPR+1)/2^20.  LCDC_FPR = 262143 (0x3FFFF)
                //Time per line = (DISP_HOR_RESOLUTION+DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH+DISP_HOR_FRONT_PORCH)/30 us = 1056/30 = 35.2us
                SSD1963PClock1 = 0x03;
                SSD1963PClock2 = 0xff;
                SSD1963PClock3 = 0xff;
                SSD1963Mode1 = 0x20;
                SSD1963Mode2 = 0x00;
    			SSD1963PixelInterface=3; //PIXEL data interface - 16-bit RGB565
    			SSD1963PixelFormat=0b01010000; //PIXEL data interface RGB565
    		} else {
    			SSD1963HorizPulseWidth = 1;
    			SSD1963HorizBackPorch = 210;
    			SSD1963HorizFrontPorch = 45;
    			SSD1963VertPulseWidth = 1;
    			SSD1963VertBackPorch = 34;
    			SSD1963VertFrontPorch = 10;
    			//Set LSHIFT freq, i.e. the DCLK with PLL freq 120MHz set previously
    			//Typical DCLK is 33.3MHz(datasheet), experiment shows 30MHz gives a stable result
    			//30MHz = 120MHz*(LCDC_FPR+1)/2^20.  LCDC_FPR = 262143 (0x3FFFF)
    			//Time per line = (DISP_HOR_RESOLUTION+DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH+DISP_HOR_FRONT_PORCH)/30 us = 1056/30 = 35.2us
    			SSD1963PClock1 = 0x03;
    			SSD1963PClock2 = 0xff;
    			SSD1963PClock3 = 0xff;
    			SSD1963Mode1 = 0x10;                                // 18-bit for 7" panel
    			SSD1963Mode2 = 0x80;                                // TTL mode
    			SSD1963PixelInterface=3; //PIXEL data interface - 16-bit RGB565
    			SSD1963PixelFormat=0b01010000; //PIXEL data interface RGB565
    		}
    		if(fullinit){
    			if(Option.DISPLAY_ORIENTATION & 1) {
    				VRes=DisplayVRes;
    				HRes=DisplayHRes;
    			} else {
    				VRes=DisplayHRes;
    				HRes=DisplayVRes;
    			}
    			if(VRes>HRes){
    				if(Option.DISPLAY_TYPE == SSD1963_5_640 || Option.DISPLAY_TYPE == SSD1963_7_640 ){
    					memswitch=266;
    					memswitch2=266+307;
    	    		} else if (Option.DISPLAY_TYPE >= SSD1963_5_8BIT){
    	    			DisplayHRes = 800;                                  // this is the 5" glass
    	    			memswitch=50;		// 480-81-368 needed in main memory
    	    			memswitch2=50+614;  //614 lines in 288K memory
    	    								// 136 lines in 64K memory
    	    			displayoffset=0;
    	    		} else {
    					memswitch=426;
    					memswitch2=426+307;
    				}
    			}
    		}
    		InitSSD1963();
    		SetFont(Option.DefaultFont);
    		PromptFont = gui_font;
    		PromptFC = gui_fcolour = Option.DefaultFC;
    		PromptBC = gui_bcolour = Option.DefaultBC;
    		ResetDisplay();
    		ClearScreen(gui_bcolour);
    		return;
    	} else if(Option.DISPLAY_TYPE == ILI9341_16) {
    		if(fullinit){
    			SetAndReserve(SSD1963_DC_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);          // config data/command as an output
    			SetAndReserve(SSD1963_RESET_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);       // config reset as an output
    			SetAndReserve(SSD1963_RD_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_WR_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);  // The WR pin is fixed but make it an output

    			SetAndReserve(SSD1963_DAT1, P_OUTPUT, 0, EXT_BOOT_RESERVED);    // config the eight pins used for the data
    			SetAndReserve(SSD1963_DAT2, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT3, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT4, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT5, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT6, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT7, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT8, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT9, P_OUTPUT, 0, EXT_BOOT_RESERVED);    // config the eight pins used for the data
    			SetAndReserve(SSD1963_DAT10, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT11, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT12, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT13, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT14, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT15, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT16, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    		}
    		HAL_Delay(100);
    		PinSetBit(SSD1963_RESET_PIN, LATCLR);
    		HAL_Delay(100);
    		PinSetBit(SSD1963_RESET_PIN, LATSET);
    		HAL_Delay(20);
    		if(Option.DISPLAY_TYPE == ILI9341_16){
    			DisplayHRes = 320;
    			DisplayVRes = 240;
    			P16_write_command(0xCB);
    			P16_write_data(0x39);
    			P16_write_data(0x2C);
    			P16_write_data(0x00);
    			P16_write_data(0x34);
    			P16_write_data(0x02);

    			P16_write_command(0xCF);
    			P16_write_data(0x00);
    			P16_write_data(0XC1);
    			P16_write_data(0X30);

    			P16_write_command(0xE8);
    			P16_write_data(0x85);
    			P16_write_data(0x00);
    			P16_write_data(0x78);

    			P16_write_command(0xEA);
    			P16_write_data(0x00);
    			P16_write_data(0x00);
 
    			P16_write_command(0xED);
    			P16_write_data(0x64);
    			P16_write_data(0x03);
    			P16_write_data(0X12);
    			P16_write_data(0X81);

    			P16_write_command(0xF7);
    			P16_write_data(0x20);
  
    			P16_write_command(0xC0);    //Power control
    			P16_write_data(0x23);   //VRH[5:0]
 
    			P16_write_command(0xC1);    //Power control
    			P16_write_data(0x10);   //SAP[2:0];BT[3:0]

    			P16_write_command(0xC5);    //VCM control
    			P16_write_data(0x3e);   //Contrast
    			P16_write_data(0x28);
 
    			P16_write_command(0xC7);    //VCM control2
    			P16_write_data(0x86);   //--
    			int i=0;
    			switch(Option.DISPLAY_ORIENTATION) {
                	case LANDSCAPE:     i=ILI9341_Landscape; break;
                	case PORTRAIT:      i=ILI9341_Portrait; break;
                	case RLANDSCAPE:    i=ILI9341_Landscape180; break;
                	case RPORTRAIT:     i=ILI9341_Portrait180; break;
    			}
    			P16_write_command(0x36);    // Memory Access Control
    			P16_write_data(i);

    			P16_write_command(0x3A);
    			P16_write_data(0x55);

    			P16_write_command(0xB1);
    			P16_write_data(0x00);
    			P16_write_data(0x18);
 
    			P16_write_command(0xB6);    // Display Function Control
    			P16_write_data(0x08);
    			P16_write_data(0x82);
    			P16_write_data(0x27);
 
    			P16_write_command(0xF2);    // 3Gamma Function Disable
    			P16_write_data(0x00);
 
    			P16_write_command(0x26);    //Gamma curve selected
    			P16_write_data(0x01);

    			P16_write_command(0xE0);    //Set Gamma
    			P16_write_data(0x0F);
    			P16_write_data(0x31);
    			P16_write_data(0x2B);
    			P16_write_data(0x0C);
    			P16_write_data(0x0E);
    			P16_write_data(0x08);
    			P16_write_data(0x4E);
    			P16_write_data(0xF1);
    			P16_write_data(0x37);
    			P16_write_data(0x07);
    			P16_write_data(0x10);
    			P16_write_data(0x03);
    			P16_write_data(0x0E);
    			P16_write_data(0x09);
    			P16_write_data(0x00);

    			P16_write_command(0XE1);    //Set Gamma
    			P16_write_data(0x00);
    			P16_write_data(0x0E);
    			P16_write_data(0x14);
    			P16_write_data(0x03);
    			P16_write_data(0x11);
    			P16_write_data(0x07);
    			P16_write_data(0x31);
    			P16_write_data(0xC1);
    			P16_write_data(0x48);
    			P16_write_data(0x08);
    			P16_write_data(0x0F);
    			P16_write_data(0x0C);
    			P16_write_data(0x31);
    			P16_write_data(0x36);
    			P16_write_data(0x0F);

    			P16_write_command(0x11);    //Exit Sleep
    			HAL_Delay(120);
				
    			P16_write_command(0x29);    //Display on
    			P16_write_command(0x2c);
    			memswitch=500;
    			memswitch2=500;
    		}
    	} else if(Option.DISPLAY_TYPE == ILI9341_8) {
    		if(fullinit){
    			SetAndReserve(SSD1963_DC_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);          // config data/command as an output
    			SetAndReserve(SSD1963_RESET_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);       // config reset as an output
    			SetAndReserve(SSD1963_RD_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_WR_PIN, P_OUTPUT, 1, EXT_BOOT_RESERVED);  // The WR pin is fixed but make it an output

    			SetAndReserve(SSD1963_DAT1, P_OUTPUT, 0, EXT_BOOT_RESERVED);    // config the eight pins used for the data
    			SetAndReserve(SSD1963_DAT2, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT3, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT4, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT5, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT6, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT7, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT8, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT9, P_OUTPUT, 0, EXT_BOOT_RESERVED);    // config the eight pins used for the data
    			SetAndReserve(SSD1963_DAT10, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT11, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT12, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT13, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT14, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT15, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    			SetAndReserve(SSD1963_DAT16, P_OUTPUT, 0, EXT_BOOT_RESERVED);
    		}
    		HAL_Delay(100);
    		PinSetBit(SSD1963_RESET_PIN, LATCLR);
    		HAL_Delay(100);
    		PinSetBit(SSD1963_RESET_PIN, LATSET);
    		HAL_Delay(20);
    		if(Option.DISPLAY_TYPE == ILI9341_8){
    			DisplayHRes = 320;
    			DisplayVRes = 240;
    			P16_write_command(0xcf);
    			P16_write_data(0x00);
    			P16_write_data(0xc1);
    			P16_write_data(0x30);

    			P16_write_command(0xed);
    			P16_write_data(0x64);
    			P16_write_data(0x03);
    			P16_write_data(0x12);
    			P16_write_data(0x81);

    			P16_write_command(0xcb);
    			P16_write_data(0x39);
    			P16_write_data(0x2c);
    			P16_write_data(0x00);
    			P16_write_data(0x34);
    			P16_write_data(0x02);

    			P16_write_command(0xea);
    			P16_write_data(0x00);
    			P16_write_data(0x00);

    			P16_write_command(0xe8);
    			P16_write_data(0x85);
    			P16_write_data(0x10);
    			P16_write_data(0x79);

    			P16_write_command(0xC0); //Power control
    			P16_write_data(0x23); //VRH[5:0]

    			P16_write_command(0xC1); //Power control
    			P16_write_data(0x11); //SAP[2:0];BT[3:0]

    			P16_write_command(0xC2);
    			P16_write_data(0x11);

    			P16_write_command(0xC5); //VCM control
    			P16_write_data(0x3d);
    			P16_write_data(0x30);

    			P16_write_command(0xc7);
    			P16_write_data(0xaa);

    			P16_write_command(0x3A);
    			P16_write_data(0x55);

    			P16_write_command(0x36); // Memory Access Control
    			P16_write_data(0x08);

    			P16_write_command(0xB1); // Frame Rate Control
    			P16_write_data(0x00);
    			P16_write_data(0x11);

    			P16_write_command(0xB6); // Display Function Control
    			P16_write_data(0x0a);
    			P16_write_data(0xa2);

    			P16_write_command(0xF2); // 3Gamma Function Disable
    			P16_write_data(0x00);

    			P16_write_command(0xF7);
    			P16_write_data(0x20);

    			P16_write_command(0xF1);
    			P16_write_data(0x01);
    			P16_write_data(0x30);

    			P16_write_command(0x26); //Gamma curve selected
    			P16_write_data(0x01);

    			P16_write_command(0xE0); //Set Gamma
    			P16_write_data(0x0f);
    			P16_write_data(0x3f);
    			P16_write_data(0x2f);
    			P16_write_data(0x0c);
    			P16_write_data(0x10);
    			P16_write_data(0x0a);
    			P16_write_data(0x53);
    			P16_write_data(0xd5);
    			P16_write_data(0x40);
    			P16_write_data(0x0a);
    			P16_write_data(0x13);
    			P16_write_data(0x03);
    			P16_write_data(0x08);
    			P16_write_data(0x03);
    			P16_write_data(0x00);

    			P16_write_command(0xE1); //Set Gamma
    			P16_write_data(0x00);
    			P16_write_data(0x00);
    			P16_write_data(0x10);
    			P16_write_data(0x03);
    			P16_write_data(0x0f);
    			P16_write_data(0x05);
    			P16_write_data(0x2c);
    			P16_write_data(0xa2);
    			P16_write_data(0x3f);
    			P16_write_data(0x05);
    			P16_write_data(0x0e);
    			P16_write_data(0x0c);
    			P16_write_data(0x37);
    			P16_write_data(0x3c);
    			P16_write_data(0x0F);
    			P16_write_command(0x11); //Exit Sleep
    			int i=0;
    			switch(Option.DISPLAY_ORIENTATION) {
                	case LANDSCAPE:     i=ILI9341_8_Landscape; break;
                	case PORTRAIT:      i=ILI9341_8_Portrait; break;
                	case RLANDSCAPE:    i=ILI9341_8_Landscape180; break;
                	case RPORTRAIT:     i=ILI9341_8_Portrait180; break;
    			}
    			P16_write_command(0x36);    // Memory Access Control
    			P16_write_data(i);
    			HAL_Delay(120);
    			P16_write_command(0x21); //display inverted
    			P16_write_command(0x29); //display on
    			P16_write_command(0x2c); //display on
    			HAL_Delay(50);
    			memswitch=500;
    			memswitch2=500;
    		}
    	} else return;
    }
    
    if(fullinit){
        if(Option.DISPLAY_ORIENTATION & 1) {
            VRes=DisplayVRes;
            HRes=DisplayHRes;
        } else {
            VRes=DisplayHRes;
            HRes=DisplayVRes;
        }
    }
    SetFont(Option.DefaultFont);
    PromptFont = gui_font;
    PromptFC = gui_fcolour = Option.DefaultFC;
    PromptBC = gui_bcolour = Option.DefaultBC;
    ResetDisplay();
    ClearScreen(gui_bcolour);
}
void DefineRegionP(int xstart, int ystart, int xend, int yend, int rw) {
    if(HRes == 0) error("Display not configured");
    P16_write_command_ZI2(ILI9341_COLADDRSET);
    P16_write_data_ZI2(xstart >> 8);
    P16_write_data_ZI2(xstart);
    P16_write_data_ZI2(xend >> 8);
    P16_write_data_ZI2(xend);
    P16_write_command_ZI2(ILI9341_PAGEADDRSET);
    P16_write_data_ZI2(ystart >> 8);
    P16_write_data_ZI2(ystart);
    P16_write_data_ZI2(yend >> 8);
    P16_write_data_ZI2(yend);
    if(rw){
        P16_write_command_ZI2(ILI9341_MEMORYWRITE);
    } else {
        P16_write_command_ZI2(ILI9341_RAMRD);
    }
}

void DefineRegionP_VIT6(int xstart, int ystart, int xend, int yend, int rw) {
    if(HRes == 0) error("Display not configured");
    P16_write_command_VIT6(ILI9341_COLADDRSET);
    P16_write_data_VIT6(xstart >> 8);
    P16_write_data_VIT6(xstart);
    P16_write_data_VIT6(xend >> 8);
    P16_write_data_VIT6(xend);
    P16_write_command_VIT6(ILI9341_PAGEADDRSET);
    P16_write_data_VIT6(ystart >> 8);
    P16_write_data_VIT6(ystart);
    P16_write_data_VIT6(yend >> 8);
    P16_write_data_VIT6(yend);
    if(rw){
        P16_write_command_VIT6(ILI9341_MEMORYWRITE);
    } else {
        P16_write_command_VIT6(ILI9341_RAMRD);
    }
}


/****************************************************************************************************
 ****************************************************************************************************

 Basic drawing primitives
 all drawing on the LCD is done using either one of these two functions

 ****************************************************************************************************
****************************************************************************************************/
void ScrollBuff(int lines){
	unsigned char *s,*d;
	int y, yy;
	if(lines>0){
		for(y=0;y<VRes-lines;y++){
			yy=y+lines;
			if(y<memswitch){
				d=(y * HRes) * 2 + screenbuff;
			} else if(y<memswitch2){
				d=((y-memswitch) * HRes) * 2 + screenbuff2;
			} else {
				d=((y-memswitch2) * HRes) * 2 + screenbuff3;
			}
			if(yy<memswitch){
				s=(yy * HRes) * 2 + screenbuff;
			} else if(yy<memswitch2){
				s=((yy-memswitch) * HRes) * 2 + screenbuff2;
			} else {
				s=((yy-memswitch2) * HRes) * 2 + screenbuff3;
			}
			memcpy(d, s, HRes*2);
		}
        DrawRectangle(0, VRes-lines, HRes - 1, VRes - 1, gui_bcolour); // erase the line to be scrolled off
        low_y=0; high_y=VRes-1; low_x=0; high_x=HRes-1;
        Display_Refresh();
    } else if(lines<0){
    	lines=-lines;
    	for(y=VRes-1;y>=lines;y--){
			yy=y-lines;
			if(y<memswitch){
				d=(y * HRes) * 2 + screenbuff;
			} else if(y<memswitch2){
				d=((y-memswitch) * HRes) * 2 + screenbuff2;
			} else {
				d=((y-memswitch2) * HRes) * 2 + screenbuff3;
			}
			if(yy<memswitch){
				s=(yy * HRes) * 2 + screenbuff;
			} else if(yy<memswitch2){
				s=((yy-memswitch) * HRes) * 2 + screenbuff2;
			} else {
				s=((yy-memswitch2) * HRes) * 2 + screenbuff3;
			}
			memcpy(d, s, HRes*2);
		}
        DrawRectangle(0, 0, HRes - 1, lines - 1, gui_bcolour); // erase the line to be scrolled off
        low_y=0; high_y=VRes-1; low_x=0; high_x=HRes-1;
        Display_Refresh();
    }
}
void ScrollBuff8(int lines){
	unsigned char *s,*d;
	int y, yy;
	if(lines>0){
		for(y=0;y<VRes-lines;y++){
			yy=y+lines;
			if(y<memswitch){
				d=(y * HRes) + screenbuff;
			} else if(y<memswitch2){
				d=((y-memswitch) * HRes) + screenbuff2;
			} else {
				d=((y-memswitch2) * HRes) + screenbuff3;
			}
			if(yy<memswitch){
				s=(yy * HRes) + screenbuff;
			} else if(yy<memswitch2){
				s=((yy-memswitch) * HRes) + screenbuff2;
			} else {
				s=((yy-memswitch2) * HRes) + screenbuff3;
			}
			memcpy(d, s, HRes);
		}
        DrawRectangle(0, VRes-lines, HRes - 1, VRes - 1, gui_bcolour); // erase the line to be scrolled off
        low_y=0; high_y=VRes-1; low_x=0; high_x=HRes-1;
        Display_Refresh();
    } else if(lines<0){
    	lines=-lines;
    	for(y=VRes-1;y>=lines;y--){
			yy=y-lines;
			if(y<memswitch){
				d=(y * HRes) + screenbuff;
			} else if(y<memswitch2){
				d=((y-memswitch) * HRes) + screenbuff2;
			} else {
				d=((y-memswitch2) * HRes) + screenbuff3;
			}
			if(yy<memswitch){
				s=(yy * HRes) + screenbuff;
			} else if(yy<memswitch2){
				s=((yy-memswitch) * HRes) + screenbuff2;
			} else {
				s=((yy-memswitch2) * HRes) + screenbuff3;
			}
			memcpy(d, s, HRes);
		}
        DrawRectangle(0, 0, HRes - 1, lines - 1, gui_bcolour); // erase the line to be scrolled off
        low_y=0; high_y=VRes-1; low_x=0; high_x=HRes-1;
        Display_Refresh();
    }
}

void DrawRectangleBuff(int x1, int y1, int x2, int y2, int c){
    int y, x,  t;
    uint16_t *s, sc;
    char hb, lb;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(x1 < 0) x1 = 0; 
    if(x1 >= HRes) x1 = HRes - 1;
    if(x2 < 0) x2 = 0; 
    if(x2 >= HRes) x2 = HRes - 1;
    if(y1 < 0) y1 = 0; 
    if(y1 >= VRes) y1 = VRes - 1;
    if(y2 < 0) y2 = 0; 
    if(y2 >= VRes) y2 = VRes - 1;
    if(y1<low_y)low_y=y1;
    if(y2>high_y)high_y=y2;
    if(x1<low_x)low_x=x1;
    if(x2>high_x)high_x=x2;
    // convert the colours to 565 format
    lb = ((c >> 16) & 0b11111000) | ((c >> 13) & 0b00000111);
    hb = ((c >> 5) & 0b11100000) | ((c >> 3) & 0b00011111);
    sc=hb<<8|lb;
    for(y=y1;y<=y2;y++){
        if(y<memswitch){
        	s=(uint16_t *)((y * HRes + x1) * 2 + screenbuff);
        } else if(y<memswitch2){
        	s=(uint16_t *)(((y-memswitch) * HRes + x1) * 2 + screenbuff2);
        } else {
        	s=(uint16_t *)(((y-memswitch2) * HRes + x1) * 2 + screenbuff3);
        }
        for(x=x1;x<=x2;x++){
        	*s++=sc;
        }
    }
}

void DrawRectangleBuff8(int x1, int y1, int x2, int y2, int c){
    int y, x,  t;
    uint8_t *s, sc;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    if(x1 < 0) x1 = 0;
    if(x1 >= HRes) x1 = HRes - 1;
    if(x2 < 0) x2 = 0;
    if(x2 >= HRes) x2 = HRes - 1;
    if(y1 < 0) y1 = 0;
    if(y1 >= VRes) y1 = VRes - 1;
    if(y2 < 0) y2 = 0;
    if(y2 >= VRes) y2 = VRes - 1;
    if(y1<low_y)low_y=y1;
    if(y2>high_y)high_y=y2;
    if(x1<low_x)low_x=x1;
    if(x2>high_x)high_x=x2;
    // convert the colours to 222 format
    sc = (((c >> 16) & 0b11000000)>>2) | (((c >> 8) & 0b11000000)>>4) | ((c & 0b11000000)>>6);
    for(y=y1;y<=y2;y++){
        if(y<memswitch){
        	s=(uint8_t *)((y * HRes + x1) + screenbuff);
        } else if(y<memswitch2){
        	s=(uint8_t *)(((y-memswitch) * HRes + x1)+ screenbuff2);
        } else {
        	s=(uint8_t *)(((y-memswitch2) * HRes + x1) + screenbuff3);
        }
        for(x=x1;x<=x2;x++){
        	*s++=sc;
        }
    }
}



void ReadBufferBuff(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
    unsigned char hb, lb, *s;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
        for(y=y1;y<=y2;y++){
            if(y<memswitch){
            	s=(y * HRes + x1) * 2 + screenbuff;
            } else if(y<memswitch2){
            	s=((y-memswitch) * HRes + x1) * 2 + screenbuff2;
            } else {
            	s=((y-memswitch2) * HRes + x1) * 2 + screenbuff3;
            }
        	for(x=x1;x<=x2;x++){
        		if(x>=0 && x<HRes && y>=0 && y<VRes){
        			hb=*s++;
        			lb=*s++;
        			*p++ = ((lb & 0x1F) << 3);
        			*p++ = ((hb & 7) << 5) | ((lb & 0b11100000)  >> 3);   //masked out b4 to b0
        			*p++ = hb & 0xF8;
        		} else {
        			s+=2;
        			p+=3;
        		}
        	}
        }
}

void ReadBufferBuff8(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
    unsigned char sc, *s;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
        for(y=y1;y<=y2;y++){
            if(y<memswitch){
            	s=(y * HRes + x1) + screenbuff;
            } else if(y<memswitch2){
            	s=((y-memswitch) * HRes + x1) + screenbuff2;
            } else {
            	s=((y-memswitch2) * HRes + x1) + screenbuff3;
            }
        	for(x=x1;x<=x2;x++){
        		if(x>=0 && x<HRes && y>=0 && y<VRes){
        			sc=*s++;
        			*p++ = ((sc & 0b00110000) << 2);
        			*p++ = ((sc & 0b00001100) << 4);
        			*p++ = ((sc & 0b00000011) << 6);
        		} else {
        			s++;
        			p+=3;
        		}
        	}
        }
}

void ReadBufferBuffFast(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
    unsigned char *s;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
        for(y=y1;y<=y2;y++){
            if(y<memswitch){
            	s=(y * HRes + x1) * 2 + screenbuff;
            } else if(y<memswitch2){
            	s=((y-memswitch) * HRes + x1) * 2 + screenbuff2;
            } else {
            	s=((y-memswitch2) * HRes + x1) * 2 + screenbuff3;
            }
        	for(x=x1;x<=x2;x++){
        		if(x>=0 && x<HRes && y>=0 && y<VRes){
        			*p++ = *s++;
        			*p++ = *s++;
        		} else {
        			s+=2;
        			p+=2;
        		}
        	}
        }
}
void ReadBufferBuffFast8(int x1, int y1, int x2, int y2, char* p) {
    int t,x,y;
    unsigned char *s;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
        for(y=y1;y<=y2;y++){
            if(y<memswitch){
            	s=(y * HRes + x1) + screenbuff;
            } else if(y<memswitch2){
            	s=((y-memswitch) * HRes + x1) + screenbuff2;
            } else {
            	s=((y-memswitch2) * HRes + x1) + screenbuff3;
            }
        	for(x=x1;x<=x2;x++){
        		if(x>=0 && x<HRes && y>=0 && y<VRes){
        			*p++ = *s++;
        		} else {
        			s++;
        			p++;
        		}
        	}
        }
}

void DrawBufferBuff(int x1, int y1, int x2, int y2, char* p) {
    int x, y, t;
    char hb, lb;
    unsigned char *s;
    union colourmap
    {
    char rgbbytes[4];
    uint32_t rgb;
    } c;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    int xx1=x1, yy1=y1, xx2=x2, yy2=y2;
    if(x1 < 0) xx1 = 0;
    if(x1 >= HRes) xx1 = HRes - 1;
    if(x2 < 0) xx2 = 0;
    if(x2 >= HRes) xx2 = HRes - 1;
    if(y1 < 0) yy1 = 0;
    if(y1 >= VRes) yy1 = VRes - 1;
    if(y2 < 0) yy2 = 0;
    if(y2 >= VRes) yy2 = VRes - 1;
    if(yy1<low_y)low_y=yy1;
    if(yy2>high_y)high_y=yy2;
    if(xx1<low_x)low_x=xx1;
    if(xx2>high_x)high_x=xx2;
    t=0;
    for(y=y1;y<=y2;y++){
        if(y<memswitch){
        	s=(y * HRes + x1) * 2 + screenbuff;
        } else if(y<memswitch2){
        	s=((y-memswitch) * HRes + x1) * 2 + screenbuff2;
        } else {
        	s=((y-memswitch2) * HRes + x1) * 2 + screenbuff3;
        }
        for(x=x1;x<=x2;x++){
        	if(x>=0 && x<HRes && y>=0 && y<VRes){
        		c.rgbbytes[3]=0;
        		c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
        		c.rgbbytes[1]=*p++;
        		c.rgbbytes[2]=*p++;
    // convert the colours to 565 format
        		lb = (((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111));
        		hb = (((c.rgb >> 5) & 0b11100000) | ((c.rgb >> 3) & 0b00011111));
        		*s++=lb;
        		*s++=hb;
        	} else {
        		s+=2;
        		p+=3;
        	}
        }
    } 
}
void DrawBufferBuff8(int x1, int y1, int x2, int y2, char* p) {
    int x, y, t;
    unsigned char *s;
    union colourmap
    {
    char rgbbytes[4];
    uint32_t rgb;
    } c;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    int xx1=x1, yy1=y1, xx2=x2, yy2=y2;
    if(x1 < 0) xx1 = 0;
    if(x1 >= HRes) xx1 = HRes - 1;
    if(x2 < 0) xx2 = 0;
    if(x2 >= HRes) xx2 = HRes - 1;
    if(y1 < 0) yy1 = 0;
    if(y1 >= VRes) yy1 = VRes - 1;
    if(y2 < 0) yy2 = 0;
    if(y2 >= VRes) yy2 = VRes - 1;
    if(yy1<low_y)low_y=yy1;
    if(yy2>high_y)high_y=yy2;
    if(xx1<low_x)low_x=xx1;
    if(xx2>high_x)high_x=xx2;
    t=0;
    for(y=y1;y<=y2;y++){
        if(y<memswitch){
        	s=(y * HRes + x1) + screenbuff;
        } else if(y<memswitch2){
        	s=((y-memswitch) * HRes + x1) + screenbuff2;
        } else {
        	s=((y-memswitch2) * HRes + x1) + screenbuff3;
        }
        for(x=x1;x<=x2;x++){
        	if(x>=0 && x<HRes && y>=0 && y<VRes){
        		c.rgbbytes[3]=0;
        		c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
        		c.rgbbytes[1]=*p++;
        		c.rgbbytes[2]=*p++;
    // convert the colours to 565 format
        		*s++ = (((c.rgb >> 16) & 0b11000000)>>2) | (((c.rgb >> 8) & 0b11000000)>>4) | ((c.rgb & 0b11000000)>>6);
        	} else {
        		s++;
        		p+=3;
        	}
        }
    }
}
void DrawBufferBuffFast(int x1, int y1, int x2, int y2, char* p) {
    int x, y, t;
    unsigned char *s;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    int xx1=x1, yy1=y1, xx2=x2, yy2=y2;
    if(x1 < 0) xx1 = 0;
    if(x1 >= HRes) xx1 = HRes - 1;
    if(x2 < 0) xx2 = 0;
    if(x2 >= HRes) xx2 = HRes - 1;
    if(y1 < 0) yy1 = 0;
    if(y1 >= VRes) yy1 = VRes - 1;
    if(y2 < 0) yy2 = 0;
    if(y2 >= VRes) yy2 = VRes - 1;
    if(yy1<low_y)low_y=yy1;
    if(yy2>high_y)high_y=yy2;
    if(xx1<low_x)low_x=xx1;
    if(xx2>high_x)high_x=xx2;
    for(y=y1;y<=y2;y++){
        if(y<memswitch){
        	s=(y * HRes + x1) * 2 + screenbuff;
        } else if(y<memswitch2){
        	s=((y-memswitch) * HRes + x1) * 2 + screenbuff2;
        } else {
        	s=((y-memswitch2) * HRes + x1) * 2 + screenbuff3;
        }
        for(x=x1;x<=x2;x++){
            if(x>=0 && x<HRes && y>=0 && y<VRes){
                *s++=*p++; //this order swaps the bytes to match the .BMP file
                *s++=*p++;
            } else {
                s+=2;
                p+=2;
            }
        }
    }

}
void DrawBufferBuffFast8(int x1, int y1, int x2, int y2, char* p) {
    int x, y, t;
    unsigned char *s;
    // make sure the coordinates are kept within the display area
    if(x2 <= x1) { t = x1; x1 = x2; x2 = t; }
    if(y2 <= y1) { t = y1; y1 = y2; y2 = t; }
    int xx1=x1, yy1=y1, xx2=x2, yy2=y2;
    if(x1 < 0) xx1 = 0;
    if(x1 >= HRes) xx1 = HRes - 1;
    if(x2 < 0) xx2 = 0;
    if(x2 >= HRes) xx2 = HRes - 1;
    if(y1 < 0) yy1 = 0;
    if(y1 >= VRes) yy1 = VRes - 1;
    if(y2 < 0) yy2 = 0;
    if(y2 >= VRes) yy2 = VRes - 1;
    if(yy1<low_y)low_y=yy1;
    if(yy2>high_y)high_y=yy2;
    if(xx1<low_x)low_x=xx1;
    if(xx2>high_x)high_x=xx2;
    for(y=y1;y<=y2;y++){
        if(y<memswitch){
        	s=(y * HRes + x1) + screenbuff;
        } else if(y<memswitch2){
        	s=((y-memswitch) * HRes + x1) + screenbuff2;
        } else {
        	s=((y-memswitch2) * HRes + x1) + screenbuff3;
        }
        for(x=x1;x<=x2;x++){
            if(x>=0 && x<HRes && y>=0 && y<VRes){
                *s++=*p++;
            } else {
                s++;
                p++;
            }
        }
    }

}
//Print the bitmap of a char on the video output
//    x, y - the top left of the char
//    width, height - size of the char's bitmap
//    scale - how much to scale the bitmap
//	  fc, bc - foreground and background colour
//    bitmap - pointer to the bitmap
void DrawBitmapBuff(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap){
    int i, j, k, m, n, t, x, y;
    int vertCoord, horizCoord, XStart, XEnd, YEnd;
    char *p=0;
    unsigned char f[2],b[2];
    union colourmap {
    unsigned char rgbbytes[4];
    unsigned int rgb;
    } c;
    bc|=0x180C18;
    // adjust when part of the bitmap is outside the displayable coordinates
    if(x1>=HRes || y1>=VRes || x1+width*scale<0 || y1+height*scale<0)return;
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= HRes) XEnd = HRes - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1; if(YEnd >= VRes) YEnd = VRes - 1;// the height of the bitmap will extend beyond the bottom margin
        if(bc == -1) {                                                     //special case of overlay text
            i = 0;
            j = width * height * scale * scale * 3;
            p = GetMemory(j);                                              //allocate some temporary memory
            ReadBuffer(XStart, y1, XEnd, YEnd, p);
        }
    // convert the colours to 565 format
        f[0]= ((fc >> 16) & 0b11111000) | ((fc >> 13) & 0b00000111);
        f[1] = ((fc >>  5) & 0b11100000) | ((fc >>  3) & 0b00011111);
        b[0] = ((bc >> 16) & 0b11111000) | ((bc >> 13) & 0b00000111);
        b[1] = ((bc >>  5) & 0b11100000) | ((bc >>  3) & 0b00011111);
        if(y1<low_y)low_y=y1;
        if(YEnd>high_y)high_y=YEnd;
        if(XStart<low_x)low_x=XStart;
        if(XEnd>high_x)high_x=XEnd;
        // switch to SPI enhanced mode for the bulk transfer
        t = n = 0;
        for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
            for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
                y=vertCoord;
                if(vertCoord++ < 0) continue;                           // we are above the top of the screen
                if(vertCoord > VRes) {                                  // we have extended beyond the bottom of the screen
                    if(p != NULL) FreeMemory(p);
                    return; 
                }                            
                horizCoord = x1;
                for(k = 0; k < width; k++) {                            // step through each bit in a scan line
                    for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
                        x=horizCoord;
                        if(horizCoord++ < 0) continue;                  // we have not reached the left margin
                        if(horizCoord > HRes) continue;                 // we are beyond the right margin
                        if(y<memswitch){
                        	t= (y * (HRes<<1)) + (x<<1);
                        	if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        		screenbuff[t++]=f[0];
                        		screenbuff[t]=f[1];
                        	} else {
                        		if(bc == -1){
                        			c.rgbbytes[0] = p[n];
                        			c.rgbbytes[1] = p[n+1];
                        			c.rgbbytes[2] = p[n+2];
                        			c.rgb|=0x180C18;
                        			screenbuff[t++] = ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
                        			screenbuff[t] =((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
                        		} else {
                        			screenbuff[t++]=b[0];
                        			screenbuff[t]=b[1];
                        		}
                        	}
                        	n += 3;
                        } else if(y<memswitch2){
                        	t= ((y-memswitch) * (HRes<<1)) + (x<<1);
                        	if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        		screenbuff2[t++]=f[0];
                        		screenbuff2[t]=f[1];
                        	} else {
                        		if(bc == -1){
                        			c.rgbbytes[0] = p[n];
                        			c.rgbbytes[1] = p[n+1];
                        			c.rgbbytes[2] = p[n+2];
                        			screenbuff2[t++] = ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
                        			screenbuff2[t] =((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
                        		} else {
                        			screenbuff2[t++]=b[0];
                        			screenbuff2[t]=b[1];
                        		}
                        	}
                        	n += 3;
                        } else {
                        	t= ((y-memswitch2) * (HRes<<1)) + (x<<1);
                        	if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        		screenbuff3[t++]=f[0];
                        		screenbuff3[t]=f[1];
                        	} else {
                        		if(bc == -1){
                        			c.rgbbytes[0] = p[n];
                        			c.rgbbytes[1] = p[n+1];
                        			c.rgbbytes[2] = p[n+2];
                        			screenbuff3[t++] = ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
                        			screenbuff3[t] =((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
                        		} else {
                        			screenbuff3[t++]=b[0];
                        			screenbuff3[t]=b[1];
                        		}
                        	}
                        	n += 3;
                        }
                    }
                }
            }
        }
    if(p != NULL) FreeMemory(p);
}
void DrawBitmapBuff8(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap){
    int i, j, k, m, n, t, x, y;
    int vertCoord, horizCoord, XStart, XEnd, YEnd;
    char *p=0;
    unsigned char f,b;
    union colourmap {
    unsigned char rgbbytes[4];
    unsigned int rgb;
    } c;
    bc|=0x180C18;
    // adjust when part of the bitmap is outside the displayable coordinates
    if(x1>=HRes || y1>=VRes || x1+width*scale<0 || y1+height*scale<0)return;
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= HRes) XEnd = HRes - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1; if(YEnd >= VRes) YEnd = VRes - 1;// the height of the bitmap will extend beyond the bottom margin
        if(bc == -1) {                                                     //special case of overlay text
            i = 0;
            j = width * height * scale * scale * 3;
            p = GetMemory(j);                                              //allocate some temporary memory
            ReadBuffer(XStart, y1, XEnd, YEnd, p);
        }
    // convert the colours to 565 format
	    f = (((fc >> 16) & 0b11000000)>>2) | (((fc >> 8) & 0b11000000)>>4) | ((fc & 0b11000000)>>6);
	    b = (((bc >> 16) & 0b11000000)>>2) | (((bc >> 8) & 0b11000000)>>4) | ((bc & 0b11000000)>>6);
        if(y1<low_y)low_y=y1;
        if(YEnd>high_y)high_y=YEnd;
        if(XStart<low_x)low_x=XStart;
        if(XEnd>high_x)high_x=XEnd;
        // switch to SPI enhanced mode for the bulk transfer
        t = n = 0;
        for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
            for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
                y=vertCoord;
                if(vertCoord++ < 0) continue;                           // we are above the top of the screen
                if(vertCoord > VRes) {                                  // we have extended beyond the bottom of the screen
                    if(p != NULL) FreeMemory(p);
                    return;
                }
                horizCoord = x1;
                for(k = 0; k < width; k++) {                            // step through each bit in a scan line
                    for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
                        x=horizCoord;
                        if(horizCoord++ < 0) continue;                  // we have not reached the left margin
                        if(horizCoord > HRes) continue;                 // we are beyond the right margin
                        if(y<memswitch){
                        	t= y * HRes + x;
                        	if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        		screenbuff[t]=f;
                        	} else {
                        		if(bc == -1){
                        			c.rgbbytes[0] = p[n];
                        			c.rgbbytes[1] = p[n+1];
                        			c.rgbbytes[2] = p[n+2];
//                        			c.rgb|=0x180C18;
                        			screenbuff[t] = (((c.rgb >> 16) & 0b11000000) | (((c.rgb >> 8) & 0b11000000)>>2) | ((c.rgb & 0b11000000)>>4))>>2;
                        		} else {
                        			screenbuff[t]=b;
                        		}
                        	}
                        	n += 3;
                        } else if(y<memswitch2){
                        	t= (y-memswitch) * HRes + x;
                        	if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        		screenbuff2[t]=f;
                        	} else {
                        		if(bc == -1){
                        			c.rgbbytes[0] = p[n];
                        			c.rgbbytes[1] = p[n+1];
                        			c.rgbbytes[2] = p[n+2];
                        			screenbuff2[t] = (((c.rgb >> 16) & 0b11000000) | (((c.rgb >> 8) & 0b11000000)>>2) | ((c.rgb & 0b11000000)>>4))>>2;
                        		} else {
                        			screenbuff2[t]=b;
                        		}
                        	}
                        	n += 3;
                        } else {
                        	t= (y-memswitch2) * HRes + x;
                        	if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        		screenbuff3[t]=f;
                        	} else {
                        		if(bc == -1){
                        			c.rgbbytes[0] = p[n];
                        			c.rgbbytes[1] = p[n+1];
                        			c.rgbbytes[2] = p[n+2];
                        			screenbuff3[t] = (((c.rgb >> 16) & 0b11000000) | (((c.rgb >> 8) & 0b11000000)>>2) | ((c.rgb & 0b11000000)>>4))>>2;
                        		} else {
                        			screenbuff3[t]=b;
                       		}
                        	}
                        	n += 3;
                        }
                    }
                }
            }
        }
    if(p != NULL) FreeMemory(p);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  wTransferState = TRANSFER_COMPLETE;
}
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  wTransferState = TRANSFER_ERROR;
}
/*
void Display_Refresh(void){
    uint8_t *t=NULL;
    const int map[64]={
    		0x0000 | 0x0000 | 0x0000,//0b000000
			0x0007 | 0x0000 | 0x0000,//0b000001
			0x000F | 0x0000 | 0x0000,//0b000010
			0x001F | 0x0000 | 0x0000,//0b000011
    		0x0000 | 0x01E0 | 0x0000,//0b000100
			0x0007 | 0x01E0 | 0x0000,//0b000101
			0x000F | 0x01E0 | 0x0000,//0b000110
			0x001F | 0x01E0 | 0x0000,//0b000111
    		0x0000 | 0x03E0 | 0x0000,//0b001000
			0x0007 | 0x03E0 | 0x0000,//0b001001
			0x000F | 0x03E0 | 0x0000,//0b001010
			0x001F | 0x03E0 | 0x0000,//0b001011
    		0x0000 | 0x07E0 | 0x0000,//0b001100
			0x0007 | 0x07E0 | 0x0000,//0b001101
			0x000F | 0x07E0 | 0x0000,//0b001110
			0x001F | 0x07E0 | 0x0000,//0b001111
//
    		0x0000 | 0x0000 | 0x3800,//0b010000
			0x0007 | 0x0000 | 0x3800,//0b010001
			0x000F | 0x0000 | 0x3800,//0b010010
			0x001F | 0x0000 | 0x3800,//0b010011
    		0x0000 | 0x01E0 | 0x3800,//0b010100
			0x0007 | 0x01E0 | 0x3800,//0b010101
			0x000F | 0x01E0 | 0x3800,//0b010110
			0x001F | 0x01E0 | 0x3800,//0b010111
    		0x0000 | 0x03E0 | 0x3800,//0b011000
			0x0007 | 0x03E0 | 0x3800,//0b011001
			0x000F | 0x03E0 | 0x3800,//0b011010
			0x001F | 0x03E0 | 0x3800,//0b011011
    		0x0000 | 0x07E0 | 0x3800,//0b011100
			0x0007 | 0x07E0 | 0x3800,//0b011101
			0x000F | 0x07E0 | 0x3800,//0b011110
			0x001F | 0x07E0 | 0x3800,//0b011111
//
    		0x0000 | 0x0000 | 0x7800,//0b100000
			0x0007 | 0x0000 | 0x7800,//0b100001
			0x000F | 0x0000 | 0x7800,//0b100010
			0x001F | 0x0000 | 0x7800,//0b100011
    		0x0000 | 0x01E0 | 0x7800,//0b100100
			0x0007 | 0x01E0 | 0x7800,//0b100101
			0x000F | 0x01E0 | 0x7800,//0b100110
			0x001F | 0x01E0 | 0x7800,//0b100111
    		0x0000 | 0x03E0 | 0x7800,//0b101000
			0x0007 | 0x03E0 | 0x7800,//0b101001
			0x000F | 0x03E0 | 0x7800,//0b101010
			0x001F | 0x03E0 | 0x7800,//0b101011
    		0x0000 | 0x07E0 | 0x7800,//0b101100
			0x0007 | 0x07E0 | 0x7800,//0b101101
			0x000F | 0x07E0 | 0x7800,//0b101110
			0x001F | 0x07E0 | 0x7800,//0b101111
//
    		0x0000 | 0x0000 | 0xF800,//0b110000
			0x0007 | 0x0000 | 0xF800,//0b110001
			0x000F | 0x0000 | 0xF800,//0b110010
			0x001F | 0x0000 | 0xF800,//0b110011
    		0x0000 | 0x01E0 | 0xF800,//0b110100
			0x0007 | 0x01E0 | 0xF800,//0b110101
			0x000F | 0x01E0 | 0xF800,//0b110110
			0x001F | 0x01E0 | 0xF800,//0b110111
    		0x0000 | 0x03E0 | 0xF800,//0b111000
			0x0007 | 0x03E0 | 0xF800,//0b111001
			0x000F | 0x03E0 | 0xF800,//0b111010
			0x001F | 0x03E0 | 0xF800,//0b111011
    		0x0000 | 0x07E0 | 0xF800,//0b111100
			0x0007 | 0x07E0 | 0xF800,//0b111101
			0x000F | 0x07E0 | 0xF800,//0b111110
			0x001F | 0x07E0 | 0xF800,//0b111111
//
    };
    if(Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9481){
        int y;
        DefineRegionSPI(low_x, low_y, high_x, high_y, 1);
        PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
        set_cs();
        // switch to SPI enhanced mode for the bulk transfer

        for(y=low_y;y<=high_y;y++){
        	if(y<memswitch){
        		t=(uint8_t *)(screenbuff+(y * HRes + low_x) * 2);
        	} else {
        		t=(uint8_t *)(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
        	}
//        	wTransferState = TRANSFER_WAIT;
//        	HAL_SPI_Transmit_IT(&GenSPI,t,(high_x-low_x+1)*2);
//          while (wTransferState != TRANSFER_COMPLETE);
        	HAL_SPI_Transmit(&GenSPI,t,(high_x-low_x+1)*2,500);
       }

        SpiCsHigh(Option.LCD_CS);                  //set CS high

        // revert to non enhanced SPI mode
        low_y=480; high_y=0; low_x=800; high_x=0;
    } else if(Option.DISPLAY_TYPE == SSD1963_4_16){
        int y, x, c;
        if(high_y>=low_y){
            SetAreaSSD1963(low_x, low_y, high_x, high_y);
            WriteSSD1963Command(CMD_WR_MEMSTART);
            for(y = low_y; y <= high_y; y++){
            	if(y<memswitch){
            		t=(screenbuff+(y * HRes + low_x) * 2);
            		for(x = low_x; x <= high_x; x++){
            			c=((*t++)<<8);
            			c|= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN;
            		}
            	} else {
            		t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
            		for(x = low_x; x <= high_x; x++){
            			c=((*t++)<<8);
            			c|= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN;
            		}
            	}
            }
        }
        low_y=480; high_y=0; low_x=800; high_x=0;
    } else if(Option.DISPLAY_TYPE >= SSD1963_5_BUFF && Option.DISPLAY_TYPE< SSD1963_5_8BIT){
        int y, x, c;
        if(high_y>=low_y){
            if(Option.DISPLAY_ORIENTATION & 1)SetAreaSSD1963(low_x+displayoffset, low_y, high_x+displayoffset, high_y);
            else SetAreaSSD1963(low_x, low_y+displayoffset, high_x, high_y+displayoffset);
            WriteSSD1963Command(CMD_WR_MEMSTART);
            if(Option.SSDspeed){
            	for(y = low_y; y <= high_y; y++){
            		if(y<memswitch){
            			t=(screenbuff+(y * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		} else if(y<memswitch2){
            			t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		} else {
            			t=(screenbuff3+((y-memswitch2) * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		}
            	}
        	} else {
            	for(y = low_y; y <= high_y; y++){
            		if(y<memswitch){
            			t=(screenbuff+(y * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		} else if(y<memswitch2){
            			t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		} else {
            			t=(screenbuff3+((y-memswitch2) * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		}
            	}
        	}
     	}
        low_y=480; high_y=0; low_x=800; high_x=0;
    } else if(Option.DISPLAY_TYPE >= SSD1963_5_8BIT){
        int y, x, c, lastc=-1;
        if(high_y>=low_y){
            if(Option.DISPLAY_ORIENTATION & 1)SetAreaSSD1963(low_x+displayoffset, low_y, high_x+displayoffset, high_y);
            else SetAreaSSD1963(low_x, low_y+displayoffset, high_x, high_y+displayoffset);
            WriteSSD1963Command(CMD_WR_MEMSTART);
            if(Option.SSDspeed){
            	for(y = low_y; y <= high_y; y++){
            		if(y<memswitch){
            			t=(screenbuff+(y * HRes + low_x) );
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		} else if(y<memswitch2){
            			t=(screenbuff2+((y-memswitch) * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		} else {
            			t=(screenbuff3+((y-memswitch2) * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		}
            	}
        	} else {
            	for(y = low_y; y <= high_y; y++){
            		if(y<memswitch){
            			t=(screenbuff+(y * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		} else if(y<memswitch2){
            			t=(screenbuff2+((y-memswitch) * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		} else {
            			t=(screenbuff3+((y-memswitch2) * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		}
            	}
        	}
     	}
        low_y=480; high_y=0; low_x=800; high_x=0;
    } else if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9341_8){
        int y, x, c;
        if(high_y>=low_y){
            DefineRegionP(low_x, low_y, high_x, high_y,1);
            if(Option.DISPLAY_TYPE	==	ILI9341_16){
            	for(y = low_y; y <= high_y; y++){
            		t=(screenbuff+(y * HRes + low_x) * 2);
            		for(x = low_x; x <= high_x; x++){
            			c=((*t++)<<8);
            			c|= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN;
            		}
            	}
            } else {
            	for(y = low_y; y <= high_y; y++){
            		t=(screenbuff+(y * HRes + low_x) * 2);
            		for(x = low_x; x <= high_x; x++){
            			c= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN;
            			c= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN;
            		}
            	}
            }
        }
    low_y=480; high_y=0; low_x=800; high_x=0;
    }
}
*/

void Display_Refresh(void){
    uint8_t *t=NULL;

    const int map[64]={
    		0x0000 | 0x0000 | 0x0000,//0b000000
			0x0007 | 0x0000 | 0x0000,//0b000001
			0x000F | 0x0000 | 0x0000,//0b000010
			0x001F | 0x0000 | 0x0000,//0b000011
    		0x0000 | 0x01E0 | 0x0000,//0b000100
			0x0007 | 0x01E0 | 0x0000,//0b000101
			0x000F | 0x01E0 | 0x0000,//0b000110
			0x001F | 0x01E0 | 0x0000,//0b000111
    		0x0000 | 0x03E0 | 0x0000,//0b001000
			0x0007 | 0x03E0 | 0x0000,//0b001001
			0x000F | 0x03E0 | 0x0000,//0b001010
			0x001F | 0x03E0 | 0x0000,//0b001011
    		0x0000 | 0x07E0 | 0x0000,//0b001100
			0x0007 | 0x07E0 | 0x0000,//0b001101
			0x000F | 0x07E0 | 0x0000,//0b001110
			0x001F | 0x07E0 | 0x0000,//0b001111
//
    		0x0000 | 0x0000 | 0x3800,//0b010000
			0x0007 | 0x0000 | 0x3800,//0b010001
			0x000F | 0x0000 | 0x3800,//0b010010
			0x001F | 0x0000 | 0x3800,//0b010011
    		0x0000 | 0x01E0 | 0x3800,//0b010100
			0x0007 | 0x01E0 | 0x3800,//0b010101
			0x000F | 0x01E0 | 0x3800,//0b010110
			0x001F | 0x01E0 | 0x3800,//0b010111
    		0x0000 | 0x03E0 | 0x3800,//0b011000
			0x0007 | 0x03E0 | 0x3800,//0b011001
			0x000F | 0x03E0 | 0x3800,//0b011010
			0x001F | 0x03E0 | 0x3800,//0b011011
    		0x0000 | 0x07E0 | 0x3800,//0b011100
			0x0007 | 0x07E0 | 0x3800,//0b011101
			0x000F | 0x07E0 | 0x3800,//0b011110
			0x001F | 0x07E0 | 0x3800,//0b011111
//
    		0x0000 | 0x0000 | 0x7800,//0b100000
			0x0007 | 0x0000 | 0x7800,//0b100001
			0x000F | 0x0000 | 0x7800,//0b100010
			0x001F | 0x0000 | 0x7800,//0b100011
    		0x0000 | 0x01E0 | 0x7800,//0b100100
			0x0007 | 0x01E0 | 0x7800,//0b100101
			0x000F | 0x01E0 | 0x7800,//0b100110
			0x001F | 0x01E0 | 0x7800,//0b100111
    		0x0000 | 0x03E0 | 0x7800,//0b101000
			0x0007 | 0x03E0 | 0x7800,//0b101001
			0x000F | 0x03E0 | 0x7800,//0b101010
			0x001F | 0x03E0 | 0x7800,//0b101011
    		0x0000 | 0x07E0 | 0x7800,//0b101100
			0x0007 | 0x07E0 | 0x7800,//0b101101
			0x000F | 0x07E0 | 0x7800,//0b101110
			0x001F | 0x07E0 | 0x7800,//0b101111
//
    		0x0000 | 0x0000 | 0xF800,//0b110000
			0x0007 | 0x0000 | 0xF800,//0b110001
			0x000F | 0x0000 | 0xF800,//0b110010
			0x001F | 0x0000 | 0xF800,//0b110011
    		0x0000 | 0x01E0 | 0xF800,//0b110100
			0x0007 | 0x01E0 | 0xF800,//0b110101
			0x000F | 0x01E0 | 0xF800,//0b110110
			0x001F | 0x01E0 | 0xF800,//0b110111
    		0x0000 | 0x03E0 | 0xF800,//0b111000
			0x0007 | 0x03E0 | 0xF800,//0b111001
			0x000F | 0x03E0 | 0xF800,//0b111010
			0x001F | 0x03E0 | 0xF800,//0b111011
    		0x0000 | 0x07E0 | 0xF800,//0b111100
			0x0007 | 0x07E0 | 0xF800,//0b111101
			0x000F | 0x07E0 | 0xF800,//0b111110
			0x001F | 0x07E0 | 0xF800,//0b111111
//
    };
    if(Option.DISPLAY_TYPE == ILI9488 ){  //RGB565 sent in 3 bytes required
    	   unsigned char hb, lb;//, *p;
    	   char f[1440];    //3*480 =1440
    	   int  k,i,j;
           int y;
            //if(!BasicRunning ) return;
            DefineRegionSPI(low_x, low_y, high_x, high_y, 1);
            PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
            set_cs();
            i=(high_x-low_x+1);
            for(y=low_y;y<=high_y;y++){
            	k=0;
            	if(y<memswitch){
            		t=(uint8_t *)(screenbuff+(y * HRes + low_x) * 2);
            		for(j = i; j > 0; j--){
            		   	 hb=*t++;
            			 lb=*t++;
            			 f[k++] = ( hb & 0xF8) ;
            			 f[k++] = ((hb & 7) << 5) | (lb >> 3);
            			 f[k++]= ((lb & 0x1F) << 3);
            		}
            	} else {
            		t=(uint8_t *)(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
            		for(j = i; j > 0; j--){
            		   	 hb=*t++;
            			 lb=*t++;
            			 f[k++] = ( hb & 0xF8) ;
            			 f[k++] = ((hb & 7) << 5) | (lb >> 3);
            			 f[k++]= ((lb & 0x1F) << 3);
            		}
            	}
               	HAL_SPI_Transmit(&GenSPI,(uint8_t *)f,i*3,500);

           }
             SpiCsHigh(Option.LCD_CS);                  //set CS high
             low_y=480; high_y=0; low_x=800; high_x=0;
             return;

      }else if(Option.DISPLAY_TYPE == ST7796S ){  //RGB565 sent in 2 bytes required RRRRRGGG GGGBBBBB
             	   unsigned char hb, lb;//, *p;
             	   char f[960];    //3*480 =1440  2*480=960
             	   int  k,i,j;
                    int y;
                     //if(!BasicRunning ) return;
                     DefineRegionSPI(low_x, low_y, high_x, high_y, 1);
                     PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
                     set_cs();
                     i=(high_x-low_x+1);
                     for(y=low_y;y<=high_y;y++){
                     	k=0;
                     	if(y<memswitch){
                     		t=(uint8_t *)(screenbuff+(y * HRes + low_x) * 2);
                     		for(j = i; j > 0; j--){
                     		   	 hb=*t++;
                     			 lb=*t++;
                     			 //f[k++] = ( hb & 0xF8) ;
                     			 //f[k++] = ((hb & 7) << 5) | (lb >> 3);
                     			 //f[k++]= ((lb & 0x1F) << 3);
                     			 f[k++] =  hb;
                     			 f[k++] =  lb;

                     		}
                     	} else {
                     		t=(uint8_t *)(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
                     		for(j = i; j > 0; j--){
                     		   	 hb=*t++;
                     			 lb=*t++;
                     			 // f[k++] = ( hb & 0xF8) ;
                     			 // f[k++] = ((hb & 7) << 5) | (lb >> 3);
                     			 // f[k++]= ((lb & 0x1F) << 3);
                     			 f[k++] =  hb;
                     		     f[k++] =  lb;
                     		}
                     	}
                        	HAL_SPI_Transmit(&GenSPI,(uint8_t *)f,i*2,500);

                    }
                      SpiCsHigh(Option.LCD_CS);                  //set CS high
                      low_y=480; high_y=0; low_x=800; high_x=0;
                      return;

    }else if(Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9481){
            int y;
            DefineRegionSPI(low_x, low_y, high_x, high_y, 1);
            PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
            set_cs();
            // switch to SPI enhanced mode for the bulk transfer

            for(y=low_y;y<=high_y;y++){
            	if(y<memswitch){
            		t=(uint8_t *)(screenbuff+(y * HRes + low_x) * 2);
            	} else {
            		t=(uint8_t *)(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
            	}
    //        	wTransferState = TRANSFER_WAIT;
    //        	HAL_SPI_Transmit_IT(&GenSPI,t,(high_x-low_x+1)*2);
    //          while (wTransferState != TRANSFER_COMPLETE);
            	HAL_SPI_Transmit(&GenSPI,t,(high_x-low_x+1)*2,500);
            }

             SpiCsHigh(Option.LCD_CS);                  //set CS high
             low_y=480; high_y=0; low_x=800; high_x=0;
             return;
     }
if(HAS_144PINS){
      if(Option.DISPLAY_TYPE == SSD1963_4 ){
	       int y, x, c,c1;
		        if(high_y>=low_y){
		            SetAreaSSD1963_VIT6(low_x, low_y, high_x, high_y);
		            WriteSSD1963Command_VIT6(CMD_WR_MEMSTART);
		            for(y = low_y; y <= high_y; y++){
		            	if(y<memswitch){
		            		t=(screenbuff+(y * HRes + low_x) * 2);
		            		for(x = low_x; x <= high_x; x++){
		            			c= *t++;
			            		c1=c<<5;
			            		GPIOE->BSRR =  (~c<<16 & 0xFF0000)  | (c & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST;
			            		c= *t++;
			            		c1=( c1 | c>>3)  & 0xFF;
			            		GPIOE->BSRR =  (~c1<<16 & 0xFF0000)  | (c1 & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST;
			            		c=c<<3;
			            		GPIOE->BSRR =  (~c<<16 & 0xFF0000)  | (c & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST;

		            		}
		            	} else {
		            		t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
		            		for(x = low_x; x <= high_x; x++){
		            			c= *t++;
			            		c1=c<<5;
			            		GPIOE->BSRR =  (~c<<16 & 0xFF0000)  | (c & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST;
			            		c= *t++;
			            		c1=( c1 | c>>3)  & 0xFF;
			            		GPIOE->BSRR =  (~c1<<16 & 0xFF0000)  | (c1 & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST;
			            		c=c<<3;
			            		GPIOE->BSRR =  (~c<<16 & 0xFF0000)  | (c & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST;
		            		}
		            	}
		            }
		        }
		        low_y=480; high_y=0; low_x=800; high_x=0;
      } else if(Option.DISPLAY_TYPE == SSD1963_4_16){
        int y, x, c;
        if(high_y>=low_y){
            SetAreaSSD1963(low_x, low_y, high_x, high_y);
            WriteSSD1963Command_ZI2(CMD_WR_MEMSTART);
            for(y = low_y; y <= high_y; y++){
            	if(y<memswitch){
            		t=(screenbuff+(y * HRes + low_x) * 2);
            		for(x = low_x; x <= high_x; x++){
            			c=((*t++)<<8);
            			c|= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN_FAST;
            		}
            	} else {
            		t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
            		for(x = low_x; x <= high_x; x++){
            			c=((*t++)<<8);
            			c|= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN_FAST;
            		}
            	}
            }
        }
        low_y=480; high_y=0; low_x=800; high_x=0;
    } else if(Option.DISPLAY_TYPE >= SSD1963_5_BUFF && Option.DISPLAY_TYPE< SSD1963_5_8BIT){
        int y, x, c;
        if(high_y>=low_y){
            if(Option.DISPLAY_ORIENTATION & 1)SetAreaSSD1963(low_x+displayoffset, low_y, high_x+displayoffset, high_y);
            else SetAreaSSD1963(low_x, low_y+displayoffset, high_x, high_y+displayoffset);
            WriteSSD1963Command_ZI2(CMD_WR_MEMSTART);
            if(Option.SSDspeed){
            	for(y = low_y; y <= high_y; y++){
            		if(y<memswitch){
            			t=(screenbuff+(y * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		} else if(y<memswitch2){
            			t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		} else {
            			t=(screenbuff3+((y-memswitch2) * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		}
            	}
        	} else {
            	for(y = low_y; y <= high_y; y++){
            		if(y<memswitch){
            			t=(screenbuff+(y * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		} else if(y<memswitch2){
            			t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		} else {
            			t=(screenbuff3+((y-memswitch2) * HRes + low_x) * 2);
            			for(x = low_x; x <= high_x; x++){
            				c=((*t++)<<8);
            				c|= *t++;
            				GPIOE->ODR = c;
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		}
            	}
        	}
     	}
        low_y=480; high_y=0; low_x=800; high_x=0;
    } else if(Option.DISPLAY_TYPE >= SSD1963_5_8BIT){
        int y, x, c, lastc=-1;
        if(high_y>=low_y){
            if(Option.DISPLAY_ORIENTATION & 1)SetAreaSSD1963(low_x+displayoffset, low_y, high_x+displayoffset, high_y);
            else SetAreaSSD1963(low_x, low_y+displayoffset, high_x, high_y+displayoffset);
            WriteSSD1963Command_ZI2(CMD_WR_MEMSTART);
            if(Option.SSDspeed){
            	for(y = low_y; y <= high_y; y++){
            		if(y<memswitch){
            			t=(screenbuff+(y * HRes + low_x) );
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		} else if(y<memswitch2){
            			t=(screenbuff2+((y-memswitch) * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		} else {
            			t=(screenbuff3+((y-memswitch2) * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN;
            			}
            		}
            	}
        	} else {
            	for(y = low_y; y <= high_y; y++){
            		if(y<memswitch){
            			t=(screenbuff+(y * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		} else if(y<memswitch2){
            			t=(screenbuff2+((y-memswitch) * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		} else {
            			t=(screenbuff3+((y-memswitch2) * HRes + low_x));
            			for(x = low_x; x <= high_x; x++){
            				c=*t++;
            				if(c!=lastc){
            					GPIOE->ODR = map[c];
            					lastc=c;
            				}
            				SSD1963_WR_TOGGLE_PIN_FAST
            			}
            		}
            	}
        	}
     	}
        low_y=480; high_y=0; low_x=800; high_x=0;
    } else if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9341_8){
        int y, x, c;
        if(high_y>=low_y){
            DefineRegionP(low_x, low_y, high_x, high_y,1);
            if(Option.DISPLAY_TYPE	==	ILI9341_16){
            	for(y = low_y; y <= high_y; y++){
            		t=(screenbuff+(y * HRes + low_x) * 2);
            		for(x = low_x; x <= high_x; x++){
            			c=((*t++)<<8);
            			c|= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN;
            		}
            	}
            } else {
            	for(y = low_y; y <= high_y; y++){
            		t=(screenbuff+(y * HRes + low_x) * 2);
            		for(x = low_x; x <= high_x; x++){
            			c= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN;
            			c= *t++;
            			GPIOE->ODR = c;
            			SSD1963_WR_TOGGLE_PIN;
            		}
            	}
            }
        }
    low_y=480; high_y=0; low_x=800; high_x=0;
    }
}else{  //100PIN VIT6
	/*
	if(Option.DISPLAY_TYPE == ILI9341 || Option.DISPLAY_TYPE == ILI9481){
	        int y;
	        DefineRegionSPI(low_x, low_y, high_x, high_y, 1);
	        PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
	        set_cs();
	        // switch to SPI enhanced mode for the bulk transfer

	        for(y=low_y;y<=high_y;y++){
	        	if(y<memswitch){
	        		t=(uint8_t *)(screenbuff+(y * HRes + low_x) * 2);
	        	} else {
	        		t=(uint8_t *)(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
	        	}
	//        	wTransferState = TRANSFER_WAIT;
	//        	HAL_SPI_Transmit_IT(&GenSPI,t,(high_x-low_x+1)*2);
	//          while (wTransferState != TRANSFER_COMPLETE);
	        	HAL_SPI_Transmit(&GenSPI,t,(high_x-low_x+1)*2,500);
	       }

	        SpiCsHigh(Option.LCD_CS);                  //set CS high

	        // revert to non enhanced SPI mode
	        low_y=480; high_y=0; low_x=800; high_x=0;
	 */
	 if(Option.DISPLAY_TYPE == SSD1963_4 ){
		        int y, x, c,c1;
		        if(high_y>=low_y){
		            SetAreaSSD1963_VIT6(low_x, low_y, high_x, high_y);
		            WriteSSD1963Command_VIT6(CMD_WR_MEMSTART);
		            for(y = low_y; y <= high_y; y++){
		            	if(y<memswitch){
		            		t=(screenbuff+(y * HRes + low_x) * 2);
		            		for(x = low_x; x <= high_x; x++){
		            			c= *t++;
			            		c1=c<<5;
			            		GPIOE->BSRR =  (~c<<16 & 0xFF0000)  | (c & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST_VIT6;
			            		c= *t++;
			            		c1=( c1 | c>>3)  & 0xFF;
			            		GPIOE->BSRR =  (~c1<<16 & 0xFF0000)  | (c1 & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST_VIT6;
			            		c=c<<3;
			            		GPIOE->BSRR =  (~c<<16 & 0xFF0000)  | (c & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST_VIT6;

		            		}
		            	} else {
		            		t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
		            		for(x = low_x; x <= high_x; x++){
		            			c= *t++;
			            		c1=c<<5;
			            		GPIOE->BSRR =  (~c<<16 & 0xFF0000)  | (c & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST_VIT6;
			            		c= *t++;
			            		c1=( c1 | c>>3)  & 0xFF;
			            		GPIOE->BSRR =  (~c1<<16 & 0xFF0000)  | (c1 & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST_VIT6;
			            		c=c<<3;
			            		GPIOE->BSRR =  (~c<<16 & 0xFF0000)  | (c & 0xFF);
			            		SSD1963_WR_TOGGLE_PIN_FAST_VIT6;
		            		}
		            	}
		            }
		        }
		        low_y=480; high_y=0; low_x=800; high_x=0;
	    } else if(Option.DISPLAY_TYPE == SSD1963_4_16 ){
	        int y, x, c;
	        if(high_y>=low_y){
	            SetAreaSSD1963_VIT6(low_x, low_y, high_x, high_y);
	            WriteSSD1963Command_VIT6(CMD_WR_MEMSTART);
	            for(y = low_y; y <= high_y; y++){
	            	if(y<memswitch){
	            		t=(screenbuff+(y * HRes + low_x) * 2);
	            		for(x = low_x; x <= high_x; x++){
	            			c=((*t++)<<8);
	            			c|= *t++;
	            			GPIOE->ODR = c;
	            			SSD1963_WR_TOGGLE_PIN_FAST_VIT6;
	            		}
	            	} else {
	            		t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
	            		for(x = low_x; x <= high_x; x++){
	            			c=((*t++)<<8);
	            			c|= *t++;
	            			GPIOE->ODR = c;
	            			SSD1963_WR_TOGGLE_PIN_FAST_VIT6;
	            		}
	            	}
	            }
	        }
	        low_y=480; high_y=0; low_x=800; high_x=0;
	    } else if(Option.DISPLAY_TYPE >= SSD1963_5_BUFF && Option.DISPLAY_TYPE< SSD1963_5_8BIT){
	        int y, x, c;
	        if(high_y>=low_y){
	            if(Option.DISPLAY_ORIENTATION & 1)SetAreaSSD1963_VIT6(low_x+displayoffset, low_y, high_x+displayoffset, high_y);
	            else SetAreaSSD1963_VIT6(low_x, low_y+displayoffset, high_x, high_y+displayoffset);
	            WriteSSD1963Command_VIT6(CMD_WR_MEMSTART);
	            if(Option.SSDspeed){
	            	for(y = low_y; y <= high_y; y++){
	            		if(y<memswitch){
	            			t=(screenbuff+(y * HRes + low_x) * 2);
	            			for(x = low_x; x <= high_x; x++){
	            				c=((*t++)<<8);
	            				c|= *t++;
	            				GPIOE->ODR = c;
	            				SSD1963_WR_TOGGLE_PIN_VIT6;
	            			}
	            		} else if(y<memswitch2){
	            			t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
	            			for(x = low_x; x <= high_x; x++){
	            				c=((*t++)<<8);
	            				c|= *t++;
	            				GPIOE->ODR = c;
	            				SSD1963_WR_TOGGLE_PIN_VIT6;
	            			}
	            		} else {
	            			t=(screenbuff3+((y-memswitch2) * HRes + low_x) * 2);
	            			for(x = low_x; x <= high_x; x++){
	            				c=((*t++)<<8);
	            				c|= *t++;
	            				GPIOE->ODR = c;
	            				SSD1963_WR_TOGGLE_PIN_VIT6;
	            			}
	            		}
	            	}
	        	  } else {
	            	for(y = low_y; y <= high_y; y++){
	            		if(y<memswitch){
	            			t=(screenbuff+(y * HRes + low_x) * 2);
	            			for(x = low_x; x <= high_x; x++){
	            				c=((*t++)<<8);
	            				c|= *t++;
	            				GPIOE->ODR = c;
	            				SSD1963_WR_TOGGLE_PIN_FAST_VIT6
	            			}
	            		} else if(y<memswitch2){
	            			t=(screenbuff2+((y-memswitch) * HRes + low_x) * 2);
	            			for(x = low_x; x <= high_x; x++){
	            				c=((*t++)<<8);
	            				c|= *t++;
	            				GPIOE->ODR = c;
	            				SSD1963_WR_TOGGLE_PIN_FAST_VIT6
	            			}
	            		} else {
	            			t=(screenbuff3+((y-memswitch2) * HRes + low_x) * 2);
	            			for(x = low_x; x <= high_x; x++){
	            				c=((*t++)<<8);
	            				c|= *t++;
	            				GPIOE->ODR = c;
	            				SSD1963_WR_TOGGLE_PIN_FAST_VIT6
	            			}
	            		}
	            	}
	        	}
	     	}
	        low_y=480; high_y=0; low_x=800; high_x=0;
	    } else if(Option.DISPLAY_TYPE >= SSD1963_5_8BIT){
	        int y, x, c, lastc=-1;
	        if(high_y>=low_y){
	            if(Option.DISPLAY_ORIENTATION & 1)SetAreaSSD1963_VIT6(low_x+displayoffset, low_y, high_x+displayoffset, high_y);
	            else SetAreaSSD1963_VIT6(low_x, low_y+displayoffset, high_x, high_y+displayoffset);
	            WriteSSD1963Command_VIT6(CMD_WR_MEMSTART);
	            if(Option.SSDspeed){
	            	for(y = low_y; y <= high_y; y++){
	            		if(y<memswitch){
	            			t=(screenbuff+(y * HRes + low_x) );
	            			for(x = low_x; x <= high_x; x++){
	            				c=*t++;
	            				if(c!=lastc){
	            					GPIOE->ODR = map[c];
	            					lastc=c;
	            				}
	            				SSD1963_WR_TOGGLE_PIN_VIT6;
	            			}
	            		} else if(y<memswitch2){
	            			t=(screenbuff2+((y-memswitch) * HRes + low_x));
	            			for(x = low_x; x <= high_x; x++){
	            				c=*t++;
	            				if(c!=lastc){
	            					GPIOE->ODR = map[c];
	            					lastc=c;
	            				}
	            				SSD1963_WR_TOGGLE_PIN_VIT6;
	            			}
	            		} else {
	            			t=(screenbuff3+((y-memswitch2) * HRes + low_x));
	            			for(x = low_x; x <= high_x; x++){
	            				c=*t++;
	            				if(c!=lastc){
	            					GPIOE->ODR = map[c];
	            					lastc=c;
	            				}
	            				SSD1963_WR_TOGGLE_PIN_VIT6;
	            			}
	            		}
	            	}
	        	  } else {
	            	for(y = low_y; y <= high_y; y++){
	            		if(y<memswitch){
	            			t=(screenbuff+(y * HRes + low_x));
	            			for(x = low_x; x <= high_x; x++){
	            				c=*t++;
	            				if(c!=lastc){
	            					GPIOE->ODR = map[c];
	            					lastc=c;
	            				}
	            				SSD1963_WR_TOGGLE_PIN_FAST_VIT6
	            			}
	            		} else if(y<memswitch2){
	            			t=(screenbuff2+((y-memswitch) * HRes + low_x));
	            			for(x = low_x; x <= high_x; x++){
	            				c=*t++;
	            				if(c!=lastc){
	            					GPIOE->ODR = map[c];
	            					lastc=c;
	            				}
	            				SSD1963_WR_TOGGLE_PIN_FAST_VIT6
	            			}
	            		} else {
	            			t=(screenbuff3+((y-memswitch2) * HRes + low_x));
	            			for(x = low_x; x <= high_x; x++){
	            				c=*t++;
	            				if(c!=lastc){
	            					GPIOE->ODR = map[c];
	            					lastc=c;
	            				}
	            				SSD1963_WR_TOGGLE_PIN_FAST_VIT6
	            			}
	            		}
	            	}
	        	}
	     	}
	        low_y=480; high_y=0; low_x=800; high_x=0;
	    } else if(Option.DISPLAY_TYPE == ILI9341_16 || Option.DISPLAY_TYPE == ILI9341_8){
	        int y, x, c;
	        if(high_y>=low_y){
	            DefineRegionP_VIT6(low_x, low_y, high_x, high_y,1);
	            if(Option.DISPLAY_TYPE	==	ILI9341_16){
	            	for(y = low_y; y <= high_y; y++){
	            		t=(screenbuff+(y * HRes + low_x) * 2);
	            		for(x = low_x; x <= high_x; x++){
	            			c=((*t++)<<8);
	            			c|= *t++;
	            			GPIOE->ODR = c;
	            			SSD1963_WR_TOGGLE_PIN_VIT6;
	            		}
	            	}
	            } else {
	            	for(y = low_y; y <= high_y; y++){
	            		t=(screenbuff+(y * HRes + low_x) * 2);
	            		for(x = low_x; x <= high_x; x++){
	            			c= *t++;
	            			GPIOE->ODR = c;
	            			SSD1963_WR_TOGGLE_PIN_VIT6;
	            			c= *t++;
	            			GPIOE->ODR = c;
	            			SSD1963_WR_TOGGLE_PIN_VIT6;
	            		}
	            	}
	            }
	        }
	    low_y=480; high_y=0; low_x=800; high_x=0;
	    }

}
}

