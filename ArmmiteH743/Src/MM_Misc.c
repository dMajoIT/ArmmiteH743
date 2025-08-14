/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

MM_Misc.c

Handles all the miscelaneous commands and functions in MMBasic.  These are commands and functions that do not
comfortably fit anywhere else.

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
#include <time.h>
#include "upng.h"
//#include "xregex.h"
#include "re.h"

struct s_inttbl inttbl[NBRINTERRUPTS];
extern char *InterruptReturn;

int TickPeriod[NBRSETTICKS+1];
volatile int TickTimer[NBRSETTICKS+1];
char *TickInt[NBRSETTICKS+1];
volatile unsigned char TickActive[NBRSETTICKS];
char *OnKeyGOSUB = NULL;
const char *daystrings[] = {"dummy","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};

char EchoOption = true;
unsigned long long int __attribute__((section(".my_section"))) saved_variable;  //  __attribute__ ((persistent));  // and this is the address

unsigned int CurrentCpuSpeed;
unsigned int PeripheralBusSpeed;
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim16;
//extern char *ADCInterrupt;
extern volatile int ADCcomplete;

extern volatile int Keycomplete;
extern volatile int DACcomplete;
extern volatile uint64_t * volatile a1point, * volatile a2point, * volatile a3point;
extern volatile MMFLOAT  * volatile a1float, * volatile a2float, * volatile a3float;
extern MMFLOAT ADCscale[3], ADCbottom[3];
extern int ADCmax;

extern volatile int ADCchannelA;
extern volatile int ADCchannelB;
extern volatile int ADCchannelC;
extern volatile int ConsoleTxBufHead;
extern volatile int ConsoleTxBufTail;
extern char *LCDList[];

extern volatile BYTE SDCardStat;
extern volatile int keyboardseen;
extern a_flist *alist;
volatile uint64_t uSecTimer=0;
extern const void * const CallTable[];
extern const char *FErrorMsg[];
//extern volatile char *DACInterrupt;
extern volatile int DACcomplete;
//extern char *GetCWD(void);
extern int codemap(char code, int pin);
extern int codecheck(char *line);
MMFLOAT optionangle=1.0;

void  setterminal(int height,int width);
extern int terminal_width,terminal_height;

char *CSubInterrupt;
volatile int CSubComplete=0;

int SaveOptionErrorSkip=0;
int SaveMMerrno;           // save the error number
char SaveMMErrMsg[MAXERRMSG];  // save the error message
extern long long int GetuSec(void);


//void fun_backup(void){
//	iret=0x38800000;
//	targ=T_INT;
//}
extern int mmOWvalue;
void integersort(int64_t *iarray, int n, long long *index, int flags, int startpoint){
    int i, j = n, s = 1;
    int64_t t;
    if((flags & 1) == 0){
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (iarray[i] < iarray[i - 1]) {
					t = iarray[i];
					iarray[i] = iarray[i - 1];
					iarray[i - 1] = t;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    } else {
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (iarray[i] > iarray[i - 1]) {
					t = iarray[i];
					iarray[i] = iarray[i - 1];
					iarray[i - 1] = t;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    }
}
void floatsort(MMFLOAT *farray, int n, long long *index, int flags, int startpoint){
    int i, j = n, s = 1;
    int64_t t;
    MMFLOAT f;
    if((flags & 1) == 0){
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (farray[i] < farray[i - 1]) {
					f = farray[i];
					farray[i] = farray[i - 1];
					farray[i - 1] = f;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    } else {
		while (s) {
			s = 0;
			for (i = 1; i < j; i++) {
				if (farray[i] > farray[i - 1]) {
					f = farray[i];
					farray[i] = farray[i - 1];
					farray[i - 1] = f;
					s = 1;
			        if(index!=NULL){
			        	t=index[i-1+startpoint];
			        	index[i-1+startpoint]=index[i+startpoint];
			        	index[i+startpoint]=t;
			        }
				}
			}
			j--;
		}
    }
}
/* enhance string sort from pico */
void stringsort(unsigned char *sarray, int n, int offset, long long *index, int flags, int startpoint){
	int ii,i, s = 1,isave;
	int k;
	unsigned char *s1,*s2,*p1,*p2;
	unsigned char temp;
	int reverse= 1-((flags & 1)<<1);
    while (s){
        s=0;
        for(i=1;i<n;i++){
            s2=i*offset+sarray;
            s1=(i-1)*offset+sarray;
            ii = *s1 < *s2 ? *s1 : *s2; //get the smaller  length
            p1 = s1 + 1; p2 = s2 + 1;
            k=0; //assume the strings match
            while((ii--) && (k==0)) {
            if(flags & 2){
                if(toupper(*p1) > toupper(*p2)){
                    k=reverse; //earlier in the array is bigger
                }
                if(toupper(*p1) < toupper(*p2)){
                    k=-reverse; //later in the array is bigger
                }
            } else {
                if(*p1 > *p2){
                    k=reverse; //earlier in the array is bigger
                }
                if(*p1 < *p2){
                    k=-reverse; //later in the array is bigger
                }
            }
            p1++; p2++;
            }
        // if up to this point the strings match
        // make the decision based on which one is shorter
            if(k==0){
                if(*s1 > *s2) k=reverse;
                if(*s1 < *s2) k=-reverse;
            }
            if (k==1){ // if earlier is bigger swap them round
                ii = *s1 > *s2 ? *s1 : *s2; //get the bigger length
                ii++;
                p1=s1;p2=s2;
                while(ii--){
                temp=*p1;
                *p1=*p2;
                *p2=temp;
                p1++; p2++;
                }
                s=1;
                if(index!=NULL){
                    isave=index[i-1+startpoint];
                    index[i-1+startpoint]=index[i+startpoint];
                    index[i+startpoint]=isave;
                }
            }
        }
    }
    if((flags & 5) == 5){
        for(i=n-1;i>=0;i--){
            s2=i*offset+sarray;
            if(*s2 !=0)break;
        }
        i++;
        if(i){
            s2=(n-i)*offset+sarray;
            memmove(s2,sarray,offset*i);
            memset(sarray,0,offset*(n-i));
            if(index!=NULL){
                long long int *newindex=(long long int *)GetTempMemory(n* sizeof(long long int));
                memmove(&newindex[n-i],&index[startpoint],i*sizeof(long long int));
                memmove(newindex,&index[startpoint+i],(n-i)*sizeof(long long int));
                memmove(&index[startpoint],newindex,n*sizeof(long long int));
            }
        }
    } else if(flags & 4){
        for(i=0;i<n;i++){
            s2=i*offset+sarray;
            if(*s2 !=0)break;
        }
        if(i){
            s2=i*offset+sarray;
            memmove(sarray,s2,offset*(n-i));
            s2=(n-i)*offset+sarray;
            memset(s2,0,offset*i);
            if(index!=NULL){
                long long int *newindex=(long long int *)GetTempMemory(n* sizeof(long long int));
                memmove(newindex,&index[startpoint+i],(n-i)*sizeof(long long int));
                memmove(&newindex[n-i],&index[startpoint],i*sizeof(long long int));
                memmove(&index[startpoint],newindex,n*sizeof(long long int));
            }
        }
    }
}
/*
void stringsort(unsigned char *sarray, int n, int offset, long long *index, int flags, int startpoint){
	int ii,i, s = 1,isave;
	int k;
	unsigned char *s1,*s2,*p1,*p2;
	unsigned char temp;
	int reverse= 1-((flags & 1)<<1);
    while (s){
      s=0;
      for(i=1;i<n;i++){
        s2=i*offset+sarray;
        s1=(i-1)*offset+sarray;
        ii = *s1 < *s2 ? *s1 : *s2; //get the smaller  length
        p1 = s1 + 1; p2 = s2 + 1;
        k=0; //assume the strings match
        while((ii--) && (k==0)) {
          if(flags & 2){
			  if(toupper(*p1) > toupper(*p2)){
				k=reverse; //earlier in the array is bigger
			  }
			  if(toupper(*p1) < toupper(*p2)){
				 k=-reverse; //later in the array is bigger
			  }
          } else {
			  if(*p1 > *p2){
				k=reverse; //earlier in the array is bigger
			  }
			  if(*p1 < *p2){
				 k=-reverse; //later in the array is bigger
			  }
          }
          p1++; p2++;
        }
      // if up to this point the strings match
      // make the decision based on which one is shorter
      if(k==0){
        if(*s1 > *s2) k=reverse;
        if(*s1 < *s2) k=-reverse;
      }
      if (k==1){ // if earlier is bigger swap them round
        ii = *s1 > *s2 ? *s1 : *s2; //get the bigger length
        ii++;
        p1=s1;p2=s2;
        while(ii--){
          temp=*p1;
          *p1=*p2;
          *p2=temp;
          p1++; p2++;
        }
        s=1;
        if(index!=NULL){
        	isave=index[i-1+startpoint];
        	index[i-1+startpoint]=index[i+startpoint];
        	index[i+startpoint]=isave;
        }
      }
    }
  }
}
*/
/*enhanced sort from pico */
void cmd_sort(void){
    MMFLOAT *a3float=NULL;
    int64_t *a3int=NULL,*a4int=NULL;
    unsigned char *a3str=NULL;
    int i, size=0, truesize,flags=0, maxsize=0, startpoint=0;
	getargs(&cmdline,9,",");
    size=parseany(argv[0],&a3float,&a3int,&a3str,&maxsize,true)-1;
    truesize=size;
    if(argc>=3 && *argv[2]){
        int card=parseintegerarray(argv[2],&a4int,2,1,NULL,true)-1;
    	if(card !=size)error("Array size mismatch");
    }
    if(argc>=5 && *argv[4])flags=getint(argv[4],0,7);
    if(argc>=7 && *argv[6])startpoint=getint(argv[6],OptionBase,size+OptionBase);
    size-=startpoint;
    if(argc==9)size=getint(argv[8],1,size+1+OptionBase)-1;
    if(startpoint)startpoint-=OptionBase;
    if(a3float!=NULL){
    	a3float+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	floatsort(a3float, size+1, a4int, flags, startpoint);
    } else if(a3int!=NULL){
    	a3int+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	integersort(a3int,  size+1, a4int, flags, startpoint);
    } else if(a3str!=NULL){
    	a3str+=((startpoint)*(maxsize+1));
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	stringsort(a3str, size+1,maxsize+1, a4int, flags, startpoint);
    }
}
/*
void cmd_sort(void){
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    MMFLOAT *a3float=NULL;
    int64_t *a3int=NULL,*a4int=NULL;
    unsigned char *a3str=NULL;
    int i, size, truesize,flags=0, maxsize=0, startpoint=0;
	getargs(&cmdline,9,",");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if(vartbl[VarIndex].type & T_NBR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            error("Argument 1 must be array");
        }
        a3float = (MMFLOAT *)ptr1;
    } else if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            error("Argument 1 must be array");
        }
        a3int = (int64_t *)ptr1;
    } else if(vartbl[VarIndex].type & T_STR) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            error("Argument 1 must be array");
        }
        a3str = (unsigned char *)ptr1;
        maxsize=vartbl[VarIndex].size;
    } else error("Argument 1 must be array");
	if((uint32_t)ptr1!=(uint32_t)vartbl[VarIndex].val.s)error("Argument 1 must be array");
    truesize=size=(vartbl[VarIndex].dims[0] - OptionBase);
    if(argc>=3 && *argv[2]){
    	ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    	if(vartbl[VarIndex].type & T_INT) {
    		if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    		if(vartbl[VarIndex].dims[0] <= 0 ) {		// Not an array
    			error("Argument 2 must be integer array");
    		}
    		a4int = (int64_t *)ptr2;
    	} else error("Argument 2 must be integer array");
    	if((vartbl[VarIndex].dims[0] - OptionBase) !=size)error("Arrays should be the same size");
		if((uint32_t)ptr2!=(uint32_t)vartbl[VarIndex].val.s)error("Argument 2 must be array");
    }
    if(argc>=5 && *argv[4])flags=getint(argv[4],0,3);
    if(argc>=7 && *argv[6])startpoint=getint(argv[6],OptionBase,size+OptionBase);
    size-=startpoint;
    if(argc==9)size=getint(argv[8],1,size+1+OptionBase)-1;
    if(startpoint)startpoint-=OptionBase;
    if(a3float!=NULL){
    	a3float+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	floatsort(a3float, size+1, a4int, flags, startpoint);
    } else if(a3int!=NULL){
    	a3int+=startpoint;
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	integersort(a3int,  size+1, a4int, flags, startpoint);
    } else if(a3str!=NULL){
    	a3str+=((startpoint)*(maxsize+1));
    	if(a4int!=NULL)for(i=0;i<truesize+1;i++)a4int[i]=i+OptionBase;
    	stringsort(a3str,  size+1,maxsize+1, a4int, flags, startpoint);
    }
}
*/
void fun_datetime(void){
    sret = GetTempStrMemory();                                    // this will last for the life of the command
	if(checkstring(ep, "NOW")){
	    RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
	    IntToStrPad(sret, day, '0', 2, 10);
	    sret[2] = '-'; IntToStrPad(sret + 3, month, '0', 2, 10);
	    sret[5] = '-'; IntToStr(sret + 6, year, 10);
	    sret[10] = ' ';
	    IntToStrPad(sret+11, hour, '0', 2, 10);
	    sret[13] = ':'; IntToStrPad(sret + 14, minute, '0', 2, 10);
	    sret[16] = ':'; IntToStrPad(sret + 17, second, '0', 2, 10);
	} else {
		struct tm  *tm;
		struct tm tma;
		tm=&tma;
		time_t timestamp = getinteger(ep);                       /* See README.md if your system lacks timegm(). */
	   // time_t timestamp = getint(ep, 0x80000000, 0x7FFFFFFF); /* See README.md if your system lacks timegm(). */
	    tm=gmtime(&timestamp);
	    IntToStrPad(sret, tm->tm_mday, '0', 2, 10);
	    sret[2] = '-'; IntToStrPad(sret + 3, tm->tm_mon+1, '0', 2, 10);
	    sret[5] = '-'; IntToStr(sret + 6, tm->tm_year+1900, 10);
	    sret[10] = ' ';
	    IntToStrPad(sret+11, tm->tm_hour, '0', 2, 10);
	    sret[13] = ':'; IntToStrPad(sret + 14, tm->tm_min, '0', 2, 10);
	    sret[16] = ':'; IntToStrPad(sret + 17, tm->tm_sec, '0', 2, 10);
	}
    CtoM(sret);
    targ = T_STR;
}
void fun_keydown(void) {
	int i,n=getint(ep,0,8);
	iret=0;
	while(MMInkey() != -1); // clear anything in the input buffer
	if(n==8){
		iret=(caps_lock ? 1: 0) |
				(num_lock ? 2: 0) |
				(scroll_lock ? 4: 0);
	} else if(n){
		iret = KeyDown[n-1];											        // this is the character
	} else {
		for(i=0;i<6;i++){
			if(KeyDown[i])iret++;
		}
	}
	targ=T_INT;
}

void fun_format(void) {
	char *p, *fmt;
	int inspec;
	getargs(&ep, 3, ",");
	if(argc%2 == 0) error("Invalid syntax");
	if(argc == 3)
		fmt = getCstring(argv[2]);
	else
		fmt = "%g";

	// check the format string for errors that might crash the CPU
	for(inspec = 0, p = fmt; *p; p++) {
		if(*p == '%') {
			inspec++;
			if(inspec > 1) error("Only one format specifier (%) allowed");
			continue;
		}

		if(inspec == 1 && (*p == 'g' || *p == 'G' || *p == 'f' || *p == 'e' || *p == 'E'|| *p == 'l'))
			inspec++;


		if(inspec == 1 && !(IsDigitinline(*p) || *p == '+' || *p == '-' || *p == '.' || *p == ' '))
			error("Illegal character in format specification");
	}
	if(inspec != 2) error("Format specification not found");
	sret = GetTempMemory(STRINGSIZE);									// this will last for the life of the command
	sprintf(sret, fmt, getnumber(argv[0]));
	CtoM(sret);
	targ=T_STR;
}
// this is invoked as a command (ie, TIMER = 0)
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command and save in the timer
/*void cmd_timer(void) {
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	uint64_t t = getinteger(++cmdline)*1000;
	HAL_NVIC_DisableIRQ(TIM16_IRQn);
	TIM16->CR1 &= ~1;
	uSecTimer=t/50000;
	__HAL_TIM_SET_COUNTER(&htim16, (uint16_t)(t % 50000));
	TIM16->CR1 |= 1;
	HAL_NVIC_EnableIRQ(TIM16_IRQn);
}*/


/*

void cmd_JumpToBootloader(void)
{
  uint32_t i=0;
//  void (*SysMemBootJump)(void);

  // Set the address of the entry point to bootloader
 //    volatile uint32_t BootAddr = 0x1FF00000;

  // Disable all interrupts
     __disable_irq();

  // Disable Systick timer
     SysTick->CTRL = 0;

  // Set the clock to the default state
     HAL_RCC_DeInit();

  // Clear Interrupt Enable Register & Interrupt Pending Register
     for (i=0;i<5;i++)
     {
	  NVIC->ICER[i]=0xFFFFFFFF;
	  NVIC->ICPR[i]=0xFFFFFFFF;
     }
  // Re-enable all interrupts
     __enable_irq();

  // Set up the jump to booloader address + 4
 //    SysMemBootJump = (void (*)(void)) (*((uint32_t *) ((BootAddr + 4))));

  // Set the main stack pointer to the bootloader stack
 //    __set_MSP(*(uint32_t *)BootAddr);

 // Call the function to jump to bootloader location
//     SysMemBootJump();
     volatile uint32_t * vect = (volatile uint32_t *)0x1FF09800;    //0x1FF00FFE     0x1FFF77DE for STM32F40xxx   0x1FFF76DE for STM32F4011xx
     asm volatile(
       "mov sp, %[boot_sp]\n\t"
       "bx %[boot_pc]\n\t"
       ::[boot_sp]"r"(vect[0]),[boot_pc]"r"(vect[1])
     );
  // Jump is done successfully
     while (1)
     {
      // Code should never reach this loop
     }
}
*/

/*
To get the USB part working, I had to omit the call to __disable_irq();

Here's my working code:

void jump_to_bootloader( void )
{
HAL_RCC_DeInit();
SysTick->CTRL = SysTick->LOAD = SysTick->VAL = 0;
__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

const uint32_t p = (*((uint32_t *) 0x1FFF0000));
__set_MSP( p );

void (*SysMemBootJump)(void);
SysMemBootJump = (void (*)(void)) (*((uint32_t *) 0x1FFF0004));
SysMemBootJump();

while( 1 ) {}
}
*/



/*
void setterminal(void){
	  char sp[20]={0};
	  strcpy(sp,"\033[8;");
	  IntToStr(&sp[strlen(sp)],Option.Height,10);
	  strcat(sp,";");
	  //IntToStr(&sp[strlen(sp)],Option.Width+1,10);
	  IntToStr(&sp[strlen(sp)],Option.Width,10);
	  strcat(sp,"t");
	  if(Option.Width>=80)SerUSBPutS(sp);	//Don'tresize terminal below 80chars
}
*/

void  setterminal(int height,int width){
	  char sp[20]={0};
	  strcpy(sp,"\033[8;");
	  IntToStr(&sp[strlen(sp)],height,10);
	  strcat(sp,";");
	  IntToStr(&sp[strlen(sp)],width+1,10);
	  strcat(sp,"t");
	  SerUSBPutS(sp);					//
}


long long int GetuSec(void){
	HAL_NVIC_DisableIRQ(TIM16_IRQn);
	TIM16->CR1 &= ~1;
	uint64_t gettime= (uint64_t) __HAL_TIM_GET_COUNTER(&htim16);
	TIM16->CR1 |= 1;
	HAL_NVIC_EnableIRQ(TIM16_IRQn);
	return gettime  + uSecTimer*50000;											        // this is the character

}
void fun_uSec(void) {
	fret=(MMFLOAT)GetuSec()/1000.0;
	targ=T_NBR;
}
void cmd_uSec(void){
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	uint64_t t = getinteger(++cmdline)*1000;
	HAL_NVIC_DisableIRQ(TIM16_IRQn);
	TIM16->CR1 &= ~1;
	uSecTimer=t/50000;
	__HAL_TIM_SET_COUNTER(&htim16, (uint16_t)(t % 50000));
	TIM16->CR1 |= 1;
	HAL_NVIC_EnableIRQ(TIM16_IRQn);

}

void fun_epoch(void){
	char *arg;
	struct tm  *tm;
	struct tm tma;
	tm=&tma;
	int d, m, y, h, min, s;
	if(!checkstring(ep, "NOW"))
	{
		arg = getCstring(ep);
		getargs(&arg, 11, "-/ :");										// this is a macro and must be the first executable stmt in a block
		if(!(argc == 11)) error("Syntax");
			d = atoi(argv[0]);
			m = atoi(argv[2]);
			y = atoi(argv[4]);
			if(d>1000){
				int tmp=d;
				d=y;
				y=tmp;
			}
			if(y >= 0 && y < 100) y += 2000;
			if(d < 1 || d > 31 || m < 1 || m > 12 || y < 1902 || y > 2999) error("Invalid date");
			h = atoi(argv[6]);
			min  = atoi(argv[8]);
			s = atoi(argv[10]);
			if(h < 0 || h > 23 || min < 0 || m > 59 || s < 0 || s > 59) error("Invalid time");
			day = d;
			month = m;
			year = y;
			tm->tm_year = y - 1900;
			tm->tm_mon = m - 1;
			tm->tm_mday = d;
			tm->tm_hour = h;
			tm->tm_min = min;
			tm->tm_sec = s;
	} else {
		RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
		tm->tm_year = year - 1900;
		tm->tm_mon = month - 1;
		tm->tm_mday = day;
		tm->tm_hour = hour;
		tm->tm_min = minute;
		tm->tm_sec = second;
	}
	    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
	    iret=timestamp;
	    targ = T_INT;
}
void cmd_pause(void) {
	static int interrupted = false;
    MMFLOAT f;
    static int64_t end,count;
    int64_t start, stop, tick;
    f = getnumber(cmdline);                                         // get the pulse width
    if(f < 0) error("Number out of bounds");
    if(f < 0.05) return;

	if(f < 1.5) {
		uSec(f * 1000);                                             // if less than 1.5mS do the pause right now
		return;                                                     // and exit straight away
    }
	if(!interrupted){
		count=(int64_t)(f*1000);
		start=GetuSec();
		tick=PauseTimer;
		while(PauseTimer==tick){}  //wait for the next clock tick
		stop=GetuSec();
		count-=(stop-start);
		end = (count % 1000); //get the number of ticks remaining
		count/=1000;
		PauseTimer=0;
	}
    if(count){
		if(InterruptReturn == NULL) {
			// we are running pause in a normal program
			// first check if we have reentered (from an interrupt) and only zero the timer if we have NOT been interrupted.
			// This means an interrupted pause will resume from where it was when interrupted
			if(!interrupted) PauseTimer = 0;
			interrupted = false;

			while(PauseTimer < count) {
				CheckAbort();
				if(check_interrupt()) {
					// if there is an interrupt fake the return point to the start of this stmt
					// and return immediately to the program processor so that it can send us off
					// to the interrupt routine.  When the interrupt routine finishes we should reexecute
					// this stmt and because the variable interrupted is static we can see that we need to
					// resume pausing rather than start a new pause time.
					while(*cmdline && *cmdline != cmdtoken) cmdline--;	// step back to find the command token
					InterruptReturn = cmdline;							// point to it
					interrupted = true;								    // show that this stmt was interrupted
					return;											    // and let the interrupt run
				}
			}
			interrupted = false;
		}
		else {
			// we are running pause in an interrupt, this is much simpler but note that
			// we use a different timer from the main pause code (above)
			IntPauseTimer = 0;
			while(IntPauseTimer < FloatToInt32(f)) CheckAbort();
		}
    }
	uSec(end);
}


#ifndef PREPARSE
//void MIPS16 cmd_longString(void){
void cmd_longString(void){
    char *tp;
    tp = checkstring(cmdline, (char *)"SETBYTE");
    if(tp){
        int64_t *dest=NULL;
        int p=0;
        uint8_t *q=NULL;
        int nbr;
        int j=0;
    	getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=(parseintegerarray(argv[0],&dest,1,1,NULL,true)-1)*8-1;
        q=(uint8_t *)&dest[1];
        p = getint(argv[2],OptionBase,j-OptionBase);
        nbr=getint(argv[4],0,255);
        q[p-OptionBase]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"APPEND");
    if(tp){
        int64_t *dest=NULL;
        char *p= NULL;
        char *q= NULL;
        int i,j,nbr;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        q+=dest[0];
        p=(char *)getstring(argv[2]);
        nbr = i = *p++;
        if(j*8 < dest[0]+i)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"TRIM");
    if(tp){
        int64_t *dest=NULL;
        uint32_t trim;
        char *p, *q=NULL;
        int i;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        trim=getint(argv[2],1,dest[0]);
        i = dest[0]-trim;
        p=q+trim;
        while(i--)*q++=*p++;
        dest[0]-=trim;
        return;
    }
    tp = checkstring(cmdline, (char *)"REPLACE");
    if(tp){
        int64_t *dest=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        p=(char *)getstring(argv[2]);
        nbr=getint(argv[4],1,dest[0]-*p+1);
        q+=nbr-1;
        i = *p++;
        while(i--)*q++=*p++;
        return;
    }
    tp = checkstring(cmdline, (char *)"LOAD");
    if(tp){
        int64_t *dest=NULL;
        char *p;
        char *q=NULL;
        int i,j;
        getargs(&tp, 5, ( char *)",");
        if(argc != 5)error("Argument count");
        int64_t nbr=getinteger(argv[2]);
        i=nbr;
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        dest[0]=0;
        p=(char *)getstring(argv[4]);
        if(nbr> *p)nbr=*p;
        p++;
        if(j*8 < dest[0]+nbr)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"LEFT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0])nbr=i=src[0];
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"RIGHT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
        getargs(&tp, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0]){
            nbr=i=src[0];
        } else p+=(src[0]-nbr);
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"MID");
    if(tp){
       int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr,start;
        getargs(&tp, 7,(char *)",");
        if(argc < 5)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        start=getint(argv[4],1,src[0]);
        if(argc==7)nbr=getinteger(argv[6]);
        else nbr=src[0];
        p+=start-1;
        if(nbr+start>src[0]){
            nbr=src[0]-start+1;
        }
        i=nbr;
        if(j*8 < nbr)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, (char *)"CLEAR");
    if(tp){
        int64_t *dest=NULL;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        dest[0]=0;
        return;
    }
    tp = checkstring(cmdline, (char *)"RESIZE");
    if(tp){
        int64_t *dest=NULL;
        int j=0;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=(parseintegerarray(argv[0],&dest,1,1,NULL,true)-1)*8;
        dest[0] = getint(argv[2], 0, j);
        return;
    }
    tp = checkstring(cmdline, (char *)"UCASE");
    if(tp){
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        i=dest[0];
        while(i--){
        if(*q >= 'a' && *q <= 'z')
            *q -= 0x20;
        q++;
        }
        return;
    }
    tp = checkstring(cmdline, (char *)"PRINT");
    if(tp){
        int64_t *dest=NULL;
        char *q=NULL;
        int j, fnbr=0;
        int docrlf=true;
        getargs(&tp, 5, ",;");
        if(argc==5)error("Syntax");
        if(argc >= 3){
            if(*argv[0] == '#')argv[0]++;                                 // check if the first arg is a file number
            fnbr = getinteger(argv[0]);                                 // get the number
            parseintegerarray(argv[2],&dest,2,1,NULL,true);
            if(*argv[3]==';')docrlf=false;
        } else {
            parseintegerarray(argv[0],&dest,1,1,NULL,true);
            if(*argv[1]==';')docrlf=false;
         }
        q=(char *)&dest[1];
        j=dest[0];
        while(j--){
            MMfputc(*q++, fnbr);
        }
        if(docrlf)MMfputs(( char *)"\2\r\n", fnbr);
        return;
    }
    tp = checkstring(cmdline, (char *)"LCASE");
    if(tp){
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
        getargs(&tp, 1, (char *)",");
        if(argc != 1)error("Argument count");
        parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        i=dest[0];
        while(i--){
            if(*q >= 'A' && *q <= 'Z')
                *q += 0x20;
            q++;
        }
        return;
    }
    tp = checkstring(cmdline, (char *)"COPY");
    if(tp){
       int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true);
        q=(char *)&dest[1];
        dest[0]=0;
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        if((j-i)*8 < src[0])error("Destination array too small");
        i=src[0];
        while(i--)*q++=*p++;
        dest[0]=src[0];
        return;
    }
    tp = checkstring(cmdline, (char *)"CONCAT");
    if(tp){
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j,d=0,s=0;
        getargs(&tp, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=parseintegerarray(argv[0],&dest,1,1,NULL,true)-1;
        q=(char *)&dest[1];
        d=dest[0];
        parseintegerarray(argv[2],&src,2,1,NULL,false);
        p=(char *)&src[1];
        i = s = src[0];
        if(j*8 < (d+s))error("Destination array too small");
        q+=d;
        while(i--)*q++=*p++;
        dest[0]+=src[0];
        return;
    }
    error("Invalid option");
}
void fun_LGetStr(void){
        char *p;
        char *s=NULL;
        int64_t *src=NULL;
        int start,nbr,j;
        getargs(&ep, 5, (char *)",");
        if(argc != 5)error("Argument count");
        j=(parseintegerarray(argv[0],&src,2,1,NULL,false)-1)*8;
        start = getint(argv[2],1,j);
        nbr = getinteger(argv[4]);
        if(nbr < 1 || nbr > MAXSTRLEN) error("Number out of bounds");
        if(start+nbr>src[0])nbr=src[0]-start+1;
        sret = GetTempMemory(STRINGSIZE);                                       // this will last for the life of the command
        s=(char *)&src[1];
        s+=(start-1);
        p=(char *)sret+1;
        *sret=nbr;
        while(nbr--)*p++=*s++;
        *p=0;
        targ = T_STR;
}

void fun_LGetByte(void){
        uint8_t *s=NULL;
        int64_t *src=NULL;
        int start,j;
    	getargs(&ep, 3, (char *)",");
        if(argc != 3)error("Argument count");
        j=(parseintegerarray(argv[0],&src,2,1,NULL,false)-1)*8;
        s=(uint8_t *)&src[1];
        start = getint(argv[2],OptionBase,j-OptionBase);
        iret=s[start-OptionBase];
        targ = T_INT;
}

// Fixes bug in LINSTR changing the regular expression if a variable - from picomite 6.00.02
// new parsing used
// Allows alternate form which evaluates the search string as a regular expression
//see fun_Instr
void fun_LInstr(void){
	    int64_t *src=NULL;
	   // void *ptr1 = NULL;
       // int64_t *dest=NULL;
        //char *srch;
        char srch[STRINGSIZE];
        char *str=NULL;
        int slen,found=0,i,j,n;
        getargs(&ep, 7, ",");
        if(argc <3  || argc > 7)error("Argument count");
        int64_t start;
        if(argc>=5 && *argv[4])start=getinteger(argv[4])-1;
        else start=0;
        /*
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            str=(char *)&dest[0];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        srch=getstring(argv[2]);
        */
        j=(parseintegerarray(argv[0],&src,2,1,NULL,false)-1);
        str=(char *)&src[0];
       // srch=(char *)getstring(argv[2]);
       strcpy((char *)srch,( char *)getstring(argv[2]));
        if(argc<7){
            slen=*srch;
            iret=0;
            //if(start>dest[0] || start<0 || slen==0 || dest[0]==0 || slen>dest[0]-start)found=1;
            if(start>src[0] || start<0 || slen==0 || src[0]==0 || slen>src[0]-start)found=1;
            if(!found){
                //n=dest[0]- slen - start;
                n=src[0]- slen - start;

                for(i = start; i <= n + start; i++) {
                    if(str[i + 8] == srch[1]) {
                        for(j = 0; j < slen; j++)
                            if(str[j + i + 8] != srch[j + 1])
                                break;
                        if(j == slen) {iret= i + 1; break;}
                    }
                }
            }
        } else { //search string is a regular expression

        	int match_length;
            MMFLOAT *temp=NULL;
            MtoC(srch);
            temp = findvar(argv[6], V_FIND);
            if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");

         	int match_idx = re_match(srch, &str[start+8], &match_length);
         	if (match_idx != -1){
              // PInt(match_idx); PInt(match_length);
       		    if(temp)*temp=(MMFLOAT)(match_length);
          		//iret=match_idx;
           		iret=match_idx+1+start;
           		if(temp)*temp=(MMFLOAT)(match_length);
           	}else{
                //MMPrintString("No Match");
              	iret=0;
             	if(temp)*temp=0.0;
          	}

	    }
        targ = T_INT;
}
/*
// Allows alternate form which evaluates the search string as a regular expression
//see fun_Instr
void fun_LInstr(void){
	    int64_t *src=NULL;
	   // void *ptr1 = NULL;
       // int64_t *dest=NULL;
        char *srch;
        char *str=NULL;
        int slen,found=0,i,j,n;
        getargs(&ep, 7, ",");
        if(argc <3  || argc > 7)error("Argument count");
        int64_t start;
        if(argc>=5 && *argv[4])start=getinteger(argv[4])-1;
        else start=0;

        j=(parseintegerarray(argv[0],&src,2,1,NULL,false)-1);
        str=(char *)&src[0];
        srch=(char *)getstring(argv[2]);
        if(argc<7){
            slen=*srch;
            iret=0;
            //if(start>dest[0] || start<0 || slen==0 || dest[0]==0 || slen>dest[0]-start)found=1;
            if(start>src[0] || start<0 || slen==0 || src[0]==0 || slen>src[0]-start)found=1;
            if(!found){
                //n=dest[0]- slen - start;
                n=src[0]- slen - start;

                for(i = start; i <= n + start; i++) {
                    if(str[i + 8] == srch[1]) {
                        for(j = 0; j < slen; j++)
                            if(str[j + i + 8] != srch[j + 1])
                                break;
                        if(j == slen) {iret= i + 1; break;}
                    }
                }
            }
        } else { //search string is a regular expression

        	int match_length;
            MMFLOAT *temp=NULL;
            MtoC(srch);
            temp = findvar(argv[6], V_FIND);
            if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");

         	int match_idx = re_match(srch, &str[start+8], &match_length);
         	if (match_idx != -1){
              // PInt(match_idx); PInt(match_length);
       		    if(temp)*temp=(MMFLOAT)(match_length);
          		//iret=match_idx;
           		iret=match_idx+1+start;
           		if(temp)*temp=(MMFLOAT)(match_length);
           	}else{
                //MMPrintString("No Match");
              	iret=0;
             	if(temp)*temp=0.0;
          	}

	    }
        targ = T_INT;
}
*/

void fun_LCompare(void){
    int64_t *dest, *src;
    char *p=NULL;
    char *q=NULL;
    int d=0,s=0,found=0;
    getargs(&ep, 3, (char *)",");
    if(argc != 3)error("Argument count");
    parseintegerarray(argv[0],&dest,1,1,NULL,false);
    q=(char *)&dest[1];
    d=dest[0];
    parseintegerarray(argv[2],&src,1,1,NULL,false);
    p=(char *)&src[1];
    s=src[0];
    while(!found) {
        if(d == 0 && s == 0) {found=1;iret=0;}
        if(d == 0 && !found) {found=1;iret=-1;}
        if(s == 0 && !found) {found=1;iret=1;}
        if(*q < *p && !found) {found=1;iret=-1;}
        if(*q > *p && !found) {found=1;iret=1;}
        q++;  p++;  d--; s--;
    }
    targ = T_INT;
}

void fun_LLen(void) {
    int64_t *dest=NULL;
    getargs(&ep, 1, (char *)",");
    if(argc != 1)error("Argument count");
    parseintegerarray(argv[0],&dest,1,1,NULL,false);
    iret=dest[0];
    targ = T_INT;
}
#endif


#ifdef PREPARSE
void cmd_longString(void){
    char *tp;
    tp = checkstring(cmdline, "SETBYTE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        int p=0;
        uint8_t *q=NULL;
        int nbr;
        int j=0;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            j=(vartbl[VarIndex].dims[0] - OptionBase)*8-1;
            dest = (long long int *)ptr1;
            q=(uint8_t *)&dest[1];
        } else error("Argument 1 must be integer array");
        p = getint(argv[2],OptionBase,j-OptionBase);
        nbr=getint(argv[4],0,255);
        q[p-OptionBase]=nbr;
         return;
    }
    tp = checkstring(cmdline, "APPEND");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *p= NULL;
        char *q= NULL;
        int i,j,nbr;
    	getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
            q+=dest[0];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        p=getstring(argv[2]);
        nbr = i = *p++;
         if(j*8 < dest[0]+i)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, "TRIM");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        uint32_t trim;
        char *p, *q=NULL;
        int i;
    	getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        //trim=getint(argv[2],1,dest[0]-1);
        trim=getint(argv[2],0,dest[0]);
        i = dest[0]-trim;
        p=q+trim;
        while(i--)*q++=*p++;
        dest[0]-=trim;
        return;
    }
    tp = checkstring(cmdline, "REPLACE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,nbr;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        p=getstring(argv[2]);
        nbr=getint(argv[4],1,dest[0]-*p+1);
        q+=nbr-1;
        i = *p++;
        while(i--)*q++=*p++;
        return;
    }
    tp = checkstring(cmdline, "LOAD");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *p;
        char *q=NULL;
        int i,j;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        int64_t nbr=getinteger(argv[2]);
        i=nbr;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            dest[0]=0;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        p=getstring(argv[4]);
        if(nbr> *p)nbr=*p;
        p++;
        if(j*8 < dest[0]+nbr)error("Integer array too small");
        while(i--)*q++=*p++;
        dest[0]+=nbr;
        return;
    }
    tp = checkstring(cmdline, "LEFT");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
        } else error("Argument 2 must be integer array");
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0])nbr=i=src[0];
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, "RIGHT");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr;
    	getargs(&tp, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
        } else error("Argument 2 must be integer array");
        nbr=i=getinteger(argv[4]);
        if(nbr>src[0]){
            nbr=i=src[0];
        } else p+=(src[0]-nbr);
        if(j*8 < i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, "MID");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i,j,nbr,start;
    	getargs(&tp, 7, ",");
        if(argc != 7)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
        } else error("Argument 2 must be integer array");
        start=getint(argv[4],1,src[0]);
        nbr=getinteger(argv[6]);
        p+=start-1;
        if(nbr+start>src[0]){
            nbr=src[0]-start+1;
        }
        i=nbr;
        if(j*8 < nbr)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=nbr;
        return;
    }
    tp = checkstring(cmdline, "CLEAR");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        getargs(&tp, 1, ",");
        if(argc != 1)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
        } else error("Argument 1 must be integer array");
        dest[0]=0;
        return;
    }
    tp = checkstring(cmdline, "RESIZE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        int j=0;
        getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            j=(vartbl[VarIndex].dims[0] - OptionBase)*8;
            dest = (long long int *)ptr1;
        } else error("Argument 1 must be integer array");
        dest[0] = getint(argv[2], 0, j);
        return;
    }
    tp = checkstring(cmdline, "UCASE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
    	getargs(&tp, 1, ",");
        if(argc != 1)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        i=dest[0];
        while(i--){
        if(*q >= 'a' && *q <= 'z')
            *q -= 0x20;
        q++;
        }
        return;
    }
    tp = checkstring(cmdline, "PRINT");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *q=NULL;
        int i, j, fnbr;
    	getargs(&tp, 5, ",;");
        if(argc < 1 || argc > 4)error("Argument count");
        if(argc > 0 && *argv[0] == '#') {								// check if the first arg is a file number
            argv[0]++;
            fnbr = getinteger(argv[0]);									// get the number
            i = 1;
            if(argc >= 2 && *argv[1] == ',') i = 2;						// and set the next argument to be looked at
        }
        else {
            fnbr = 0;													// no file number so default to the standard output
            i = 0;
        }
        if(argc>=1){
            ptr1 = findvar(argv[i], V_FIND | V_EMPTY_OK);
            if(vartbl[VarIndex].type & T_INT) {
                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                    error("Argument must be integer array");
                }
                dest = (long long int *)ptr1;
                q=(char *)&dest[1];
            } else error("Argument must be integer array");
            j=dest[0];
            while(j--){
                MMfputc(*q++, fnbr);
            }
            i++;
        }
        if(argc > i){
            if(*argv[i] == ';') return;
        }
        MMfputs("\2\r\n", fnbr);
        return;
    }
    tp = checkstring(cmdline, "LCASE");
    if(tp){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *q=NULL;
        int i;
    	getargs(&tp, 1, ",");
        if(argc != 1)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        i=dest[0];
        while(i--){
            if(*q >= 'A' && *q <= 'Z')
                *q += 0x20;
            q++;
        }
        return;
    }
    tp = checkstring(cmdline, "COPY");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j;
    	getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            dest[0]=0;
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
            i=src[0];
        } else error("Argument 2 must be integer array");
        if(j*8 <i)error("Destination array too small");
        while(i--)*q++=*p++;
        dest[0]=src[0];
        return;
    }
    tp = checkstring(cmdline, "CONCAT");
    if(tp){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest=NULL, *src=NULL;
        char *p=NULL;
        char *q=NULL;
        int i=0,j,d=0,s=0;
    	getargs(&tp, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            d=dest[0];
            q=(char *)&dest[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
            i = s = src[0];
        } else error("Argument 2 must be integer array");
        if(j*8 < (d+s))error("Destination array too small");
        q+=d;
        while(i--)*q++=*p++;
        dest[0]+=src[0];
        return;
    }
    error("Invalid option");
}
void fun_LGetStr(void){
        void *ptr1 = NULL;
        char *p;
        char *s=NULL;
        int64_t *src=NULL;
        int start,nbr,j;
    	getargs(&ep, 5, ",");
        if(argc != 5)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            src = (int64_t *)ptr1;
            s=(char *)&src[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase)*8;
        start = getint(argv[2],1,j);
	nbr = getinteger(argv[4]);
	if(nbr < 1 || nbr > MAXSTRLEN) error("Number out of bounds");
        if(start+nbr>src[0])nbr=src[0]-start+1;
	sret = GetTempStrMemory();                                       // this will last for the life of the command
        s+=(start-1);
        p=sret+1;
        *sret=nbr;
        while(nbr--)*p++=*s++;
        *p=0;
        targ = T_STR;
}

