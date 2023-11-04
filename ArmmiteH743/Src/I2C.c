/************************************************************************************************************************
Micromite

I2C.c

Routines to handle I2C access.

Copyright 2011 Gerard Sexton
This file is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

************************************************************************************************************************/


#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
extern I2C_HandleTypeDef hi2c1, hi2c2;

// Declare functions
void i2cEnable(char *p);
void i2cDisable(char *p);
void i2cSend(char *p);
void i2cReceive(char *p);
void i2c_disable(void);
void i2c_enable(int bps);
void i2c_masterCommand(int timer);
void i2cCheck(char *p);
void i2c2Enable(char *p);
void i2c2Disable(char *p);
void i2c2Send(char *p);
void i2c2Receive(char *p);
void i2c2_disable(void);
void i2c2_enable(int bps);
void i2c2_masterCommand(int timer);
void i2c2Check(char *p);
static MMFLOAT *I2C_Rcvbuf_Float;										// pointer to the master receive buffer for a MMFLOAT
static long long int *I2C_Rcvbuf_Int;								// pointer to the master receive buffer for an integer
static char *I2C_Rcvbuf_String;										// pointer to the master receive buffer for a string
static unsigned int I2C_Addr;										// I2C device address
static volatile unsigned int I2C_Sendlen;							// length of the master send buffer
static volatile unsigned int I2C_Rcvlen;							// length of the master receive buffer
static unsigned char I2C_Send_Buffer[256];                                   // I2C send buffer
static unsigned int I2C_enabled;									// I2C enable marker
static unsigned int I2C_Timeout;									// master timeout value
static unsigned int I2C_Status;										// status flags
int mmI2Cvalue;
	// value of MM.I2C
	static MMFLOAT *I2C2_Rcvbuf_Float;										// pointer to the master receive buffer for a MMFLOAT
	static long long int *I2C2_Rcvbuf_Int;								// pointer to the master receive buffer for an integer
	static char *I2C2_Rcvbuf_String;										// pointer to the master receive buffer for a string
	static unsigned int I2C2_Addr;										// I2C device address
	static volatile unsigned int I2C2_Sendlen;							// length of the master send buffer
	static volatile unsigned int I2C2_Rcvlen;							// length of the master receive buffer
	static unsigned char I2C2_Send_Buffer[256];                                   // I2C send buffer
	static unsigned int I2C2_enabled;									// I2C enable marker
	static unsigned int I2C2_Timeout;									// master timeout value
	static unsigned int I2C2_Status;										// status flags
static int fifo=0;
int cameraopen=0;
extern void fastwrite480(int fnbr);
extern RTC_HandleTypeDef hrtc;
extern void SaveToBuffer(void);
extern void CompareToBuffer(void);
/*******************************************************************************************
							  I2C related commands in MMBasic
                              ===============================
These are the functions responsible for executing the I2C related commands in MMBasic
They are supported by utility functions that are grouped at the end of this file

********************************************************************************************/
#ifdef STM32H743xx
#define ov7670_address 0x21

void ov7670_set(char a, char b){
        char buff[2];
        //send the command
        I2C_Sendlen = 2;                                                // send one byte
        I2C_Rcvlen = 0;
        I2C_Send_Buffer[0] = a;                                         // the first register to read
        I2C_Send_Buffer[1] = b;                                         // the first register to read
        I2C_Addr = ov7670_address;                                      // address of the device 0x21
        i2c_masterCommand(1);
        if(mmI2Cvalue) error("I2C failure");
        if(a==REG_COM7 && b==COM7_RESET){
            HAL_Delay(200);
            return;
        }
        I2C_Sendlen = 1;                                                // send one byte
        i2c_masterCommand(1);
        if(mmI2Cvalue) error("I2C failure");
        //read the value
        I2C_Rcvbuf_String = buff;                                       // we want a string of bytes
        I2C_Rcvbuf_Float = NULL;
        I2C_Rcvbuf_Int = NULL;
        I2C_Rcvlen = 1;                                                 // get 7 bytes
        I2C_Sendlen = 0;
        I2C_Addr = ov7670_address;                                                // address of the device
        i2c_masterCommand(1);
        if(mmI2Cvalue) error("I2C failure");
        if(buff[0]!=b) error("Camera Config Failure");
        return;
}
void CloseCamera(void){
	if(cameraopen){
		SetAndReserve(OV7670_DAT0, P_INPUT, 0, EXT_NOT_CONFIG);    // config the eight pins used for the data
		SetAndReserve(OV7670_DAT1, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_DAT2, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_DAT3, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_DAT4, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_DAT5, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_DAT6, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_DAT7, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_PCLK, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_VSYNC, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_HREF, P_INPUT, 0, EXT_NOT_CONFIG);
		SetAndReserve(OV7670_XCLK, P_INPUT, 0, EXT_NOT_CONFIG);    // config the eight pins used for the data
		cameraopen=0;
	}
    return;
}

