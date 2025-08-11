/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

MM_Custom.c

Handles all the custom commands and functions in this implementation of MMBasic.  These are commands and functions
that are not normally part of MMbasic.  This is a good place to insert your own customised commands.

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

#include <stdio.h>

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
#include <math.h>
#include <complex.h>
MMFLOAT PI;
typedef MMFLOAT complex cplx;
#define max(x, y) (((x) > (y)) ? (x) : (y))


/*************************************************************************************************************************
**************************************************************************************************************************
IMPORTANT:
This module is empty and should be used for your special functions and commands.  In the standard distribution this file
will never be changed, so your code should be safe here.  You should avoid placing commands and functions in other files as
they may be changed and you would then need to re insert your changes in a new release of the source.

**************************************************************************************************************************
**************************************************************************************************************************/

/********************************************************************************************************************************************
 custom commands and functions
 each function is responsible for decoding a command
 all function names are in the form cmd_xxxx() (for a basic command) or fun_xxxx() (for a basic function) so, if you want to search for the
 function responsible for the NAME command look for cmd_name

 There are 4 items of information that are setup before the command is run.
 All these are globals.

 int cmdtoken	This is the token number of the command (some commands can handle multiple
				statement types and this helps them differentiate)

 char *cmdline	This is the command line terminated with a zero char and trimmed of leading
				spaces.  It may exist anywhere in memory (or even ROM).

 char *nextstmt	This is a pointer to the next statement to be executed.  The only thing a
				command can do with it is save it or change it to some other location.

 char *CurrentLinePtr  This is read only and is set to NULL if the command is in immediate mode.

 The only actions a command can do to change the program flow is to change nextstmt or
 execute longjmp(mark, 1) if it wants to abort the program.

 ********************************************************************************************************************************************/
/**
  * @brief  Sets the TIM Autoreload Register value on runtime without calling
  *         another time any Init function.
  * @param  __HANDLE__: TIM handle.
  * @param  __AUTORELOAD__: specifies the Counter register new value.
  * @retval None
  */
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim13;
//uint64_t *d1point=NULL, *d2point=NULL;
//int d1max, d2max;
int64_t *d1point=NULL, *d2point=NULL;
int d1max, d2max;
volatile int d1pos, d2pos;
//volatile uint64_t * volatile  a1point=NULL, * volatile a2point=NULL, *volatile  a3point=NULL;
volatile MMFLOAT * volatile a1float=NULL, * volatile a2float=NULL, * volatile a3float=NULL;

int64_t *a1point=NULL, *a2point=NULL, *a3point=NULL;
//MMFLOAT *a1float=NULL, *a2float=NULL, *a3float=NULL;
extern MMFLOAT ADCscale[3], ADCbottom[3];
MMFLOAT ADCscale[3], ADCbottom[3];
extern const MMFLOAT ADCdiv[];

int ADCmax=0;
int ADCScaled;
volatile int ADCpos=0;
extern int ADC_init(int32_t pin, int fast);
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
volatile int ADCchannelA=0;
volatile int ADCchannelB=0;
volatile int ADCchannelC=0;
int ADCtriggervalue=0;
int ADCtriggertimeout=0;
int ADCtriggerchannel=0;
int ADCnegativeslope=0;
char *ADCInterrupt;
volatile char *DACInterrupt;
int ADCNumchannels=0;
extern const MMFLOAT ADCdiv[];
int overrun;
volatile int ADCcomplete = false;
volatile int DACcomplete = false;
/**
  * @brief TIM13 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM13_Init(int prescale, int period)
{

  /* USER CODE BEGIN TIM13_Init 0 */

  /* USER CODE END TIM13_Init 0 */

  /* USER CODE BEGIN TIM13_Init 1 */

  /* USER CODE END TIM13_Init 1 */
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = prescale;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = period;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM13_Init 2 */
  if (HAL_TIM_Base_Start_IT(&htim13) != HAL_OK)
  {
    /* Starting Error */
	  error("HAL_TIM_Base_Start_IT");
  }
  /* USER CODE END TIM13_Init 2 */

}

