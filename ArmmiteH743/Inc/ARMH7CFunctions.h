/*******************************************************************************************
 *
 *  Definitions used when calling MMBasic Interpreter API Functions from CFunctions
 *  For Armmite H7 MMBasic V5.07.01
 *
 *  This file is public domain and may be used without license.
 *
 *	V1.0
 *	V1.1 unsigned int magic struct option_s, Interrupt (CSubComplete) added
 *	V1.2 added F1-F9 Keys. USBPolling and noLED to unsigned char for uint8 added CFuncInt3&4
 *	V1.3 added #define MAXKEYLEN
 ******************************************************************************************/
#define MAXVARLEN           32                      // maximum length of a variable name
#define MAXDIM              5                       // maximum nbr of dimensions to an array
#define MMFLOAT double
#define MAXKEYLEN 64


//Addresses in the API Table for the pointers to each function
#define BaseAddress   0x80402a0 
#define Vector_uSec               (*(int*)(BaseAddress+0x00))       // void uSec(unsigned int us)
#define Vector_putConsole         (*(int*)(BaseAddress+0x04))       // void putConsole(int c)
#define Vector_getConsole         (*(int*)(BaseAddress+0x08))       // int getConsole(void)
#define Vector_ExtCfg             (*(int*)(BaseAddress+0x0c))       // void ExtCfg(int pin, int cfg, int option)
#define Vector_ExtSet             (*(int*)(BaseAddress+0x10))       // void ExtSet(int pin, int val)
#define Vector_ExtInp             (*(int*)(BaseAddress+0x14))      // int ExtInp(int pin)
#define Vector_PinSetBit          (*(int*)(BaseAddress+0x18))      // void PinSetBit(int pin, unsigned int offset)
#define Vector_PinRead            (*(int*)(BaseAddress+0x1c))      // int PinRead(int pin)
#define Vector_MMPrintString      (*(int*)(BaseAddress+0x20))      // void MMPrintString(char* s)
#define Vector_IntToStr           (*(int*)(BaseAddress+0x24))      // void IntToStr(char *strr, long long int nbr, unsigned int base)
#define Vector_CheckAbort         (*(int*)(BaseAddress+0x28))       // void CheckAbort(void)
#define Vector_GetMemory          (*(int*)(BaseAddress+0x2c))       // void *GetMemory(size_t msize);
#define Vector_GetTempMemory      (*(int*)(BaseAddress+0x30))      // void *GetTempMemory(int NbrBytes)
#define Vector_FreeMemory         (*(int*)(BaseAddress+0x34))      // void FreeMemory(void *addr)
#define Vector_DrawRectangle      *(unsigned int *)(int*)(BaseAddress+0x38)          // void DrawRectangle(int x1, int y1, int x2, int y2, int c)
#define Vector_DrawBitmap         *(unsigned int *)(int*)(BaseAddress+0x3c)         // void DrawBitmap(int x1, int y1, int width, int height, int scale, int fg, int bg, unsigned char *bitmap )
#define Vector_DrawLine           (*(int*)(BaseAddress+0x40))      // void DrawLine(int x1, int y1, int x2, int y2, int w, int c)
#define Vector_FontTable          (*(int*)(BaseAddress+0x44))      // const unsigned char *FontTable[FONT_NBR]
#define Vector_ExtCurrentConfig   (*(int*)(BaseAddress+0x48))       // int ExtCurrentConfig[NBRPINS + 1];
#define Vector_HRes               (*(int*)(BaseAddress+0x4c))       // HRes
#define Vector_VRes               (*(int*)(BaseAddress+0x50))     // VRes
#define Vector_SoftReset          (*(int*)(BaseAddress+0x54))      // void SoftReset(void)
#define Vector_error              (*(int*)(BaseAddress+0x58))       // void error(char *msg)
#define Vector_ProgFlash          (*(int*)(BaseAddress+0x5c))      // ProgFlash
#define Vector_vartbl             (*(int*)(BaseAddress+0x60))       // vartbl
#define Vector_varcnt             (*(int*)(BaseAddress+0x64))      // varcnt
#define Vector_DrawBuffer         *(unsigned int *)(int*)(BaseAddress+0x68)       // void DrawRectangle(int x1, int y1, int x2, int y2, int c)
#define Vector_ReadBuffer         *(unsigned int *)(int*)(BaseAddress+0x6c)          // void DrawRectangle(int x1, int y1, int x2, int y2, int c)
#define Vector_FloatToStr         (*(int*)(BaseAddress+0x70))     // convert a float to a string including scientific notation if necessary
#define Vector_ExecuteProgram     (*(int*)(BaseAddress+0x74))       // void ExecuteProgram(char *fname)
#define Vector_CFuncmSec          (*(int*)(BaseAddress+0x78))       // CFuncmSec
#define Vector_CFuncRam           (*(int*)(BaseAddress+0x7c))       // StartOfCFuncRam
#define Vector_ScrollLCD          *(unsigned int *)(int*)(BaseAddress+0x80)        // void scrollLCD(int lines, int blank)
#define Vector_IntToFloat         (*(int*)(BaseAddress+0x84))       	// MMFLOAT IntToFloat(long long int a)
#define Vector_FloatToInt         (*(int*)(BaseAddress+0x88))       	// long long int FloatToInt64(MMFLOAT x)
#define Vector_Option             (*(int*)(BaseAddress+0x8c))       	// Option
#define Vector_CFuncInt1          (*(int*)(BaseAddress+0x90))        // CFuncInt1
#define Vector_CFuncInt2          (*(int*)(BaseAddress+0x94))        // CFuncInt2
#define Vector_CFuncInt3          (*(int*)(BaseAddress+0x98))        // CFuncInt3
#define Vector_CFuncInt4          (*(int*)(BaseAddress+0x9c))        // CFuncInt4
#define Vector_Sine               (*(int*)(BaseAddress+0xa0))      	// MMFLOAT sin(MMFLOAT)
#define Vector_DrawCircle         (*(int*)(BaseAddress+0xa4))      	// DrawCircle(int x, int y, int radius, int w, int c, int fill, MMFLOAT aspect)
#define Vector_DrawTriangle       (*(int*)(BaseAddress+0xa8))     	// DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int c, int fill)
#define Vector_Timer   			  (*(int*)(BaseAddress+0xac))      	// uint64_t timer(void)
#define Vector_CSubComplete		  (*(unsigned int *)(BaseAddress+0xb0))       // CSubComplete
//Macros to call each function.

