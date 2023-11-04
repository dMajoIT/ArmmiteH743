/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

Flash.c

Handles saving and restoring from flash.

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
//#include "stm32f4xx_flash.h"
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
uint32_t GetSector(uint32_t Address);
// The CFUNCTION data comes after the program in program memory
// and this is used to point to its start
unsigned char *CFunctionFlash = NULL;
unsigned char *CFunctionLibrary = NULL;
volatile struct option_s Option, *SOption;
char * ProgMemory;
static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t SectorError = 0;
volatile union u_flash {
  uint64_t i64[4];
  uint8_t  i8[32];
  uint32_t  i32[8];
} FlashWord, FlashWordSave;
volatile int i8p=0;
extern volatile int ConsoleRxBufHead, ConsoleRxBufTail;
extern volatile int ConsoleTxBufHead, ConsoleTxBufTail;
int sectorsave;
// globals used when writing bytes to flash
volatile uint32_t realflashpointer;
volatile uint8_t FlashDone=0;

// erase the flash and init the variables used to buffer bytes for writing to the flash
//void FlashWriteInit(int sector,int errabort) {
int FlashWriteInit(int sector) {
//	__IO uint32_t SectorsWRPStatus = 0xFFF;
    // Unlock the Flash to enable the flash control register access
	int i;
	uint32_t *j;
    SCB_DisableICache() ;
    SCB_DisableDCache() ;
	HAL_FLASH_Unlock();
	i8p=0;
	for(i=0;i<8;i++)FlashWord.i32[i]=0xFFFFFFFF;
	sectorsave=sector;
    // Clear pending flags (if any)

    // Get the number of the start and end sectors

       // Device voltage range supposed to be [2.7V to 3.6V], the operation will
       //  be done by word
      if(sector == PROGRAM_FLASH){
    	  realflashpointer=FLASH_PROGRAM_ADDR;
    	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    	  EraseInitStruct.Banks         = FLASH_BANK_2;
    	  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    	  EraseInitStruct.Sector = GetSector(FLASH_PROGRAM_ADDR);
    	  EraseInitStruct.NbSectors = PROG_FLASH_SIZE / 0x20000;
   		  FlashDone=0;
		  if(HAL_FLASHEx_Erase_IT(&EraseInitStruct)== HAL_OK){
			  while(FlashDone==0){
					CheckKeyboard();
			  }
			  j=(uint32_t *)FLASH_PROGRAM_ADDR;
			  while(j<(uint32_t *)(FLASH_PROGRAM_ADDR+PROG_FLASH_SIZE)){
				  //if(*j++ != 0xFFFFFFFF && errabort)error("Failed to erase Program memory");
				  if(*j++ != 0xFFFFFFFF)return 1;
			  }
		  } else return 1;
    	  //} else if(errabort)error("Failed to erase Program memory");
      }
      if(sector == LIBRARY_FLASH){
         	  realflashpointer=FLASH_LIBRARY_ADDR;
         	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
         	  EraseInitStruct.Banks         = FLASH_BANK_2;
         	  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
         	  EraseInitStruct.Sector = GetSector(FLASH_LIBRARY_ADDR);
         	  EraseInitStruct.NbSectors = 1;
        		  FlashDone=0;
     		  if(HAL_FLASHEx_Erase_IT(&EraseInitStruct)== HAL_OK){
     			  while(FlashDone==0){
     					CheckKeyboard();
     			  }
     			  j=(uint32_t *)FLASH_LIBRARY_ADDR;
     			  while(j<(uint32_t *)(FLASH_LIBRARY_ADDR+0x20000)){
     				 // if(*j++ != 0xFFFFFFFF && errabort)error("Failed to erase Library memory");
     				 if(*j++ != 0xFFFFFFFF)return 1;
     			  }
     		  } else return 1;
         	  //} else if(errabort)error("Failed to erase Library memory");
           }
      if(sector == SAVED_OPTIONS_FLASH){
      	  realflashpointer=FLASH_SAVED_OPTION_ADDR ;
    	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    	  EraseInitStruct.Banks         = FLASH_BANK_2;
    	  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    	  EraseInitStruct.Sector = GetSector(FLASH_SAVED_OPTION_ADDR);
    	  EraseInitStruct.NbSectors = 1;
   		  FlashDone=0;
		  if(HAL_FLASHEx_Erase_IT(&EraseInitStruct)== HAL_OK){
			  while(FlashDone==0){
					CheckKeyboard();
			  }
			  j=(uint32_t *)FLASH_SAVED_OPTION_ADDR;
			  while(j<(uint32_t *)(FLASH_SAVED_OPTION_ADDR+0x20000)){
				  //if(*j++ != 0xFFFFFFFF && errabort)error("Failed to erase Options memory");
				  if(*j++ != 0xFFFFFFFF)return 1;
			  }
		  } else return 1;
    	  //} else if(errabort)error("Failed to erase options memory");
      }
      if(sector == SAVED_VARS_FLASH){
       	  realflashpointer=FLASH_SAVED_VAR_ADDR ;
    	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    	  EraseInitStruct.Banks         = FLASH_BANK_2;
    	  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    	  EraseInitStruct.Sector = GetSector(FLASH_SAVED_VAR_ADDR);
    	  EraseInitStruct.NbSectors = 1;
		  if(HAL_FLASHEx_Erase_IT(&EraseInitStruct)== HAL_OK){
			  while(FlashDone==0){
					CheckKeyboard();
			  }
			  j=(uint32_t *)FLASH_SAVED_VAR_ADDR;
			  while(j<(uint32_t *)(FLASH_SAVED_VAR_ADDR+0x20000)){
				 // if(*j++ != 0xFFFFFFFF && errabort)error("Failed to erase Saved Vars memory");
				  if(*j++ != 0xFFFFFFFF)return 1;
			  }
		  } else return 1;
    	  //} else if(errabort)error("Failed to erase Saved Vars memory");
      }
      SCB_EnableICache() ;
      SCB_EnableDCache() ;
      return 0;
}
void FlashWriteBlock(void){
    int i,tries=0;
    uint32_t address=realflashpointer-32;
    uint32_t *there = (uint32_t *)address;
    if(address % 32)error("Flash write address");
    if(sectorsave == PROGRAM_FLASH && (address<FLASH_PROGRAM_ADDR || address>=FLASH_PROGRAM_ADDR+PROG_FLASH_SIZE))error("PROGRAM_FLASH location");
    if(sectorsave == LIBRARY_FLASH && (address<FLASH_LIBRARY_ADDR || address>=FLASH_LIBRARY_ADDR+0x20000))error("LIBRARY_FLASH location");
    if(sectorsave == SAVED_OPTIONS_FLASH && (address<FLASH_SAVED_OPTION_ADDR || address>=FLASH_SAVED_OPTION_ADDR+0x20000))error("SAVED_OPTIONS_FLASH location");
    if(sectorsave == SAVED_VARS_FLASH && (address<FLASH_SAVED_VAR_ADDR || address>=FLASH_SAVED_VAR_ADDR+0x20000))error("SAVED_VARS_FLASH location");
    for(i=0;i<8;i++){
    	if(there[i]!=0xFFFFFFFF) error("flash not erased");
    }
	//if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uint64_t)((uint32_t)FlashWord.i64)) != HAL_OK)
	//{
	//	error("Flash write fail");
	//}
	while((HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uint64_t)((uint32_t)FlashWord.i64)) != HAL_OK) && (tries++ < 10));
	if(tries==10)error("Flash write fail");
