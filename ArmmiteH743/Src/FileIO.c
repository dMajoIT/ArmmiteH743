/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

FileIO.c

Does all the SD Card related file I/O in MMBasic.

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

//** SD CARD INCLUDES ***********************************************************
#include "ff.h"
#include "diskio.h"
#include "sys/stat.h"
#include "picojpeg.h"

int OptionFileErrorAbort = true;
extern char SDPath[4]; /* SD card logical drive path */
extern void Display_Refresh(void);
extern BYTE MDD_SDSPI_CardDetectState(void);
// FatFs definitions
FATFS FatFs;
DIR dj;
FILINFO fno;

char FileGetChar(int fnbr);
char FilePutChar(char c, int fnbr);
int FileEOF(int fnbr);
char *GetCWD(void);
void File_CloseAll(void);
int InitSDCard(void);
char *ChangeToDir(char *p);
void LoadImage(char *p);
void LoadFont(char *p);
int dirflags;
FRESULT FSerror;
volatile BYTE SDCardStat = STA_NOINIT | STA_NODISK;
volatile int diskcheckrate = 0;
extern 	volatile unsigned int checkSD;

extern unsigned int __attribute__((section(".my_section"))) _excep_cause;
extern int as_strcmpi (const char *s1, const char *s2);
#define SDbufferSize 512
static char *SDbuffer[MAXOPENFILES+1]={NULL};
int buffpointer[MAXOPENFILES+1]={0};
static uint32_t lastfptr[MAXOPENFILES+1]={[0 ... MAXOPENFILES ] = -1};
uint32_t fmode[MAXOPENFILES+1]={0};
static unsigned int bw[MAXOPENFILES+1]={[0 ... MAXOPENFILES ] = -1};
extern char ConsoleRxBuf[CONSOLE_RX_BUF_SIZE];
extern volatile int ConsoleRxBufHead;
extern volatile int ConsoleRxBufTail;
unsigned char pjpeg_need_bytes_callback(unsigned  char* pBuf, unsigned char buf_size,unsigned char *pBytes_actually_read, void *pCallback_data);
void LoadJPGImage(char *p);

// 8*8*4 bytes * 3 = 768
int16_t *gCoeffBuf;

// 8*8*4 bytes * 3 = 768
uint8_t *gMCUBufR;
uint8_t *gMCUBufG;
uint8_t *gMCUBufB;

// 256 bytes
int16_t *gQuant0;
int16_t *gQuant1;
uint8_t *gHuffVal2;
uint8_t *gHuffVal3;
uint8_t *gInBuf;

extern RTC_HandleTypeDef hrtc;

/*****************************************************************************************
Mapping of errors reported by the file system to MMBasic file errors
*****************************************************************************************/
const int ErrorMap[] = {        0,                                  // 0
                                1,                                  // Assertion failed
                                2,                                  // Low level I/O error
                                3,                                  // No response from SDcard
                                4,                                  // Could not find the file
                                5,                                  // Could not find the path
                                6,                                  // The path name format is invalid
                                7,                                  // Prohibited access or directory full
                                8,                                  // Directory exists or path to it cannot be found
                                9,                                  // The file/directory object is invalid
                               10,                                  // SD card is write protected
                               11,                                  // The logical drive number is invalid
                               12,                                  // The volume has no work area
                               13,                                  // Not a FAT volume
                               14,                                  // Format aborted
                               15,                                  // Could not access volume
                               16,                                  // File sharing policy
                               17,                                  // Buffer could not be allocated
                               18,                                  // Too many open files
                               19,                                  // Parameter is invalid
							   20									// Not present
                            };

/******************************************************************************************
Text for the file related error messages reported by MMBasic
******************************************************************************************/
const char *FErrorMsg[] = {	"Succeeded ",
		"A hard error occurred in the low level disk I/O layer",
		"Assertion failed",
		"SD Card not found",
		"Could not find the file",
		"Could not find the path",
		"The path name format is invalid",
		"FAccess denied due to prohibited access or directory full",
		"Access denied due to prohibited access",
		"The file/directory object is invalid",
		"The physical drive is write protected",
		"The logical drive number is invalid",
		"The volume has no work area",
		"There is no valid FAT volume",
		"The f_mkfs() aborted due to any problem",
		"Could not get a grant to access the volume within defined period",
		"The operation is rejected according to the file sharing policy",
		"LFN working buffer could not be allocated",
		"Number of open files > FF_FS_LOCK",
		"Given parameter is invalid",
		"SD card not present"
};




//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// MMBASIC COMMANDS & FUNCTIONS FOR THE SDCARD /////////////////////////////

