/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

SPI-LCD.c

This is the driver for SPI LCDs in MMBasic.
The core SPI LCD driver was written and developed by Peter Mather of the Back Shed Forum (http://www.thebackshed.com/forum/forum_topics.asp?FID=16)

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

#include <stdarg.h>
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#ifdef STM32F4version
	#define GenSPI hspi2
#endif


void DefineRegionSPI(int xstart, int ystart, int xend, int yend, int rw);
void DrawBitmapSPI(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap);
int CurrentSPISpeed=NONE_SPI_SPEED;
extern SPI_HandleTypeDef GenSPI;
extern int codecheck(char *line);
extern int codemap(char code, int pin);
#define SPIsend(a) {uint8_t b=a;HAL_SPI_Transmit(&GenSPI,&b,1,500);}
#define SPIqueue(a) {HAL_SPI_Transmit(&GenSPI,a,2,500);}
#define SPIsend2(a) {SPIsend(0);SPIsend(a);}


// utility function for routines that want to reserve a pin for special I/O
// this ignores any previous settings and forces the pin to its new state
// pin is the pin number
// inp is true if an input or false if an output
// init is the value used to initialise the pin if it is an output (hi or lo)
// type is the final tag for the pin in ExtCurrentConfig[]
void MIPS16 SetAndReserve(int pin, int inp, int init, int type) {
    if(pin == 0) return;                                            // do nothing if not set
    GPIO_InitTypeDef GPIO_InitDef;
    if(inp) {
		GPIO_InitDef.Mode = GPIO_MODE_INPUT;
    } else {
        PinSetBit(pin, init ? LATSET : LATCLR);                     // set LAT
    	GPIO_InitDef.Mode = GPIO_MODE_OUTPUT_PP;
    }
	GPIO_InitDef.Pull = GPIO_NOPULL; //set as input with no pullup or down
	GPIO_InitDef.Pin = PinDef[pin].bitnbr;
	GPIO_InitDef.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(PinDef[pin].sfr, &GPIO_InitDef);
    ExtCurrentConfig[pin] = type;
}


void MIPS16 ConfigDisplaySPI(char *p) {
	int DISPLAY_TYPE=0;
	int p1, p2, p3;
    getargs(&p, 9, ",");
    if(!(argc == 7 || argc == 9)) error("Argument count or display type");

    if(checkstring(argv[0], "ILI9163")) {
        DISPLAY_TYPE = ILI9163;
    } else if(checkstring(argv[0], "ST7735")) {
        DISPLAY_TYPE = ST7735;
	} else
        error("Invalid display type");

    if(checkstring(argv[2], "L") || checkstring(argv[2], "LANDSCAPE"))
        Option.DISPLAY_ORIENTATION = LANDSCAPE;
    else if(checkstring(argv[2], "P") || checkstring(argv[2], "PORTRAIT"))
        Option.DISPLAY_ORIENTATION = PORTRAIT;
    else if(checkstring(argv[2], "RL") || checkstring(argv[2], "RLANDSCAPE"))
        Option.DISPLAY_ORIENTATION = RLANDSCAPE;
    else if(checkstring(argv[2], "RP") || checkstring(argv[2], "RPORTRAIT"))
        Option.DISPLAY_ORIENTATION = RPORTRAIT;
    else error("Orientation");
	p1 = getinteger(argv[4]);
	p2 = getinteger(argv[6]);
    CheckPin(p1, CP_IGNORE_INUSE);
    CheckPin(p2, CP_IGNORE_INUSE);
    if(argc == 9) {
	p3 = getinteger(argv[8]);
        CheckPin(p3, CP_IGNORE_INUSE);
        Option.LCD_CS = p3;
    } else
        Option.LCD_CS = 0;

    Option.LCD_CD = p1;
    Option.LCD_Reset = p2;
    Option.DISPLAY_TYPE = DISPLAY_TYPE;
    Option.TOUCH_XZERO = TOUCH_NOT_CALIBRATED;                      // record the touch feature as not calibrated
}



// initialise the display controller
// this is used in the initial boot sequence of the Micromite
void MIPS16 InitDisplaySPI(int fullinit) {

	if(Option.DISPLAY_TYPE <= SSD_PANEL || Option.DISPLAY_TYPE > SPI_PANEL) return;
    if(fullinit) {
        SetAndReserve(Option.LCD_CD, P_OUTPUT, 1, EXT_BOOT_RESERVED);                            // config data/command as an output
        SetAndReserve(Option.LCD_Reset, P_OUTPUT, 1, EXT_BOOT_RESERVED);                         // config reset as an output
        if(Option.LCD_CS) SetAndReserve(Option.LCD_CS, P_OUTPUT, 1, EXT_BOOT_RESERVED);          // config chip select as an output

        // open the SPI port and reserve the I/O pins
        OpenSpiChannel();

        // setup the pointers to the drawing primitives
        DrawRectangle = DrawRectangleSPI;
        DrawBitmap = DrawBitmapSPI;
        DrawBuffer = DrawBufferSPI;
        if(Option.DISPLAY_TYPE == ILI9341)ReadBuffer = ReadBufferSPI;
    }

    // the parameters for the display panel are set here
    // the initialisation sequences and the SPI driver code was written by Peter Mather (matherp on The Back Shed forum)
    switch(Option.DISPLAY_TYPE) {

        case ILI9163:
            DisplayHRes = 128;
            DisplayVRes = 128;
            ResetController();
			spi_write_command(ILI9163_SLPOUT);                      //exit sleep
			HAL_Delay(5);
			spi_write_cd(ILI9163_PIXFMT,1,0x05);
			HAL_Delay(5);
  			spi_write_cd(ILI9163_GAMMASET,1,0x04);                  //0x04
			HAL_Delay(2);
  			spi_write_cd(ILI9163_GAMRSEL,1,0x01);
			HAL_Delay(2);
  			spi_write_command(ILI9163_NORML);
  			spi_write_cd(ILI9163_DFUNCTR,2,0b11111111,0b00000110);  //
  			spi_write_cd(ILI9163_PGAMMAC,15,0x36,0x29,0x12,0x22,0x1C,0x15,0x42,0xB7,0x2F,0x13,0x12,0x0A,0x11,0x0B,0x06);//Positive Gamma Correction Setting
  			spi_write_cd(ILI9163_NGAMMAC,15,0x09,0x16,0x2D,0x0D,0x13,0x15,0x40,0x48,0x53,0x0C,0x1D,0x25,0x2E,0x34,0x39);//Negative Gamma Correction Setting
  			spi_write_cd(ILI9163_FRMCTR1,2,0x08,0x02);              //0x0C//0x08
			HAL_Delay(2);
  			spi_write_cd(ILI9163_DINVCTR,1,0x07);
			HAL_Delay(2);
  			spi_write_cd(ILI9163_PWCTR1,2,0x0A,0x02);               //4.30 - 0x0A
			HAL_Delay(2);
  			spi_write_cd(ILI9163_PWCTR2,1,0x02);
			HAL_Delay(2);
  			spi_write_cd(ILI9163_VCOMCTR1,2,0x50,99);               //0x50
			HAL_Delay(2);
  			spi_write_cd(ILI9163_VCOMOFFS,1,0);                     //0x40
			HAL_Delay(2);
  			spi_write_cd(ILI9163_VSCLLDEF,5,0,0,DisplayVRes,0,0);
 			spi_write_command(ILI9163_DISPON);                      //display ON
			HAL_Delay(2);
            switch(Option.DISPLAY_ORIENTATION) {
                case LANDSCAPE:     spi_write_cd(ILI9163_MADCTL,1,ILI9163_Landscape); break;
                case PORTRAIT:      spi_write_cd(ILI9163_MADCTL,1,ILI9163_Portrait); break;
                case RLANDSCAPE:    spi_write_cd(ILI9163_MADCTL,1,ILI9163_Landscape180); break;
                case RPORTRAIT:     spi_write_cd(ILI9163_MADCTL,1,ILI9163_Portrait180); break;
            }
			HAL_Delay(2);
            break;
        case ST7735:
            DisplayHRes = 160;
            DisplayVRes = 128;
            ResetController();
			spi_write_command(ST7735_SLPOUT);                       //out of sleep mode
			HAL_Delay(500);
			spi_write_cd(ST7735_FRMCTR1,3,0x01,0x2C,0x2d);          //frame rate control - normal mode
			spi_write_cd(ST7735_FRMCTR2,3,0x01,0x2C,0x2D);          //frame rate control - idle mode
			spi_write_cd(ST7735_FRMCTR3,6,0x01,0x2c,0x2D,0x01,0x2C,0x2D);//frame rate control - partial mode
			spi_write_cd(ST7735_INVCTR,1,0x07);                     //display inversion control
			spi_write_cd(ST7735_PWCTR1,3,0xA2,0x02,0x84);           //power control
			spi_write_cd(ST7735_PWCTR2,1,0xC5);                     //power control
			spi_write_cd(ST7735_PWCTR3,2,0x0A,0x00);                //power control
			spi_write_cd(ST7735_PWCTR4,2,0x8A,0x2A);                //power control
			spi_write_cd(ST7735_PWCTR5,2,0x8A,0xEE);                //power control
			spi_write_cd(ST7735_VMCTR1,1,0x0E);                     //power control
			spi_write_command(ST7735_INVOFF);                       //don't invert display
			spi_write_cd(ST7735_MADCTL,1,0xC0);                     //memory access control (directions);
			spi_write_cd(ST7735_COLMOD,1,0x05);                     //set color mode
			spi_write_cd(ST7735_CASET,4,0,0,0,0x7F);                //column addr set
			spi_write_cd(ST7735_RASET,4,0,0,0,0x9F);                //row addr set
			spi_write_cd(ST7735_GMCTRP1,16,0x02,0x1c,0x07,0x12,0x37,0x32,0x29,0x2D,0x25,0x29,0x2B,0x39,0x00,0x01,0x03,0x10);
			spi_write_cd(ST7735_GMCTRN1,16,0x03,0x1d,0x07,0x06,0x2E,0x2c,0x29,0x2d,0x2E,0x2E,0x37,0x3f,0x00,0x00,0x02,0x10);
			spi_write_command(ST7735_NORON);                        //normal display on
			HAL_Delay(10);
			spi_write_command(ST7735_DISPON);
            switch(Option.DISPLAY_ORIENTATION) {
                case LANDSCAPE:     spi_write_cd(ST7735_MADCTL, 1, ST7735_Landscape); break;
                case PORTRAIT:      spi_write_cd(ST7735_MADCTL, 1, ST7735_Portrait); break;
                case RLANDSCAPE:    spi_write_cd(ST7735_MADCTL, 1, ST7735_Landscape180); break;
                case RPORTRAIT:     spi_write_cd(ST7735_MADCTL, 1, ST7735_Portrait180); break;
            }
            break;
    }

    if(Option.DISPLAY_ORIENTATION & 1) {
        VRes=DisplayVRes;
        HRes=DisplayHRes;
    } else {
        VRes=DisplayHRes;
        HRes=DisplayVRes;
    }

    ResetDisplay();
    ClearScreen(gui_bcolour);
}


// set Chip Select for the LCD low
// this also checks the configuration of the SPI channel and if required reconfigures it to suit the LCD controller
void set_cs(void) {
    SpiCsLow(Option.LCD_CS, LCD_SPI_SPEED);
}



void spi_write_data(unsigned char data){
    PinSetBit(Option.LCD_CD, LATSET);
    set_cs();
    if(Option.DISPLAY_TYPE == ILI9481)	{SPIsend2(data);}
    else {SPIsend(data);}
    SpiCsHigh(Option.LCD_CS);
}


void spi_write_command(unsigned char data){
    PinSetBit(Option.LCD_CD, LATCLR);
    set_cs();
    if(Option.DISPLAY_TYPE == ILI9481)	{SPIsend2(data);}
    else {SPIsend(data);}
    SpiCsHigh(Option.LCD_CS);
}


void spi_write_cd(unsigned char command, int data, ...){
   int i;
   va_list ap;
   va_start(ap, data);
   spi_write_command(command);
   for(i = 0; i < data; i++) spi_write_data((char)va_arg(ap, int));
   va_end(ap);
}


void MIPS16 ResetController(void){
//    PinSetBit(Option.LCD_Reset, LATSET);
    HAL_Delay(100);
    PinSetBit(Option.LCD_Reset, LATCLR);
    HAL_Delay(100);
    PinSetBit(Option.LCD_Reset, LATSET);
    HAL_Delay(200);
    spi_write_command(ILI9341_SOFTRESET);                           //software reset
    HAL_Delay(200);
}


void DefineRegionSPI(int xstart, int ystart, int xend, int yend, int rw) {
    if(HRes == 0) error("Display not configured");
    if(Option.DISPLAY_TYPE == ILI9481){
    	if(rw) set_cs();
    	PinSetBit(Option.LCD_CD, LATCLR);
    	SPIsend2(ILI9341_COLADDRSET);
    	PinSetBit(Option.LCD_CD, LATSET);
    	SPIsend2(xstart >> 8);
    	SPIsend2(xstart);
    	SPIsend2(xend >> 8);
    	SPIsend2(xend);
    	PinSetBit(Option.LCD_CD, LATCLR);
    	SPIsend2(ILI9341_PAGEADDRSET);
    	PinSetBit(Option.LCD_CD, LATSET);
    	SPIsend2(ystart >> 8);
    	SPIsend2(ystart);
    	SPIsend2(yend >> 8);
    	SPIsend2(yend);
    	PinSetBit(Option.LCD_CD, LATCLR);
    	if(rw) {
    		SPIsend2(ILI9341_MEMORYWRITE);
    	} else {
    		SPIsend2(ILI9341_RAMRD);
    	}
    	PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    } else {
    	if(rw) set_cs();
    	PinSetBit(Option.LCD_CD, LATCLR);
    	SPIsend(ILI9341_COLADDRSET);
    	PinSetBit(Option.LCD_CD, LATSET);
    	SPIsend(xstart >> 8);
    	SPIsend(xstart);
    	SPIsend(xend >> 8);
    	SPIsend(xend);
    	PinSetBit(Option.LCD_CD, LATCLR);
    	SPIsend(ILI9341_PAGEADDRSET);
    	PinSetBit(Option.LCD_CD, LATSET);
    	SPIsend(ystart >> 8);
    	SPIsend(ystart);
    	SPIsend(yend >> 8);
    	SPIsend(yend);
    	PinSetBit(Option.LCD_CD, LATCLR);
    	if(rw) {
    		SPIsend(ILI9341_MEMORYWRITE);
    	} else {
    		SPIsend(ILI9341_RAMRD);
    	}
    	PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    }
}

/****************************************************************************************************
 ****************************************************************************************************

 Basic drawing primitives
 all drawing on the LCD is done using either one of these two functions

 ****************************************************************************************************
****************************************************************************************************/


// Draw a filled rectangle
// this is the basic drawing promitive used by most drawing routines
//    x1, y1, x2, y2 - the coordinates
//    c - the colour
void DrawRectangleSPI(int x1, int y1, int x2, int y2, int c){
	int i,t;
    unsigned char col[2], *p;
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

    // convert the colours to 565 format
    col[0]= ((c >> 16) & 0b11111000) | ((c >> 13) & 0b00000111);
    col[1] = ((c >>  5) & 0b11100000) | ((c >>  3) & 0b00011111);

    DefineRegionSPI(x1, y1, x2, y2, 1);
    PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    set_cs();
	i = x2 - x1 + 1;
	i*=2;
	p=GetMemory(i);
	for(t=0;t<i;t+=2){p[t]=col[0];p[t+1]=col[1];}
	for(t=y1;t<=y2;t++)	HAL_SPI_Transmit(&GenSPI,p,i,500);
	FreeMemory(p);
    SpiCsHigh(Option.LCD_CS);                                       //set CS high
}


void ReadBufferSPI(int x1, int y1, int x2, int y2, char* p) {
    int r, N, t;
    unsigned char h,l;
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
    N=(x2- x1+1) * (y2- y1+1) * 3;
    spi_write_cd(ILI9341_PIXELFORMAT,1,0x66); //change to RDB666 for read
    PinSetBit(Option.LCD_CS, LATCLR);
    DefineRegionSPI(x1, y1, x2, y2, 0);
    HAL_SPI_TransmitReceive(&GenSPI,&h,&l,1,500);
    r=0;
	HAL_SPI_Receive(&GenSPI,(uint8_t *)p,N,500);

    PinSetBit(Option.LCD_CD, LATCLR);
    SpiCsHigh(Option.LCD_CS);                  //set CS high
    // revert to non enhanced SPI mode
    spi_write_cd(ILI9341_PIXELFORMAT,1,0x55); //change back to rdb565
    r=0;
    while(N) {
        h=(uint8_t)p[r+2];
        l=(uint8_t)p[r];
        p[r]=(int8_t)h;
        p[r+2]=(int8_t)l;
        r+=3;
        N-=3;
    }
}

void DrawBufferSPI(int x1, int y1, int x2, int y2, char* p) {
	volatile int i, j, k, t;
    int memory;
    volatile char *q;
    union colourmap
    {
    char rgbbytes[4];
    unsigned int rgb;
    } c;
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


    DefineRegionSPI(x1, y1, x2, y2, 1);
    PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    set_cs();
	i = x2 - x1 + 1;
	i *= (y2 - y1 + 1);
	i*=2;
	// switch to SPI enhanced mode for the bulk transfer
	memory=FreeSpaceOnHeap()/2;
	if((i) < memory){
		q=GetMemory(i);
		k=0;
	    for(j = (x2 - x1 + 1) * (y2 - y1 + 1); j > 0; j--){
	        c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
	        c.rgbbytes[1]=*p++;
	        c.rgbbytes[2]=*p++;
	    // convert the colours to 565 format
	        // convert the colours to 565 format
	        q[k++]= ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
	        q[k++] = ((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
	    }
		HAL_SPI_Transmit(&GenSPI,(uint8_t *)q,i,500);
		FreeMemory((void *)q);
	} else {
		int n;
		q=GetMemory(memory);
		n=memory;
		while(i){
			if(i>=n){
				k=0;
			    for(j = n/2; j > 0; j--){
			        c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
			        c.rgbbytes[1]=*p++;
			        c.rgbbytes[2]=*p++;
			    // convert the colours to 565 format
			        // convert the colours to 565 format
			        q[k++]= ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
			        q[k++] = ((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
			    }
				HAL_SPI_Transmit(&GenSPI,(uint8_t *)q,n,500);
				i-=n;
			} else {
				k=0;
			    for(j = i/2; j > 0; j--){
			        c.rgbbytes[0]=*p++; //this order swaps the bytes to match the .BMP file
			        c.rgbbytes[1]=*p++;
			        c.rgbbytes[2]=*p++;
			    // convert the colours to 565 format
			        // convert the colours to 565 format
			        q[k++]= ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
			        q[k++] = ((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
			    }
				HAL_SPI_Transmit(&GenSPI,(uint8_t *)q,i,500);
				i=0;
			}

		}
		FreeMemory((void *)q);
		MMPrintString("\r\n");

	}

    SpiCsHigh(Option.LCD_CS);                  //set CS high

    // revert to non enhanced SPI mode
}


//Print the bitmap of a char on the video output
//    x, y - the top left of the char
//    width, height - size of the char's bitmap
//    scale - how much to scale the bitmap
//	  fc, bc - foreground and background colour
//    bitmap - pointer to the bitmap
void DrawBitmapSPI(int x1, int y1, int width, int height, int scale, int fc, int bc, unsigned char *bitmap){
    int i, j, k, m, n;
    char f[2],b[2];
      int vertCoord, horizCoord, XStart, XEnd, YEnd;
#if defined(MX470) || defined(STM32)
    char *p=0;
    union colourmap {
    char rgbbytes[4];
    unsigned int rgb;
    } c;
    if(bc == -1 && Option.DISPLAY_TYPE != ILI9341) bc = 0xFFFFFF;
#endif
    if(x1>=HRes || y1>=VRes || x1+width*scale<0 || y1+height*scale<0)return;
    // adjust when part of the bitmap is outside the displayable coordinates
    vertCoord = y1; if(y1 < 0) y1 = 0;                                 // the y coord is above the top of the screen
    XStart = x1; if(XStart < 0) XStart = 0;                            // the x coord is to the left of the left marginn
    XEnd = x1 + (width * scale) - 1; if(XEnd >= HRes) XEnd = HRes - 1; // the width of the bitmap will extend beyond the right margin
    YEnd = y1 + (height * scale) - 1; if(YEnd >= VRes) YEnd = VRes - 1;// the height of the bitmap will extend beyond the bottom margin
#if defined(MX470) || defined(STM32)
    if(bc == -1) {                                                     //special case of overlay text
        i = 0;
        j = width * height * scale * scale * 3;
        p = GetMemory(j);                                              //allocate some temporary memory
        ReadBuffer(XStart, y1, XEnd, YEnd, p);
    }
#endif
    // convert the colours to 565 format
    f[0]= ((fc >> 16) & 0b11111000) | ((fc >> 13) & 0b00000111);
    f[1] = ((fc >>  5) & 0b11100000) | ((fc >>  3) & 0b00011111);
    b[0] = ((bc >> 16) & 0b11111000) | ((bc >> 13) & 0b00000111);
    b[1] = ((bc >>  5) & 0b11100000) | ((bc >>  3) & 0b00011111);
    DefineRegionSPI(XStart, y1, XEnd, YEnd, 1);


    PinSetBit(Option.LCD_CD, LATSET);                               //set CD high
    set_cs();
    n = 0;
    for(i = 0; i < height; i++) {                                   // step thru the font scan line by line
        for(j = 0; j < scale; j++) {                                // repeat lines to scale the font
            if(vertCoord++ < 0) continue;                           // we are above the top of the screen
            if(vertCoord > VRes) {                                  // we have extended beyond the bottom of the screen
#if defined(MX470) || defined(STM32)
                if(p != NULL) FreeMemory(p);
#endif
                return;
            }
            horizCoord = x1;
            for(k = 0; k < width; k++) {                            // step through each bit in a scan line
                for(m = 0; m < scale; m++) {                        // repeat pixels to scale in the x axis
                    if(horizCoord++ < 0) continue;                  // we have not reached the left margin
                    if(horizCoord > HRes) continue;                 // we are beyond the right margin
                    if((bitmap[((i * width) + k)/8] >> (((height * width) - ((i * width) + k) - 1) %8)) & 1) {
                        SPIqueue((uint8_t *)&f);
                    } else {
#if defined(MX470) || defined(STM32)
                        if(bc == -1){
                            c.rgbbytes[0] = p[n];
                            c.rgbbytes[1] = p[n+1];
                            c.rgbbytes[2] = p[n+2];
                            b[0] = ((c.rgb >> 16) & 0b11111000) | ((c.rgb >> 13) & 0b00000111);
                            b[1] = ((c.rgb >>  5) & 0b11100000) | ((c.rgb >>  3) & 0b00011111);
                        } 
                        SPIqueue((uint8_t *)&b);

#else
                        SPIqueue((uint8_t *)&b);
#endif
                    }
                    n += 3;
                }
            }
        }
    }

    SpiCsHigh(Option.LCD_CS);                                       //set CS high

    // revert to non enhanced SPI mode
#if defined(MX470) || defined(STM32)
    if(p != NULL) FreeMemory(p);
#endif

}


// the default function for DrawRectangle() and DrawBitmap()
void DisplayNotSet(void) {
    error("Display not configured");
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// These three functions allow the SPI port to be used by multiple drivers (LCD/touch/SD card)
// The BASIC use of the SPI port does NOT use these functions
// The MX170 uses SPI channel 1 which is shared by the BASIC program
// The MX470 uses SPI channel 2 which it has exclusive control of (needed because touch can be used at any time)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// config the SPI port for output
// it will not touch the port if it has already been opened
#ifndef STM32F4version
void MIPS16 OpenSpiChannel(void) {

    if(ExtCurrentConfig[SPISYS_OUT_PIN] != EXT_BOOT_RESERVED) {
        ExtCfg(SPISYS_OUT_PIN, EXT_BOOT_RESERVED, 0);
        ExtCfg(SPISYS_INP_PIN, EXT_BOOT_RESERVED, 0);
        ExtCfg(SPISYS_CLK_PIN, EXT_BOOT_RESERVED, 0);
        CurrentSPISpeed=NONE_SPI_SPEED;
    }

}
#else
void MIPS16 OpenSpiChannel(void) {
    if(ExtCurrentConfig[SPI2_OUT_PIN] != EXT_BOOT_RESERVED) {
        ExtCfg(SPI2_OUT_PIN, EXT_BOOT_RESERVED, 0);
        ExtCfg(SPI2_INP_PIN, EXT_BOOT_RESERVED, 0);
        ExtCfg(SPI2_CLK_PIN, EXT_BOOT_RESERVED, 0);
        CurrentSPISpeed=NONE_SPI_SPEED;
    }
}
#endif

void SPISpeedSet(int speed){
    if(CurrentSPISpeed != speed){
        HAL_SPI_DeInit(&GenSPI);
    	if(speed==LCD_SPI_SPEED){
    		CurrentSPISpeed=LCD_SPI_SPEED;
    		GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    		GenSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
    		GenSPI.Init.BaudRatePrescaler = (Option.DISPLAY_TYPE==ILI9481 ? SPI_BAUDRATEPRESCALER_4 : SPI_BAUDRATEPRESCALER_2);
    	} else if(speed==SD_SLOW_SPI_SPEED){
    		CurrentSPISpeed=SD_SLOW_SPI_SPEED;
    		GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    		GenSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
    		GenSPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_256;
    	} else if(speed==SD_FAST_SPI_SPEED){
    		GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    		GenSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
    		CurrentSPISpeed=SD_FAST_SPI_SPEED;
    		GenSPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_4;
    	} else if(speed==TOUCH_SPI_SPEED){
    		if(Option.DISPLAY_TYPE!=ILI9481){
    			GenSPI.Init.CLKPolarity = SPI_POLARITY_HIGH;
    			GenSPI.Init.CLKPhase = SPI_PHASE_2EDGE;
    			CurrentSPISpeed=TOUCH_SPI_SPEED;
    			GenSPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_256;
    		} else {
    			GenSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    			GenSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
    			CurrentSPISpeed=TOUCH_SPI_SPEED;
    			GenSPI.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_64;
    		}
    	}
   	  HAL_SPI_Init(&GenSPI);
    }

}
// set the chip select for the SPI to low (enabled)
// if the SPI is currently set to a different mode or baudrate this will change it accordingly
// also, it checks if the chip select pin needs to be changed
void SpiCsLow(int pin, int speed) {
	SPISpeedSet(speed);
    if(pin) PinSetBit(pin, LATCLR);                                 // set CS low
}


// set the chip select for SPI to high (disabled)
void SpiCsHigh(int pin) {
    if(pin) PinSetBit(pin, LATSET);                                 // set CS high
}