void cmd_Camera(void){
	if(HAS_100PINS)error("Not supported on 100 pin boards");
	GPIO_InitTypeDef GPIO_InitStruct;
    int us=128000000;
    int OV7670I2C;
    char buff[2];
    char *p;
    int reg,val,width=640;
    if(HRes<width)width=HRes;
    if(Option.DISPLAY_TYPE < SSD1963_5_640)error("Display must be 640x480 SSD1963 in buffered mode");
    if((p = checkstring(cmdline, "OPEN")) !=NULL) {

        if(cameraopen)error("Already Open");
        SetAndReserve(OV7670_DAT0, P_INPUT, 0, EXT_COM_RESERVED);    // config the eight pins used for the data
        SetAndReserve(OV7670_DAT1, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_DAT2, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_DAT3, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_DAT4, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_DAT5, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_DAT6, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_DAT7, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_PCLK, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_VSYNC, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_HREF, P_INPUT, 0, EXT_COM_RESERVED);
        SetAndReserve(OV7670_XCLK, P_OUTPUT, 1, EXT_COM_RESERVED);
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI48, RCC_MCODIV_4);
    	if(!I2C_enabled) {
            I2C_Timeout = 50;
            i2c_enable(100);                                            // initialise the I2C interface
            OV7670I2C =  true;
        } else
            OV7670I2C = false;
        uSec(5000);
        I2C_Sendlen = 1;                                                // send one byte
        I2C_Rcvlen = 0;
        *I2C_Send_Buffer = REG_PID;                                           // the first register to read
        I2C_Addr = ov7670_address;                                                // address of the device
    	I2C_Status = 0;
        i2c_masterCommand(1);
        if(mmI2Cvalue) error("I2C failure % ",mmI2Cvalue);
        I2C_Rcvbuf_String = buff;                                       // we want a string of bytes
        I2C_Rcvbuf_Float = NULL;
        I2C_Rcvbuf_Int = NULL;
        I2C_Rcvlen = 1;                                                 // get 7 bytes
        I2C_Sendlen = 0;
        I2C_Addr = ov7670_address;                                                // address of the device
    	I2C_Status = 0;
        i2c_masterCommand(1);
        if(mmI2Cvalue) error("I2C failure");
        if(buff[0]!=118) error("Camera not found");
        ov7670_set(0x12, 0x80);                  // RESET CAMERA
        ov7670_set(0x12, 0x80);                  // RESET CAMERA
    	ov7670_set( REG_RGB444, 0 );
        ov7670_set(REG_COM10, 0x02);               // 0x02   VSYNC negative (http://nasulica.homelinux.org/?p=959)
 		ov7670_set( REG_MVFP, 0x37);
 		if(fifo) {
            ov7670_set( REG_CLKRC, 0x81);
        }  else  ov7670_set( REG_CLKRC, 0x83);
 	    ov7670_set(REG_COM11, 0x0A) ;
        ov7670_set(REG_COM7, COM7_RGB) ;
 		ov7670_set( REG_COM1, 0);
 		ov7670_set( REG_COM15, COM15_RGB565);
 		ov7670_set(REG_COM9, 0x2A);
        ov7670_set(REG_TSLB, 0x04);                // 0D = UYVY  04 = YUYV
        ov7670_set(REG_COM13, 0x88 );
 		ov7670_set(REG_HSTART,0x13);
 		ov7670_set(REG_HSTOP, 0x01);
 		ov7670_set(REG_HREF, 0xb6);
 		ov7670_set(REG_VSTART, 0x02);
 		ov7670_set(REG_VSTOP, 0x7a);
 		ov7670_set(REG_VREF, 0x0a);
 		ov7670_set(REG_COM5, 0x61 );
 		ov7670_set(REG_COM6, 0x4b);
 		ov7670_set(0x16, 0x02 );
 		ov7670_set(0x21, 0x02 );
 		ov7670_set( 0x22, 0x91);
 		ov7670_set(0x29, 0x07 );
 		ov7670_set( 0x33, 0x0b);
 		ov7670_set(0x35, 0x0b );
 		ov7670_set( 0x37, 0x1d);
 		ov7670_set(0x38, 0x71 );
 		ov7670_set( 0x39, 0x2a);
 		ov7670_set(REG_COM12, 0x78 );

 		ov7670_set( 0x4d, 0x40);
 		ov7670_set(0x4e, 0x20 );
 		ov7670_set( REG_GFIX, 0 );
 		ov7670_set( 0x74, 0x10);
 		ov7670_set(0x8d, 0x4f );
 		ov7670_set( 0x8e, 0);
 		ov7670_set(0x8f, 0 );
 		ov7670_set( 0x90, 0 );
 		ov7670_set(0x91, 0 );
 		ov7670_set( 0x96, 0);
 		ov7670_set(0x9a, 0 );

 		ov7670_set( 0xb0, 0x84);
 		ov7670_set(0xb1, 0x0c );
 		ov7670_set( 0xb2, 0x0e);
 		ov7670_set(0xb3, 0x82 );//
 		ov7670_set( 0xb8, 0x0a);
// 		ov7670_set(0x7a, 0x20); //gamma correction
// 		ov7670_set(0x7b, 0x10);
// 		ov7670_set(0x7c, 0x1e);
// 		ov7670_set(0x7d, 0x35);
// 		ov7670_set(0x7e, 0x5a);
// 		ov7670_set(0x7f, 0x69);
// 		ov7670_set(0x80, 0x76);
// 		ov7670_set(0x81, 0x80);
// 		ov7670_set(0x82, 0x88);
// 		ov7670_set(0x83, 0x8f);
// 		ov7670_set(0x84, 0x96);
// 		ov7670_set(0x85, 0xa3);
// 		ov7670_set(0x86, 0xaf);
// 		ov7670_set(0x87, 0xc4);
// 		ov7670_set(0x88, 0xd7);
// 		ov7670_set(0x89, 0xe8);
 // AGC and AEC parameters. Note we start by disabling those features,
 //then turn them only after tweaking the values.
 		ov7670_set(0x13, 0xe7);
 		ov7670_set(0x00, 0);
 		ov7670_set(0x10, 0);
 		ov7670_set(0x0d, 0x40);
 		ov7670_set(0x14, 0x18);
 		ov7670_set(0xa5, 0x05);
 		ov7670_set(0xab, 0x07);
 		ov7670_set(0x24, 0x95);
 		ov7670_set(0x25, 0x33);
 		ov7670_set(0x26, 0xe3);
 		ov7670_set(0x9f, 0x78);
 		ov7670_set(0xa0, 0x68);
 		ov7670_set(0xa1, 0x03);
 		ov7670_set(0xa6, 0xd8);
 		ov7670_set(0xa7, 0xd8);
 		ov7670_set(0xa8, 0xf0);
 		ov7670_set(0xa9, 0x90);
 		ov7670_set(0xaa, 0x94);
 		ov7670_set(0x13, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT | COM8_AGC | COM8_AEC);
 // Almost all of these are magic "reserved" values. */
 		ov7670_set(0x0e, 0x61);
 		ov7670_set(0x0f, 0x4b);
 		ov7670_set(0x16, 0x02);
// 		ov7670_set(0x1e, 0x27);
 		ov7670_set(0x21, 0x02);
 		ov7670_set(0x22, 0x91);
 		ov7670_set(0x29, 0x07);
 		ov7670_set(0x33, 0x0b);
 		ov7670_set(0x35, 0x0b);
 		ov7670_set(0x37, 0x1d);
 		ov7670_set(0x38, 0x71);
 		ov7670_set(0x39, 0x2a);
// 		ov7670_set(0x3c, 0x78);
 		ov7670_set(0x4d, 0x40);
 		ov7670_set(0x4e, 0x20);
 		ov7670_set(0x69, 0);
// 		ov7670_set(0x6b, 0x0a);
 		ov7670_set(0x74, 0x10);
 		ov7670_set(0x8d, 0x4f);
 		ov7670_set(0x8e, 0);
 		ov7670_set(0x8f, 0);
 		ov7670_set(0x90, 0);
 		ov7670_set(0x91, 0);
 		ov7670_set(0x96, 0);
 		ov7670_set(0x9a, 0);
 		ov7670_set(0xb0, 0x84);
 		ov7670_set(0xb1, 0x0c);
 		ov7670_set(0xb2, 0x0e);
 		ov7670_set(0xb3, 0x82);
 		ov7670_set(0xb8, 0x0a);
 // More reserved magic, some of which tweaks white balance */
 		ov7670_set(0x43, 0x0a);
 		ov7670_set(0x44, 0xf0);
 		ov7670_set(0x45, 0x34);
 		ov7670_set(0x46, 0x58);
 		ov7670_set(0x47, 0x28);
 		ov7670_set(0x48, 0x3a);
 		ov7670_set(0x59, 0x88);
 		ov7670_set(0x5a, 0x88);
 		ov7670_set(0x5b, 0x44);
 		ov7670_set(0x5c, 0x67);
 		ov7670_set(0x5d, 0x49);
 		ov7670_set(0x5e, 0x0e);
 		ov7670_set(0x6c, 0x0a);
 		ov7670_set(0x6d, 0x55);
 		ov7670_set(0x6e, 0x11);
 		ov7670_set(0x6f, 0x9f);
 		ov7670_set(0x6a, 0x40);
 		ov7670_set(0x01, 0x40);
 		ov7670_set(0x02, 0x60);
            // COLOR SETTING
        ov7670_set(0x4f, 0x80);
        ov7670_set(0x50, 0x80);
        ov7670_set(0x51, 0x00);
        ov7670_set(0x52, 0x22);
        ov7670_set(0x53, 0x5e);
        ov7670_set(0x54, 0x80);
        ov7670_set(0x56, 0x50);
        ov7670_set(0x58, 0x9e);
        ov7670_set(0x59, 0x88);
        ov7670_set(0x5a, 0x88);
        ov7670_set(0x5b, 0x44);
        ov7670_set(0x5c, 0x67);
        ov7670_set(0x5d, 0x49);
        ov7670_set(0x5e, 0x0e);
        ov7670_set(0x69, 0x00);
        ov7670_set(0x6a, 0x40);
        ov7670_set(0x6b, 0x0a);
        ov7670_set(0x6c, 0x0a);
        ov7670_set(0x6d, 0x55);
        ov7670_set(0x6e, 0x11);
        ov7670_set(0x6f, 0x9f);

        ov7670_set(0xb0, 0x84);
 		ov7670_set(0x13, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT | COM8_AGC | COM8_AEC | COM8_AWB);
 // Matrix coefficients */
 		ov7670_set(0x4f, 0xc0);
 		ov7670_set(0x50, 0xc0);
 		ov7670_set(0x51, 0);
 		ov7670_set(0x52, 0x33);
 		ov7670_set(0x53, 0x9d);
 		ov7670_set(0x54, 0xc0);
 		ov7670_set(0x58, 0x9e);

 		ov7670_set(0x41, 0x08);
 		ov7670_set(0x3f, 0);
 		ov7670_set(0x75, 0x05);
 		ov7670_set(0x76, 0xe1);
 		ov7670_set(0x4c, 0);
 		ov7670_set(0x77, 0x01);
 		ov7670_set(0x3d, 0xc3);
 		ov7670_set(0x4b, 0x09);
 		ov7670_set(0x41, 0x38);
 		ov7670_set(0x56, 0x40);

 		ov7670_set(0x34, 0x11);
 		ov7670_set(0x3b, COM11_EXP | COM11_HZAUTO);
 		ov7670_set(0xa4, 0x88);
 		ov7670_set(0x96, 0);
 		ov7670_set(0x97, 0x30);
 		ov7670_set(0x98, 0x20);
 		ov7670_set(0x99, 0x30);
 		ov7670_set(0x9a, 0x84);
 		ov7670_set(0x9b, 0x29);
 		ov7670_set(0x9c, 0x03);
 		ov7670_set(0x9d, 0x4c);
 		ov7670_set(0x9e, 0x3f);
 		ov7670_set(0x78, 0x04);
 // Extra-weird stuff. Some sort of multiplexor register */
 		ov7670_set(0x79, 0x01);
 		ov7670_set(0xc8, 0xf0);
 		ov7670_set(0x79, 0x0f);
 		ov7670_set(0xc8, 0x00);
 		ov7670_set(0x79, 0x10);
 		ov7670_set(0xc8, 0x7e);
 		ov7670_set(0x79, 0x0a);
 		ov7670_set(0xc8, 0x80);
 		ov7670_set(0x79, 0x0b);
 		ov7670_set(0xc8, 0x01);
 		ov7670_set(0x79, 0x0c);
 		ov7670_set(0xc8, 0x0f);
 		ov7670_set(0x79, 0x0d);
 		ov7670_set(0xc8, 0x20);
 		ov7670_set(0x79, 0x09);
 		ov7670_set(0xc8, 0x80);
 		ov7670_set(0x79, 0x02);
 		ov7670_set(0xc8, 0xc0);
 		ov7670_set(0x79, 0x03);
 		ov7670_set(0xc8, 0x40);
 		ov7670_set(0x79, 0x05);
 		ov7670_set(0xc8, 0x30);
 		ov7670_set(0x79, 0x26);
        if(OV7670I2C) i2c_disable();
        cameraopen=1;
        return;
    } else if((p = checkstring(cmdline, "CAPTURE")) !=NULL){
    	unsigned short a,c;
    	int i,y;
    	unsigned char *s;
        SetAreaSSD1963(80, 0, HRes+80, VRes-1);
        WriteSSD1963Command(CMD_WR_MEMSTART);
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
                // first byte
                while(!ST_PCLK); // wait for clock to go high
                a=((*s++) <<8);
                i++;
                while (ST_PCLK); // wait for clock to go back low
                // second byte
                *s=GPIOC->IDR & 0xFF;
                a|=*s;
                s++;
                while (!ST_PCLK); // wait for clock to go high
                GPIOE->ODR=a;
            	GPIOG->BSRR= GPIO_PIN_10<<16;
                while (ST_PCLK); // wait for clock to go back low
                GPIOG->BSRR= GPIO_PIN_10;
            }
        }
        __enable_irq();
        RtcGetTime();
        return;
    } else if(checkstring(cmdline, "BACKUP")){
        SaveToBuffer();
        return;
    } else if(checkstring(cmdline, "CLOSE")){
        if(!cameraopen)error("Camera not OPEN");
        CloseCamera();
        return;
    } else if((p = checkstring(cmdline, "REGISTER")) !=NULL){
        if(!cameraopen)error("Camera not OPEN");
        getargs(&p, 3, ",");
        if(argc != 3) error("Argument count");
        reg = getint(argv[0], 0, 0xFF);
        val = getint(argv[2], 0, 0xFF);
        if(I2C_Send_Buffer == NULL) {
            I2C_Timeout = 5;
            i2c_enable(100);                                            // initialise the I2C interface
            PinSetBit(P_I2C_SDA, CNPUSET);
            PinSetBit(P_I2C_SCL, CNPUSET);
            OV7670I2C =  true;
        } else
            OV7670I2C = false;
 		ov7670_set(reg, val);
        if(OV7670I2C) i2c_disable();
        return;
    } else error("Invalid Command");
}
#endif