#define uSec(a)                         ((void (*)(unsigned int )) Vector_uSec) (a)
#define putConsole(a)                   ((void (*)(char)) Vector_putConsole) (a)
#define getConsole()                    ((int (*)(void)) Vector_getConsole) ()
#define ExtCfg(a,b,c)                   ((void (*)(int, int, int)) Vector_ExtCfg) (a,b,c)
#define ExtSet(a,b)                     ((void(*)(int, int)) Vector_ExtSet) (a,b)
#define ExtInp(a)                       ((int(*)(int)) Vector_ExtInp) (a)
#define PinSetBit(a,b)                  ((void(*)(int, int)) Vector_PinSetBit) (a,b)
#define PinRead(a)                      ((int(*)(int)) Vector_PinRead) (a)
#define MMPrintString(a)                ((void (*)(char*)) Vector_MMPrintString) (a)
#define IntToStr(a,b,c)                 ((void (*)(char *, long long int, unsigned int)) Vector_IntToStr) (a,b,c)
#define CheckAbort()                    ((void (*)(void)) Vector_CheckAbort) ()
#define GetMemory(a)                    ((void* (*)(int)) Vector_GetMemory) (a)
#define GetTempMemory(a)                ((void* (*)(int)) Vector_GetTempMemory) (a)
#define FreeMemory(a)                   ((void (*)(void *)) Vector_FreeMemory) (a)
#define DrawRectangle(a,b,c,d,e)        ((void (*)(int,int,int,int,int)) (*(unsigned int *)Vector_DrawRectangle)) (a,b,c,d,e)
#define DrawRectangleVector             (*(unsigned int *)Vector_DrawRectangle)
#define DrawBitmap(a,b,c,d,e,f,g,h)     ((void (*)(int,int,int,int,int,int,int, char*)) (*(unsigned int *)Vector_DrawBitmap)) (a,b,c,d,e,f,g,h)
#define DrawBitmapVector                (*(unsigned int *)Vector_DrawBitmap)
#define DrawLine(a,b,c,d,e,f)           ((void (*)(int,int,int,int,int,int)) Vector_DrawLine) (a,b,c,d,e,f)
#define FontTable                       (void*)((int*)(Vector_FontTable))
#define ExtCurrentConfig                ((int *) Vector_ExtCurrentConfig)
#define HRes                            (*(unsigned int *) Vector_HRes)
#define VRes                            (*(unsigned int *) Vector_VRes)
#define SoftReset()                     ((void (*)(void)) Vector_SoftReset) ()
#define error(a)                        ((void (*)(char *)) Vector_error) (a)
#define ProgFlash                       ((int *) Vector_ProgFlash)
#define vartbl                          (*(struct s_vartbl *) Vector_vartbl)
#define varcnt                          (*(unsigned int *) Vector_varcnt)
#define DrawBuffer(a,b,c,d,e)           ((void (*)(int,int,int,int,char *)) (*(unsigned int *)Vector_DrawBuffer)) (a,b,c,d,e)
#define DrawBufferVector                (*(unsigned int *)Vector_DrawBuffer)
#define ReadBuffer(a,b,c,d,e)           ((void (*)(int,int,int,int,char *)) (*(unsigned int *)Vector_ReadBuffer)) (a,b,c,d,e)
#define ReadBufferVector                (*(unsigned int *)Vector_ReadBuffer)
#define FloatToStr(a,b,c,d,e)           ((void (*)(char *, MMFLOAT, int, int, char)) Vector_FloatToStr) (a,b,c,d,e)
// NOTE:  The argument to RunBasicSub is a string specifying the name of the BASIC subroutine to be executed.
//        It MUST be terminated with TWO null chars.
#define RunBasicSub(a)                  ((void (*)(char *)) Vector_ExecuteProgram) (a)
#define CFuncmSec                       (*(unsigned int *) Vector_CFuncmSec)
#define CFuncRam                        ((int *) Vector_CFuncRam)
#define ScrollLCD(a,b)                  ((void (*)(int, int)) (*(unsigned int *)Vector_ScrollLCD)) (a, b)
//#define ScrollLCDVector                 (*(unsigned int *)Vector_ScrollLCD)
//#define ScrollBufferV(a,b)              ((void (*)(int, int)) (*(unsigned int *)Vector_ScrollBufferV)) (a, b)
//#define ScrollBufferVVector             (*(unsigned int *)Vector_ScrollBufferV)
//#define ScrollBufferH(a)                ((void (*)(int)) (*(unsigned int *)Vector_ScrollBufferH)) (a)
//#define ScrollBufferHVector             (*(unsigned int *)Vector_ScrollBufferH)
//#define DrawBufferFast(a,b,c,d,e)       ((void (*)(int,int,int,int, char*)) (*(unsigned int *)Vector_DrawBufferFast)) (a,b,c,d,e)
//#define DrawBufferFastVector            (*(unsigned int *)Vector_DrawBufferFast)
//#define ReadBufferFast(a,b,c,d,e)       ((void (*)(int,int,int,int, char*)) (*(unsigned int *)Vector_ReadBufferFast)) (a,b,c,d,e)
//#define ReadBufferFastVector            (*(unsigned int *)Vector_ReadBufferFast)
//#define MoveBufferFast(a,b,c,d,e,f,g)   ((void (*)(int,int,int,int, int,int,int)) (*(unsigned int *)Vector_MoveBuffer)) (a,b,c,d,e,f,g)
//#define MoveBufferFastVector            (*(unsigned int *)Vector_MoveBuffer)
#define DrawPixel(a,b,c)                ((void(*)(int, int, int)) Vector_DrawPixel) (a,b,c)
//#define RoutineChecks()                 ((void (*)(void)) Vector_RoutineChecks) ()
//#define GetPageAddress(a)               ((int(*)(int)) Vector_GetPageAddress) (a)
//#define memcpy(a,b,c)                   ((void (*)(void *, void *, int)) Vector_mycopysafe) (a,b,c)
#define IntToFloat(a)                   ((MMFLOAT (*)(long long)) Vector_IntToFloat) (a)
#define FloatToInt(a)                   ((long long (*)(MMFLOAT)) Vector_FloatToInt) (a)
//#define Option                          ({struct option_s *optionstructurepointer; optionstructurepointer=(void *)(unsigned int)Vector_Option;})
#define Option                          (*(struct option_s *)(unsigned int)Vector_Option)
//#define vartbl                          (*(struct s_vartbl *) Vector_vartbl)
//#define ReadPageAddress                 (*(unsigned int *) Vector_ReadPageAddress)
//#define WritePageAddress                (*(unsigned int *) Vector_WritePageAddress)
#define uSecTimer                       ((unsigned long long (*)(void)) Vector_Timer)
#define CFuncInt1                       (*(unsigned int *) Vector_CFuncInt1)
#define CFuncInt2                       (*(unsigned int *) Vector_CFuncInt2)
#define CFuncInt3                       (*(unsigned int *) Vector_CFuncInt3)
#define CFuncInt4                       (*(unsigned int *) Vector_CFuncInt4)
#define Interrupt                    	(*(unsigned int *) Vector_CSubComplete)