//    SCB_DisableICache() ;
//    SCB_DisableDCache() ;
//    SCB_EnableICache() ;
//    SCB_EnableDCache() ;
	for(i=0;i<8;i++)FlashWord.i32[i]=0xFFFFFFFF;
}
// write a byte to flash
// this will buffer four bytes so that the write to flash can be a word
void FlashWriteByte(unsigned char b) {
	realflashpointer++;
	FlashWord.i8[i8p]=b;
	i8p++;
	i8p %= 32;
	if(i8p==0){
		FlashWriteBlock();
	}
}

void FlashWriteAlign(void) {
	  while(i8p != 0) {
		  FlashWriteByte(0x0);
	  }
	  FlashWriteWord(0xFFFFFFFF);
}

// utility routine used by SaveProgramToFlash() and cmd_var to write a byte to flash while erasing the page if needed
void FlashWriteWord(unsigned int i) {
	FlashWriteByte(i & 0xFF);
	FlashWriteByte((i>>8) & 0xFF);
	FlashWriteByte((i>>16) & 0xFF);
	FlashWriteByte((i>>24) & 0xFF);
}


// flush any bytes in the buffer to flash
void FlashWriteClose(void) {
	  while(i8p != 0) {
		  FlashWriteByte(0xff);
	  }
}



/*******************************************************************************************************************
 Code to execute a CFunction saved in flash
*******************************************************************************************************************/
/*******************************************************************************************************************
 VARSAVE and VARSAVE RESTORE commands

 Variables are saved in flash as follows:
 Numeric variables:
     1 byte  = variable type
     ? bytes = the variable's name in uppercase
     1 byte  = zero byte terminating the variable's name
     4 or 8 bytes = the value of the variable
 String variables:
     1 byte  = variable type
     ? bytes = the variable's name in uppercase
     1 byte  = zero byte terminating the variable's name
     1 bytes = length of the variable's string
     ? bytes = the variables string

********************************************************************************************************************/