static void MX_TIM6_Init(int prescale, int period)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = prescale;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = period;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    error("HAL_TIM_Base_Init");
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    error("HAL_TIMEx_MasterConfigSynchronization");
  }
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    /* Initialization Error */
    error("HAL_TIM_Base_Init");
  }

  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
  {
    /* Starting Error */
	  error("HAL_TIM_Base_Start_IT");
  }
}
static void MX_TIM7_Init(int prescale, int period)
{

  /* USER CODE BEGIN TIM6_Init 0 */
	HAL_TIM_Base_MspDeInit(&htim7);
  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = prescale;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = period;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    error("HAL_TIM_Base_Init");
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    error("HAL_TIMEx_MasterConfigSynchronization");
  }
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    /* Initialization Error */
    error("HAL_TIM_Base_Init");
  }

  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
  {
    /* Starting Error */
	  error("HAL_TIM_Base_Start_IT");
  }
}
MMFLOAT headingrange(MMFLOAT n){
	while(n<0)n+=360.0;
	while(n>=360.0)n-=360.0;
	return n;
}
void cmd_fasttick(char *in){
	getargs(&in, 3, ",");
	if(!(argc == 3)) error("Argument count");
    MMFLOAT freq=getnumber(argv[0])*2.0L;
	if(freq == 0) {
		TickInt[NBRSETTICKS] = NULL;										// turn off the interrupt
		HAL_TIM_Base_Stop(&htim13);
	} else {
		int prescale=1;
		int period=(int)(MMFLOAT)SystemCoreClock/2.0/freq/(MMFLOAT)(prescale+1);
		while(period>65535){
			prescale++;
			prescale*=2;
			prescale--;
			period=(int)(MMFLOAT)SystemCoreClock/2.0/freq/(MMFLOAT)(prescale+1);
		}
		prescale++;
		prescale*=2;
		prescale--;
		TickInt[NBRSETTICKS] = GetIntAddress(argv[2]);					    // get a pointer to the interrupt routine
		TickTimer[NBRSETTICKS] = 0;										    // set the timer running
		InterruptUsed = true;
		MX_TIM13_Init(prescale,period);
	}
}