//#define FastTimer                       ((unsigned long long  (*)(void)) Vector_FastTimer)
//#define TicksPerUsec                    (*(unsigned int *) Vector_TicksPerUsec)
//#define map(a)							((int(*)(int)) Vector_Map) (a)
#define Sine(a)                         ((MMFLOAT (*)(MMFLOAT)) Vector_Sine) (a)
//#define VideoColour                     (*(int *) Vector_VideoColour)
#define DrawCircle(a,b,c,d,e,f,g)       ((void (*)(int,int,int,int,int,int,MMFLOAT)) Vector_DrawCircle) (a,b,c,d,e,f,g)
#define DrawTriangle(a,b,c,d,e,f,g,h)   ((void (*)(int,int,int,int,int,int,int,int)) Vector_DrawTriangle) (a,b,c,d,e,f,g,h)
// the structure of the variable table, passed to the CFunction as a pointer Vector_vartbl which is #defined as vartbl
struct s_vartbl {                               // structure of the variable table
    char name[MAXVARLEN];                       // variable's name
    char type;                                  // its type (T_NUM, T_INT or T_STR)
    char level;                                 // its subroutine or function level (used to track local variables)
    unsigned char size;                         // the number of chars to allocate for each element in a string array
    char dummy;
    int __attribute__ ((aligned (4))) dims[MAXDIM];                     // the dimensions. it is an array if the first dimension is NOT zero
    union u_val{
        MMFLOAT f;                              // the value if it is a float
        long long int i;                        // the value if it is an integer
        MMFLOAT *fa;                            // pointer to the allocated memory if it is an array of floats
        long long int *ia;                      // pointer to the allocated memory if it is an array of integers
        char *s;                                // pointer to the allocated memory if it is a string
    }  __attribute__ ((aligned (8))) val;
} __attribute__ ((aligned (8))) val;