void fun_LGetByte(void){
        void *ptr1 = NULL;
        uint8_t *s=NULL;
        int64_t *src=NULL;
        int start,j;
    	getargs(&ep, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            src = (int64_t *)ptr1;
            s=(uint8_t *)&src[1];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase)*8-1;
        start = getint(argv[2],OptionBase,j-OptionBase);
        iret=s[start-OptionBase];
        targ = T_INT;
}

// Allows alternate form which evaluates the search string as a regular expression
//see fun_Instr
void fun_LInstr(void){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *srch;
        char *str=NULL;
        int slen,found=0,i,j,n;
        getargs(&ep, 7, ",");
        if(argc <3  || argc > 7)error("Argument count");
        int64_t start;
        if(argc>=5 && *argv[4])start=getinteger(argv[4])-1;
        else start=0;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            str=(char *)&dest[0];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        srch=getstring(argv[2]);
        if(argc<7){
            slen=*srch;
            iret=0;
            if(start>dest[0] || start<0 || slen==0 || dest[0]==0 || slen>dest[0]-start)found=1;
            if(!found){
                n=dest[0]- slen - start;

                for(i = start; i <= n + start; i++) {
                    if(str[i + 8] == srch[1]) {
                        for(j = 0; j < slen; j++)
                            if(str[j + i + 8] != srch[j + 1])
                                break;
                        if(j == slen) {iret= i + 1; break;}
                    }
                }
            }
        } else { //search string is a regular expression
        	/*
        	int match_length;

        	//int  re_match(const char* pattern, const char* text, int* matchlength);

        	MMFLOAT *temp=NULL;
            char *s=GetTempMemory(STRINGSIZE), *p=GetTempMemory(STRINGSIZE);
       		strcpy(s,getCstring(argv[0+n]));
       		strcpy(p,getCstring(argv[2+n]));
       		if(argc==5+n){  // This is the passed variable to return the match length
       			temp = findvar(argv[4+n], V_FIND);
       			if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");
       		}
       		targ=T_INT;
        	//int match_idx = re_matchp(pattern, &s[start], &match_length);
        	int match_idx = re_match(p, &s[start], &match_length);
        	if (match_idx != -1){
        	 // PInt(match_idx); PInt(match_length);
        	    if(temp)*temp=(MMFLOAT)(match_length);
        	    //iret=match_idx;
        	    iret=match_idx+1+start;
        		if(temp)*temp=(MMFLOAT)(match_length);
        	}else{
        		//MMPrintString("No Match");
        		iret=0;
        		if(temp)*temp=0.0;
        	}
*/

        	int match_length;
           // regex_t regex;
           // int reti;
           // regmatch_t pmatch;
            MMFLOAT *temp=NULL;
            MtoC(srch);
            temp = findvar(argv[6], V_FIND);
            if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");
           // reti = regcomp(&regex, srch, 0);
           // if( reti ) {
           //     regfree(&regex);
           //     error("Could not compile regex");
           // }

            //int match_idx = re_matchp(pattern, &s[start], &match_length);
         	int match_idx = re_match(srch, &str[start+8], &match_length);
         	if (match_idx != -1){
              // PInt(match_idx); PInt(match_length);
       		    if(temp)*temp=(MMFLOAT)(match_length);
          		//iret=match_idx;
           		iret=match_idx+1+start;
           		if(temp)*temp=(MMFLOAT)(match_length);
           	}else{
                //MMPrintString("No Match");
              	iret=0;
             	if(temp)*temp=0.0;
          	}

	       // reti = regexec(&regex, &str[start+8], 1, &pmatch, 0);
           // if( !reti ){
           //     iret=pmatch.rm_so+1+start;
           //     if(temp)*temp=(MMFLOAT)(pmatch.rm_eo-pmatch.rm_so);
           // }
           // else if( reti == REG_NOMATCH ){
           //     iret=0;
           //     if(temp)*temp=0.0;
           // }
           // else{
		   //     regfree(&regex);
           //     error("Regex execution error");
           // }
		   // regfree(&regex);
        }
        targ = T_INT;
}
/*
// Allows alternate form which evaluates the search string as a regular expression
//see fun_Instr
void fun_LInstr(void){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *srch;
        char *str=NULL;
        int slen,found=0,i,j,n;
        getargs(&ep, 7, ",");
        if(argc <3  || argc > 7)error("Argument count");
        int64_t start;
        if(argc>=5 && *argv[4])start=getinteger(argv[4])-1;
        else start=0;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {      // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            str=(char *)&dest[0];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        srch=getstring(argv[2]);
        if(argc<7){
            slen=*srch;
            iret=0;
            if(start>dest[0] || start<0 || slen==0 || dest[0]==0 || slen>dest[0]-start)found=1;
            if(!found){
                n=dest[0]- slen - start;

                for(i = start; i <= n + start; i++) {
                    if(str[i + 8] == srch[1]) {
                        for(j = 0; j < slen; j++)
                            if(str[j + i + 8] != srch[j + 1])
                                break;
                        if(j == slen) {iret= i + 1; break;}
                    }
                }
            }
        } else { //search string is a regular expression
            regex_t regex;
            int reti;
            regmatch_t pmatch;
            MMFLOAT *temp=NULL;
            MtoC(srch);
            temp = findvar(argv[6], V_FIND);
            if(!(vartbl[VarIndex].type & T_NBR)) error("Invalid variable");
            reti = regcomp(&regex, srch, 0);
            if( reti ) {
                regfree(&regex);
                error("Could not compile regex");
            }
	        reti = regexec(&regex, &str[start+8], 1, &pmatch, 0);
            if( !reti ){
                iret=pmatch.rm_so+1+start;
                if(temp)*temp=(MMFLOAT)(pmatch.rm_eo-pmatch.rm_so);
            }
            else if( reti == REG_NOMATCH ){
                iret=0;
                if(temp)*temp=0.0;
            }
            else{
		        regfree(&regex);
                error("Regex execution error");
            }
		    regfree(&regex);
        }
        targ = T_INT;
}

*/
/*
void fun_LInstr(void){
        void *ptr1 = NULL;
        int64_t *dest=NULL;
        char *srch;
        char *str=NULL;
        int slen,found=0,i,j,n;
    	getargs(&ep, 5, ",");
        if(argc <3  || argc > 5)error("Argument count");
        int64_t start;
        if(argc==5)start=getinteger(argv[4])-1;
        else start=0;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            str=(char *)&dest[0];
        } else error("Argument 1 must be integer array");
        j=(vartbl[VarIndex].dims[0] - OptionBase);
        srch=getstring(argv[2]);
        slen=*srch;
        iret=0;
        if(start>dest[0] || start<0 || slen==0 || dest[0]==0 || slen>dest[0]-start)found=1;
        if(!found){
            n=dest[0]- slen - start;

            for(i = start; i <= n + start; i++) {
                if(str[i + 8] == srch[1]) {
                    for(j = 0; j < slen; j++)
                        if(str[j + i + 8] != srch[j + 1])
                            break;
                    if(j == slen) {iret= i + 1; break;}
                }
            }
        }
        targ = T_INT;
}
*/
void fun_LCompare(void){
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest, *src;
        char *p=NULL;
        char *q=NULL;
        int d=0,s=0,found=0;
    	getargs(&ep, 3, ",");
        if(argc != 3)error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q=(char *)&dest[1];
            d=dest[0];
        } else error("Argument 1 must be integer array");
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p=(char *)&src[1];
            s=src[0];
        } else error("Argument 2 must be integer array");
    while(!found) {
        if(d == 0 && s == 0) {found=1;iret=0;}
        if(d == 0 && !found) {found=1;iret=-1;}
        if(s == 0 && !found) {found=1;iret=1;}
        if(*q < *p && !found) {found=1;iret=-1;}
        if(*q > *p && !found) {found=1;iret=1;}
        q++;  p++;  d--; s--;
    }
        targ = T_INT;
}