/*******************************************************************************************************************
 The variables are stored in a reserved flash area (which in total is 2K).
 The first few bytes are used for the options. So we must save the options in RAM before we erase, then write the
 options back.  The variables saved by this command are then written to flash starting just after the options.
********************************************************************************************************************/
void cmd_var(void) {
    char *p, *buf, *bufp, *varp, *vdata, lastc;
    int i, j, nbr = 1, nbr2=1, array, type, SaveDefaultType;
    int VarList[MAX_ARG_COUNT];
    char *VarDataList[MAX_ARG_COUNT];
    char *SavedVarsFlash;
    if((p = checkstring(cmdline, "CLEAR"))) {
        checkend(p);
        ClearSavedVars(1);
        return;
    }
    if((p = checkstring(cmdline, "RESTORE"))) {
        char b[MAXVARLEN + 3];
        checkend(p);
        SavedVarsFlash = (char*)FLASH_SAVED_VAR_ADDR;      // point to where the variables were saved
        if(*SavedVarsFlash == 0xFF) return;                          // zero in this location means that nothing has ever been saved
        SaveDefaultType = DefaultType;                              // save the default type
        bufp = SavedVarsFlash;   // point to where the variables were saved
        while(*bufp != 0xff) {                                      // 0xff is the end of the variable list
            type = *bufp++;                                         // get the variable type
            array = type & 0x80;  type &= 0x6f;                     // set array to true if it is an array
            if(!(type==T_INT || type==T_STR || type==T_NBR)){
            	ClearSavedVars(1);
            	error("Saved VARS corrupt - memory cleared %",type);
            }
            DefaultType = TypeMask(type);                           // and set the default type to this
            if(array) {
                strcpy(b, bufp);
                strcat(b, "()");
                vdata = findvar(b, type | V_EMPTY_OK | V_NOFIND_ERR);     // find an array
            } else
                vdata = findvar(bufp, type | V_FIND);               // find or create a non arrayed variable
            if(TypeMask(vartbl[VarIndex].type) != TypeMask(type)) error("$ type conflict", bufp);
            if(vartbl[VarIndex].type & T_CONST) error("$ is a constant", bufp);
            bufp += strlen((char *)bufp) + 1;                       // step over the name and the terminating zero byte
            if(array) {                                             // an array has the data size in the next two bytes
                nbr = *bufp++;
                nbr |= (*bufp++) << 8;
                nbr |= (*bufp++) << 16;
                nbr |= (*bufp++) << 24;
                nbr2 = 1;
                for(j = 0; vartbl[VarIndex].dims[j] != 0 && j < MAXDIM; j++)
                    nbr2 *= (vartbl[VarIndex].dims[j] + 1 - OptionBase);
                if(type & T_STR) nbr2 *= vartbl[VarIndex].size +1;
                if(type & T_NBR) nbr2 *= sizeof(MMFLOAT);
                if(type & T_INT) nbr2 *= sizeof(long long int);
                if(nbr2!=nbr)error("Array size");
            } else {
               if(type & T_STR) nbr = *bufp + 1;
               if(type & T_NBR) nbr = sizeof(MMFLOAT);
               if(type & T_INT) nbr = sizeof(long long int);
            }
            while(nbr--) *vdata++ = *bufp++;                        // copy the data
        }
        DefaultType = SaveDefaultType;
        return;
    }

     if((p = checkstring(cmdline, "SAVE"))) {
        getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");                  // getargs macro must be the first executable stmt in a block
        if(argc && (argc & 0x01) == 0) error("Invalid syntax");

        // befor we start, run through the arguments checking for errors
        // before we start, run through the arguments checking for errors
        for(i = 0; i < argc; i += 2) {
            checkend(skipvar(argv[i], false));
            VarDataList[i/2] = findvar(argv[i], V_NOFIND_ERR | V_EMPTY_OK);
            VarList[i/2] = VarIndex;
            if((vartbl[VarIndex].type & (T_CONST | T_PTR)) || vartbl[VarIndex].level != 0) error("Invalid variable");
            p = &argv[i][strlen(argv[i]) - 1];                      // pointer to the last char
            if(*p == ')') {                                         // strip off any empty brackets which indicate an array
                p--;
                if(*p == ' ') p--;
                if(*p == '(')
                    *p = 0;
                else
                    error("Invalid variable");
            }
        }
        // load the current variable save table into RAM
        // while doing this skip any variables that are in the argument list for this save
        bufp = buf = GetTempMemory(SAVEDVARS_FLASH_SIZE);           // build the saved variable table in RAM
        SavedVarsFlash = (char*)FLASH_SAVED_VAR_ADDR;      // point to where the variables were saved
        varp = SavedVarsFlash;   // point to where the variables were saved
        while(*varp != 0 && *varp != 0xff) {                        // 0 or 0xff is the end of the variable list
            type = *varp++;                                         // get the variable type
            array = type & 0x80;  type &= 0x6f;                     // set array to true if it is an array
            if(!(type==T_INT || type==T_STR || type==T_NBR)){
            	ClearSavedVars(1);
            	error("Saved VARS corrupt - memory cleared");
            }
            vdata = varp;                                           // save a pointer to the name
            while(*varp) varp++;                                    // skip the name
            varp++;                                                 // and the terminating zero byte
            if(array) {                                             // an array has the data size in the next two bytes
                 nbr = (varp[0] | (varp[1] << 8) | (varp[2] << 16) | (varp[3] << 24)) + 4;
            } else {
                if(type & T_STR) nbr = *varp + 1;
                if(type & T_NBR) nbr = sizeof(MMFLOAT);
                if(type & T_INT) nbr = sizeof(long long int);
            }
            for(i = 0; i < argc; i += 2) {                          // scan the argument list
                p = &argv[i][strlen(argv[i]) - 1];                  // pointer to the last char
                lastc = *p;                                         // get the last char
                if(lastc <= '%') *p = 0;                            // remove the type suffix for the compare
                if(strncasecmp(vdata, argv[i], MAXVARLEN) == 0) {   // does the entry have the same name?
                    while(nbr--) varp++;                            // found matching variable, skip over the entry in flash (ie, do not copy to RAM)
                    i = 9999;                                       // force the termination of the for loop
                }
                *p = lastc;                                         // restore the type suffix
            }
            // finished scanning the argument list, did we find a matching variable?
            // if not, copy this entry to RAM
            if(i < 9999) {
                *bufp++ = type | array;
                while(*vdata) *bufp++ = *vdata++;                   // copy the name
                *bufp++ = *vdata++;                                 // and the terminating zero byte
                while(nbr--) *bufp++ = *varp++;                     // copy the data
            }
        }


        // initialise for writing to the flash
        //FlashWriteInit(SAVED_VARS_FLASH,1);
        i=3;
        while(FlashWriteInit(SAVED_VARS_FLASH) && i)i--;                     // erase flash
        if(i==0)error("Failed to erase flash memory");

        // now write the variables in RAM recovered from the var save list
        while(buf < bufp){
        	FlashWriteByte(*buf++);
        }

        // now save the variables listed in this invocation of VAR SAVE
        for(i = 0; i < argc; i += 2) {
            VarIndex = VarList[i/2];                                // previously saved index to the variable
            vdata = VarDataList[i/2];                               // pointer to the variable's data
            type = TypeMask(vartbl[VarIndex].type);                 // get the variable's type
            type |= (vartbl[VarIndex].type & T_IMPLIED);            // set the implied flag
            array = (vartbl[VarIndex].dims[0] != 0);

            nbr = 1;                                                // number of elements to save
            if(array) {                                             // if this is an array calculate the number of elements
                for(j = 0; vartbl[VarIndex].dims[j] != 0 && j < MAXDIM; j++)
                    nbr *= (vartbl[VarIndex].dims[j] + 1 - OptionBase);
                type |= 0x80;                                       // an array has the top bit set
            }

            if(type & T_STR) {
                if(array)
                    nbr *= (vartbl[VarIndex].size + 1);
                else
                    nbr = *vdata + 1;                               // for a simple string variable just save the string
            }
            if(type & T_NBR) nbr *= sizeof(MMFLOAT);
            if(type & T_INT) nbr *= sizeof(long long int);
            if((uint32_t)realflashpointer - (uint32_t)SavedVarsFlash + 36 + nbr > SAVEDVARS_FLASH_SIZE) {
                FlashWriteClose();
                error("Not enough memory");
            }
            FlashWriteByte(type);                              // save its type
            for(j = 0, p = vartbl[VarIndex].name; *p && j < MAXVARLEN; p++, j++)
                FlashWriteByte(*p);                            // save the name
            FlashWriteByte(0);                                 // terminate the name
            if(array) {                                             // if it is an array save the number of data bytes
               FlashWriteByte(nbr); FlashWriteByte(nbr >> 8); FlashWriteByte(nbr >>16); FlashWriteByte(nbr >>24);
            }
            while(nbr--) FlashWriteByte(*vdata++);             // write the data
        }
        FlashWriteClose();
        return;
     }
    error("Unknown command");
}