void cmd_i2c(void) {
    char *p;//, *pp;

    if((p = checkstring(cmdline, "OPEN")) != NULL)
        i2cEnable(p);
    else if((p = checkstring(cmdline, "CLOSE")) != NULL)
        i2cDisable(p);
    else if((p = checkstring(cmdline, "WRITE")) != NULL)
        i2cSend(p);
    else if((p = checkstring(cmdline, "READ")) != NULL)
        i2cReceive(p);
    else if((p = checkstring(cmdline, "CHECK")) != NULL)
        i2cCheck(p);
    else
        error("Unknown command");
}
void cmd_i2c2(void) {
    char *p;//, *pp;

    if((p = checkstring(cmdline, "OPEN")) != NULL)
        i2c2Enable(p);
    else if((p = checkstring(cmdline, "CLOSE")) != NULL)
        i2c2Disable(p);
    else if((p = checkstring(cmdline, "WRITE")) != NULL)
        i2c2Send(p);
    else if((p = checkstring(cmdline, "READ")) != NULL)
        i2c2Receive(p);
    else if((p = checkstring(cmdline, "CHECK")) != NULL)
        i2c2Check(p);
    else
        error("Unknown command");
}
/*
 * *Initialize RTC and set the Time and Date
*/
void RtcGetTime(void){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	int localsecs;
	if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	localsecs=sTime.Seconds;
	if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	milliseconds=1000-((sTime.SubSeconds*1000)/sTime.SecondFraction);
	if(sTime.SubSeconds<5){
		do {
			if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
			{
				error("RTC hardware error");
			}
			if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
			{
				error("RTC hardware error");
			}
			milliseconds=1000-((sTime.SubSeconds*1000)/sTime.SecondFraction);
		} while(localsecs==sTime.Seconds);
	}
	second = sTime.Seconds;
	minute = sTime.Minutes;
	hour = sTime.Hours;
	day = sDate.Date;
	month = sDate.Month;
	year = sDate.Year+2000;
	day_of_week=sDate.WeekDay;
}