void fun_LLen(void) {
    void *ptr1 = NULL;
    int64_t *dest=NULL;
    getargs(&ep, 1, ",");
    if(argc != 1)error("Argument count");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_INT) {
        if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
            error("Argument 1 must be integer array");
        }
        dest = (long long int *)ptr1;
    } else error("Argument 1 must be integer array");
    iret=dest[0];
    targ = T_INT;
}

#endif

void update_clock(void){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	sTime.Hours = hour;
	sTime.Minutes = minute;
	sTime.Seconds = second;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
	sDate.WeekDay = day_of_week;
	sDate.Month = month;
	sDate.Date = day;
	sDate.Year = year-2000;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		error("RTC hardware error");
	}
}


// this is invoked as a command (ie, date$ = "6/7/2010")
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command, split it up and save in the system counters
void cmd_date(void) {
	char *arg;
	struct tm  *tm;
	struct tm tma;
	tm=&tma;
	int dd, mm, yy;
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
	arg = getCstring(cmdline);
	{
		getargs(&arg, 5, "-/");										// this is a macro and must be the first executable stmt in a block
		if(argc != 5) error("Syntax");
		dd = atoi(argv[0]);
		mm = atoi(argv[2]);
		yy = atoi(argv[4]);
		if(yy >= 0 && yy < 100) yy += 2000;
	    //check year
	    if(yy>=1900 && yy<=9999)
	    {
	        //check month
	        if(mm>=1 && mm<=12)
	        {
	            //check days
	            if((dd>=1 && dd<=31) && (mm==1 || mm==3 || mm==5 || mm==7 || mm==8 || mm==10 || mm==12))
	                {}
	            else if((dd>=1 && dd<=30) && (mm==4 || mm==6 || mm==9 || mm==11))
	                {}
	            else if((dd>=1 && dd<=28) && (mm==2))
	                {}
	            else if(dd==29 && mm==2 && (yy%400==0 ||(yy%4==0 && yy%100!=0)))
	                {}
	            else
	                error("Day is invalid");
	        }
	        else
	        {
	            error("Month is not valid");
	        }
	    }
	    else
	    {
	        error("Year is not valid");
	    }

		mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		day = dd;
		month = mm;
		year = yy;
	    tm->tm_year = year - 1900;
	    tm->tm_mon = month - 1;
	    tm->tm_mday = day;
	    tm->tm_hour = hour;
	    tm->tm_min = minute;
	    tm->tm_sec = second;
	    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
	    tm=gmtime(&timestamp);
	    day_of_week=tm->tm_wday;
	    if(day_of_week==0)day_of_week=7;
		update_clock();
		mT4IntEnable(1);       										// enable interrupt
	}
}