/**********************************************************************************************
   These routines are used to load or save the global Option structure from/to flash.
   These options are stored in the beginning of the flash used to save stored variables.
***********************************************************************************************/


int xxxSaveOptions(int errabort) {
	int i;
    char *p, *SavedOptionsFlash;
    SavedOptionsFlash=(char*)FLASH_SAVED_OPTION_ADDR;
    p = (char *)&Option;
    for(i = 0; i < sizeof(struct option_s); i++) {
    	if(SavedOptionsFlash[i] != *p) goto skipreturn;
    	p++;
    }
    return 0;                                                         // exit if the option has already been set (ie, nothing to do)
    skipreturn:

    while(!(ConsoleTxBufHead == ConsoleTxBufTail));                    // wait for the console UART to send whatever is in its buffer
    p = (char *)&Option;
    //FlashWriteInit(SAVED_OPTIONS_FLASH,errabort);                // erase the page
    i=3;
    while(FlashWriteInit(SAVED_OPTIONS_FLASH) && i)i--;                     // erase flash
    if(i==0)error("Failed to erase flash memory");

    for(i = 0; i < sizeof(struct option_s); i++){
    	FlashWriteByte(*p);    // write the changed page back to flash
    	p++;
    }
    FlashWriteClose();
    return 1;
}

void SaveOptions(int errabort) {
	int i;
    char *p, *SavedOptionsFlash;
    SavedOptionsFlash=(char*)FLASH_SAVED_OPTION_ADDR;
    p = (char *)&Option;
    for(i = 0; i < sizeof(struct option_s); i++) {
    	if(SavedOptionsFlash[i] != *p) goto skipreturn;
    	p++;
    }
    return;                                                         // exit if the option has already been set (ie, nothing to do)
    skipreturn:

    while(!(ConsoleTxBufHead == ConsoleTxBufTail));                    // wait for the console UART to send whatever is in its buffer
    p = (char *)&Option;
    //FlashWriteInit(SAVED_OPTIONS_FLASH,errabort);                // erase the page
    i=3;
    while(FlashWriteInit(SAVED_OPTIONS_FLASH) && i)i--;                     // erase flash
    if(i==0)error("Failed to erase flash memory");
    for(i = 0; i < sizeof(struct option_s); i++){
    	FlashWriteByte(*p);    // write the changed page back to flash
    	p++;
    }
    FlashWriteClose();
}



void LoadOptions(void) {
    memcpy((struct option_s *)&Option, (struct option_s *)FLASH_SAVED_OPTION_ADDR, sizeof(struct option_s));
}