// enable the I2C1 module - master mode
void i2cEnable(char *p) {
	int speed, timeout;
	getargs(&p, 3, ",");
	if(argc != 3) error("Invalid syntax");
	speed = getinteger(argv[0]);
	if(!(speed ==100 || speed == 400 || speed==1000)) error("Valid speeds 100, 400, 1000");
	timeout = getinteger(argv[2]);
	if(timeout < 0 || (timeout > 0 && timeout < 100)) error("Number out of bounds" );
	if(I2C_enabled) error("I2C already OPEN");
	I2C_Timeout = timeout;
	i2c_enable(speed);

}
// enable the I2C2 module - master mode
void i2c2Enable(char *p) {
	int speed, timeout;
	getargs(&p, 3, ",");
	if(argc != 3) error("Invalid syntax");
	speed = getinteger(argv[0]);
	if(!(speed ==100 || speed == 400 || speed==1000)) error("Valid speeds 100, 400, 1000");
	timeout = getinteger(argv[2]);
	if(timeout < 0 || (timeout > 0 && timeout < 100)) error("Number out of bounds" );
	if(I2C2_enabled) error("I2C already OPEN");
	I2C2_Timeout = timeout;
	i2c2_enable(speed);

}


// disable the I2C1 module - master mode
void i2cDisable(char *p) {
	i2c_disable();
}