// this is invoked as a function
void fun_date(void) {
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
    IntToStrPad(sret, day, '0', 2, 10);
    sret[2] = '-'; IntToStrPad(sret + 3, month, '0', 2, 10);
    sret[5] = '-'; IntToStr(sret + 6, year, 10);
	CtoM(sret);
    targ = T_STR;
}

// this is invoked as a function
void fun_day(void) {
    char *arg;
    struct tm  *tm;
    struct tm tma;
    tm=&tma;
    time_t time_of_day;
    int i;
    sret = GetTempStrMemory();                                    // this will last for the life of the command
    int d, m, y;
    if(!checkstring(ep, "NOW"))
    {
        arg = getCstring(ep);
        getargs(&arg, 5, "-/");										// this is a macro and must be the first executable stmt in a block
        if(!(argc == 5))error("Syntax");
        d = atoi(argv[0]);
        m = atoi(argv[2]);
        y = atoi(argv[4]);
		if(d>1000){
			int tmp=d;
			d=y;
			y=tmp;
		}
        if(y >= 0 && y < 100) y += 2000;
        if(d < 1 || d > 31 || m < 1 || m > 12 || y < 1902 || y > 2999) error("Invalid date");
        tm->tm_year = y - 1900;
        tm->tm_mon = m - 1;
        tm->tm_mday = d;
        tm->tm_hour = 0;
        tm->tm_min = 0;
        tm->tm_sec = 0;
        time_of_day = timegm(tm);
        tm=gmtime(&time_of_day);
        i=tm->tm_wday;
        if(i==0)i=7;
    	strcpy(sret,daystrings[i]);
    } else {
        RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
    	strcpy(sret,daystrings[day_of_week]);
    }
    CtoM(sret);
    targ = T_STR;
}

// this is invoked as a command (ie, time$ = "6:10:45")
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command, split it up and save in the system counters
void cmd_time(void) {
	char *arg;
	int h = 0;
	int m = 0;
	int s = 0;
    MMFLOAT f;
    long long int i64;
    char *ss;
    int t=0;
    int offset;
	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Syntax");
	++cmdline;
    evaluate(cmdline, &f, &i64, &ss, &t, false);
	if(t==T_STR){
	arg = getCstring(cmdline);
	{
		getargs(&arg, 5, ":");								// this is a macro and must be the first executable stmt in a block
		if(argc%2 == 0) error("Syntax");
		h = atoi(argv[0]);
		if(argc >= 3) m = atoi(argv[2]);
		if(argc == 5) s = atoi(argv[4]);
		if(h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) error("Invalid time");
		mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		hour = h;
		minute = m;
		second = s;
		SecondsTimer = 0;
		update_clock();
    	mT4IntEnable(1);       										// enable interrupt
    }
	} else {
		struct tm  *tm;
		struct tm tma;
		tm=&tma;
		offset=getinteger(cmdline);
		RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
		tm->tm_year = year - 1900;
		tm->tm_mon = month - 1;
		tm->tm_mday = day;
		tm->tm_hour = hour;
		tm->tm_min = minute;
		tm->tm_sec = second;
	    time_t timestamp = timegm(tm); /* See README.md if your system lacks timegm(). */
	    timestamp+=offset;
	    tm=gmtime(&timestamp);
		mT4IntEnable(0);       										// disable the timer interrupt to prevent any conflicts while updating
		hour = tm->tm_hour;
		minute = tm->tm_min;
		second = tm->tm_sec;
		SecondsTimer = 0;
		update_clock();
    	mT4IntEnable(1);       										// enable interrupt
	}
}




// this is invoked as a function
void fun_time(void) {
	sret = GetTempStrMemory();									// this will last for the life of the command
    RtcGetTime();									// disable the timer interrupt to prevent any conflicts while updating
    IntToStrPad(sret, hour, '0', 2, 10);
    sret[2] = ':'; IntToStrPad(sret + 3, minute, '0', 2, 10);
    sret[5] = ':'; IntToStrPad(sret + 6, second, '0', 2, 10);
    if(Option.fulltime){
    	sret[8] = '.'; IntToStrPad(sret + 9, milliseconds, '0', 3, 10);
    }
	CtoM(sret);
    targ = T_STR;
}



void cmd_ireturn(void){
  if(InterruptReturn == NULL) error("Not in interrupt");
  checkend(cmdline);
  nextstmt = InterruptReturn;
  if(LocalIndex)    ClearVars(LocalIndex--);                        // delete any local variables
    TempMemoryIsChanged = true;                                     // signal that temporary memory should be checked
    *CurrentInterruptName = 0;                                        // for static vars we are not in an interrupt
    InterruptReturn = NULL;
    if(DelayedDrawKeyboard) {
        DelayedDrawKeyboard = false;
        DrawKeyboard(1);                                            // the pop-up GUI keyboard should be drawn AFTER the pen down interrupt
    }
    if(DelayedDrawFmtBox) {
        DelayedDrawFmtBox = false;
        DrawFmtBox(1);                                              // the pop-up GUI keyboard should be drawn AFTER the pen down interrupt
    }
    if(SaveOptionErrorSkip>0)OptionErrorSkip=SaveOptionErrorSkip+1; //fix from winbasic
    strcpy( MMErrMsg, SaveMMErrMsg);   //restore saved error messages
    MMerrno=SaveMMerrno;              //restore saved MMerrno

}

/*
// set up the tick interrupt
void cmd_settick(void){
	int period;
	int irq;
	getargs(&cmdline, 5, ",");
	if(!(argc == 3 || argc == 5)) error("Argument count");
	period = getint(argv[0], 0, INT_MAX);
	if(argc == 5)
	    irq = getint(argv[4], 1, NBRSETTICKS) - 1;
	else
	    irq = 0;
	if(period == 0) {
		TickInt[irq] = NULL;										// turn off the interrupt
    } else {
		TickPeriod[irq] = period;
		TickInt[irq] = GetIntAddress(argv[2]);					    // get a pointer to the interrupt routine
		TickTimer[irq] = 0;										    // set the timer running
		InterruptUsed = true;

	}
}
*/

// set up the tick interrupt
void cmd_settick(void){
	int period;
	int irq=0;;

    getargs(&cmdline, 5, ",");
    if(!(argc == 3 || argc == 5)) error("Argument count");
    period = getint(argv[0], 0, INT_MAX);

    if(argc == 5) irq = getint(argv[4], 1, NBRSETTICKS) - 1;
    if(strcasecmp(argv[0],"PAUSE")==0){
            TickActive[irq]=0;
            return;
        } else if(strcasecmp(argv[0],"RESUME")==0){
            TickActive[irq]=1;
            return;
        } else period = getint(argv[0], -1, INT_MAX);
        if(period == 0) {
            TickInt[irq] = NULL;                                        // turn off the interrupt
        } else {
            TickPeriod[irq] = period;
            TickInt[irq] = GetIntAddress(argv[2]);                      // get a pointer to the interrupt routine
            TickTimer[irq] = 0;                                         // set the timer running
            InterruptUsed = true;
            TickActive[irq]=1;

        }

}