// erase all flash memory and reset the options to their defaults
// used on initial firmware run
void ResetAllOptions(void) {
	Option.magic = 0x15642903;
    Option.Height = SCREENHEIGHT;                                   // reset the options to their defaults
    Option.Width = SCREENWIDTH;
    Option.PIN = 0;
    Option.Baudrate = CONSOLE_BAUDRATE;
    Option.Autorun = false;
    Option.Listcase = CONFIG_TITLE;
    Option.Tab = 2;
    Option.Invert = false;
    Option.ColourCode = false;
    Option.DISPLAY_TYPE = 0;
    Option.DISPLAY_ORIENTATION = 0;
    Option.TOUCH_XSCALE = 0;
    Option.TOUCH_CS = 0;
    Option.TOUCH_IRQ = 0;
   	Option.LCD_CD = 0;
   	Option.LCD_Reset = 0;
   	Option.LCD_CS = 0;
    Option.DISPLAY_CONSOLE = 0;
    Option.DefaultFont = 0x01;
    Option.DefaultFC = WHITE;
    Option.DefaultBC = BLACK;
    Option.RTC_Calibrate = 0;
	Option.USBPolling = 3;
    Option.SerialConDisabled = 0;
    Option.KeyboardConfig = 0;
    Option.SDCARD_CS = 0;
    Option.SD_CD = 0;
    Option.SD_WP = 0;
    Option.SSDspeed = 0;
    Option.TOUCH_Click = 0;
    Option.TOUCH_XZERO = -999999;
    Option.DefaultBrightness = 50;
    Option.SerialPullup = 1;
    Option.MaxCtrls=201;
    Option.USBKeyboard = NO_KEYBOARD;
    Option.USBpower = 0;
	Option.RepeatStart=600;
	Option.RepeatRate=150;
    Option.Refresh = 0;
	Option.FlashPages = 1;
	Option.DISPLAY_WIDTH = 0;
	Option.DISPLAY_HEIGHT = 0;
	Option.CPUspeed = (HAL_GetREVID()==0x1003 ? 400 : 480);
	Option.noLED=0;
	Option.fulltime=0;
	mymemset((char *)Option.F1key,0,sizeof(Option.F1key));
	mymemset((char *)Option.F5key,0,sizeof(Option.F5key));
	mymemset((char *)Option.F6key,0,sizeof(Option.F6key));
	mymemset((char *)Option.F7key,0,sizeof(Option.F7key));
	mymemset((char *)Option.F8key,0,sizeof(Option.F8key));
	mymemset((char *)Option.F9key,0,sizeof(Option.F9key));
}
void ClearSavedVars(int errabort) {
    //FlashWriteInit(SAVED_VARS_FLASH, errabort);                              // initialise for writing to the flash
    int i=3;
    while(FlashWriteInit(SAVED_VARS_FLASH) && i)i--;                     // erase flash
    if(i==0)error("Failed to erase flash memory");
}





/*******************************************************************************************************************
 The LIBRARY commands
 The library uses top of page (4) of the 4 available program flash pages (1-4)
 It holds code saved with LIBRARY SAVE.  It includes BASIC
 code and CFunctions extracted from the library BASIC code.
 When library is in use the Program is limited to pages(1-3)

 The program flash layout is:
 ===================   << PROG_FLASH_SIZEMAX    (the total size of the program flash area in bytes)
 |                 |
 |    library      |
 |   CFunctions    |
 ===================   << CFunctionLibrary   (starting address of the library CFunctions)
 |    library      |
 |     BASIC       |   Page 4
 |     code        |
 ===================   << Option.ProgFlashSize   ( Indicates the library start)
 |                 |
 |     free        |
 |   program       | page 3
 |    flash        |
 |                 |
 |                 |
 |                 |
 |                 |
 ~~~~~~~~~~~~~~~~~~~ Page 2
 |     main        |
 |    program      |       this contains the CFunctions extracted from the main BASIC program
 |   CFunctions    |
 ===================   << CFunctionFlash   (starting address of the main program CFunctions)
 |      main       |
 |     BASIC       |
 |    program      | Page 1
 ===================   << ProgMemory   (starting address of the program flash area)

********************************************************************************************************************/