// disable the I2C2 module - master mode
void i2c2Disable(char *p) {
	i2c2_disable();
}

// send data to an I2C slave - master mode
void i2cSend(char *p) {
	int addr, i2c_options, sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;

	getargs(&p, 99, ",");
	if(!(argc & 0x01) || (argc < 7)) error("Invalid syntax");
	if(!I2C_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	i2c_options = getinteger(argv[2]);
	if(i2c_options < 0 || i2c_options > 3) error("Number out of bounds");
	I2C_Status = 0;
	if(i2c_options & 0x01) I2C_Status = I2C_Status_BusHold;
	I2C_Addr = addr;
	sendlen = getinteger(argv[4]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 7) {		// numeric expressions for data
		if(sendlen != ((argc - 5) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C_Send_Buffer[i] = getinteger(argv[i + i + 6]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[6], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	I2C_Sendlen = sendlen;
	I2C_Rcvlen = 0;

	i2c_masterCommand(1);
}
// send data to an I2C slave - master mode
void i2c2Send(char *p) {
	int addr, i2c2_options, sendlen, i;
	void *ptr = NULL;
	unsigned char *cptr = NULL;

	getargs(&p, 99, ",");
	if(!(argc & 0x01) || (argc < 7)) error("Invalid syntax");
	if(!I2C2_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	i2c2_options = getinteger(argv[2]);
	if(i2c2_options < 0 || i2c2_options > 3) error("Number out of bounds");
	I2C2_Status = 0;
	if(i2c2_options & 0x01) I2C2_Status = I2C_Status_BusHold;
	I2C2_Addr = addr;
	sendlen = getinteger(argv[4]);
	if(sendlen < 1 || sendlen > 255) error("Number out of bounds");

	if(sendlen == 1 || argc > 7) {		// numeric expressions for data
		if(sendlen != ((argc - 5) >> 1)) error("Incorrect argument count");
		for (i = 0; i < sendlen; i++) {
			I2C2_Send_Buffer[i] = getinteger(argv[i + i + 6]);
		}
	} else {		// an array of MMFLOAT, integer or a string
		ptr = findvar(argv[6], V_NOFIND_NULL | V_EMPTY_OK);
		if(ptr == NULL) error("Invalid variable");
		if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].dims[0] == 0) {		// string
			cptr = (unsigned char *)ptr;
			cptr++;																	// skip the length byte in a MMBasic string
			for (i = 0; i < sendlen; i++) {
				I2C2_Send_Buffer[i] = (int)(*(cptr + i));
			}
		} else if((vartbl[VarIndex].type & T_NBR) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// numeric array
			if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C2_Send_Buffer[i] = (int)(*((MMFLOAT *)ptr + i));
				}
			}
		} else if((vartbl[VarIndex].type & T_INT) && vartbl[VarIndex].dims[0] > 0 && vartbl[VarIndex].dims[1] == 0) {		// integer array
			if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + sendlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) ) {
				error("Insufficient data");
			} else {
				for (i = 0; i < sendlen; i++) {
					I2C2_Send_Buffer[i] = (int)(*((long long int *)ptr + i));
				}
			}
		} else error("Invalid variable");
	}
	I2C2_Sendlen = sendlen;
	I2C2_Rcvlen = 0;

	i2c2_masterCommand(1);
}