void cmd_option(void) {
	char *tp;
	tp = checkstring(cmdline, "NOCHECK");
	if(tp) {
		if(checkstring(tp, "ON"))	{ OptionNoCheck=true; return; }
		if(checkstring(tp, "OFF"))	{ OptionNoCheck=false; return; }
        return;
	}
	tp = checkstring(cmdline, "BASE");
	if(tp) {
		if(DimUsed) error("Must be before DIM or LOCAL");
		OptionBase = getint(tp, 0, 1);
		return;
	}
	tp = checkstring(cmdline, "ESCAPE");
	    if(tp) {
	        OptionEscape = true;
	        return;
	}
    tp = checkstring(cmdline, "CONSOLE");
	   if(tp) {
	      if(checkstring(tp, "BOTH"))OptionConsole=3;
	      else if(checkstring(tp, "SERIAL"))OptionConsole=1;
	      else if(checkstring(tp, "SCREEN"))OptionConsole=2;
	      else if(checkstring(tp,"NONE"))OptionConsole=0;
	      else error("Syntax");
	      return;
	}
	//tp = checkstring(cmdline, "CHARS");
	//    if(tp) {
	//    	if(checkstring(tp, "OFF"))	{OptionChars = false; return; }
	 //       OptionChars = true;
	//        return;
	//}
	tp = checkstring(cmdline, "EXPLICIT");
	if(tp) {
//        if(varcnt != 0) error("Variables already defined");
		OptionExplicit = true;
		return;
	}

	tp = checkstring(cmdline, "ANGLE");
	if(tp) {
		if(checkstring(tp, "DEGREES"))	{ optionangle=RADCONV; return; }
		if(checkstring(tp, "RADIANS"))	{ optionangle=1.0; return; }
	}


    tp = checkstring(cmdline, "DEFAULT");
	if(tp) {
		if(checkstring(tp, "INTEGER"))	{ DefaultType = T_INT; 	return; }
		if(checkstring(tp, "FLOAT"))	{ DefaultType = T_NBR; 	return; }
		if(checkstring(tp, "STRING"))	{ DefaultType = T_STR; 	return; }
		if(checkstring(tp, "NONE"))	    { DefaultType = T_NOTYPE; 	return; }
	}

	tp = checkstring(cmdline, "BREAK");
	if(tp) {
		BreakKey = getinteger(tp);
		return;
	}
    tp = checkstring(cmdline, "F1");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F1key))error("Maximum % characters",MAXKEYLEN-1);
		else strcpy((char *)Option.F1key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "F5");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F5key))error("Maximum % characters",MAXKEYLEN-1);
		else strcpy((char *)Option.F5key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "F6");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F6key))error("Maximum % characters",MAXKEYLEN-1);
		else strcpy((char *)Option.F6key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "F7");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F7key))error("Maximum % characters",MAXKEYLEN-1);
		else strcpy((char *)Option.F7key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "F8");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F8key))error("Maximum % characters",MAXKEYLEN-1);
		else strcpy((char *)Option.F8key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "F9");
	if(tp) {
		char p[STRINGSIZE];
		strcpy(p,getCstring(tp));
		if(strlen(p)>=sizeof(Option.F9key))error("Maximum % characters",MAXKEYLEN-1);
		else strcpy((char *)Option.F9key, p);
		SaveOptions(1);
		return;
	}
    tp = checkstring(cmdline, "MILLISECONDS");
	if(tp) {
		if(checkstring(tp, "ON"))		{ Option.fulltime = true; return; }
		if(checkstring(tp, "OFF"))		{ Option.fulltime = false; return;  }
	}

    tp = checkstring(cmdline, "AUTORUN");
	if(tp) {
		if(checkstring(tp, "ON"))		{ Option.Autorun = true; SaveOptions(1); return; }
		if(checkstring(tp, "OFF"))		{ Option.Autorun = false; SaveOptions(1); return;  }
	}

#ifndef STM32F4version
	tp = checkstring(cmdline, "AUTOREFRESH");
	if(tp) {
        if(!((Option.DISPLAY_TYPE > SPI_PANEL && Option.DISPLAY_TYPE != USER )|| (Option.DISPLAY_TYPE==SSD1963_4_16)|| (Option.DISPLAY_TYPE==SSD1963_4)))error("Not valid for this display");
		if(checkstring(tp, "ON"))		{
			Option.Refresh = 1;
			Display_Refresh();
			return;
		}
		if(checkstring(tp, "OFF"))		{ Option.Refresh = 0; return; }
	}
#endif
	tp = checkstring(cmdline, "CASE");
	if(tp) {
		if(checkstring(tp, "LOWER"))	{ Option.Listcase = CONFIG_LOWER; SaveOptions(1); return; }
		if(checkstring(tp, "UPPER"))	{ Option.Listcase = CONFIG_UPPER; SaveOptions(1); return; }
		if(checkstring(tp, "TITLE"))	{ Option.Listcase = CONFIG_TITLE; SaveOptions(1); return; }
	}

    tp = checkstring(cmdline, "TAB");
	if(tp) {
		if(checkstring(tp, "2"))		{ Option.Tab = 2; SaveOptions(1); return; }
		if(checkstring(tp, "3"))		{ Option.Tab = 3; SaveOptions(1); return; }
		if(checkstring(tp, "4"))		{ Option.Tab = 4; SaveOptions(1); return; }
		if(checkstring(tp, "8"))		{ Option.Tab = 8; SaveOptions(1); return; }
	}
    tp = checkstring(cmdline, "BAUDRATE");
	if(tp) {
		//if(Option.SerialConDisabled!=0)error("Invalid with USB console");
    	if(CurrentLinePtr) error("Invalid in a program");
        int i;
		i = getinteger(tp);
        if(i > PeripheralBusSpeed/17) error("Baud rate too high");
        if(i < 100) error("Number out of bounds");
        Option.Baudrate = i;
        SaveOptions(1);
        MMPrintString("Restart to activate");                // set the console baud rate
		return;
	}
#ifdef STM32H743xx
    tp = checkstring(cmdline, "CPU SPEED");
	if(tp) {
    	if(CurrentLinePtr) error("Invalid in a program");
        int i;
		i = getint(tp,10,600);
        Option.CPUspeed = i;
        SaveOptions(1);
        MMPrintString("Restart to activate");                // set the console baud rate
		return;
	}
#endif
    tp = checkstring(cmdline, "VCC");
	if(tp) {
        MMFLOAT f;
		f = getnumber(tp);
        if(f > 3.6) error("VCC too high");
        if(f < 1.8) error("VCC too low");
        VCC=f;
		return;
	}

	tp = checkstring(cmdline, "PIN");
	if(tp) {
    	int i;
		i = getint(tp, 0, 99999999);
        Option.PIN = i;
        SaveOptions(1);
		return;
	}

	/*
    tp = checkstring(cmdline, "DISPLAY");
	if(tp) {
        getargs(&tp, 3, ",");
        if(Option.DISPLAY_CONSOLE) error("Cannot change LCD console");
		Option.Height = getint(argv[0], 5, 100);
        if(argc == 3) Option.Width = getint(argv[2], 37, 240);
        //setterminal();
        SaveOptions(1);
		return;
	}
	*/

	tp = checkstring(cmdline, "DISPLAY");
	if(tp) {
	    getargs(&tp, 3, (char *)",");
	    if(Option.DISPLAY_CONSOLE && argc>0 ) error("Cannot change LCD console");
	    if(argc >= 1) Option.Height = getint(argv[0], 5, 100);
	    if(argc == 3) Option.Width = getint(argv[2], 37, 240);
	    if (Option.DISPLAY_CONSOLE) {
	        setterminal((Option.Height > SCREENHEIGHT)?Option.Height:SCREENHEIGHT,(Option.Width > SCREENWIDTH)?Option.Width:SCREENWIDTH);                                                    // or height is > 24
        }else{
            setterminal(Option.Height,Option.Width);
        }
        if(argc >= 1 )SaveOptions(1);  //Only save if necessary
	    return;
	}
	tp = checkstring(cmdline, "LCDPANEL");
    if(tp) {
        getargs(&tp, 13, ",");
        if(str_equal(argv[0], "USER")) {
            if(Option.DISPLAY_TYPE) error("Display already configured");
            if(argc != 5) error("Argument count");
            HRes = DisplayHRes = getint(argv[2], 1, 10000);
            VRes = DisplayVRes = getint(argv[4], 1, 10000);
            Option.DISPLAY_TYPE = USER;
            // setup the pointers to the drawing primitives
            DrawRectangle = DrawRectangleUser;
            DrawBitmap = DrawBitmapUser;
            return;
        }
    }
    tp = checkstring(cmdline, "COLOURCODE");
    if(tp == NULL) tp = checkstring(cmdline, "COLORCODE");
	if(tp) {
		if(checkstring(tp, "ON"))		{ Option.ColourCode = true; SaveOptions(1); return; }
		if(checkstring(tp, "OFF"))		{ Option.ColourCode = false; SaveOptions(1); return;  }
	}

 /*
	tp = checkstring(cmdline, "SAVE");
	if(tp) {
	    if(!CurrentLinePtr) return;
           if(SaveOptions(1)) {
        	  // Restart if options have been saved
	           _excep_code = RESTART_DOAUTORUN;
	          // while(ConsoleTxBufTail != ConsoleTxBufHead);
	          // uSec(10000);
	           SoftReset();                            // this will restart the processor
	       }
	        return;
	}

 */	                                              // this will restart the processor

	OtherOptions();
}

#define STM32_UUID       ((uint8_t*)0x1FF1E800)
// board SN 48 bit
static inline uint64_t mix(uint64_t h)
{
    h ^= h >> 23;
    h *= 0x2127599bf4325c37ULL;
    h ^= h >> 47;
    //
    return h;
}

uint64_t fastHash64(const void * buf, size_t len, uint64_t seed)
{
    const uint64_t m = 0x880355f21e6d1965ULL;   //Always as unsigned long long
    const uint64_t * pos = (const uint64_t*)buf;
    const uint64_t * end = pos + (len / 8);
    const unsigned char * pos2;
    uint64_t h = seed ^ (len * m);
    uint64_t v;

    while(pos != end)
    {
        v  = *pos++;
        h ^= mix(v);
        h *= m;
    }

    pos2 = (const unsigned char*)pos;
    v = 0;

    switch(len & 7)
    {
        case 7: v ^= (uint64_t)pos2[6] << 48;
        case 6: v ^= (uint64_t)pos2[5] << 40;
        case 5: v ^= (uint64_t)pos2[4] << 32;
        case 4: v ^= (uint64_t)pos2[3] << 24;
        case 3: v ^= (uint64_t)pos2[2] << 16;
        case 2: v ^= (uint64_t)pos2[1] << 8;
        case 1: v ^= (uint64_t)pos2[0];
                h ^= mix(v);
                h *= m;
    }

    return mix(h);
}

int64_t getBoardSerial(void)
{
  uint64_t hash = fastHash64(STM32_UUID, 12, 1234554321) & 0xFFFFFFFFFFFF;
  return hash;
}

// function (which looks like a pre defined variable) to return the type of platform
void fun_device(void){
	sret = GetTempStrMemory();									// this will last for the life of the command
    strcpy(sret, "ARMmite H7");
    CtoM(sret);
    targ = T_STR;
}