void MIPS16 cmd_library(void) {
    char *p,  *pp , *m, *MemBuff, *TempPtr, rem;
    int i, j, k, InCFun, InQuote, CmdExpected;
    unsigned int CFunDefAddr[100], *CFunHexAddr[100] , *CFunHexLibAddr[100] ;
    /********************************************************************************************************************
     ******* LIBRARY SAVE **********************************************************************************************/
    if((p = checkstring(cmdline, "SAVE"))) {
    	if(Option.FlashPages==4) error("Library Flash in used FlashPages=4");
        if(CurrentLinePtr) error("Invalid in a program");
        if(*ProgMemory != 0x01) return;
        checkend(p);
        ClearRuntime();
        TempPtr = m = MemBuff = GetTempMemory(EDIT_BUFFER_SIZE);

        rem = GetCommandValue("Rem");
        InQuote = InCFun = j = 0;
        CmdExpected = true;

        // first copy the current program code residing in the Library area to RAM
        if(Option.ProgFlashSize != PROG_FLASH_SIZEMAX) {
        //if(Option.FlashPages < 4 && Option.ProgFlashSize==FLASH_LIBRARY_ADDR){
            p = ProgMemory + Option.ProgFlashSize;
            //p = (char*)FLASH_LIBRARY_ADDR;
            while(!(p[0] == 0 && p[1] == 0)) *m++ = *p++;
              *m++ = 0;                                               // terminate the last line
        }
     //dump(m, 256);
       Option.ProgFlashSize=LIBRARY_FLASH_OFFSET;
       // SaveOptions(1);

        // then copy the current contents of the program memory to RAM
        p = ProgMemory;
        while(*p != 0xff) {
            if(p[0] == 0 && p[1] == 0) break;                       // end of the program

            if(*p == T_NEWLINE) {
                TempPtr = m;
                CurrentLinePtr = p;
                *m++ = *p++;
                CmdExpected = true;                                 // if true we can expect a command next (possibly a CFunction, etc)
                if(*p == 0) {                                       // if this is an empty line we can skip it
                    p++;
                    if(*p == 0) break;                              // end of the program or module
                    m--;
                    continue;
                }
            }

            if(*p == T_LINENBR) {
//                TempPtr = m;
                *m++ = *p++; *m++ = *p++; *m++ = *p++;              // copy the line number
                skipspace(p);
            }

            if(*p == T_LABEL) {
                for(i = p[1] + 2; i > 0; i--) *m++ = *p++;          // copy the label
//                TempPtr = m;
                skipspace(p);
            }

            if(CmdExpected && ( *p == GetCommandValue("End CSub") || *p == GetCommandValue("End DefineFont"))) {
                InCFun = false;                                     // found an  END CSUB or END DEFINEFONT token
            }

            if(InCFun) {
                skipline(p);                                        // skip the body of a CFunction
                m = TempPtr;                                        // don't copy the new line
                continue;
            }

            //if(CmdExpected && ( *p == cmdCSUB || *p == GetCommandValue("DefineFont"))) {    // found a  CSUB or DEFINEFONT token
           	if(CmdExpected && ( *p == cmdCSUB || *p == cmdCFUN || *p == GetCommandValue("DefineFont"))) {    // found a  CSUB or DEFINEFONT token
                CFunDefAddr[++j] = (unsigned int)m;                 // save its address so that the binary copy in the library can point to it
                while(*p) *m++ = *p++;                              // copy the declaration
                InCFun = true;
            }

            if(CmdExpected && *p == rem) {                          // found a REM token
                skipline(p);
                m = TempPtr;                                        // don't copy the new line tokens
                continue;
            }

            if(*p >= C_BASETOKEN || IsNamestart(*p))
                CmdExpected = false;                                // stop looking for a CFunction, etc on this line

            if(*p == '"') InQuote = !InQuote;                       // found the start/end of a string

            if(*p == '\'' && !InQuote) {                            // found a modern remark char
                skipline(p);
                //PIntHC(*(m-3)); PIntHC(*(m-2)); PIntHC(*(m-1)); PIntHC(*(m));
                //MMPrintString("\r\n");
                //if(*(m-3) == 0x01) {        //Original condition from Micromites
                /* Check to see if comment is the first thing on the line or its only preceded by spaces.
                Spaces have been reduced to a single space so we treat a comment with 1 space before it
                as a comment line to be omitted.
                */
                if((*(m-1) == 0x01) ||  ((*(m-2) == 0x01) && (*(m-1) == 0x20))){
                    m = TempPtr;                                    // if the comment was the only thing on the line don't copy the line at all
                    continue;
                } else
                    p--;
            }

            if(*p == ' ' && !InQuote) {                             // found a space
                if(*(m-1) == ' ') m--;                              // don't copy the space if the preceeding char was a space
            }

            if(p[0] == 0 && p[1] == 0) break;                       // end of the program
            *m++ = *p++;
        }
        while(*p == 0) *m++ = *p++;                                 // the end of the program can have multiple zeros
        *m++ = *p++;;                                               // copy the terminating 0xff
       // dump(MemBuff, 256,101);
        while((unsigned int)p & 0b11) p++;
        while((unsigned int)m & 0b11) *m++ = 0xff;                  // step to the next word boundary


        // now copy the CFunction/CSub/Font data
        // =====================================
        // the format of a CFunction in flash is:
        //   Unsigned Int - Address of the CFunction/CSub/Font in program memory (points to the token).  For a font it is zero.
        //   Unsigned Int - The length of the CFunction/CSub/Font in bytes including the Offset (see below)
        //   Unsigned Int - The Offset (in words) to the main() function (ie, the entry point to the CFunction/CSub).  The specs for the font if it is a font.
        //   word1..wordN - The CFunction code
        // The next CFunction starts immediately following the last word of the previous CFunction

        // first, copy any CFunctions residing in the library area to RAM
        k = 0;                                                      // this is used to index CFunHexLibAddr[] for later adjustment of a CFun's address
        if(CFunctionLibrary != NULL) {
            pp = (char *)CFunctionLibrary;
            while(*(unsigned int *)pp != 0xffffffff) {
                CFunHexLibAddr[++k] = (unsigned int *)m;            // save the address for later adjustment
                j = (*(unsigned int *)(pp + 4)) + 8;                // calculate the total size of the CFunction
                while(j--) *m++ = *pp++;                            // copy it into RAM
            }
        }

        // then, copy any CFunctions in program memory to RAM
        i = 0;                                                      // this is used to index CFunHexAddr[] for later adjustment of a CFun's address
        while(*(unsigned int *)p != 0xffffffff) {
            CFunHexAddr[++i] = (unsigned int *)m;                   // save the address for later adjustment
            j = (*(unsigned int *)(p + 4)) + 8;                     // calculate the total size of the CFunction
            while(j--) *m++ = *p++;                                 // copy it into RAM
        }

        // we have now copied all the CFunctions into RAM



        // calculate the starting point of the library code (located in the top of the program space in flash)
       //TempPtr = (ProgMemory + PROG_FLASH_SIZE) - (((m - MemBuff) + (FLASH_PAGE_SIZE - 1)) & (~(FLASH_PAGE_SIZE - 1)));

        // calculate the size of the library code  to  end on a word boundary
        j=(((m - MemBuff) + (0x4 - 1)) & (~(0x4 - 1)));
        //We only have reserved 128K of flash for library code .
        //Error if we try to use too much
        if (j > 128*1024) error("Library too big, > 128K");

        // calculate the starting point of the library code (located in the top of the program space in flash)
        //TempPtr = (ProgMemory + PROG_FLASH_SIZE) - j;
        //Set the library starting point
        TempPtr=(char*)FLASH_LIBRARY_ADDR;
        // TempPtr = (ProgMemory + PROG_FLASH_SIZE) - (((m - MemBuff) + (0x4 - 1)) & (~(0x4 - 1)));

        // now adjust the addresses of the declaration in each CFunction header
        // do not adjust a font who's "address" is zero

        // first, CFunctions that were already in the library
        for(; k > 0; k--) {
            if(*CFunHexLibAddr[k]  > FONT_TABLE_SIZE) *CFunHexLibAddr[k] -= ((unsigned int)(ProgMemory + Option.ProgFlashSize) - (unsigned int)TempPtr);
        }

        // then, CFunctions that are being added to the library
        for(; i > 0; i--) {
            if(*CFunHexAddr[i]  > FONT_TABLE_SIZE) *CFunHexAddr[i] = (CFunDefAddr[i] - (unsigned int)MemBuff) + (unsigned int)TempPtr;
        }

        //******************************************************************************
        //now write the library from ram to the library flash area
        // initialise for writing to the flash
        //FlashWriteInit(LIBRARY_FLASH,1);
        i=3;
        while(FlashWriteInit(LIBRARY_FLASH) && i)i--;                     // erase flash
        if(i==0)error("Failed to erase flash memory");
        i=0;
        for(k = 0; k < m - MemBuff; k++){        // write to the flash byte by byte
           FlashWriteByte(MemBuff[k]);
        }
        FlashWriteClose();


        if(MMCharPos > 1) MMPrintString("\r\n");                    // message should be on a new line
        MMPrintString("Library Saved ");
        //IntToStr(tknbuf, PROG_FLASH_SIZE-Option.ProgFlashSize, 10);
        IntToStr(tknbuf, k, 10);
        MMPrintString(tknbuf);
        MMPrintString(" bytes\r\n");
        fflush(stdout);
        uSec(2000);
        //Now call the new command that will clear the current program memory
        //and return to the command prompt.
        cmdline = ""; CurrentLinePtr = NULL;    // keep the NEW command happy
        cmd_new();                              //  delete the program,add the library code and return to the command prompt

    }


    /********************************************************************************************************************
    ******* LIBRARY DELETE **********************************************************************************************/

     if(checkstring(cmdline, "DELETE")) {
       int i;
       if(Option.ProgFlashSize == PROG_FLASH_SIZEMAX) return;
       // if(Option.FlashPages < 4 && Option.ProgFlashSize==FLASH_LIBRARY_ADDR){

           Option.ProgFlashSize = PROG_FLASH_SIZEMAX;
           SaveOptions(1);
          // FlashWriteInit(LIBRARY_FLASH,1);
           i=3;
           while(FlashWriteInit(LIBRARY_FLASH) && i)i--;                     // erase flash
           if(i==0)error("Failed to erase flash memory");
           FlashWriteClose();
           return;
           // DONT CLEAR PROGRAM MEMORY - Micromite and Picomite DONT.
           // Clear Program Memory and also the Library at the end.
           //cmdline = ""; CurrentLinePtr = NULL;    // keep the NEW command happy
           //cmd_new();                              //  delete any program,and the library code and return to the command prompt
      //  }

     }

     /********************************************************************************************************************
     ******* LIBRARY LIST **********************************************************************************************/
     if(checkstring(cmdline, "LIST ALL")) {
        if(Option.ProgFlashSize == PROG_FLASH_SIZEMAX) return;
        ListProgram(ProgMemory + Option.ProgFlashSize, true);
        return;
     }

     if(checkstring(cmdline, "LIST")) {
        if(Option.ProgFlashSize == PROG_FLASH_SIZEMAX) return;
        ListProgram(ProgMemory + Option.ProgFlashSize, false);  //TempPtr=(char*)FLASH_LIBRARY_ADDR;
        //ListProgram((char*)FLASH_LIBRARY_ADDR, false);  //TempPtr=(char*)FLASH_LIBRARY_ADDR;
        return;
     }


     //****************** START of diagnostic commands ******************************************************************
     /******* LIBRARY SIZE **********************************************************************************************/
           if((p = checkstring(cmdline, "SIZE"))) {
           if(Option.FlashPages==4) error("Library Flash in used FlashPages=4");
           //    if(CurrentLinePtr) error("Invalid in a program");
           //    if(*ProgMemory != 0x01) return;
           //    checkend(p);
           //    ClearRuntime();
           // TempPtr = m = MemBuff = GetTempMemory(EDIT_BUFFER_SIZE);
            k = 0;
            // first count the normal program code residing in the Library
            if(Option.ProgFlashSize != PROG_FLASH_SIZEMAX) {
                p = ProgMemory + Option.ProgFlashSize;
                while(!(p[0] == 0 && p[1] == 0)) {
                	p++; k++;
                }
                while(*p == 0){ // the end of the program can have multiple zeros -count them
                    p++;k++;
                }
                p++; k++;    //get 0xFF that ends the program and count it
                while((unsigned int)p & 0b11) { //count to the next word boundary
                   	p++;k++;
                }
            }
               // now count the CFunction/CSub/Font data
               // =====================================
               // the format of a CFunction in flash is:
               //   Unsigned Int - Address of the CFunction/CSub/Font in program memory (points to the token).  For a font it is zero.
               //   Unsigned Int - The length of the CFunction/CSub/Font in bytes including the Offset (see below)
               //   Unsigned Int - The Offset (in words) to the main() function (ie, the entry point to the CFunction/CSub).  The specs for the font if it is a font.
               //   word1..wordN - The CFunction code
               // The next CFunction starts immediately following the last word of the previous CFunction

               if(CFunctionLibrary != NULL) {
                   pp = (char *)CFunctionLibrary;
                   while(*(unsigned int *)pp != 0xffffffff) {
                       j = (*(unsigned int *)(pp + 4)) + 8;  // calculate the total size of the CFunction
                       while(j--){
                    	  pp++;k++;
                       }
                   }
               }

               if(MMCharPos > 1) MMPrintString("\r\n");                    // message should be on a new line
               MMPrintString("Library Size ");
               IntToStr(tknbuf, k, 10);
               MMPrintString(tknbuf);
               MMPrintString(" bytes\r\n");
              // m = ProgMemory + Option.ProgFlashSize;
              //  dump(m, 256);
               return;

           }

     if(checkstring(cmdline, "PPROG")) {
              if(CurrentLinePtr) error("Invalid in a program");
                 char *p;
             //  if(Option.PROG_FLASH_SIZE== MAX_PROG_SIZE) return;
               p = ProgMemory ;
               dump(p,256,2000);
               dump(p+256,256,2001);
               dump(p+512,256,2002);
               dump(p+768,256,2003);
               dump(p+1024,256,2004);
               //ListProgram(ProgMemory + Option.PROG_FLASH_SIZE, false);
               return;
      }
     if(checkstring(cmdline, "PLIB")) {
        if(CurrentLinePtr) error("Invalid in a program");
         char *p;
         if(Option.ProgFlashSize == PROG_FLASH_SIZEMAX) return;
         p = ProgMemory + Option.ProgFlashSize;
         //dump(p-256,256,-1);
         dump(p,256,5000);
         dump(p+256,256,5001);
         dump(p+512,256,5002);
         dump(p+768,256,5003);
         dump(p+1024,256,5004);
        //ListProgram(ProgMemory + Option.PROG_FLASH_SIZE, false);
         return;
     }
     //****************** End of diagnostic commands ******************************
     error("Unknown command");
}