void i2cCheck(char *p) {
	int addr;
	getargs(&p, 1, ",");
	if(!I2C_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
    if(addr<1 || addr>0x7F)error("Invalid I2C address");
    addr<<=1;
	mmI2Cvalue = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)addr, 2, 10);
}
void i2c2Check(char *p) {
	int addr;
	getargs(&p, 1, ",");
	if(!I2C2_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
    if(addr<1 || addr>0x7F)error("Invalid I2C address");
    addr<<=1;
	mmI2Cvalue = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)addr, 2, 10);
}
// receive data from an I2C slave - master mode
void i2cReceive(char *p) {
	int addr, i2c_options, rcvlen;
	void *ptr = NULL;
	getargs(&p, 7, ",");
	if(argc != 7) error("Invalid syntax");
	if(!I2C_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	i2c_options = getinteger(argv[2]);
	if(i2c_options < 0 || i2c_options > 3) error("Number out of bounds");
	I2C_Status = 0;
	if(i2c_options & 0x01) I2C_Status = I2C_Status_BusHold;
	I2C_Addr = addr;
	rcvlen = getinteger(argv[4]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");

	ptr = findvar(argv[6], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C_Rcvbuf_Int = (long long int *)ptr;
        I2C_Rcvbuf_Float = NULL;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C_Rcvbuf_String = (char *)ptr + 1;
        I2C_Rcvbuf_Float = NULL;
        I2C_Rcvbuf_Int = NULL;
    } else error("Invalid variable");
	I2C_Rcvlen = rcvlen;

	I2C_Sendlen = 0;

	i2c_masterCommand(1);
}
// receive data from an I2C slave - master mode
void i2c2Receive(char *p) {
	int addr, i2c2_options, rcvlen;
	void *ptr = NULL;
	getargs(&p, 7, ",");
	if(argc != 7) error("Invalid syntax");
	if(!I2C2_enabled)error("I2C not open");
	addr = getinteger(argv[0]);
	i2c2_options = getinteger(argv[2]);
	if(i2c2_options < 0 || i2c2_options > 3) error("Number out of bounds");
	I2C2_Status = 0;
	if(i2c2_options & 0x01) I2C2_Status = I2C_Status_BusHold;
	I2C2_Addr = addr;
	rcvlen = getinteger(argv[4]);
	if(rcvlen < 1 || rcvlen > 255) error("Number out of bounds");

	ptr = findvar(argv[6], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
	if(ptr == NULL) error("Invalid variable");
	if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((MMFLOAT *)ptr - vartbl[VarIndex].val.fa) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C2_Rcvbuf_Float = (MMFLOAT*)ptr;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            if(rcvlen != 1) error("Invalid variable");
        } else {		// An array
            if( (((long long int *)ptr - vartbl[VarIndex].val.ia) + rcvlen) > (vartbl[VarIndex].dims[0] + 1 - OptionBase) )
                error("Insufficient space in array");
        }
        I2C2_Rcvbuf_Int = (long long int *)ptr;
        I2C2_Rcvbuf_Float = NULL;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid variable");
        *(char *)ptr = rcvlen;
        I2C2_Rcvbuf_String = (char *)ptr + 1;
        I2C2_Rcvbuf_Float = NULL;
        I2C2_Rcvbuf_Int = NULL;
    } else error("Invalid variable");
	I2C2_Rcvlen = rcvlen;

	I2C2_Sendlen = 0;

	i2c2_masterCommand(1);
}

/**************************************************************************************************
Enable the I2C1 module - master mode
***************************************************************************************************/
void i2c_enable(int bps) {
    HAL_I2C_DeInit(&hi2c1);
//#ifdef STM32F4version
    ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
    PinSetBit(P_I2C_SDA, TRISSET);
    if(PinRead(P_I2C_SDA) == 0) {
    	int i;
        // it appears as if something is holding SLA low
        // try pulsing the clock to get rid of it
        PinSetBit(P_I2C_SCL, TRISCLR);
        PinSetBit(P_I2C_SCL, LATCLR);
        for(i = 0; i < 20; i++) {
            uSec(25);
            PinSetBit(P_I2C_SCL, LATINV);
            if(PinRead(P_I2C_SDA) == 0) break;
        }
        PinSetBit(P_I2C_SCL, TRISSET);
    }
    ExtCfg(P_I2C_SCL, EXT_COM_RESERVED, 0);
    ExtCfg(P_I2C_SDA, EXT_COM_RESERVED, 0);
   // if(bps==100) hi2c1.Init.ClockSpeed = 100000;
   // if(bps==400) hi2c1.Init.ClockSpeed = 400000;
//#else
    if(bps==100) hi2c1.Init.Timing = 0x10707DBC;
    if(bps==400) hi2c1.Init.Timing = 0x00602173;
    if(bps==1000)hi2c1.Init.Timing = 0x00300B29;
//#endif
    HAL_I2C_Init(&hi2c1);
	I2C_enabled=1;
}
void i2c2_enable(int bps) {
    HAL_I2C_DeInit(&hi2c2);
//#ifdef STM32F4version
    ExtCfg(P_I2C2_SDA, EXT_NOT_CONFIG, 0);
    ExtCfg(P_I2C2_SCL, EXT_NOT_CONFIG, 0);
    PinSetBit(P_I2C2_SDA, TRISSET);
    if(PinRead(P_I2C2_SDA) == 0) {
        int i;
        // it appears as if something is holding SLA low
        // try pulsing the clock to get rid of it
        PinSetBit(P_I2C2_SCL, TRISCLR);
        PinSetBit(P_I2C2_SCL, LATCLR);
        for(i = 0; i < 20; i++) {
           uSec(25);
           PinSetBit(P_I2C2_SCL, LATINV);
           if(PinRead(P_I2C2_SDA) == 0) break;
        }
        PinSetBit(P_I2C2_SCL, TRISSET);
    }
    ExtCfg(P_I2C2_SCL, EXT_COM_RESERVED, 0);
    ExtCfg(P_I2C2_SDA, EXT_COM_RESERVED, 0);
//    if(bps==100) hi2c2.Init.ClockSpeed = 100000;
//    if(bps==400) hi2c2.Init.ClockSpeed = 400000;
//#else
    if(bps==100) hi2c2.Init.Timing = 0x10707DBC;
    if(bps==400) hi2c2.Init.Timing = 0x00602173;
    if(bps==1000)hi2c2.Init.Timing = 0x00300B29;
//#endif
    HAL_I2C_Init(&hi2c2);
	I2C2_enabled=1;
}

/**************************************************************************************************
Disable the I2C1 module - master mode
***************************************************************************************************/
void i2c_disable() {
    I2C_Status = I2C_Status_Disable;
	I2C_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
    I2C_Rcvbuf_Float = NULL;
    I2C_Rcvbuf_Int = NULL;
	I2C_Sendlen = 0;												// length of the master send buffer
	I2C_Rcvlen = 0;													// length of the master receive buffer
	I2C_Addr = 0;													// I2C device address
	I2C_Timeout = 0;												// master timeout value
	HAL_I2C_DeInit(&hi2c1);
	I2C_enabled = 0;
	ExtCfg(P_I2C_SDA, EXT_NOT_CONFIG, 0);
	ExtCfg(P_I2C_SCL, EXT_NOT_CONFIG, 0);
}
void i2c2_disable() {
    I2C2_Status = I2C_Status_Disable;
	I2C2_Rcvbuf_String = NULL;                                       // pointer to the master receive buffer
    I2C2_Rcvbuf_Float = NULL;
    I2C2_Rcvbuf_Int = NULL;
	I2C2_Sendlen = 0;												// length of the master send buffer
	I2C2_Rcvlen = 0;													// length of the master receive buffer
	I2C2_Addr = 0;													// I2C device address
	I2C2_Timeout = 0;												// master timeout value
	HAL_I2C_DeInit(&hi2c2);
	I2C2_enabled = 0;
	ExtCfg(P_I2C2_SDA, EXT_NOT_CONFIG, 0);
	ExtCfg(P_I2C2_SCL, EXT_NOT_CONFIG, 0);
}
/**************************************************************************************************
Send and/or Receive data - master mode
***************************************************************************************************/
void i2c_masterCommand(int timer) {
//	unsigned char start_type,
	unsigned char i,i2caddr=I2C_Addr<<1,I2C_Rcv_Buffer[256];
	if(I2C_Sendlen){
		mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)i2caddr, I2C_Send_Buffer, I2C_Sendlen, I2C_Timeout);
	}
	if(I2C_Rcvlen){
		mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c1, (uint16_t)i2caddr, (uint8_t *)I2C_Rcv_Buffer, I2C_Rcvlen, I2C_Timeout);
				for(i=0;i<I2C_Rcvlen;i++){
					if(I2C_Rcvbuf_String!=NULL){
						*I2C_Rcvbuf_String=I2C_Rcv_Buffer[i];
						I2C_Rcvbuf_String++;
					}
					if(I2C_Rcvbuf_Float!=NULL){
						*I2C_Rcvbuf_Float=I2C_Rcv_Buffer[i];
						I2C_Rcvbuf_Float++;
					}
					if(I2C_Rcvbuf_Int!=NULL){
						*I2C_Rcvbuf_Int=I2C_Rcv_Buffer[i];
						I2C_Rcvbuf_Int++;
					}
				}
	}
}