//  Useful macros


// Types used to define a variable in the variable table (vartbl).   Often they are ORed together.
// Also used in tokens and arguments to functions
#define T_NOTYPE       0                            // type not set or discovered
#define T_NBR       0x01                            // number (or float) type
#define T_STR       0x02                            // string type
#define T_INT       0x04                            // 64 bit integer type
#define T_PTR       0x08                            // the variable points to another variable's data
#define T_IMPLIED   0x10                            // the variables type does not have to be specified with a suffix
#define T_CONST     0x20                            // the contents of this variable cannot be changed
#define T_BLOCKED   0x40                            // Hash table entry blocked after ERASE


//***************************************************************************************************
// Constants and definitions copied from the Micromite MkII and Micromite Plus source
//***************************************************************************************************

//The Option structure
struct option_s {
	unsigned int magic;
    char Autorun;
    char Tab;
    char Invert;
    char Listcase;
    char Height;
    char Width;
    short dummy;
    unsigned int  PIN;
    unsigned int  Baudrate;
    int  ColourCode;

    // display related
    char DISPLAY_TYPE;
    char DISPLAY_ORIENTATION;

    // touch related
    unsigned char TOUCH_CS;
    unsigned char TOUCH_IRQ;
    char TOUCH_SWAPXY;
    char dummyc[3];
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
    unsigned char noLED;
    volatile unsigned char  USBPolling;   //volatile unsigned int USBPolling;
    unsigned char F1key[MAXKEYLEN]; //268
    unsigned char F5key[MAXKEYLEN]; //268
    unsigned char F6key[MAXKEYLEN]; //332
    unsigned char F7key[MAXKEYLEN]; //396
    unsigned char F8key[MAXKEYLEN]; //460
    unsigned char F9key[MAXKEYLEN]; //524

};