// erase all flash memory and reset the options to their defaultsholds K1 pins 9 an 10 together on startup
void ResetAllFlash(int errabort) {
	int i;
	ResetAllOptions();
	PROG_FLASH_SIZE = Option.FlashPages * 0x20000;
    //Option.ProgFlashSize = PROG_FLASH_SIZE;
    Option.ProgFlashSize = PROG_FLASH_SIZEMAX;
	SaveOptions(errabort);                                     //  and write them to flash
	ClearSavedVars(errabort);

	// FlashWriteInit(SAVED_VARS_FLASH, errabort);           	   // erase saved vars
    i=3;
    while(FlashWriteInit(SAVED_VARS_FLASH) && i)i--;                     // erase flash
    if(i==0)error("Failed to erase flash memory");

    //FlashWriteInit(PROGRAM_FLASH, errabort);                     // erase program memory
    i=3;
    while(FlashWriteInit(PROGRAM_FLASH) && i)i--;                     // erase flash
    if(i==0)error("Failed to erase flash memory");

    FlashWriteByte(0); FlashWriteByte(0);              // terminate the program in flash
    FlashWriteClose();


}

/**
  * @brief  Gets the sector of a given address
  * @param  Address Address of the FLASH Memory
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if(((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1)) || \
     ((Address < ADDR_FLASH_SECTOR_1_BANK2) && (Address >= ADDR_FLASH_SECTOR_0_BANK2)))
  {
    sector = FLASH_SECTOR_0;
  }
  else if(((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_2_BANK2) && (Address >= ADDR_FLASH_SECTOR_1_BANK2)))
  {
    sector = FLASH_SECTOR_1;
  }
  else if(((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_3_BANK2) && (Address >= ADDR_FLASH_SECTOR_2_BANK2)))
  {
    sector = FLASH_SECTOR_2;
  }
  else if(((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_4_BANK2) && (Address >= ADDR_FLASH_SECTOR_3_BANK2)))
  {
    sector = FLASH_SECTOR_3;
  }
  else if(((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_5_BANK2) && (Address >= ADDR_FLASH_SECTOR_4_BANK2)))
  {
    sector = FLASH_SECTOR_4;
  }
  else if(((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_6_BANK2) && (Address >= ADDR_FLASH_SECTOR_5_BANK2)))
  {
    sector = FLASH_SECTOR_5;
  }
  else if(((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1)) || \
          ((Address < ADDR_FLASH_SECTOR_7_BANK2) && (Address >= ADDR_FLASH_SECTOR_6_BANK2)))
  {
    sector = FLASH_SECTOR_6;
  }
  else if(((Address < ADDR_FLASH_SECTOR_0_BANK2) && (Address >= ADDR_FLASH_SECTOR_7_BANK1)) || \
          ((Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7_BANK2)))
  {
     sector = FLASH_SECTOR_7;
  }
  else
  {
    sector = FLASH_SECTOR_7;
  }

  return sector;
}