#define maxH VRes
#define maxW HRes
/*
void cmd_turtle(void){
	char *tp;
    if(Option.DISPLAY_TYPE == 0) error("Display not configured");
	tp = checkstring(cmdline, "RESET");
	if(tp) {
		if(turtle_init_not_done)turtle_init();
		ClearScreen(gui_bcolour);
		turtle_reset();
		return;
	}
	tp = checkstring(cmdline, "DRAW TURTLE");
	if(tp) {
		if(turtle_init_not_done)turtle_init();
		turtle_draw_turtle();
		return;
	}
	tp = checkstring(cmdline, "PEN UP");
	if(tp) {
		if(turtle_init_not_done)turtle_init();
		turtle_pen_up();
		return;
	}
	tp = checkstring(cmdline, "PEN DOWN");
	if(tp) {
		if(turtle_init_not_done)turtle_init();
		turtle_pen_down();
		return;
	}
	tp = checkstring(cmdline, "DOT");
	if(tp) {
		if(turtle_init_not_done)turtle_init();
		turtle_dot();
		return;
	}
	tp = checkstring(cmdline, "BEGIN FILL");
	if(tp) {
		if(turtle_init_not_done)turtle_init();
		turtle_begin_fill();
		return;
	}
	tp = checkstring(cmdline, "END FILL");
	if(tp) {
		if(turtle_init_not_done)turtle_init();
		turtle_end_fill(main_turtle_poly_vertex_count);
		return;
	}
	tp = checkstring(cmdline, "FORWARD");
	if(tp) {
    	int n = getint(tp, -sqrt(maxW*maxW + maxH*maxH), sqrt(maxW*maxW + maxH*maxH));
		if(turtle_init_not_done)turtle_init();
		turtle_forward(n);
		return;
	}
	tp = checkstring(cmdline, "BACKWARD");
	if(tp) {
    	int n = getint(tp, -sqrt(maxW*maxW + maxH*maxH), sqrt(maxW*maxW + maxH*maxH));
		if(turtle_init_not_done)turtle_init();
		turtle_backward(n);
		return;
	}
	tp = checkstring(cmdline, "TURN LEFT");
	if(tp) {
    	MMFLOAT n = getnumber(tp);
    	n=headingrange(n);
		if(turtle_init_not_done)turtle_init();
		turtle_turn_right(n);
		return;
	}
	tp = checkstring(cmdline, "HEADING");
	if(tp) {
    	MMFLOAT n = getnumber(tp);
    	n=n-90;
    	n=headingrange(n);
		if(turtle_init_not_done)turtle_init();
		turtle_set_heading(n);
		return;
	}
	tp = checkstring(cmdline, "PEN COLOUR");
	if(tp) {
    	int n = getint(tp, 0, WHITE);
		if(turtle_init_not_done)turtle_init();
		turtle_set_pen_color((n>>16) & 0xFF, (n>>8) & 0xFF , n & 0xFF);
		return;
	}

	tp = checkstring(cmdline, "FILL COLOUR");
	if(tp) {
    	int n = getint(tp, 0, WHITE);
		if(turtle_init_not_done)turtle_init();
		turtle_set_fill_color((n>>16) & 0xFF, (n>>8) & 0xFF , n & 0xFF);
		return;
	}
	tp = checkstring(cmdline, "TURN RIGHT");
	if(tp) {
    	MMFLOAT n = getnumber(tp);
    	n=headingrange(n);
		if(turtle_init_not_done)turtle_init();
		turtle_turn_left(n);
		return;
	}
	tp = checkstring(cmdline, "MOVE");
	if(tp) {
        getargs(&tp, 3, ",");                              // this is a macro and must be the first executable stmt in a block
    	int x = getint(argv[0], 0,maxW);
    	int y = getint(argv[2], 0,maxH);
		if(turtle_init_not_done)turtle_init();
		turtle_goto(x, y);
		return;
	}
	tp = checkstring(cmdline, "DRAW PIXEL");
	if(tp) {
        getargs(&tp, 3, ",");                              // this is a macro and must be the first executable stmt in a block
    	int x = getint(argv[0], 0,maxW);
    	int y = getint(argv[2], 0,maxH);
 		if(turtle_init_not_done)turtle_init();
		turtle_draw_pixel(x, y);
		return;
	}
	tp = checkstring(cmdline, "FILL PIXEL");
	if(tp) {
        getargs(&tp, 3, ",");                              // this is a macro and must be the first executable stmt in a block
    	int x = getint(argv[0], 0,maxW);
    	int y = getint(argv[2], 0,maxH);
 		if(turtle_init_not_done)turtle_init();
		turtle_fill_pixel(x, y);
		return;
	}
	tp = checkstring(cmdline, "DRAW LINE");
	if(tp) {
        getargs(&tp, 7, ",");                              // this is a macro and must be the first executable stmt in a block
    	int x0 = getint(argv[0], 0,maxW);
    	int y0 = getint(argv[2], 0,maxH);
    	int x1 = getint(argv[4], 0,maxW);
    	int y1 = getint(argv[6], 0,maxH);
		if(turtle_init_not_done)turtle_init();
		turtle_draw_line(x0, y0, x1, y1);
		return;
	}
	tp = checkstring(cmdline, "DRAW CIRCLE");
	if(tp) {
        getargs(&tp, 5, ",");                              // this is a macro and must be the first executable stmt in a block
    	int x = getint(argv[0], 0,maxW);
    	int y = getint(argv[2], 0,maxH);
    	int r = getint(argv[4], 0,max(maxW, maxH));
		if(turtle_init_not_done)turtle_init();
		turtle_draw_circle(x, y, r);
		return;
	}
	error("Syntax");

}
*/