void i2c2_masterCommand(int timer) {
//	unsigned char start_type,
	unsigned char i,i2c2addr=I2C2_Addr<<1,I2C2_Rcv_Buffer[256];
	if(I2C2_Sendlen){
		mmI2Cvalue=HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)i2c2addr, I2C2_Send_Buffer, I2C2_Sendlen, I2C2_Timeout);
	}
	if(I2C2_Rcvlen){
		mmI2Cvalue=HAL_I2C_Master_Receive(&hi2c2, (uint16_t)i2c2addr, (uint8_t *)I2C2_Rcv_Buffer, I2C2_Rcvlen, I2C2_Timeout);
				for(i=0;i<I2C2_Rcvlen;i++){
					if(I2C2_Rcvbuf_String!=NULL){
						*I2C2_Rcvbuf_String=I2C2_Rcv_Buffer[i];
						I2C2_Rcvbuf_String++;
					}
					if(I2C2_Rcvbuf_Float!=NULL){
						*I2C2_Rcvbuf_Float=I2C2_Rcv_Buffer[i];
						I2C2_Rcvbuf_Float++;
					}
					if(I2C2_Rcvbuf_Int!=NULL){
						*I2C2_Rcvbuf_Int=I2C2_Rcv_Buffer[i];
						I2C2_Rcvbuf_Int++;
					}
				}
	}
}

void fun_mmi2c(void) {
	iret = mmI2Cvalue;
    targ = T_INT;
}