// Define the offsets from the PORT address
// these are used by GetPortAddr(a,b)
#define ANSEL               -8
#define ANSELCLR            -7
#define ANSELSET            -6
#define ANSELINV            -5
#define TRIS                -4
#define TRISCLR             -3
#define TRISSET             -2
#define TRISINV             -1
#define PORT                0
#define PORTCLR             1
#define PORTSET             2
#define PORTINV             3
#define LAT                 4
#define LATCLR              5
#define LATSET              6
#define LATINV              7
#define ODC                 8
#define ODCCLR              9
#define ODCSET              10
#define ODCINV              11
#define CNPU                12
#define CNPUCLR             13
#define CNPUSET             14
#define CNPUINV             15
#define CNPD                16
#define CNPDCLR             17
#define CNPDSET             18
#define CNPDINV             19
#define CNCON               20
#define CNCONCLR            21
#define CNCONSET            22
#define CNCONINV            23
#define CNEN                24
#define CNENCLR             25
#define CNENSET             26
#define CNENINV             27
#define CNSTAT              28
#define CNSTATCLR           29
#define CNSTATSET           30
#define CNSTATINV           31

// configurations for an I/O pin
// these are used by ExtCfg(a,b,c)
#define EXT_NOT_CONFIG          0
#define EXT_ANA_IN				1
#define EXT_DIG_IN				2
#define EXT_FREQ_IN				3
#define EXT_PER_IN				4
#define EXT_CNT_IN				5
#define EXT_INT_HI				6
#define EXT_INT_LO				7
#define EXT_DIG_OUT				8
#define EXT_OC_OUT				9
#define EXT_INT_BOTH			10
#define EXT_COM_RESERVED        100                 // this pin is reserved and SETPIN and PIN cannot be used
#define EXT_BOOT_RESERVED       101                 // this pin is reserved at bootup and cannot be used
#define EXT_DS18B20_RESERVED    102                 // this pin is reserved for DS18B20 and cannot be used


#define NOP()                   __asm volatile ("nop")
#define USERLCDPANEL            25