//#ifndef STM32F4version
void ADCclose(void){
	if(ADCchannelA){
		HAL_TIM_Base_Stop(&htim7);
		HAL_TIM_Base_DeInit(&htim7);
		if (HAL_ADC_DeInit(&hadc1) != HAL_OK)
		{
			// ADC de-initialization Error /
			error("HAL_ADC_DeInitA");
		}
		ExtCfg(ADCchannelA, EXT_NOT_CONFIG, 0);
		}
	if(ADCchannelB){
		if (HAL_ADC_DeInit(&hadc3) != HAL_OK)
		{
		    //* ADC de-initialization Error/
		    error("HAL_ADC_DeInitB");
		}
		ExtCfg(ADCchannelB, EXT_NOT_CONFIG, 0);
	}
	if(ADCchannelC){
		if (HAL_ADC_DeInit(&hadc2) != HAL_OK)
		{
		    //* ADC de-initialization Error/
		    error("HAL_ADC_DeInitC");
		}
		ExtCfg(ADCchannelC, EXT_NOT_CONFIG, 0);
	}
	ADCchannelA = ADCchannelB = ADCchannelC = ADCtriggerchannel = ADCNumchannels = ADCpos = ADCmax = ADCScaled=0;
	ADCInterrupt=NULL;
	ADCcomplete = false;
}