void fun_info(void){
	char *tp;
	sret = GetTempStrMemory();									// this will last for the life of the command
	tp=checkstring(ep, "FONT POINTER");
	  if(tp){
		iret=(int64_t)((uint32_t)&FontTable[getint(tp,1,FONT_TABLE_SIZE)-1]);
		targ=T_INT;
		return;
	  }
    tp=checkstring(ep, "FONT ADDRESS");
	  if(tp){
		iret=(int64_t)((uint32_t)FontTable[getint(tp,1,FONT_TABLE_SIZE)-1]);
		targ=T_INT;
		return;
	  }

	  tp=checkstring(ep, "BACKUP");
	   if(tp){
	  	iret=0x38800000;
	  	targ=T_INT;
	  	return;
	   }


	tp=checkstring(ep, "OPTION");
	 if(tp){
        if(checkstring(tp, "AUTORUN")){
			if(Option.Autorun == false)strcpy(sret,"Off");
            else strcpy(sret,"On");
		} else if(checkstring(tp, "EXPLICIT")){
			if(OptionExplicit == false)strcpy(sret,"Off");
			else strcpy(sret,"On");

		} else if(checkstring(tp, "DEFAULT")){
			if(DefaultType == T_INT)strcpy(sret,"Integer");
			else if(DefaultType == T_NBR)strcpy(sret,"Float");
			else if(DefaultType == T_STR)strcpy(sret,"String");
			else strcpy(sret,"None");
		} else if(checkstring(tp, "BASE")){
			if(OptionBase==1)iret=1;
			else iret=0;
			targ=T_INT;
			return;
		} else if(checkstring(tp, "BREAK")){
			iret=BreakKey;
			targ=T_INT;
			return;
		} else if(checkstring(tp, "ANGLE")){
			if(optionangle==1.0)strcpy(sret,"RADIANS");
			else strcpy(sret,"DEGREES");
            CtoM(sret);
            targ=T_STR;
            return;
		} else if(checkstring(tp, "VCC")){
		    fret=VCC;
		    targ=T_NBR;
		    return;

		} else error("Syntax ");
		CtoM(sret);
	    targ=T_STR;
		return;
     }



    tp=checkstring(ep, "CALLTABLE");
      if(tp){
    	iret = (int64_t)(uint32_t)CallTable;
        targ = T_INT;
        return;
      }
    tp=checkstring(ep, "PROGRAM");
      if(tp){
        iret = (int64_t)(uint32_t)ProgMemory;
        targ = T_INT;
        return;
      }
    if((tp=checkstring(ep, "EXISTS DIR"))){
		char *p = getFstring(tp);
		targ=T_INT;
		iret=ExistsDir(p);
		return;
    }
	if((tp=checkstring(ep, "EXISTS FILE"))){
		char *p = getFstring(tp);
		iret=ExistsFile(p);
		if(iret==0){  //return -2 if its a directory
		  iret=ExistsDir(p);
		  if (iret==1)iret=-2;
		}
		targ=T_INT;
		return;
	}
	tp=checkstring(ep, "FILESIZE");
	 if(tp){
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
		static DIR djd;
		static FILINFO fnod;
		memset(&djd,0,sizeof(DIR));
		memset(&fnod,0,sizeof(FILINFO));
		char *p = getFstring(tp);

		ErrorCheck(0);
		FSerror = f_stat(p, &fnod);
		if(FSerror != FR_OK){ iret=-1; targ=T_INT; strcpy(MMErrMsg,FErrorMsg[4]); return;}
		if((fnod.fattrib & AM_DIR)){ iret=-2; targ=T_INT; strcpy(MMErrMsg,FErrorMsg[4]); return;}
		iret=fnod.fsize;
		targ=T_INT;
		return;
	 }
    if(checkstring(ep, "HEAP")){
         iret=FreeSpaceOnHeap();
         targ=T_INT;
         return;
    }
	tp=checkstring(ep, "MODIFIED");
	 if(tp){
		if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
	    static DIR djd;
	    static FILINFO fnod;
		memset(&djd,0,sizeof(DIR));
		memset(&fnod,0,sizeof(FILINFO));
		char *p = getFstring(tp);
		ErrorCheck(0);
		FSerror = f_stat(p, &fnod);
		if(FSerror != FR_OK){ iret=-1; targ=T_STR; strcpy(MMErrMsg,FErrorMsg[4]); return;}
//		if((fnod.fattrib & AM_DIR)){ iret=-2; targ=T_INT; strcpy(MMErrMsg,FErrorMsg[4]); return;}
	    IntToStr(sret , ((fnod.fdate>>9)&0x7F)+1980, 10);
	    sret[4] = '-'; IntToStrPad(sret + 5, (fnod.fdate>>5)&0xF, '0', 2, 10);
	    sret[7] = '-'; IntToStrPad(sret + 8, fnod.fdate&0x1F, '0', 2, 10);
	    sret[10] = ' ';
	    IntToStrPad(sret+11, (fnod.ftime>>11)&0x1F, '0', 2, 10);
	    sret[13] = ':'; IntToStrPad(sret + 14, (fnod.ftime>>5)&0x3F, '0', 2, 10);
	    sret[16] = ':'; IntToStrPad(sret + 17, (fnod.ftime&0x1F)*2, '0', 2, 10);
		CtoM(sret);
	    targ=T_STR;
		return;
	 }
/*
	 tp=checkstring(ep,"DIRECTORY");    //from CMM2
	 	if(tp){
	 		//strcpy(sret,filepath);
	 		//if(sret[strlen(sret)-1]!='/')strcat(sret,"/");
	 		strcpy(sret,"A");
	 		strcat(sret,GetCWD());
	 		if(sret[strlen(sret)-1]!='/')strcat(sret,"/");
	 		CtoM(sret);
	 	    targ=T_STR;
	 		return;
	 	}
*/

	 if (checkstring(ep, "LINE")) {
	    if (!CurrentLinePtr) {
	        strcpy(sret, "UNKNOWN");
	    } else if(CurrentLinePtr < ProgMemory + Option.ProgFlashSize) {
	    	sprintf(sret, "%d", CountLines(CurrentLinePtr));
	    } else {
	    	strcpy(sret, "LIBRARY");
	    }
	    CtoM(sret);
	    targ=T_STR;
	    return;
	 }

	 tp=checkstring(ep, "NBRPINS");
	  if(tp){
		if(HAS_144PINS)iret=144;
		if(HAS_100PINS)iret=100;
		targ=T_INT;
		return;
	  }

	tp=checkstring(ep, "CPUREVID");
	  if(tp){
		IntToStr(sret,HAL_GetREVID(),16);
		CtoM(sret);
		targ=T_STR;
		return;
	  }

	 tp=checkstring(ep, "PIN");
	  if(tp){
       int pin;
       char code;
       if((code=codecheck(tp)))tp+=2;
       pin = getinteger(tp);
	   if(code)pin=codemap(code, pin);
	   if(IsInvalidPin(pin))strcpy(sret,"Invalid");
	   else if(ExtCurrentConfig[pin] & CP_IGNORE_RESERVED)strcpy(sret,"Reserved");
	   else if(ExtCurrentConfig[pin]) strcpy(sret,"In Use");
       else strcpy(sret,"Unused");
       CtoM(sret);
       targ=T_STR;
	   return;
     }
/*
    tp=checkstring(ep, "PINNO");
    if(tp){
	    int pin;
	    char code;
	    if((code=codecheck(tp)))tp+=2;
	    // if(!(code=codecheck(tp)))tp+=2;
	    // else ("Syntax");
	    pin = getinteger(tp);
	    if(code)pin=codemap(code,pin);
	    if(IsInvalidPin(pin))error("Invalid pin");
	    iret=pin;
	    targ=T_INT;
	    return;
	}

*/
	     /*
	      tp=checkstring(ep, "PINNO");
	         if(tp){
	             int pin;
	             char code;
	             if((code=codecheck(tp)))tp+=2;
	             pin = getinteger(tp);
	             if(code)pin=codemap(code,pin);
	             if(IsInvalidPin(pin))error("Invalid pin");
	             iret=pin;
	             targ=T_INT;
	             return;
	      }
	      */

/*
	         tp=checkstring(ep, "PINNO");
	         if(tp){
	         	 int pin;
	           	 MMFLOAT f;
	           	 long long int i64;
	             char *ss;
	           	 int t=0;
	           	 char code, *ptr;
	           	 char *string=GetTempMemory(STRINGSIZE);
	           	 skipspace(tp);
	           	 evaluate(tp, &f, &i64, &ss, &t, false);
	             if(t & T_STR ){
	                ptr=(char *)getCstring(tp);
	                strcpy(string,ptr);
	             } else {
	                strcpy(string,(char *)tp);
	             }
	             if((code=codecheck( ( char *)string)))string+=2;
	             pin = getinteger((char *)string);
	             if(code)pin=codemap(code,pin);
	             if(IsInvalidPin(pin))error("Invalid pin");
	             targ=T_INT;
	             iret=pin;
	             return;
	         }

*/


     tp=checkstring(ep, "PINNO");
     if(tp){
     	 int pin;
       	 MMFLOAT f;
       	 long long int i64;
         char *ss;
       	 int t=0;
       	 char code, *ptr;
       	 char *string=GetTempMemory(STRINGSIZE);
       	 skipspace(tp);
       	 if((code=codecheck(tp))){
       		//MMPrintString("NONSTRING CODE OK");PRet();
       		tp+=2;
       	 	pin = getinteger(tp);
       	    if(code)pin=codemap(code,pin);
       	    if(IsInvalidPin(pin))error("Invalid pin");
       	    iret=pin;
       	    targ=T_INT;
            return;
       	 }else{
       		 //if starts with P error as invalid
       		 if (*tp=='P' || *tp=='p' ){error("Variable name can't begin with P or p");}
       	   	 evaluate(tp, &f, &i64, &ss, &t, false);
             if(t & T_STR ){
               ptr=(char *)getCstring(tp);
               strcpy(string,ptr);
               if(!(code=codecheck( (char *)string)))error("Invalid pin identifier");
            	  // MMPrintString("STRING CODE OK");PRet();
               string+=2;
               pin = getinteger((char *)string);
               if(code)pin=codemap(code,pin);
               if(IsInvalidPin(pin))error("Invalid pin");
               targ=T_INT;
               iret=pin;
               return;

             }else{
            	MMPrintString("NOT A STRING");PRet();
            	return;
             }

       	 }
     }

/*
    if((tp=checkstring(ep, (unsigned char *)"PINNO"))){
            int pin;
            MMFLOAT f;
            long long int i64;
            unsigned char *ss;
            int t=0;
            char code, *ptr;
            char *string=GetTempMemory(STRINGSIZE);
            skipspace(tp);
            if(codecheck(tp))evaluate(tp, &f, &i64, &ss, &t, false);
            if(t & T_STR ){
                ptr=(char *)getCstring(tp);
                strcpy(string,ptr);
            } else {
                strcpy(string,(char *)tp);
            }
            if(!(code=codecheck( (unsigned char *)string)))string+=2;
            else error("Syntax");
            pin = getinteger((unsigned char *)string);
            if(!code)pin=codemap(pin);
            if(IsInvalidPin(pin))error("Invalid pin");
            iret=pin;
            targ=T_INT;
            return;
*/
/*
     if((tp=checkstring(ep, (unsigned char *)"PINNO"))){
         int pin;
         MMFLOAT f;
         long long int i64;
         unsigned char *ss;
         int t=0;
         char code, *ptr;
         char *string=GetTempMemory(STRINGSIZE);
         skipspace(tp);

         if(codecheck(tp))evaluate(tp, &f, &i64, &ss, &t, false);
         if(t & T_STR ){
             ptr=(char *)getCstring(tp);
             strcpy(string,ptr);
         } else {
             strcpy(string,(char *)tp);
         }

         if(!(code=codecheck( (unsigned char *)string)))string+=2;
         else error("Syntax");
         pin = getinteger((unsigned char *)string);
         if(!code)pin=codemap(pin);

         if(IsInvalidPin(pin))error("Invalid pin");
         iret=pin;
         targ=T_INT;
         return;
*/
//	tp=checkstring(ep, "PIN");
//	  if(tp){
//		int pin;
//		//pin = getint(tp, 1, NBRPINS); //fromMM2
//		pin = getinteger(tp);
//		if(IsInvalidPin(pin))strcpy(sret,"Invalid");
//		else if(ExtCurrentConfig[pin] & CP_IGNORE_RESERVED)strcpy(sret,"Reserved");
//		else if(ExtCurrentConfig[pin]) strcpy(sret,"In Use");
//		else strcpy(sret,"Unused");
//	  } else {
		if(checkstring(ep, "CPUSPEED")){
			IntToStr(sret,SystemCoreClock,10);
		} else if(checkstring(ep, "AUTORUN")){
			if(Option.Autorun == false)strcpy(sret,"Off");
			else strcpy(sret,"On");
		} else if(checkstring(ep, "DEVICE")){
			fun_device();
			return;
		} else if(checkstring(ep, "DISK SIZE")){
			if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
			FATFS *fs;
			DWORD fre_clust;
			/* Get volume information and free clusters of drive 1 */
			f_getfree("0:", &fre_clust, &fs);
			/* Get total sectors and free sectors */
			iret= (uint64_t)(fs->n_fatent - 2) * (uint64_t)fs->csize *(uint64_t)FF_MAX_SS;
			targ=T_INT;
			return;
		 } else if(checkstring(ep, "FREE SPACE")){
			if(!InitSDCard()) error((char *)FErrorMsg[20]);					// setup the SD card
			FATFS *fs;
			DWORD fre_clust;
			/* Get volume information and free clusters of drive 1 */
			f_getfree("0:", &fre_clust, &fs);
			/* Get total sectors and free sectors */
			iret = (uint64_t)fre_clust * (uint64_t)fs->csize  *(uint64_t)FF_MAX_SS;
			targ=T_INT;
			return;

		 } else if(checkstring(ep, "FONTWIDTH")){
			fun_mmcharwidth();
			return;

		 } else if(checkstring(ep, "FONTHEIGHT")){
			fun_mmcharheight();
			return;

		 } else if(checkstring(ep, "ID")){  //Unique ID 12 bytes
	    	int i;
	        char id_out[25];
	        // Generate hex one nibble at a time
	        for (i = 0; i<24; i++) {
	           int nibble = (TM_ID_GetUnique8(i/2) >> (4 - 4*(i%2)) ) & 0xf;
	           id_out[i] = (char)(nibble < 10 ? nibble + '0' : nibble + 'A' - 10);
	        }
	        id_out[i] = 0;
	        strcpy(sret,id_out);

	     } else if(checkstring(ep, "ID48")){  //Unique ID  48 bits
	    	iret=(int64_t)getBoardSerial();
			targ=T_INT;
			return;

		 } else if(checkstring(ep, "HPOS")){
			iret = CurrentX;
			targ=T_INT;
			return;
		 } else if(checkstring(ep, "VPOS")){
			iret = CurrentY;
			targ=T_INT;
			return;

		 } else if(checkstring(ep, "ONEWIRE")){
			iret = mmOWvalue;
			targ=T_INT;
			return;
		 } else if(checkstring(ep, "VERSION")){
			fun_version();
			return;
		 } else if(checkstring(ep, "I2C")){
			iret = mmI2Cvalue;
			targ=T_INT;
			return;
		 } else if(checkstring(ep, "VARCNT")){
			iret=(int64_t)((uint32_t)varcnt);
			targ=T_INT;
			return;
	     } else if(checkstring(ep, "BOOT")){
	    	 if(_restart_reason == 0x0)strcpy(sret, "Power On");
	    	 else if(_restart_reason == 0x1)strcpy(sret, "Reset Switch");
	    	 else if(_restart_reason == 0x2)strcpy(sret, "MMBasic Reset");
	    	 else if(_restart_reason == 0x3)strcpy(sret, "CPU RESTART");
	    	 else if(_restart_reason == 0x4)strcpy(sret, "Watchdog");
	    	 else if(_restart_reason == 0x5)strcpy(sret, "EXECUTE Timeout");
	    	 else if(_restart_reason == 0x6)strcpy(sret, "HEAP Restart");
	    	 else strcpy(sret, "Unknown");

		 } else if(checkstring(ep, "ERRNO")){
			iret = MMerrno;
			targ=T_INT;
			return;
		 } else if(checkstring(ep, "ERRMSG")){
			strcpy(sret, MMErrMsg);
		 } else if(checkstring(ep, "KEYBOARD")){
			strcpy(sret,(keyboardseen? "Connected":"Not Connected"));
		 } else if(checkstring(ep, "LCDPANEL")){
			strcpy(sret,LCDList[(uint8_t)Option.DISPLAY_TYPE]);
		 } else if(checkstring(ep, "TOUCH")){
			if(Option.TOUCH_CS == false)strcpy(sret,"Disabled");
			else if(Option.TOUCH_XZERO == TOUCH_NOT_CALIBRATED)strcpy(sret,"Not calibrated");
			else strcpy(sret,"Ready");

		 } else if(checkstring(ep, "CONSOLE")){
		   	if(Option.DISPLAY_CONSOLE==false)strcpy(sret,"NOCONSOLE");
		   	else strcpy(sret,"CONSOLE");

		 } else if(checkstring(ep, "SDCARD")){
			if(Option.SDCARD_CS == false)strcpy(sret,"Disabled");
			else if(!Option.SD_CD) {
				int i=OptionFileErrorAbort;
				OptionFileErrorAbort=0;
				if(!InitSDCard())strcpy(sret,"Not present");
				else  strcpy(sret,"Ready");
				OptionFileErrorAbort=i;
			}
			else if((SDCardStat & (STA_NODISK | STA_NOINIT))==(STA_NODISK | STA_NOINIT)) strcpy(sret,"Not present");
			else if(!(SDCardStat & STA_NOINIT)) strcpy(sret,"Ready");
			else strcpy(sret,"Unused");
		 } else if(checkstring(ep, "TRACK")){
			if(CurrentlyPlaying == P_MP3 || CurrentlyPlaying == P_FLAC || CurrentlyPlaying == P_WAV) strcpy(sret,&alist[trackplaying].fn[1]);
			else strcpy(sret,"OFF");
		 } else if(checkstring(ep, "SOUND")){
			switch(CurrentlyPlaying){
			case P_NOTHING:strcpy(sret,"OFF");break;
			case P_PAUSE_TONE:
			case P_PAUSE_MP3:
			case P_PAUSE_WAV:
			case P_PAUSE_MOD:
			case P_PAUSE_SOUND:
			case P_PAUSE_FLAC:strcpy(sret,"PAUSED");break;
			case P_TONE:strcpy(sret,"TONE");break;
			case P_WAV:strcpy(sret,"WAV");break;
			case P_MP3:strcpy(sret,"MP3");break;
			case P_MOD:strcpy(sret,"MODFILE");break;
			case P_TTS:strcpy(sret,"TTS");break;
			case P_FLAC:strcpy(sret,"FLAC");break;
			case P_DAC:strcpy(sret,"DAC");break;
			case P_SOUND:strcpy(sret,"SOUND");break;
			case P_SYNC:break;
			}
		 } else error("Syntax - Not a valid item");

    CtoM(sret);
	targ=T_STR;
}