void ConfigSDCard(char *p) {
    getargs(&p, 5, ",");
    if((argc <1) || argc>5) error("Argument count");

    Option.SDCARD_CS = Option.SD_CD = Option.SD_WP = 0;
    CheckPin(getinteger(argv[0]), CP_IGNORE_INUSE); // | CP_IGNORE_BOOTRES | CP_IGNORE_RESERVED);
    Option.SDCARD_CS = getinteger(argv[0]);
    if(argc<3)return;
    if(*argv[2]) {
        CheckPin(abs(getinteger(argv[2])), CP_IGNORE_INUSE); // | CP_IGNORE_BOOTRES | CP_IGNORE_RESERVED);
        Option.SD_CD = getinteger(argv[2]);
    }
    if(argc<5)return;
    if(*argv[4]) {
        CheckPin(abs(getinteger(argv[4])), CP_IGNORE_INUSE); // | CP_IGNORE_BOOTRES | CP_IGNORE_RESERVED);
        Option.SD_WP = getinteger(argv[4]);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup the SD Card based on the settings saved in flash
void InitFileIO(void) {
    if(!Option.SDCARD_CS) return;
//    FIL_Fil=(FIL *)&_splim;
    SetAndReserve(Option.SDCARD_CS, P_OUTPUT, 1, EXT_BOOT_RESERVED);           // config CS as an output
    if(Option.SD_CD) {
        SetAndReserve(abs(Option.SD_CD), P_INPUT, 0, EXT_BOOT_RESERVED);       // config card detect as an input
        PinSetBit(abs(Option.SD_CD), CNPUSET);                                 // set a pullup on it
    }
    if(Option.SD_WP) {
        SetAndReserve(abs(Option.SD_WP), P_INPUT, 0, EXT_BOOT_RESERVED);       // config write protect as an input
        PinSetBit(abs(Option.SD_WP), CNPUSET);                                 // set a pullup on it
    }
    if(Option.SDCARD_CS==79){ //Onboard SDCARD for WEAct and DevEBox 100 Pin boards Bitbanged SPI
    	     //Bitbang MISO
    		  GPIO_InitTypeDef GPIO_InitStruct = {0};
    		  GPIO_InitStruct.Pin = SD_MISO_Pin; //SD card MISO
    		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    		  GPIO_InitStruct.Pull = GPIO_PULLUP;
    		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    		  HAL_GPIO_Init(SD_MISO_GPIO_Port, &GPIO_InitStruct);

    		  //Bitbang CLK PC12
    		  SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;
    		  GPIO_InitStruct.Pin = SD_CLK_Pin; //SD card CLK
    		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    		  GPIO_InitStruct.Pull = GPIO_NOPULL;
    		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    		  HAL_GPIO_Init(SD_CLK_GPIO_Port, &GPIO_InitStruct);
    		  SD_CLK_GPIO_Port->BSRR = SD_CLK_Pin<<16;

    		  //Bitbang MOSI
    		  SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;
    		  GPIO_InitStruct.Pin = SD_MOSI_Pin; //SD card MOSI
    		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    		  GPIO_InitStruct.Pull = GPIO_NOPULL;
    		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    		  HAL_GPIO_Init(SD_MOSI_GPIO_Port, &GPIO_InitStruct);
    		  SD_MOSI_GPIO_Port->BSRR = SD_MOSI_Pin;

    		  if(ExtCurrentConfig[SD_OUT_PIN] != EXT_BOOT_RESERVED) {
    		         ExtCfg(SD_OUT_PIN, EXT_BOOT_RESERVED, 0);
    		         ExtCfg(SD_INP_PIN, EXT_BOOT_RESERVED, 0);
    		         ExtCfg(SD_CLK_PIN, EXT_BOOT_RESERVED, 0);
    		        // CurrentSPISpeed=NONE_SPI_SPEED;
    		  }

    }else{
            OpenSpiChannel();
   	}
}

void cmd_save(void) {
    int fnbr;
    unsigned int nbr;
    char *p, *pp, *flinebuf;
    int x,y,w,h, filesize;
        unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
        unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
        unsigned char bmppad[3] = {0,0,0};
    int i;
    if(!InitSDCard()) return;
	int maxH=VRes;
    int maxW=HRes;
    fnbr = FindFreeFileNbr();
     if((p = checkstring(cmdline, "IMAGE")) !=NULL){
    	getargs(&p,9,",");
        if(!InitSDCard()) return;
        if((void *)ReadBuffer == (void *)DisplayNotSet) error("SAVE IMAGE not available on this display");
        pp = getFstring(argv[0]);
        if(argc!=1 && argc!=9)error("Syntax");
        if(strchr(pp, '.') == NULL) strcat(pp, ".BMP");
        if(!BasicFileOpen(pp, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
        if(argc==1){
        	x=0; y=0; h=maxH; w=maxW;
        } else {
        	x=getint(argv[2],0,maxW-1);
        	y=getint(argv[4],0,maxH-1);
        	w=getint(argv[6],1,maxW-x);
        	h=getint(argv[8],1,maxH-y);
        }
        filesize=54 + 3*w*h;
        bmpfileheader[ 2] = (unsigned char)(filesize    );
        bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
        bmpfileheader[ 4] = (unsigned char)(filesize>>16);
        bmpfileheader[ 5] = (unsigned char)(filesize>>24);

        bmpinfoheader[ 4] = (unsigned char)(       w    );
        bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
        bmpinfoheader[ 6] = (unsigned char)(       w>>16);
        bmpinfoheader[ 7] = (unsigned char)(       w>>24);
        bmpinfoheader[ 8] = (unsigned char)(       h    );
        bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
        bmpinfoheader[10] = (unsigned char)(       h>>16);
        bmpinfoheader[11] = (unsigned char)(       h>>24);
		f_write(FileTable[fnbr].fptr, bmpfileheader, 14, &nbr);
		f_write(FileTable[fnbr].fptr, bmpinfoheader, 40, &nbr);
        flinebuf = GetTempMemory(maxW * 4);
        for(i = y+h-1; i >= y; i--){
           ReadBuffer(x, i, x+w-1, i, flinebuf);
           f_write(FileTable[fnbr].fptr, flinebuf, w*3, &nbr);
           if((w*3) % 4 !=0) f_write(FileTable[fnbr].fptr, bmppad, 4-((w*3) % 4) , &nbr);
        }
        FileClose(fnbr);
        return;
    } else if((p = checkstring(cmdline, "DATA")) !=NULL){
		getargs(&p,5,",");
        if(!InitSDCard()) return;
		if(argc!=5)error("Syntax");
		pp = getFstring(argv[0]);
		if(strchr(pp, '.') == NULL) strcat(pp, ".DAT");
		uint32_t address=(GetPeekAddr(argv[2]) & 0b11111111111111111111111111111100);
		uint32_t size=getint(argv[4],1,0x7FFFFFFF);
		for(uint32_t i=address;i<address+size;i++)if(!PEEKRANGE(i)) error("Address");
		if(!BasicFileOpen(pp, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
		f_write(FileTable[fnbr].fptr,  (char *)address, size, &nbr);
		if(nbr!=size)error("File write error");
        FileClose(fnbr);
        return;

    } else if((p = checkstring(cmdline, "NVM")) !=NULL){
    	char *q;
    	int i;
    	uint32_t *qq;
		q=getFstring(p);
    	qq=(uint32_t*)q;
		if(strlen(q)>128)error("String length");
		for(i=0;i<32;i++){
			HAL_RTCEx_BKUPWrite (&hrtc, i, qq[i]);
		}
        return;

    } else {
        char b[STRINGSIZE];
        if(!InitSDCard()) return;
        p = getFstring(cmdline);                           // get the file name and change to the directory
        if(strchr(p, '.') == NULL) strcat(p, ".BAS");
        if(!BasicFileOpen(p, fnbr, FA_WRITE | FA_CREATE_ALWAYS)) return;
        p  = ProgMemory;
        while(!(*p == 0 || *p == 0xff)) {                               // this is a safety precaution
            p = llist(b, p);                                            // expand the line
            pp = b;
            while(*pp) FilePutChar(*pp++, fnbr);                        // write the line to the SD card
            FilePutChar('\r', fnbr); FilePutChar('\n', fnbr);           // terminate the line
            if(p[0] == 0 && p[1] == 0) break;                           // end of the listing ?
        }
    FileClose(fnbr);
    }
}

// load a file into program memory
int FileLoadProgram(char *fname) {
    int fnbr;
    char *p, *buf;
    int c;

    if(!InitSDCard()) return false;
    ClearProgram();												    // clear any leftovers from the previous program
    fnbr = FindFreeFileNbr();
    p = getFstring(fname);
    if(strchr(p, '.') == NULL) strcat(p, ".BAS");
    if(!BasicFileOpen(p, fnbr, FA_READ)) return false;
    p = buf = GetTempMemory(EDIT_BUFFER_SIZE - 256*6);          // leave space for the couple of buffers defined and the file handle
    while(!FileEOF(fnbr)) {                                     // while waiting for the end of file
        if((p - buf) >= EDIT_BUFFER_SIZE - 256*6) error("Not enough memory");
        c = FileGetChar(fnbr) & 0x7f;
        if(IsPrint(c) || c == '\r' || c == '\n' || c == TAB) {
            if(c == TAB) c = ' ';
            *p++ = c;                                               // get the input into RAM
        }
    }
    *p = 0;                                                         // terminate the string in RAM
    FileClose(fnbr);
    SaveProgramToFlash(buf, false);
    return true;
}
void cmd_load(void) {
    int autorun = false;
    char *p;

    p = checkstring(cmdline, "IMAGE");
	if(p) {
        LoadImage(p);
        if(Option.Refresh)Display_Refresh();
        return;
    }
    p = checkstring(cmdline, "JPG");
    if(p) {
        LoadJPGImage(p);
        return;
    }

	p = checkstring(cmdline, "NVM");
	if(p) {
    	char *q=NULL;
    	int i;
    	uint32_t *qq;
    	q = findvar(p, V_FIND);
    	if(!(vartbl[VarIndex].type & T_STR)) error("Invalid variable");
    	qq=(uint32_t*)q;
		for(i=0;i<32;i++){
			qq[i]=HAL_RTCEx_BKUPRead (&hrtc, i);
		}
		q=CtoM(q);
		return;
	}

	p = checkstring(cmdline, "DATA");
	if(p) {
	    int fnbr;
	    unsigned int nbr;
		static FILINFO fnod;
	    char *pp;
		getargs(&p,3,",");
		if(argc!=3)error("Syntax");
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
		pp = getFstring(argv[0]);
		if(strchr(pp, '.') == NULL) strcat(pp, ".DAT");
		uint32_t address=(GetPokeAddr(argv[2]) & 0b11111111111111111111111111111100);
		FSerror = f_stat(pp, &fnod);
		if(FSerror != FR_OK)error((char *)FErrorMsg[4]);
		if((fnod.fattrib & AM_DIR))error((char *)FErrorMsg[4]);
		uint32_t size=fnod.fsize;
		for(uint32_t i=address;i<address+size;i++)if(!POKERANGE(i)) error("Address");
	    fnbr = FindFreeFileNbr();
		if(!BasicFileOpen(pp, fnbr, FA_READ)) return;
		f_read(FileTable[fnbr].fptr,  (char *)address, size, &nbr);
		if(nbr!=size)error("File read error");
	    FileClose(fnbr);
	    return;
	}
     getargs(&cmdline, 3, ",");
    if(!(argc & 1) || argc == 0) error("Syntax");
    if(argc == 3) {
        if(toupper(*argv[2]) == 'R')
            autorun = true;
        else
            error("Syntax");
    } else if(CurrentLinePtr != NULL)
        error("Invalid in a program");

    if(!FileLoadProgram(argv[0])) return;
    ClearRuntime();

    if(autorun) {
        if(*ProgMemory != 0x01) return;                              // no program to run
        ClearRuntime();
        WatchdogSet = false;
        PrepareProgram(true);
        IgnorePIN = false;
        if(Option.ProgFlashSize != PROG_FLASH_SIZEMAX) ExecuteProgram(ProgMemory + Option.ProgFlashSize);       // run anything that might be in the library
        nextstmt = ProgMemory;
    }
}



// search for a volume label, directory or file
// s$ = DIR$(fspec, DIR|FILE|ALL)       will return the first entry
// s$ = DIR$()                          will return the next
// If s$ is empty then no (more) files found
void fun_dir(void) {
    static DIR djd;
    char *p;
    static FILINFO fnod;
    static char pp[32];
    getargs(&ep, 3, ",");
    if(argc != 0) dirflags = -1;
    if(!(argc <= 3)) error("Syntax");

    if(argc == 3) {
        if(checkstring(argv[2], "DIR"))
            dirflags = AM_DIR;
        else if(checkstring(argv[2], "FILE"))
            dirflags = -1;
        else if(checkstring(argv[2], "ALL"))
            dirflags = 0;
        else
            error("Invalid flag specification");
    }


    if(argc != 0) {
        // this must be the first call eg:  DIR$("*.*", FILE)
        p = getFstring(argv[0]);
        strcpy(pp,p);
        djd.pat = pp;
        if(!InitSDCard()) return;                                   // setup the SD card
        FSerror = f_opendir(&djd, "");
        ErrorCheck(0);
    }
        if(disk_status(0) & STA_NOINIT){
           f_closedir(&djd);
            error("SD card not found!");
        }
        if(dirflags == AM_DIR){
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && (fnod.fattrib & AM_DIR) && !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }
        else if(dirflags == -1){
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && !(fnod.fattrib & AM_DIR)&& !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }
        else {
            for (;;) {
                FSerror = f_readdir(&djd, &fnod);		// Get a directory item
                if (FSerror != FR_OK || !fnod.fname[0]) break;	// Terminate if any error or end of directory
                if (pattern_matching(pp, fnod.fname, 0, 0) && !(fnod.fattrib & AM_SYS)) break;		// Test for the file name
            }
        }

    if (FSerror != FR_OK || !fnod.fname[0])f_closedir(&djd);
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    strcpy(sret, fnod.fname);
    CtoM(sret);                                                     // convert to a MMBasic style string
    targ = T_STR;
}


void cmd_mkdir(void) {
    char *p;
    p = getFstring(cmdline);                                        // get the directory name and convert to a standard C string
    if(p[1] == ':') *p = toupper(*p) - 'A' + '0';                   // convert a DOS style disk name to FatFs device number
    if(!InitSDCard()) return;
    FSerror = f_mkdir(p);
    ErrorCheck(0);
}



void cmd_rmdir(void){
    char *p;
    p = getFstring(cmdline);                                        // get the directory name and convert to a standard C string
    if(p[1] == ':') *p = toupper(*p) - 'A' + '0';                   // convert a DOS style disk name to FatFs device number
    if(!InitSDCard()) return;
    FSerror = f_unlink(p);
    ErrorCheck(0);
}



void cmd_chdir(void){
    char *p;
    p = getFstring(cmdline);                                        // get the directory name and convert to a standard C string
    if(p[1] == ':') *p = toupper(*p) - 'A' + '0';                   // convert a DOS style disk name to FatFs device number
    if(!InitSDCard()) return;
    FSerror = f_chdir(p);
    ErrorCheck(0);
}


/* A: added as not returned by getCWD with current setuo */
void fun_cwd(void) {
	//strcpy(sret,filepath);
    //sret = CtoM(GetCWD());
	strcpy(sret,"A");
	strcat(sret,GetCWD());
	CtoM(sret);
   // sret = CtoM(GetCWD());
    targ = T_STR;
}



void cmd_kill(void){
    char *p;
    p = getFstring(cmdline);                                        // get the file name
    if(p[1] == ':') *p = toupper(*p) - 'A' + '0';                   // convert a DOS style disk name to FatFs device number
    if(!InitSDCard()) return;
    FSerror = f_unlink(p);
    ErrorCheck(0);
}



void cmd_seek(void) {
    int fnbr, idx;
    char *buff;
    getargs(&cmdline, 5, ",");
    if(argc != 3) error("Syntax");
    if(*argv[0] == '#') argv[0]++;
    fnbr = getinteger(argv[0]);
    if(fnbr < 1 || fnbr > MAXOPENFILES || FileTable[fnbr].com <= MAXCOMPORTS) error("File number");
    if(FileTable[fnbr].com == 0) error("File number #% is not open", fnbr);
    if(!InitSDCard()) return;
    idx = getinteger(argv[2]) - 1;
    if(idx < 0) idx = 0;
    if(fmode[fnbr] & FA_WRITE){
        FSerror = f_lseek(FileTable[fnbr].fptr,idx);
        ErrorCheck(fnbr);
    } else {
    	buff=SDbuffer[fnbr];
    	FSerror = f_lseek(FileTable[fnbr].fptr,idx - (idx % 512));
    	ErrorCheck(fnbr);
    	FSerror = f_read(FileTable[fnbr].fptr, buff,SDbufferSize, &bw[fnbr]);
    	ErrorCheck(fnbr);
    	buffpointer[fnbr]=idx % 512;
    	lastfptr[fnbr]=(uint32_t)FileTable[fnbr].fptr;
    }
}

void cmd_name(void) {
    char *old, *new, ss[2];
    ss[0] = tokenAS;                                                // this will be used to split up the argument line
    ss[1] = 0;
    {                                                               // start a new block
        getargs(&cmdline, 3, ss);                                   // getargs macro must be the first executable stmt in a block
        if(argc != 3) error("Syntax");
        old = getFstring(argv[0]);                                  // get the old name
        if(old[1] == ':') *old = toupper(*old) - 'A' + '0';         // convert a DOS style disk name to FatFs device number
        new = getFstring(argv[2]);                                  // get the new name
        if(new[1] == ':') *new = toupper(*new) - 'A' + '0';         // convert a DOS style disk name to FatFs device number
        if(!InitSDCard()) return;
        FSerror = f_rename(old, new);
        ErrorCheck(0);
    }
}


extern int BMP_bDecode(int x, int y, int fnbr);

void LoadImage(char *p) {
	int fnbr;
	int xOrigin, yOrigin;

	// get the command line arguments
	getargs(&p, 5, ",");                                            // this MUST be the first executable line in the function
    if(argc == 0) error("Argument count");
    if(!InitSDCard()) return;

    p = getFstring(argv[0]);                                        // get the file name

    xOrigin = yOrigin = 0;
	if(argc >= 3) xOrigin = getinteger(argv[2]);                    // get the x origin (optional) argument
	if(argc == 5) yOrigin = getinteger(argv[4]);                    // get the y origin (optional) argument

	// open the file
	if(strchr(p, '.') == NULL) strcat(p, ".BMP");
	fnbr = FindFreeFileNbr();
    if(!BasicFileOpen(p, fnbr, FA_READ)) return;
    BMP_bDecode(xOrigin, yOrigin, fnbr);
    FileClose(fnbr);
}
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

static uint g_nInFileSize;
static uint g_nInFileOfs;
static int jpgfnbr;
unsigned char pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
   uint n,n_read;

   n = min(g_nInFileSize - g_nInFileOfs, buf_size);
   f_read(FileTable[jpgfnbr].fptr,pBuf,n,&n_read);
   if (n != n_read)
      return PJPG_STREAM_READ_ERROR;
   *pBytes_actually_read = (unsigned char)(n);
   g_nInFileOfs += n;
   return 0;
}

void LoadJPGImage(char *p) {
    pjpeg_image_info_t image_info;
    int mcu_x = 0;
    int mcu_y = 0;
    uint row_pitch;
    uint8_t status;
    gCoeffBuf=(int16_t *)GetTempMemory(8*8*sizeof(int16_t));
    gMCUBufR=(uint8_t *)GetTempMemory(256);
    gMCUBufG=(uint8_t *)GetTempMemory(256);
    gMCUBufB=(uint8_t *)GetTempMemory(256);
    gQuant0=(int16_t *)GetTempMemory(8*8*sizeof(int16_t));
    gQuant1=(int16_t *)GetTempMemory(8*8*sizeof(int16_t));
    gHuffVal2=(uint8_t *)GetTempMemory(256);
    gHuffVal3=(uint8_t *)GetTempMemory(256);
    gInBuf=(uint8_t *)GetTempMemory(PJPG_MAX_IN_BUF_SIZE);
    g_nInFileSize=g_nInFileOfs=0;

	int xOrigin, yOrigin;

	// get the command line arguments
	getargs(&p, 5, ",");                                            // this MUST be the first executable line in the function
    if(argc == 0) error("Argument count");
    if(!InitSDCard()) return;

    p = getFstring(argv[0]);                                        // get the file name

    xOrigin = yOrigin = 0;
	if(argc >= 3) xOrigin = getint(argv[2],0,HRes-1);                    // get the x origin (optional) argument
	if(argc == 5) yOrigin = getint(argv[4],0,VRes-1);                    // get the y origin (optional) argument

	// open the file
	if(strchr(p, '.') == NULL) strcat(p, ".JPG");
	jpgfnbr = FindFreeFileNbr();
    if(!BasicFileOpen(p, jpgfnbr, FA_READ)) return;
    g_nInFileSize = f_size(FileTable[jpgfnbr].fptr);
    status = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, NULL, 0);

    if (status)
    {
        if (status == PJPG_UNSUPPORTED_MODE)
        {
            FileClose(jpgfnbr);
            error("Progressive JPEG files are not supported");
        }
        FileClose(jpgfnbr);
        error("pjpeg_decode_init() failed with status %", status);
    }

    row_pitch = image_info.m_MCUWidth * image_info.m_comps;

    unsigned char *imageblock=GetTempMemory(image_info.m_MCUHeight*image_info.m_MCUWidth*image_info.m_comps);
    for ( ; ; )
    {
        uint8_t *pDst_row = imageblock;
          int y, x;

          status = pjpeg_decode_mcu();

          if (status)
        {
             if (status != PJPG_NO_MORE_BLOCKS)
         {
            FileClose(jpgfnbr);
            error("pjpeg_decode_mcu() failed with status %", status);
         }
         break;
      }

      if (mcu_y >= image_info.m_MCUSPerCol)
      {
            FileClose(jpgfnbr);
         return;
      }
/*    for(int i=0;i<image_info.m_MCUHeight*image_info.m_MCUWidth ;i++){
          imageblock[i*3+2]=image_info.m_pMCUBufR[i];
          imageblock[i*3+1]=image_info.m_pMCUBufG[i];
          imageblock[i*3]=image_info.m_pMCUBufB[i];
      }*/
//         pDst_row = pImage + (mcu_y * image_info.m_MCUHeight) * row_pitch + (mcu_x * image_info.m_MCUWidth * image_info.m_comps);

         for (y = 0; y < image_info.m_MCUHeight; y += 8)
         {
            const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));
            for (x = 0; x < image_info.m_MCUWidth; x += 8)
            {
               uint8_t *pDst_block = pDst_row + x * image_info.m_comps;
               // Compute source byte offset of the block in the decoder's MCU buffer.
               uint src_ofs = (x * 8U) + (y * 16U);
               const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
               const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
               const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

               const int bx_limit = min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

               {
                  int bx, by;
                  for (by = 0; by < by_limit; by++)
                  {
                     uint8_t *pDst = pDst_block;

                     for (bx = 0; bx < bx_limit; bx++)
                     {
                        pDst[2] = *pSrcR++;
                        pDst[1] = *pSrcG++;
                        pDst[0] = *pSrcB++;
                        pDst += 3;
                     }

                     pSrcR += (8 - bx_limit);
                     pSrcG += (8 - bx_limit);
                     pSrcB += (8 - bx_limit);

                     pDst_block += row_pitch;
                  }
               }
            }
            pDst_row += (row_pitch * 8);
         }

      x=mcu_x*image_info.m_MCUWidth+xOrigin;
      y=mcu_y*image_info.m_MCUHeight+yOrigin;
      if(y<VRes && x<HRes){
        int yend=min(VRes-1,y+image_info.m_MCUHeight-1);
        int xend=min(HRes-1,x+image_info.m_MCUWidth-1);
        if(xend<x+image_info.m_MCUWidth-1){
            // need to get rid of some pixels to remove artifacts
            xend=HRes-1;
            unsigned char *s=imageblock;
            unsigned char *d=imageblock;
            for(int yp=0;yp<image_info.m_MCUHeight;yp++){
                for(int xp=0;xp<image_info.m_MCUWidth;xp++){
                    if(xp<xend-x+1){
                        *d++=*s++;
                        *d++=*s++;
                        *d++=*s++;
                    } else {
                        s+=3;
                    }
                }
            }
        }
        DrawBuffer(x,y,xend,yend,(char *)imageblock);
      }
      if(y>=VRes){ //nothing useful left to process
        FileClose(jpgfnbr);
        return;
      }
      mcu_x++;
      if (mcu_x == image_info.m_MCUSPerRow)
      {
         mcu_x = 0;
         mcu_y++;
      }
   }
    FileClose(jpgfnbr);
}



#define MAXFILES 1000
typedef struct ss_flist {
    char fn[FF_MAX_LFN];
    int fs; //file size
    int fd; //file date
    int ft; //file time
} s_flist;

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
}

void cmd_files(void) {
	int i, dirs, ListCnt;
	char *p, *q;
	int fcnt;
	char ts[STRINGSIZE] = "";
	s_flist *flist;
    static DIR djd;
    static FILINFO fnod;
	memset(&djd,0,sizeof(DIR));
	memset(&fnod,0,sizeof(FILINFO));
	if(CurrentLinePtr) error("Invalid in a program");
//	OptionFileErrorAbort = 0;
    fcnt = 0;
   if(*cmdline)
		p = getFstring(cmdline);
    else
		p = "*";

    if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
    flist=GetMemory(sizeof(s_flist)*MAXFILES);
     // print the current directory
    q = GetCWD();
    MMPrintString("A");MMPrintString(q); MMPrintString("\r\n");

    // search for the first file/dir
    FSerror = f_findfirst(&djd, &fnod, "", p);
    ErrorCheck(0);
    // add the file to the list, search for the next and keep looping until no more files
    while(FSerror == FR_OK && fnod.fname[0]) {
        if(fcnt >= MAXFILES) {
            	FreeMemory(flist);
            	f_closedir(&djd);
                error("Too many files to list");
        }
        if(!(fnod.fattrib & (AM_SYS | AM_HID))){
            // add a prefix to each line so that directories will sort ahead of files
            if(fnod.fattrib & AM_DIR)
                ts[0] = 'D';
            else
                ts[0] = 'F';

            // and concatenate the filename found
            strcpy(&ts[1], fnod.fname);

            // sort the file name into place in the array
            for(i = fcnt; i > 0; i--) {
                if( strcicmp((flist[i - 1].fn), (ts)) > 0)
                    flist[i] = flist[i - 1];
                else
                    break;
            }
            strcpy(flist[i].fn, ts);
            flist[i].fs = fnod.fsize;
            fcnt++;
        }
        FSerror = f_findnext(&djd, &fnod);
   }

    // list the files with a pause every screen full
	ListCnt = 2;
	for(i = dirs = 0; i < fcnt; i++) {
		CheckKeyboard();
        if(MMAbort) {
            FreeMemory(flist);
            f_closedir(&djd);
            WDTimer = 0;                                                // turn off the watchdog timer
            memset(inpbuf,0,STRINGSIZE);
            longjmp(mark, 1);                                           // jump back to the input prompt
        }
		if(flist[i].fn[0] == 'D') {
    		dirs++;
            MMPrintString("   <DIR>  ");
		}
		else {
            IntToStrPad(ts, flist[i].fs, ' ', 10, 10); MMPrintString(ts);
            MMPrintString("  ");
        }
        MMPrintString(flist[i].fn + 1);
		MMPrintString("\r\n");
		// check if it is more than a screen full
		if(++ListCnt >= Option.Height && i < fcnt) {
			MMPrintString("PRESS ANY KEY ...");
			MMgetchar();
			MMPrintString("\r                 \r");
			ListCnt = 1;
		}
	}
    // display the summary
    IntToStr(ts, dirs, 10); MMPrintString(ts);
    MMPrintString(" director"); MMPrintString(dirs==1?"y, ":"ies, ");
    IntToStr(ts, fcnt - dirs, 10); MMPrintString(ts);
    MMPrintString(" file"); MMPrintString((fcnt-dirs)==1?"":"s");
	MMPrintString("\r\n");
    FreeMemory(flist);
    f_closedir(&djd);
    memset(inpbuf,0,STRINGSIZE);
	longjmp(mark, 1);                                               // jump back to the input prompt
}
int resolve_path(char *path,char *result,char *pos)
{
    if (*path == '/') {
	*result = '/';
	pos = result+1;
	path++;
    }
    *pos = 0;
    if (!*path) return 0;
    while (1) {
	char *slash;
	struct stat st;
	st.st_mode=0;
	slash = *path ? strchr(path,'/') : NULL;
	if (slash) *slash = 0;
	if (!path[0] || (path[0] == '.' &&
	  (!path[1] || (path[1] == '.' && !path[2])))) {
	    pos--;
	    if (pos != result && path[0] && path[1])
		while (*--pos != '/');
	}
	else {
	    strcpy(pos,path);
//	    if (lstat(result,&st) < 0) return -1;
	    if (S_ISLNK(st.st_mode)) {
		char buf[PATH_MAX];
//		if (readlink(result,buf,sizeof(buf)) < 0) return -1;
		*pos = 0;
		if (slash) {
		    *slash = '/';
		    strcat(buf,slash);
		}
		strcpy(path,buf);
		if (*path == '/') result[1] = 0;
		pos = strchr(result,0);
		continue;
	    }
	    pos = strchr(result,0);
	}
	if (slash) {
	    *pos++ = '/';
	    path = slash+1;
	}
	*pos = 0;
	if (!slash) break;
    }
    return 0;
}

char fullpathname[FF_MAX_LFN];

void fullpath(char *q){
	char *p=GetTempMemory(STRINGSIZE);
	char *rp=GetTempMemory(STRINGSIZE);
//	int i;
	strcpy(p,q);
	memset(fullpathname,0,sizeof(fullpathname));
	strcpy(fullpathname,GetCWD());
   // for(i=0;i<strlen(p);i++)if(p[i]=='\\')p[i]='/';  //allow backslash for the DOS oldies
    if(strcmp(p,".")==0 || strlen(p)==0){
    	memmove(fullpathname, &fullpathname[2],strlen(fullpathname));
//    	MMPrintString("Now: ");MMPrintString(fullpathname);PRet();
    	return; //nothing to do
    }
    if(p[1]==':'){ //modify the requested path so that if the disk is specified the pathname is absolute and starts with /
    	if(p[2]=='/')p+=2;
    	else {
    		p[1]='/';
    		p++;
    	}
    }
    if (*p=='/'){ //absolute path specified
    	strcpy(rp,"A:");
    	strcat(rp,p);
    } else { // relative path specified
    	strcpy(rp,fullpathname); //copy the current pathname
        if(rp[strlen(rp)-1]!='/')  strcat(rp,"/"); //make sure the previous pathname ends in slash, will only be the case at root
    	strcat(rp,p); //append the new pathname
    }
	strcpy(fullpathname,rp); //set the new pathname
	resolve_path(fullpathname,rp,rp); //resolve to single absolute path
	if(strcmp(rp,"A:")==0 || strcmp(rp,"0:")==0 )strcat(rp,"/"); //if root append the slash
	strcpy(fullpathname,rp); //store this back to the filepath variable
	memmove(fullpathname, &fullpathname[2],strlen(fullpathname));
//	MMPrintString("Now: ");MMPrintString(fullpathname);PRet();
}
/*void cmd_files(void){
    ClearVars(0);
	int i, j, c, dirs, ListCnt, currentsize;
	uint32_t currentdate;
	char *p, extension[8];
	int fcnt, sortorder=0;
	char ts[FF_MAX_LFN] = {0};
    char pp[FF_MAX_LFN] = {0};
    char q[FF_MAX_LFN]={0};
	static s_flist *flist=NULL;
	//s_flist *flist=NULL;             //from CMM2
    DIR djd;
    FILINFO fnod;
	//static DIR djd;                 //from CMM2
	//static FILINFO fnod;            //from CMM2
	memset(&djd,0,sizeof(DIR));
	memset(&fnod,0,sizeof(FILINFO));
    fcnt = 0;
    if(*cmdline){
    	getargs(&cmdline,3,",");
    	if(!(argc==1 || argc==3))error("Syntax");
    	p = getFstring(argv[0]);
        i=strlen(p)-1;
        while(i>0 && !(p[i] == 92 || p[i]==47))i--;
        if(i>0){
        	memcpy(q,p,i);
            // for(j=0;j<strlen(q);j++)if(q[j]=='\\')q[j]='/';  //allow backslash for the DOS oldies
        	if(q[1]==':')q[0]='0';
        	i++;
        }
        strcpy(pp,&p[i]);
        if((pp[0]==47 || pp[0]==92) && i==0){
        	strcpy(q,&pp[1]);
        	strcpy(pp,q);
        	strcpy(q,"0:/");
        }
    	if(argc==3){
    		if(checkstring(argv[2], "NAME"))sortorder=0;
    		else if(checkstring(argv[2], "TIME"))sortorder=1;
    		else if(checkstring(argv[2], "SIZE"))sortorder=2;
    		else if(checkstring(argv[2], "TYPE"))sortorder=3;
    		else error("Syntax");
    	}
    }
    if(pp[0]==0)strcpy(pp,"*");
    if(CurrentLinePtr) error("Invalid in a program");
    if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
    if(flist)FreeMemorySafe((void *)&flist);
    flist=GetMemory(sizeof(s_flist)*MAXFILES);
    fullpath(q);
    MMPrintString("A:");
	MMPrintString(fullpathname);
    MMPrintString("\r\n");

    // search for the first file/dir
    FSerror = f_findfirst(&djd, &fnod, fullpathname, pp);
    ErrorCheck(0);
    // add the file to the list, search for the next and keep looping until no more files
    while(FSerror == FR_OK && fnod.fname[0]) {
        if(fcnt >= MAXFILES) {
        		FreeMemorySafe((void *)&flist);
            	f_closedir(&djd);
                error("Too many files to list");
        }
        if(!(fnod.fattrib & (AM_SYS | AM_HID))){
            // add a prefix to each line so that directories will sort ahead of files
            if(fnod.fattrib & AM_DIR){
                ts[0] = 'D';
                currentdate=0xFFFFFFFF;
                fnod.fdate=0xFFFF;
                fnod.ftime=0xFFFF;
                memset(extension,'+',sizeof(extension));
            	extension[sizeof(extension)-1]=0;
            } else {
                ts[0] = 'F';
                currentdate=(fnod.fdate<<16) | fnod.ftime;
                if(fnod.fname[strlen(fnod.fname)-1]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-1]);
                else if(fnod.fname[strlen(fnod.fname)-2]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-2]);
                else if(fnod.fname[strlen(fnod.fname)-3]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-3]);
                else if(fnod.fname[strlen(fnod.fname)-4]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-4]);
                else if(fnod.fname[strlen(fnod.fname)-5]=='.') strcpy(extension,&fnod.fname[strlen(fnod.fname)-5]);
                else {
                	memset(extension,'.',sizeof(extension));
                	extension[sizeof(extension)-1]=0;
                }
           }
            currentsize=fnod.fsize;
            // and concatenate the filename found
            strcpy(&ts[1], fnod.fname);
            // sort the file name into place in the array
            if(sortorder==0){
            	for(i = fcnt; i > 0; i--) {
            		if( strcicmp((flist[i - 1].fn), (ts)) > 0)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
            } else if(sortorder==2){
            	for(i = fcnt; i > 0; i--) {
            		if( (flist[i - 1].fs) > currentsize)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
            } else if(sortorder==3){
            	for(i = fcnt; i > 0; i--) {
            		char e2[8];
                    if(flist[i - 1].fn[strlen(flist[i - 1].fn)-1]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-1]);
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-2]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-2]);
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-3]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-3]);
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-4]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-4]);
                    else if(flist[i - 1].fn[strlen(flist[i - 1].fn)-5]=='.') strcpy(e2,&flist[i - 1].fn[strlen(flist[i - 1].fn)-5]);
                    else {
                    	if(flist[i - 1].fn[0]=='D'){
                    		memset(e2,'+',sizeof(e2));
                        	e2[sizeof(e2)-1]=0;
                    	} else {
                    		memset(e2,'.',sizeof(e2));
                        	e2[sizeof(e2)-1]=0;
                    	}
                    }
            		if( strcicmp((e2), (extension)) > 0)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}
        	} else {
            	for(i = fcnt; i > 0; i--) {
            		if( ((flist[i - 1].fd<<16) |flist[i - 1].ft)  < currentdate)
            			flist[i] = flist[i - 1];
            		else
            			break;
            	}

        	}
            strcpy(flist[i].fn, ts);
            flist[i].fs = fnod.fsize;
            flist[i].fd = fnod.fdate;
            flist[i].ft = fnod.ftime;
            fcnt++;
        }
        FSerror = f_findnext(&djd, &fnod);
   }
    // list the files with a pause every screen full
	ListCnt = 2;
	for(i = dirs = 0; i < fcnt; i++) {
		if(MMAbort) {
        	FreeMemorySafe((void *)&flist);
            f_closedir(&djd);
            WDTimer = 0;                                                // turn off the watchdog timer
            memset(inpbuf,0,STRINGSIZE);
            longjmp(mark, 1);
        }
		if(flist[i].fn[0] == 'D') {
    		dirs++;
            MMPrintString("   <DIR>  ");
		}
		else {
		    IntToStrPad(ts, (flist[i].ft>>11)&0x1F, '0', 2, 10);
		    ts[2] = ':'; IntToStrPad(ts + 3, (flist[i].ft >>5)&0x3F, '0', 2, 10);
		    ts[5]=' ';
		    IntToStrPad(ts + 6, flist[i].fd & 0x1F, '0', 2, 10);
		    ts[8] = '-'; IntToStrPad(ts + 9, (flist[i].fd >> 5)&0xF, '0', 2, 10);
		    ts[11] = '-'; IntToStr(ts + 12 , ((flist[i].fd >> 9)& 0x7F )+1980, 10);
		    ts[16] =' ';
		    IntToStrPad(ts+17, flist[i].fs, ' ', 10, 10); MMPrintString(ts);
            MMPrintString("  ");
        }
        MMPrintString(flist[i].fn + 1);
		MMPrintString("\r\n");
		// check if it is more than a screen full
		if(++ListCnt >= Option.Height && i < fcnt) {
			MMPrintString("PRESS ANY KEY ...");
            do {
                ShowCursor(1);
                if(MMAbort) {
                    FreeMemorySafe((void *)&flist);
                    f_closedir(&djd);
                    WDTimer = 0;                                                // turn off the watchdog timer
                    memset(inpbuf,0,STRINGSIZE);
                    ShowCursor(false);
                    longjmp(mark, 1);
                }
                c=-1;
                if(ConsoleRxBufHead != ConsoleRxBufTail) {                            // if the queue has something in it
                    c = ConsoleRxBuf[ConsoleRxBufTail];
                    ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;   // advance the head of the queue
                }
            } while(c == -1);
            ShowCursor(0);
			MMPrintString("\r                 \r");
			ListCnt = 1;
		}
	}
    // display the summary
    IntToStr(ts, dirs, 10); MMPrintString(ts);
    MMPrintString(" director"); MMPrintString(dirs==1?"y, ":"ies, ");
    IntToStr(ts, fcnt - dirs, 10); MMPrintString(ts);
    MMPrintString(" file"); MMPrintString((fcnt-dirs)==1?"":"s");
	MMPrintString("\r\n");
	FreeMemorySafe((void *)&flist);
    f_closedir(&djd);
    memset(inpbuf,0,STRINGSIZE);
    longjmp(mark, 1);
}*/






//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// ERROR HANDLING ////////////////////////////////////////////


void ErrorThrow(int e) {
    MMerrno = e;
    FSerror = e;
    strcpy(MMErrMsg, (char *)FErrorMsg[e]);
    if(e && OptionFileErrorAbort) error(MMErrMsg);
    return;
}


void ErrorCheck(int fnbr) {                                         //checks for an error, if fnbr is specified frees up the filehandle before sending error
    int e;
    e = (int)FSerror;
    if(fnbr != 0 && e != 0) ForceFileClose(fnbr);
    if(e >= 1 && e <= 19) ErrorThrow(ErrorMap[e]);
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// GENERAL I/O ////////////////////////////////////////////


void FileOpen(char *fname, char *fmode, char *ffnbr) {
    int fnbr;
    BYTE mode = 0;
    if(str_equal(fmode, "OUTPUT"))
        mode = FA_WRITE | FA_CREATE_ALWAYS;
    else if(str_equal(fmode, "APPEND"))
        mode = FA_WRITE | FA_OPEN_APPEND;
    else if(str_equal(fmode, "INPUT"))
        mode = FA_READ;
    else if(str_equal(fmode, "RANDOM"))
        mode = FA_WRITE | FA_OPEN_APPEND | FA_READ;
    else
        error("File access mode");

    if(*ffnbr == '#') ffnbr++;
    fnbr = getinteger(ffnbr);
    BasicFileOpen(fname, fnbr, mode);
}


// this performs the basic duties of opening a file, all file opens in MMBasic should use this
// it will open the file, set the FileTable[] entry and populate the file descriptor
// it returns with true if successful or false if an error
int BasicFileOpen(char *fname, int fnbr, int mode) {
    if(fnbr < 1 || fnbr > MAXOPENFILES) error("File number");
    if(FileTable[fnbr].com != 0) error("File number already open");
    if(!InitSDCard()) return false;
    // if we are writing check the write protect pin (negative pin number means that low = write protect)
    //if((mode & FA_WRITE) && ((Option.SD_WP > 0 && PinRead(Option.SD_WP)) || (Option.SD_WP < 0 && !PinRead(Option.SD_WP))))
    //if((mode & FA_WRITE) && ((Option.SD_WP > 0 && PinRead(Option.SD_WP)) || ((Option.SD_WP < 0) && !PinRead(Option.SD_WP)))){
        //ErrorThrow(4);

    //}else {
        FileTable[fnbr].fptr = GetMemory(sizeof(FIL));              // allocate the file descriptor
        SDbuffer[fnbr]=GetMemory(SDbufferSize);
        if(fname[1] == ':') *fname = toupper(*fname) - 'A' + '0';   // convert a DOS style disk name to FatFs device number
        FSerror = f_open(FileTable[fnbr].fptr, fname, mode);        // open it
        ErrorCheck(fnbr);
        buffpointer[fnbr]=0;
        lastfptr[fnbr]=-1;
        bw[fnbr]=-1;
        fmode[fnbr]=mode;
    //}

    if(FSerror) {
        ForceFileClose(fnbr);
        return false;
    } else
        return true;
}


//close the file and free up the file handle
// it will generate an error if needed
void FileClose(int fnbr) {
    ForceFileClose(fnbr);
    ErrorThrow(FSerror);
}


//close the file and free up the file handle
// it will NOT generate an error
void ForceFileClose(int fnbr) {
    if(fnbr && FileTable[fnbr].fptr != NULL){
        FSerror = f_close(FileTable[fnbr].fptr);
        FreeMemory(FileTable[fnbr].fptr);
        FreeMemory(SDbuffer[fnbr]);
        FileTable[fnbr].fptr = NULL;
        buffpointer[fnbr]=0;
        lastfptr[fnbr]=-1;
        bw[fnbr]=-1;
        fmode[fnbr]=0;
    }
}


char FileGetChar(int fnbr) {
    char ch;
    char *buff=SDbuffer[fnbr];
;
    if(!InitSDCard()) return 0;
    if(fmode[fnbr] & FA_WRITE){
        FSerror = f_read(FileTable[fnbr].fptr, &ch,1, &bw[fnbr]);
        ErrorCheck(fnbr);
    } else {
    	if(!(lastfptr[fnbr]==(uint32_t)FileTable[fnbr].fptr && buffpointer[fnbr]<SDbufferSize)){
    		FSerror = f_read(FileTable[fnbr].fptr, buff,SDbufferSize, &bw[fnbr]);
    		ErrorCheck(fnbr);
    		buffpointer[fnbr]=0;
    		lastfptr[fnbr]=(uint32_t)FileTable[fnbr].fptr;
    	}
    	ch=buff[buffpointer[fnbr]];
    	buffpointer[fnbr]++;
    }
    return ch;
}


void FilePutStr(int count, char *c, int fnbr){
    unsigned int bw;
    InitSDCard();
    FSerror = f_write(FileTable[fnbr].fptr, c, count, &bw);
    ErrorCheck(fnbr);
}


char FilePutChar(char c, int fnbr) {
    static char t;
    unsigned int bw;
    t = c;
    if(!InitSDCard()) return 0;
    FSerror = f_write(FileTable[fnbr].fptr, &t, 1, &bw);
    lastfptr[fnbr]=-1; //invalidate the read file buffer
    ErrorCheck(fnbr);
    return t;
}



int FileEOF(int fnbr) {
    int i;
    if(!InitSDCard()) return 0;
    if(buffpointer[fnbr]<=bw[fnbr]-1) i=0;
    else {
    	i = f_eof(FileTable[fnbr].fptr);
    }
    return i;
}



char *GetCWD(void) {
    char *b;
    int i;
    b = GetTempStrMemory();
    if(!InitSDCard()) return b;
    FSerror = f_getcwd(b, STRINGSIZE);
    for(i=1;i<=strlen(b);i++)b[i-1]=b[i];
    ErrorCheck(0);
    return b;
}



int InitSDCard(void) {
    int i;
    ErrorThrow(0);    // reset mm.errno to zero
    if(!Option.SDCARD_CS) error("SD card not configured");
    if(!MDD_SDSPI_CardDetectState()) { ErrorThrow(20); return false; }  // error if the card is not present
    if(!(SDCardStat & STA_NOINIT))
    	return true;                     // if the card is present and has been initialised we have nothing to do
    for(i = 0; i < MAXOPENFILES; i++)
        if(FileTable[i].com > MAXCOMPORTS)
            if(FileTable[i].fptr != NULL)
                ForceFileClose(i);
    i = f_mount(&FatFs, "", 1);
    if(i) { ErrorThrow(ErrorMap[i]); return false; }
    return true;

}



// finds the first available free file number.  Throws an error if no free file numbers
int FindFreeFileNbr(void) {
    int i;
    for(i = 1; i <= MAXOPENFILES; i++)
        if(FileTable[i].com == 0) return i;
    error("Too many files open");
    return 0;
}


// check the SD card to see if it has been removed.  Also check WAV playback
// this is called from cmd_pause(), the main ExecuteProgram() loop and the console's MMgetchar()
void CheckSDCard(void) {
    if(CurrentlyPlaying == P_WAV || CurrentlyPlaying == P_FLAC || CurrentlyPlaying == P_MP3 || CurrentlyPlaying == P_MOD)
        checkWAVinput();
    else {
#ifndef STM32F4version
        if((diskcheckrate++ % 65536) == 0) {
            if(!(SDCardStat & STA_NOINIT)){ //the card is supposed to be initialised - lets check
                char buff[4];
                if (disk_ioctl(0, MMC_GET_OCR, buff) != RES_OK){
                    BYTE s;
                    s = SDCardStat;
                    s |= (STA_NODISK | STA_NOINIT);
                    SDCardStat = s;
               }
            }
        }
#else
        if(checkSD){
        	checkSD=0;
        	if(!(SDCardStat & STA_NOINIT)){ //the card is supposed to be initialised - lets check
            	if(BSP_SD_GetCardState()!= MSD_OK){
            		FATFS_UnLinkDriver(SDPath);
                    BYTE s;
                    s = SDCardStat;
                    s |= (STA_NODISK | STA_NOINIT);
                    SDCardStat = s;
            	}
        	}
        }
#endif
    }
}

int ExistsFile(char *p){
    int retval=0;
	DIR djd;
	FILINFO fnod;
	memset(&djd,0,sizeof(DIR));
	memset(&fnod,0,sizeof(FILINFO));
	if(!InitSDCard()) return -1;
	FSerror = f_stat(p, &fnod);
	if(FSerror != FR_OK)iret=0;
	else if(!(fnod.fattrib & AM_DIR))retval=1;
    return retval;
}
int ExistsDir(char *p){
    int ireturn=0;
    if(strcmp(p,"/")==0 || strcmp(p,"/.")==0 || strcmp(p,"./")==0 || strcmp(p,"/..")==0 )return 1;
	DIR djd;
	FILINFO fnod;
	memset(&djd,0,sizeof(DIR));
	memset(&fnod,0,sizeof(FILINFO));
	if(p[strlen(p)-1]=='/')strcat(p,".");
	if(!InitSDCard()) return -1;
	FSerror = f_stat(p, &fnod);
	if(FSerror != FR_OK)ireturn=0;
	else if((fnod.fattrib & AM_DIR))ireturn=1;
    return ireturn;
}