void cmd_ADC(void){
	char *tp;
    static int  prescale, period;
	ADC_ChannelConfTypeDef sConfigA, sConfigB, sConfigC;
	int ADCb,a,b,c;
    MMFLOAT freq=0.0;
   	tp = checkstring(cmdline, "OPEN");
	if(tp) {
     	getargs(&tp, 9, ",");
     	if(argc<3)error("Syntax");
		if(ADCchannelA)error("ADC already open");
        memset(&sConfigA,0,sizeof(ADC_ChannelConfTypeDef));
    	memset(&sConfigB,0,sizeof(ADC_ChannelConfTypeDef));
    	memset(&sConfigC,0,sizeof(ADC_ChannelConfTypeDef));
    	ADCchannelA=ADCchannelB = ADCchannelC = 0;
    	if(argc == 9) {
        	InterruptUsed = true;
        	ADCInterrupt = GetIntAddress(argv[8]);							// get the interrupt location
    	} else
    		ADCInterrupt = NULL;
        freq=getnumber(argv[0])*2.0L;
        ADCb=16;
        if(freq>1000000.0)error("Frequency greater than 500KHz");
        if(freq>960000.0 && SystemCoreClock==400000)error("Frequency greater than 480KHz");
        if(freq>20000.0)ADCb=14;
        if(freq>40000.0)ADCb=12;
        if(freq>80000.0)ADCb=10;
        if(freq>320000.0)ADCb=8;
    	prescale=1;
        period=(int)(MMFLOAT)SystemCoreClock/2.0/freq/(MMFLOAT)(prescale+1);
        while(period>65535){
        	prescale++;
        	prescale*=2;
        	prescale--;
            period=(int)(MMFLOAT)SystemCoreClock/2.0/freq/(MMFLOAT)(prescale+1);
        }
    	prescale++;
    	prescale*=2;
    	prescale--;

    	char code;
    	if((code=codecheck(argv[2])))argv[2]+=2;
    	ADCchannelA=getint(argv[2],0,NBRPINS);
    	if(code)ADCchannelA=codemap(code, ADCchannelA);

    	if(argc>3 && *argv[4]){
       	   if((code=codecheck(argv[4])))argv[4]+=2;
    	   ADCchannelB=getint(argv[4],0,NBRPINS);
    	   if(code)ADCchannelB=codemap(code, ADCchannelB);
    	}
       	if(argc>5 && *argv[6]){
       	  if((code=codecheck(argv[6])))argv[6]+=2;
    	  ADCchannelC=getint(argv[6],0,NBRPINS);
    	  if(code)ADCchannelC=codemap(code, ADCchannelC);
    	}

       	if(PinDef[ADCchannelA].ADC!=ADC1){
    	   ADCchannelA = ADCchannelB = ADCchannelC = 0;
    	   error("First channel must use ANALOG_A pin");
    	}
    	a=ADCchannelA;ADCchannelA=0;
    	CheckPin(a, CP_IGNORE_INUSE);
    	ADCchannelA=a;

        ADCNumchannels=1;
        if(ADCchannelB){
           	if(PinDef[ADCchannelB].ADC!=ADC3){
        	    ADCchannelA = ADCchannelB = ADCchannelC = 0;
        	    error("Second channel must use ANALOG_C pin");
        	}
            a=ADCchannelA;ADCchannelA=0;
            b=ADCchannelB;ADCchannelB=0;
           	CheckPin(b, CP_IGNORE_INUSE);
           	ADCchannelA=a;ADCchannelB=b;

            ADCNumchannels++;
        }
        if(ADCchannelC){
           	if(PinDef[ADCchannelC].ADC!=ADC2){
        	    ADCchannelA = ADCchannelB = ADCchannelC = 0;
        	    error("Third channel must use ANALOG_B pin");
        	}
            a=ADCchannelA;ADCchannelA=0;
            b=ADCchannelB;ADCchannelB=0;
            c=ADCchannelC;ADCchannelC=0;
            CheckPin(c, CP_IGNORE_INUSE);
            ADCchannelA=a;ADCchannelB=b;ADCchannelC=c;

            ADCNumchannels++;
        }
        ExtCfg(ADCchannelA, EXT_ANA_IN, ADCb);
        ExtCfg(ADCchannelA, EXT_COM_RESERVED, 0);
        ADC_init(ADCchannelA, 1);
    	sConfigA.Channel      = PinDef[ADCchannelA].ADCchannel;               /* Sampled channel number */
    	sConfigA.Rank         = ADC_REGULAR_RANK_1;         /* Rank of sampled channel number ADCx_CHANNEL */
    	sConfigA.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;   /* Sampling time (number of clock cycles unit) */
    	sConfigA.SingleDiff   = ADC_SINGLE_ENDED;           /* Single-ended input channel */
    	sConfigA.OffsetNumber = ADC_OFFSET_NONE;            /* No offset subtraction */
    	sConfigA.Offset = 0;                                /* Parameter discarded because offset correction is disabled */


    	if (HAL_ADC_ConfigChannel(&hadc1, &sConfigA) != HAL_OK)
    	{
    		/* Channel Configuration Error */
		    error("HAL_ADC_ConfigChannelA");
    	}
        if(ADCchannelB){
        	ExtCfg(ADCchannelB, EXT_ANA_IN, ADCb);
        	ExtCfg(ADCchannelB, EXT_COM_RESERVED, 0);
        	ADC_init(ADCchannelB ,1);
        	sConfigB.Channel      = PinDef[ADCchannelB].ADCchannel;                /* Sampled channel number */
        	sConfigB.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
        	sConfigB.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;    /* Sampling time (number of clock cycles unit) */
        	sConfigB.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
        	sConfigB.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
        	sConfigB.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


        	if (HAL_ADC_ConfigChannel(&hadc3, &sConfigB) != HAL_OK)
        	{
        	//	/* Channel Configuration Error */
      		    error("HAL_ADC_ConfigChannelB");
        	}
        }
        if(ADCchannelC){
        	ExtCfg(ADCchannelC, EXT_ANA_IN, ADCb);
        	ExtCfg(ADCchannelC, EXT_COM_RESERVED, 0);
        	ADC_init(ADCchannelC ,1);
        	sConfigC.Channel      = PinDef[ADCchannelC].ADCchannel;                /* Sampled channel number */
        	sConfigC.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
        	sConfigC.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;    /* Sampling time (number of clock cycles unit) */
        	sConfigC.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
        	sConfigC.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
        	sConfigC.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */


        	if (HAL_ADC_ConfigChannel(&hadc2, &sConfigC) != HAL_OK)
        	{
        		/* Channel Configuration Error */
      		    error("HAL_ADC_ConfigChannelC");
        	}
        }
    	return;
	}


	tp = checkstring(cmdline, "START");
	if(tp) {
     	//getargs(&tp, 5, ",");
     	getargs(&tp, 17, ",");
		if(!ADCchannelA)error("ADC not open");
        if(!(argc >= 1))error("Argument count");
        //a1point=NULL; a2point=NULL; a3point=NULL;
        a1float=NULL; a2float=NULL; a3float=NULL;
        ADCmax=0;
        ADCpos=0;
        ADCScaled=0;
        MMFLOAT top;

        ADCscale[0]= VCC/ADCdiv[ADCbits[ADCchannelA]];
        ADCscale[1]= VCC/ADCdiv[ADCbits[ADCchannelB]];
        ADCscale[2]= VCC/ADCdiv[ADCbits[ADCchannelC]];
        ADCbottom[0]=0;
        ADCbottom[1]=0;
        ADCbottom[2]=0;

        int card;
        ADCmax=parsefloatrarray(argv[0], (MMFLOAT **)&a1float, 1, 1, NULL, true)-1;
        a1point = (int64_t *)a1float;
        if(argc>=3 && *argv[2]){
           if(!ADCchannelB)error("Second channel not open");
           card=parsefloatrarray(argv[2], (MMFLOAT **)&a2float, 2, 1, NULL, true)-1;
           if(card!=ADCmax)error("Array size mismatch %,%",card, ADCmax);
           a2point = (int64_t *)a2float;
        }
        if(argc>=5 && *argv[4]){
            if(!ADCchannelC)error("Third channel not open");
            card=parsefloatrarray(argv[4], (MMFLOAT **)&a3float, 3, 1, NULL, true)-1;
            if(card!=ADCmax)error("Array size mismatch %,%",card, ADCmax);
            a3point = (int64_t *)a3float;
        }
        if(argc>=9){
            ADCbottom[0]=getnumber(argv[6]);
            top=getnumber(argv[8]);
            ADCscale[0]= (top-ADCbottom[0])/ADCdiv[ADCbits[ADCchannelA]];
            ADCScaled=1;
        }
        if(argc>=13){
            ADCbottom[1]=getnumber(argv[10]);
            top=getnumber(argv[12]);
            ADCscale[1]= (top-ADCbottom[1])/ADCdiv[ADCbits[ADCchannelB]];
        }
        if(argc>=17){
            ADCbottom[2]=getnumber(argv[14]);
            top=getnumber(argv[16]);
            ADCscale[2]= (top-ADCbottom[2])/ADCdiv[ADCbits[ADCchannelC]];
        }




		if (HAL_ADC_Start(&hadc1) != HAL_OK)
		{
			/* Start Conversation Error */
			error("HAL_ADC_StartA");
		}
		if(ADCchannelB){
			if (HAL_ADC_Start(&hadc3) != HAL_OK)
			{
				/* Start Conversation Error */
				error("HAL_ADC_StartB");
			}
		}
		if(ADCchannelC){
			if (HAL_ADC_Start(&hadc2) != HAL_OK)
			{
				/* Start Conversation Error */
				error("HAL_ADC_StartC");
			}
		}
		ADCcomplete=0;
		overrun=0;
		MX_TIM7_Init(prescale,period);
		if(ADCInterrupt==NULL){
    		while(!ADCcomplete){
    			CheckAbort();
    		    CheckSDCard();
    		}

 		}
    	return;
	}
	tp = checkstring(cmdline, "CLOSE");
	if(tp) {
		if(!ADCchannelA)error("ADC not open");
		ADCclose();
		return;
	}
	tp = checkstring(cmdline, "TRIGGER");
	if(tp) {
		MMFLOAT voltage;
     	getargs(&tp, 5, ",");
     	if(argc!=3 && argc!=5)error("Syntax");
     	ADCtriggerchannel=getint(argv[0],1,ADCNumchannels);
    	voltage = getnumber(argv[2]);
    	if(voltage<=-VCC || voltage >=VCC) error("Invalid Voltage");
		ADCnegativeslope=0;
    	if(voltage<0.0){
    		ADCnegativeslope=1;
    		voltage=-voltage;
    	}
		ADCtriggervalue=(int)(voltage/VCC*65535);
		//ADCtriggervalue=(int)(voltage/VCC*ADCdiv[ADCbits[ADCchannelA]]);
		if(argc==5 && *argv[4]){
		     ADCtriggertimeout = getnumber(argv[4]);
		}else{
		     ADCtriggertimeout =0;
		}
    	return;
	}
	tp = checkstring(cmdline, "FREQUENCY");
	if(tp) {
		int newbits;
     	getargs(&tp, 1, ",");
 		if(!ADCchannelA)error("ADC not open");
        freq=getnumber(argv[0])*2.0L;
        newbits=16;
        if(freq>1000000.0)error("Frequency greater than 500KHz");
        if(freq>960000.0 && SystemCoreClock==400000)error("Frequency greater than 480KHz");
        if(freq>20000.0)newbits=14;
        if(freq>40000.0)newbits=12;
        if(freq>80000.0)newbits=10;
        if(freq>320000.0)newbits=8;
		if(ADCbits[ADCchannelA]!=newbits)error("Invalid frequency change - use CLOSE then OPEN");
    	prescale=1;
        period=(int)(MMFLOAT)SystemCoreClock/2.0/freq/(MMFLOAT)(prescale+1);
        while(period>65535){
        	prescale++;
        	prescale*=2;
        	prescale--;
            period=(int)(MMFLOAT)SystemCoreClock/2.0/freq/(MMFLOAT)(prescale+1);
        }
    	prescale++;
    	prescale*=2;
    	prescale--;
    	return;
	}
}
//#endif
void dacclose(void){
	CurrentlyPlaying=P_NOTHING;
	if(d1max){
		HAL_TIM_Base_Stop(&htim6);
		HAL_TIM_Base_DeInit(&htim6);
	}
    HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2000);
    HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2000);
	d1max=0;
	d2max=0;
}