void cmd_watchdog(void) {
    int i;

    if(checkstring(cmdline, "OFF") != NULL) {
        WDTimer = 0;
    } else {
        i = getinteger(cmdline);
        if(i < 1) error("Invalid argument");
        WDTimer = i;
    }
}


void fun_restart(void) {
    iret = WatchdogSet;
    targ = T_INT;
}


//#define CPUSLEEP
void cmd_cpu(void) {
    char *p;
#ifdef CPUSLEEP
    int nsecs;  //GerryL SLEEP
#endif
//    while(!UARTTransmissionHasCompleted(UART1));                    // wait for the console UART to send whatever is in its buffer

    if((p = checkstring(cmdline, "RESTART"))) {
    	MMPrintString("\r\n");
        _excep_code = RESET_COMMAND;
    	while(ConsoleTxBufTail != ConsoleTxBufHead);
    	uSec(10000);
        SoftReset();                                                // this will restart the processor ? only works when not in debug
#ifdef CPUSLEEP
    }
    else if((p = checkstring(cmdline, "SLEEP"))) {
        skipspace(p);
        if(!(*p == 0 || *p =='\'')) {
            // it is: CPU SLEEP seconds
          getargs(&p, 3, ",");
          nsecs = getint(argv[0], 0, 0xffff);     // get the sleep time
          if(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, nsecs,RTC_WAKEUPCLOCK_CK_SPRE_16BITS) == HAL_TIMEOUT){
        	  Error_Handler();
          }
          __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG(); // here in case pin change and system reset & rerun
          HAL_PWR_EnterSTOPMode (PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
          __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();

         } else {
            // it is just CPU SLEEP, wakeup by pin going Lo to Hi
            // up to user to ensure outstanding tasks completed
            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
         }
          RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
          RCC_OscInitTypeDef RCC_OscInitStruct = {0};
          uint32_t pFLatency = 0;
          HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
          RCC_OscInitStruct. OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
          RCC_OscInitStruct.HSEState         = RCC_HSE_ON;
          RCC_OscInitStruct.PLL.PLLState     = RCC_PLL_ON;
          if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
          {
                   Error_Handler();
          }
          HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
          //RCC_ClkInitStruct. ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
          RCC_ClkInitStruct. ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK1 |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
          RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
          if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
          {
                   Error_Handler();
          }
#endif
    } else error("Syntax");
}

void cmd_csubinterrupt(void){
    getargs(&cmdline,1,",");
    if(argc != 0){
        if(checkstring(argv[0],"0")){
            CSubInterrupt = NULL;
            CSubComplete=0;
        } else {
            CSubInterrupt = GetIntAddress(argv[0]);
            CSubComplete=0;
            InterruptUsed = true;
        }
    } else CSubComplete=1;
}

void cmd_cfunction(void) {
    char *p, EndToken;
    if(cmdtoken == cmdCFUN)
            EndToken = GetCommandValue("End CFunction");                // this terminates a CFUNCTION
    else
      if(cmdtoken == cmdCSUB)
             EndToken = GetCommandValue("End CSub");                 // this terminates a CSUB
      else
             EndToken = GetCommandValue("End DefineFont");           // this terminates a DefineFont
    p = cmdline;
    while(*p != 0xff) {
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0) error("Missing END declaration");               // end of the program
        if(*p == T_NEWLINE) p++;                                    // skip over the newline token
        if(*p == T_LINENBR) p += 3;                                 // skip over the line number
        skipspace(p);
        if(*p == T_LABEL) {
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        if(*p == EndToken) {                                        // found an END token
            nextstmt = p;
            skipelement(nextstmt);
            return;
        }
        p++;
    }
}




// utility function used by cmd_poke() to validate an address
unsigned int GetPokeAddr(char *p) {
    unsigned int i;
    i = getinteger(p);
    if(!POKERANGE(i)) error("Address");
    return i;
}



void cmd_poke(void) {
    char *p;
    void *pp;

//#define POKE_DISPLAY
//#ifdef POKE_DISPLAY

    char *q;
    if((p = checkstring(cmdline, "DISPLAY"))){


        if(!Option.DISPLAY_TYPE)error("Display not configured");

        if((q=checkstring(p,"HRES"))){
            HRes=getint(q,0,1920);
        } else if((q=checkstring(p,"VRES"))){
            VRes=getint(q,0,1200);

        } else {
            getargs(&p,(MAX_ARG_COUNT * 2) - 3,",");
            if(!argc)return;

           	if((Option.DISPLAY_TYPE >= SSD1963_4 && Option.DISPLAY_TYPE <= SSD1963_8_16)|| (Option.DISPLAY_TYPE >= SSD1963_5_BUFF && Option.DISPLAY_TYPE <= SSD1963_8_8BIT)){
               	WriteSSD1963Command(getinteger(argv[0]));
                for(int i = 2; i < argc; i += 2) {
                   	WriteDataSSD1963(getinteger(argv[i]));
                }
                return;
           	} else if(Option.DISPLAY_TYPE==IPS_4_16){
           		WriteSSD1963Command(getinteger(argv[0]));
           	    for(int i = 2; i < argc; i += 2) {
           	        WriteDataSSD1963(getinteger(argv[i]));
           	    }
           	    return;
            } else if(Option.DISPLAY_TYPE>SSD_PANEL && Option.DISPLAY_TYPE<=SPI_PANEL_END && Option.DISPLAY_TYPE!=USER){
                spi_write_command(getinteger(argv[0]));
                for(int i = 2; i < argc; i += 2) {
                    spi_write_data(getinteger(argv[i]));
                }
                return;

            } else
               error("Display not supported");


        } //error("Syntax");

    } else {
//#endif

	getargs(&cmdline, 5, ",");
    if((p = checkstring(argv[0], "BYTE"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	uint8_t *padd=(uint8_t *)(a);
        *padd = getinteger(argv[2]);
        padd = (uint8_t *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if((p = checkstring(argv[0], "SHORT"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 2)error("Address not divisible by 2");
    	uint16_t *padd=(uint16_t *)(a);
        *padd = getinteger(argv[2]);
        padd = (uint16_t *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if((p = checkstring(argv[0], "WORD"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 4)error("Address not divisible by 4");
    	uint32_t *padd=(uint32_t *)(a);
        *padd = getinteger(argv[2]);
        padd = (uint32_t *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if((p = checkstring(argv[0], "INTEGER"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 8)error("Address not divisible by 8");
    	uint64_t *padd=(uint64_t *)(a);
    	*padd = getinteger(argv[2]);
        padd = (uint64_t *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }
    if((p = checkstring(argv[0], "FLOAT"))) {
    	if(argc != 3) error("Argument count");
    	uint32_t a=GetPokeAddr(p);
    	if(a % 8)error("Address not divisible by 8");
    	MMFLOAT *padd=(MMFLOAT *)(a);
    	*padd = getnumber(argv[2]);
        padd = (MMFLOAT *)((uint32_t)padd & 0xFFFFFFE0);
        SCB_CleanDCache_by_Addr((uint32_t *)padd, 32);
        return;
    }

    if(argc != 5) error("Argument count");

    if(checkstring(argv[0], "VARTBL")) {
        *((char *)vartbl + (unsigned int)getinteger(argv[2])) = getinteger(argv[4]);
        return;
    }
    if((p = checkstring(argv[0], "VAR"))) {
		pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    	*((char *)pp + (unsigned int)getinteger(argv[2])) = getinteger(argv[4]);
        return;
    }
    // the default is the old syntax of:   POKE hiaddr, loaddr, byte
    *(char *)(((int)getinteger(argv[0]) << 16) + (int)getinteger(argv[2])) = getinteger(argv[4]);
}
}



// function to find a CFunction
// only used by fun_peek() below
unsigned int GetCFunAddr(int *ip, int i) {
    while(*ip != 0xffffffff) {
        if(*ip++ == (unsigned int)subfun[i]) {                      // if we have a match
            ip++;                                                   // step over the size word
            i = *ip++;                                              // get the offset
            return (unsigned int)(ip + i);                          // return the entry point
        }
        ip += (*ip + 4) / sizeof(unsigned int);
    }
    return 0;
}




// utility function used by fun_peek() to validate an address
unsigned int GetPeekAddr(char *p) {
    unsigned int i;
    i = getinteger(p);
    if(!PEEKRANGE(i)) error("Address");
    return i;
}


// Will return a byte within the PIC32 virtual memory space.
void fun_peek(void) {
    char *p;
    void *pp;
	getargs(&ep, 3, ",");
    if((p = checkstring(argv[0], "BYTE"))){
        if(argc != 1) error("Syntax");
        iret = *(unsigned char *)GetPeekAddr(p);
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "VARADDR"))){
        if(argc != 1) error("Syntax");
        pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        iret = (unsigned int)pp;
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "VARHEADER"))){
        if(argc != 1) error("Syntax");
        pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        iret = (unsigned int)&vartbl[VarIndex];
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "CFUNADDR"))){
        if(argc != 1) error("Syntax");
        int i = FindSubFun(p, true);                                    // search for a function first
        if(i == -1) i = FindSubFun(p, false);                       // and if not found try for a subroutine
        //if(i == -1 || !(*subfun[i] == cmdCSUB)) error("Invalid argument");
        if(i == -1 || !(*subfun[i] == cmdCFUN || *subfun[i] == cmdCSUB)) error("Invalid argument");
        // search through program flash and the library looking for a match to the function being called
        int j = GetCFunAddr((int *)CFunctionFlash, i);
        //if(!j) error("Internal fault (sorry)");
        if(!j) j = GetCFunAddr((int *)CFunctionLibrary, i);    //Check the library
        if(!j) error("Internal fault 6(sorry)");
        iret = (unsigned int)j;                                     // return the entry point
        targ = T_INT;
        return;
    }

    if((p = checkstring(argv[0], "WORD"))){
        if(argc != 1) error("Syntax");
        iret = *(unsigned int *)(GetPeekAddr(p) & 0b11111111111111111111111111111100);
        targ = T_INT;
        return;
        }
    if((p = checkstring(argv[0], "SHORT"))){
        if(argc != 1) error("Syntax");
        iret = *(unsigned short *)(GetPeekAddr(p) & 0b11111111111111111111111111111110);
        targ = T_INT;
        return;
        }
    if((p = checkstring(argv[0], "INTEGER"))){
        if(argc != 1) error("Syntax");
        iret = *(uint64_t *)(GetPeekAddr(p) & 0xFFFFFFF8);
        targ = T_INT;
        return;
        }

    if((p = checkstring(argv[0], "FLOAT"))){
        if(argc != 1) error("Syntax");
        fret = *(MMFLOAT *)(GetPeekAddr(p) & 0xFFFFFFF8);
        targ = T_NBR;
        return;
        }

    if(argc != 3) error("Syntax");

    if((checkstring(argv[0], "PROGMEM"))){
        iret = *((char *)ProgMemory + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }

    if((checkstring(argv[0], "VARTBL"))){
        iret = *((char *)vartbl + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }

    if((p = checkstring(argv[0], "VAR"))){
		pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        iret = *((char *)pp + (int)getinteger(argv[2]));
        targ = T_INT;
        return;
    }

    // default action is the old syntax of  b = PEEK(hiaddr, loaddr)
	iret = *(char *)(((int)getinteger(argv[0]) << 16) + (int)getinteger(argv[2]));
    targ = T_INT;
}


// remove unnecessary text
void MIPS16 CrunchData(char **p, int c) {
    static char inquotes, lastch, incomment;

    if(c == '\n') c = '\r';                                         // CR is the end of line terminator
    if(c == 0  || c == '\r' ) {
        inquotes = false; incomment = false;                        // newline so reset our flags
        if(c) {
            if(lastch == '\r') return;                              // remove two newlines in a row (ie, empty lines)
            *((*p)++) = '\r';
        }
        lastch = '\r';
        return;
    }

    if(incomment) return;                                           // discard comments
    if(c == ' ' && lastch == '\r') return;                          // trim all spaces at the start of the line
    if(c == '"') inquotes = !inquotes;
    if(inquotes) {
        *((*p)++) = c;                                              // copy everything within quotes
        return;
    }
    if(c == '\'') {                                                 // skip everything following a comment
        incomment = true;
        return;
    }
    if(c == ' ' && (lastch == ' ' || lastch == ',')) {
        lastch = ' ';
        return;                                                     // remove more than one space or a space after a comma
    }
    *((*p)++) = lastch = c;
}


void MIPS16 cmd_autosave(void) {
    char *buf, *p;
    int c, prevc = 0, crunch = false,append = false;
    if(CurrentLinePtr) error("Invalid in a program");
    char *tp=(char *)checkstring(cmdline,( char *)"APPEND");
    if(tp){
        // ClearRuntime()  Only want to do some of this
    	ClearVars(0);
        CloseAudio(1);
        CloseAllFiles();
        ClearExternalIO();                                              // this MUST come before InitHeap()

       // p = buf = GetMemory(EDIT_BUFFER_SIZE);
        p = buf = GetTempMemory(EDIT_BUFFER_SIZE);
        char * fromp  = (char *)ProgMemory;
        p = buf;
        while(*fromp != 0xff) {
            if(*fromp == T_NEWLINE) {
                fromp = ( char *)llist(( char *)p, ( char *)fromp);                                // otherwise expand the line
                p += strlen((char *)p);
                *p++ = '\n'; *p = 0;
            }
            // finally, is it the end of the program?
            if(fromp[0] == 0 || fromp[0] == 0xff) break;
        }
        append=true;
        goto readin;
    }
    if(*cmdline) {
        if(toupper(*cmdline) == 'C')
            crunch = true;
        else
            error("Unknown command");
    }

    ClearProgram();                                            // clear any leftovers from the previous program
    p = buf = GetTempMemory(EDIT_BUFFER_SIZE);
    CrunchData(&p, 0);                                         // initialise the crunch data subroutine

readin:
    while((c = MMInkey()) != 0x1a && c!=F1 && c!=F2) {         // while waiting for the end of text char
        if(append){
        	append=false;
        	if( c == '\n') continue;                  // throw away an initial line feed which can follow the command
        }else{
       	  if(p == buf && c == '\n') continue;         // throw away an initial line feed which can follow the command
        }
        if((p - buf) >= EDIT_BUFFER_SIZE) error("Not enough memory");
        if(IsPrint(c) || c == '\r' || c == '\n' || c == TAB) {
            if(c == TAB) c = ' ';
            if(crunch)
                CrunchData(&p, c);                                  // insert into RAM after throwing away comments. etc
            else
                *p++ = c;                                           // insert the input into RAM
            {
                if(!(c == '\n' && prevc == '\r')) MMputchar(c);     // and echo it
                if(c == '\r') MMputchar('\n');
            }
            prevc = c;
        }
    }
    *p = 0;                                                         // terminate the string in RAM
    while(getConsole() != -1);                                      // clear any rubbish in the input
//    ClearSavedVars();                                               // clear any saved variables
    SaveProgramToFlash(buf, true);
    ClearVars(0);
    if(c==F2){
        //ClearVars(0);
        strcpy(inpbuf,"RUN\r\n");
       // multi=false;
        tokenise(true);                                             // turn into executable code
        ExecuteProgram(tknbuf);                                     // execute the line straight away
    }
}


/*

void MIPS16 cmd_autosave(void) {
    char *buf, *p;
    int c, prevc = 0, crunch = false;

    if(CurrentLinePtr) error("Invalid in a program");
    if(*cmdline) {
        if(toupper(*cmdline) == 'C')
            crunch = true;
        else
            error("Unknown command");
    }

    ClearProgram();                                                 // clear any leftovers from the previous program
    p = buf = GetTempMemory(EDIT_BUFFER_SIZE);
    CrunchData(&p, 0);                                              // initialise the crunch data subroutine
    //while((c = (getConsole() & 0x7f)) != 0x1a) {                    // while waiting for the end of text char
   	while((c = MMInkey()) != 0x1a && c!=F1 && c!=F2) {
    	if(p == buf && c == '\n') continue;                         // throw away an initial line feed which can follow the command
        if((p - buf) >= EDIT_BUFFER_SIZE) error("Not enough memory");
        if(IsPrint(c) || c == '\r' || c == '\n' || c == TAB) {
            if(c == TAB) c = ' ';
            if(crunch)
                CrunchData(&p, c);                                  // insert into RAM after throwing away comments. etc
            else
                *p++ = c;                                           // insert the input into RAM
            {
                if(!(c == '\n' && prevc == '\r')) MMputchar(c);     // and echo it
                if(c == '\r') MMputchar('\n');
            }
            prevc = c;
        }
    }
    *p = 0;                                                         // terminate the string in RAM
    while(getConsole() != -1);                                      // clear any rubbish in the input
    SaveProgramToFlash(buf, true);
    if(c==F2){
           ClearVars(0);
           strcpy(inpbuf,"RUN\r\n");
           tokenise(true);                                             // turn into executable code
           ExecuteProgram(tknbuf);                                     // execute the line straight away
    }

}
*/



/***********************************************************************************************
interrupt check

The priority of interrupts (highest to low) is:
PID
Touch (MM+ only)
CFunction Interrupt
ON KEY
I2C Slave Rx
I2C Slave Tx
COM1
COM2
COM3 (MM+ only)
COM4 (MM+ only)
GUI Int Down (MM+ only)
GUI Int Up (MM+ only)
WAV Finished (MM+ only)
IR Receive
Interrupt command/CSub Interrupt
I/O Pin Interrupts in order of definition
Tick Interrupts (1 to 4 in that order)

************************************************************************************************/

// check if an interrupt has occured and if so, set the next command to the interrupt routine
// will return true if interrupt detected or false if not
int check_interrupt(void) {
	int i, v;
	char *intaddr;
	static char rti[2];
    for(int i=1;i<=MAXPID;i++){
         if(PIDchannels[i].interrupt!=NULL && GetuSec()>PIDchannels[i].timenext && PIDchannels[i].active){
             PIDchannels[i].timenext=GetuSec()+(PIDchannels[i].PIDparams->T * 1000000);
             intaddr=(char *)PIDchannels[i].interrupt;
             goto GotAnInterrupt;
         }
    }

#if defined(MX470) || defined(STM32)
    ProcessTouch();
    CheckSDCard();
    processgps();
    if(CheckGuiFlag) CheckGui();                                    // This implements a LED flash
#endif

//	if(CFuncInt) CallCFuncInt();                                    // check if the CFunction wants to do anything (see CFunction.c)
    if(!InterruptUsed) return 0;                                    // quick exit if there are no interrupts set
	if(InterruptReturn != NULL || CurrentLinePtr == NULL) return 0;	// skip if we are in an interrupt or in immediate mode

    // check for an  ON KEY loc  interrupt
    if(OnKeyGOSUB && kbhitConsole()) {
        intaddr = OnKeyGOSUB;							            // set the next stmt to the interrupt location
		goto GotAnInterrupt;
    }

    if(KeyInterrupt != NULL && Keycomplete) {
   		Keycomplete=false;
   		intaddr = KeyInterrupt;									    // set the next stmt to the interrupt location
   		goto GotAnInterrupt;
   	}

//	if ((I2C_Status & I2C_Status_Interrupt) && (I2C_Status & I2C_Status_Completed)) {
//		I2C_Status &= ~I2C_Status_Completed;						// clear completed flag
//		intaddr = I2C_IntLine;										// set the next stmt to the interrupt location
//		goto GotAnInterrupt;
//	}

#ifdef INCLUDE_I2C_SLAVE

	if ((I2C_Status & I2C_Status_Slave_Receive_Rdy)) {
		I2C_Status &= ~I2C_Status_Slave_Receive_Rdy;	            // clear completed flag
		intaddr = I2C_Slave_Receive_IntLine;						// set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
	if ((I2C_Status & I2C_Status_Slave_Send_Rdy)) {
		I2C_Status &= ~I2C_Status_Slave_Send_Rdy;					// clear completed flag
		intaddr = I2C_Slave_Send_IntLine;							// set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
#endif

  // interrupt routines for the serial ports
  if(com1 || com2 || com3 || com4){ 	// interrupt routines for the serial ports
	if(com1_interrupt != NULL && SerialRxStatus(1) >= com1_ilevel) {// do we need to interrupt?
		intaddr = com1_interrupt;									// set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
	if(com1_TX_interrupt && com1_TX_complete){
		intaddr=com1_TX_interrupt;
		com1_TX_complete=false;
		goto GotAnInterrupt;
	}
	if(com2_interrupt != NULL && SerialRxStatus(2) >= com2_ilevel) {// do we need to interrupt?
		intaddr = com2_interrupt;									// set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
	if(com2_TX_interrupt && com2_TX_complete){
		intaddr=com2_TX_interrupt;
		com2_TX_complete=false;
		goto GotAnInterrupt;
	}
    if(com3_interrupt != NULL && SerialRxStatus(3) >= com3_ilevel) {// do we need to interrupt?
		intaddr = com3_interrupt;									// set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
	if(com3_TX_interrupt && com3_TX_complete){
		intaddr=com3_TX_interrupt;
		com3_TX_complete=false;
		goto GotAnInterrupt;
	}
	if(com4_interrupt != NULL && SerialRxStatus(4) >= com4_ilevel) {// do we need to interrupt?
		intaddr = com4_interrupt;									// set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}

	if(com4_TX_interrupt && com4_TX_complete){
		intaddr=com4_TX_interrupt;
		com4_TX_complete=false;
		goto GotAnInterrupt;
	}
  }

    if(gui_int_down && GuiIntDownVector) {                          // interrupt on pen down
    	intaddr = GuiIntDownVector;                                 // get a pointer to the interrupt routine
        gui_int_down = false;
		goto GotAnInterrupt;
	}

    if(gui_int_up && GuiIntUpVector) {
    	intaddr = GuiIntUpVector;                                   // get a pointer to the interrupt routine
        gui_int_up = false;
		goto GotAnInterrupt;
	}

    if(WAVInterrupt != NULL && WAVcomplete) {
        WAVcomplete=false;
		intaddr = WAVInterrupt;									    // set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
    if(COLLISIONInterrupt != NULL && CollisionFound) {
        CollisionFound=false;
		intaddr = COLLISIONInterrupt;									    // set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}
    if(ADCInterrupt != NULL && ADCcomplete){
 		ADCcomplete=false;
    	intaddr = ADCInterrupt;									    // set the next stmt to the interrupt location
    	//for(i=0;i<=ADCmax;i++){
    	//  a1float[i]=((MMFLOAT)(ADCscale[0]*a1point[i]) + ADCbottom[0] );
    	//  if(ADCchannelB)a2float[i]=((MMFLOAT)(ADCscale[1]*a2point[i]) + ADCbottom[1] );
    	//  if(ADCchannelC)a3float[i]=((MMFLOAT)(ADCscale[2]*a3point[i]) + ADCbottom[2] );
    	//}
    	goto GotAnInterrupt;
    }

    if(DACInterrupt != NULL && DACcomplete){
 		DACcomplete=false;
    	intaddr = (char *)DACInterrupt;									    // set the next stmt to the interrupt location
    	goto GotAnInterrupt;
    }

    if(IrGotMsg && IrInterrupt != NULL) {
        IrGotMsg = false;
		intaddr = IrInterrupt;									    // set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}

    if(KeypadInterrupt != NULL && KeypadCheck()) {
		intaddr = KeypadInterrupt;									// set the next stmt to the interrupt location
		goto GotAnInterrupt;
	}

    if(CSubInterrupt != NULL && CSubComplete) {
           	intaddr = CSubInterrupt;                                  // set the next stmt to the interrupt location
        	CSubComplete=0;
        	goto GotAnInterrupt;
    }


	for(i = 0; i < NBRINTERRUPTS; i++) {                            // scan through the interrupt table
		if(inttbl[i].pin != 0) {                                    // if this entry has an interrupt pin set
			v = ExtInp(inttbl[i].pin);								// get the current value of the pin
			// check if interrupt occured
			if((inttbl[i].lohi == T_HILO && v < inttbl[i].last) || (inttbl[i].lohi == T_LOHI && v > inttbl[i].last) || (inttbl[i].lohi == T_BOTH && v != inttbl[i].last)) {
				intaddr = inttbl[i].intp;							// set the next stmt to the interrupt location
				inttbl[i].last = v;									// save the new pin value
				goto GotAnInterrupt;
			} else
				inttbl[i].last = v;									// no interrupt, just update the pin value
		}
	}

	// check if one of the tick interrupts is enabled and if it has occured
	for(i = 0; i < NBRSETTICKS; i++) {
    	if(TickInt[i] != NULL && TickTimer[i] > TickPeriod[i]) {
    		// reset for the next tick but skip any ticks completely missed
    		while(TickTimer[i] > TickPeriod[i]) TickTimer[i] -= TickPeriod[i];
    		intaddr = TickInt[i];
    		goto GotAnInterrupt;
    	}
    }

    // if no interrupt was found then return having done nothing
	return 0;

    // an interrupt was found if we jumped to here
GotAnInterrupt:
    LocalIndex++;                                                   // IRETURN will decrement this
    if(OptionErrorSkip>0)SaveOptionErrorSkip=OptionErrorSkip;
    else SaveOptionErrorSkip = 0;
    OptionErrorSkip=0;
    strcpy( SaveMMErrMsg, MMErrMsg);   //save error message and clear
    *MMErrMsg=0;
    SaveMMerrno=MMerrno;              // saved MMerrno and clear
    MMerrno=0;
    InterruptReturn = nextstmt;                                     // for when IRETURN is executed
    // if the interrupt is pointing to a SUB token we need to call a subroutine
    if(*intaddr == cmdSUB) {
    	strncpy(CurrentInterruptName, intaddr + 1, MAXVARLEN);
        rti[0] = cmdIRET;                                           // setup a dummy IRETURN command
        rti[1] = 0;
        if(gosubindex >= MAXGOSUB) error("Too many SUBs for interrupt");
        errorstack[gosubindex] = CurrentLinePtr;
    	gosubstack[gosubindex++] = rti;                             // return from the subroutine to the dummy IRETURN command
        LocalIndex++;                                               // return from the subroutine will decrement LocalIndex
        skipelement(intaddr);                                       // point to the body of the subroutine
    }

    nextstmt = intaddr;                                             // the next command will be in the interrupt routine
    return 1;
}



// get the address for a MMBasic interrupt
// this will handle a line number, a label or a subroutine
// all areas of MMBasic that can generate an interrupt use this function
char *GetIntAddress(char *p) {
    int i;
	if(IsNamestart(*p)) {                                           // if it starts with a valid name char
    	i = FindSubFun(p, 0);                                       // try to find a matching subroutine
    	if(i == -1)
		    return findlabel(p);					                // if a subroutine was NOT found it must be a label
		else
		    return subfun[i];                                       // if a subroutine was found, return the address of the sub
	}

	return findline(getinteger(p), true);	                        // otherwise try for a line number
}
void *mymalloc(size_t size){
	static int32_t heaptop=0;
	unsigned int *a = malloc(size);
	if((uint32_t)a+size>heaptop)heaptop=(uint32_t)a+size;
	if(heaptop+0x100 > __get_MSP()) {
	    _excep_code = RESTART_HEAP;                            // otherwise do an automatic reset
		uSec(10000);
	    SoftReset();                                                // this will restart the processor
	}
	return a;
}
void fun_json(void){
    char *json_string=NULL;
    const cJSON *root = NULL;
    void *ptr1 = NULL;
    char *p;
	int64_t *dest=NULL;
    MMFLOAT tempd;
    int i,j,k,mode,index;
    char field[32],num[6];
    getargs(&ep, 3, ",");
    char *a=GetTempStrMemory();
    cJSON_Hooks myhooks;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_INT) {
    if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
        error("Argument 1 must be integer array");
    }
    dest = (long long int *)ptr1;
    json_string=(char *)&dest[1];
    } else error("Argument 1 must be integer array");
    myhooks.malloc_fn = mymalloc;
    myhooks.free_fn = free;
    cJSON_InitHooks(&myhooks);
    cJSON * parse = cJSON_Parse(json_string);
    if(parse==NULL)error("Invalid JSON data");
    root=parse;
    p=getCstring(argv[2]);
    int len = strlen(p);
    memset(field,0,32);
    memset(num,0,6);
    i=0;j=0;k=0;mode=0;
    while(i<len){
        if(p[i]=='['){ //start of index
            mode=1;
            field[j]=0;
            root = cJSON_GetObjectItemCaseSensitive(root, field);
            memset(field,0,32);
            j=0;
        }
        if(p[i]==']'){
            num[k]=0;
            index=atoi(num);
            root = cJSON_GetArrayItem(root, index);
            memset(num,0,6);
            k=0;
        }
        if(p[i]=='.'){ //new field separator
            if(mode==0){
                field[j]=0;
                root = cJSON_GetObjectItemCaseSensitive(root, field);
             memset(field,0,32);
                j=0;
            } else { //step past the dot after a close bracket
                mode=0;
            }
        } else  {
            if(mode==0)field[j++]=p[i];
            else if(p[i]!='[')num[k++]=p[i];
        }
        i++;
    }
    root = cJSON_GetObjectItem(root, field);

    if (cJSON_IsObject(root)){
        cJSON_Delete(parse);
        error("Not an item");
        return;
    }
    if (cJSON_IsInvalid(root)){
        cJSON_Delete(parse);
        error("Not an item");
        return;
    }
    if (cJSON_IsNumber(root))
    {
        tempd = root->valuedouble;

        if((MMFLOAT)((int64_t)tempd)==tempd) IntToStr(a,(int64_t)tempd,10);
        else FloatToStr(a, tempd, 0, STR_AUTO_PRECISION, ' ');   // set the string value to be saved
        cJSON_Delete(parse);
        sret=a;
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    if (cJSON_IsBool(root)){
        int64_t tempint;
        tempint=root->valueint;
        cJSON_Delete(parse);
        if(tempint)strcpy(sret,"true");
        else strcpy(sret,"false");
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    if (cJSON_IsString(root)){
        strcpy(a,root->valuestring);
        cJSON_Delete(parse);
        sret=a;
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
}
#include "diskio.h"