void cmd_DAC(void){
	char *tp;
    int channel;
    uint16_t dacvalue;
    float voltage;
    MMFLOAT freq;
   // void *ptr1 = NULL;
   // void *ptr2 = NULL;
    int  prescale, period;
    tp = checkstring(cmdline, "STOP");
    if(tp){
    	dacclose();
    	return;
    }
    tp = checkstring(cmdline, "START");
    if(tp){
     	getargs(&tp, 7, ",");
        if(!(CurrentlyPlaying == P_NOTHING || CurrentlyPlaying == P_DAC )) error("DAC in use");
        if(CurrentlyPlaying == P_DAC)dacclose();
        if(!(argc == 7 ||argc == 5 || argc == 3))error("Argument count");
        freq=getnumber(argv[0]);
        if(freq>500000) error("Frequency > 500KHz");
        d1max=0;
        d2max=0;
        d1pos=0;
        d2pos=0;
        d1max=parseintegerarray(argv[2],&d1point,1,1, NULL, false)-1;
               //PInt(d1max);
        if(argc==5){
            d2max=parseintegerarray(argv[4],&d2point,2,1, NULL, false)-1;
              // PInt(d2max);
        }
        /*
        ptr1 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if(vartbl[VarIndex].type & T_INT) {
            if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                error("Argument 2 must be integer array");
            }
            d1point = (uint64_t *)ptr1;
        } else error("Argument 2 must be integer array");
        d1max=(vartbl[VarIndex].dims[0] - OptionBase);
        if(argc>=5 && *argv[4]){
            ptr2 = findvar(argv[4], V_FIND | V_EMPTY_OK);
            if(vartbl[VarIndex].type & T_INT) {
                if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
                    error("Argument 3 must be integer array");
                }
                d2point = (uint64_t *)ptr2;
            } else error("Argument 3 must be integer array");
            d2max=(vartbl[VarIndex].dims[0] - OptionBase);
        }
        */
    	if(argc == 7) {
    		DACcomplete = false;
    		InterruptUsed = true;
        	DACInterrupt = GetIntAddress(argv[6]);							// get the interrupt location
    	} else
    		DACInterrupt = NULL;
        CurrentlyPlaying=P_DAC;
    	prescale=1;
        period=(int)(MMFLOAT)SystemCoreClock/2.0/freq/(MMFLOAT)(prescale+1);
        while(period>65535){
        	prescale++;
        	prescale*=2;
        	prescale--;
            period=(int)(MMFLOAT)SystemCoreClock/2.0/freq/(MMFLOAT)(prescale+1);
        }
    	MX_TIM6_Init(prescale,period);
    	return;
    }
	getargs(&cmdline, 9, ",");
	if((argc & 0x01) == 0 || argc < 3) error("Invalid syntax");

    channel = getint(argv[0], 1, 2);
	voltage = getnumber(argv[2]);
	if(voltage<0 || voltage >=VCC) error("Invalid Voltage");
	dacvalue=(uint16_t)(voltage/VCC*4096.0);
    switch(channel) {
        case 1:  {
        	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1, DAC_ALIGN_12B_R, dacvalue);
        	break;
        }
        case 2:  {
        	HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2, DAC_ALIGN_12B_R, dacvalue);
        	break;
        default: error("No such DAC");
        }
    }
}
