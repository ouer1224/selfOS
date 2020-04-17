/* main.c                              Copyright NXP 2016




 */

#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "FlexCAN.h"
#include "clocks_and_modes.h"

#include <stdint.h>
#include "task.h"
#include "cortex_m4_register.h"



#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i" : : : "memory")
#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i" : : : "memory")

extern int OSTest(int);
extern void SysTick_Handler(void);

#define TASKA_STK_SIZE 1024
#define TASKB_STK_SIZE 1024



static unsigned int taskB_Stk[TASKB_STK_SIZE];
static unsigned int taskC_Stk[TASKB_STK_SIZE];
static unsigned int taskD_Stk[TASKB_STK_SIZE];
static unsigned int taskE_Stk[TASKB_STK_SIZE];



volatile unsigned int * gSCB_SHP14=(void *)0xE000ED22,*gSCB_ICSR=0xE000ED04;
unsigned int gPENDSV_PRI=0xFF;

uint32_t *pr_taskA=taskB_Stk+TASKA_STK_SIZE-1;


uint32_t count_sp=0;

uint32_t addr_psp=0;


volatile struct mdos_task_struct taskA;
volatile struct mdos_task_struct taskB;
volatile struct mdos_task_struct taskC;




void WDOG_disable (void){
  WDOG->CNT=0xD928C520; 	/* Unlock watchdog */
  WDOG->TOVAL=0x0000FFFF;	/* Maximum timeout value */
  WDOG->CS = 0x00002100;    /* Disable watchdog */
}

void PORT_init (void) {
  PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTE */
  PORTE->PCR[4] |= PORT_PCR_MUX(5); /* Port E4: MUX = ALT5, CAN0_RX */
  PORTE->PCR[5] |= PORT_PCR_MUX(5); /* Port E5: MUX = ALT5, CAN0_TX */
  PCC->PCCn[PCC_PORTD_INDEX ]|=PCC_PCCn_CGC_MASK;   /* Enable clock for PORTD */

  
  PORTD->PCR[15] =  0x00000100;     /* Port D16: MUX = GPIO (to green LED) */
  PTD->PDDR |= 1<<15;               /* Port D16: Data direction = output */

  PORTD->PCR[16] =  0x00000100;     /* Port D16: MUX = GPIO (to green LED) */
  PTD->PDDR |= 1<<16;               /* Port D16: Data direction = output */

  PORTD->PCR[0] =  0x00000100;     /* Port D16: MUX = GPIO (to green LED) */
  PTD->PDDR |= 1<<0;               /* Port D16: Data direction = output */



}
void Dlyms(int tick) {
    int i,j,k=0;

    for (i=0;i<2000;i++) {
        for (j=0;j<300;j++) {
            k++;
        }
    }
}

#define __SRAM_BASE_ADDR		0x1FFF8000
#define __SRAM_SIZE				0x0000F000

#define STACK_SIZE  0x400
void CleanRAM(void)
{
    uint32_t cnt;
    uint32_t *pRam;

    /*
     * clean system SRAM
     */
    cnt = (__SRAM_SIZE - STACK_SIZE)/4;
    pRam = (uint32_t*)__SRAM_BASE_ADDR;

    while(cnt--)
        *pRam++ = 0;

}

void task_blink_red(void)
{

	PTD->PTOR |= 1<<15;

}

void task_blink_green(void)
{

	PTD->PTOR |= 1<<16;

}
void task_blink_blue(void)
{

	PTD->PTOR |= 1<<0;

}



#define FTM_IN_CLOCK	(0x01)
void FTM3_init_40MHZ(void)
{
		//此处使用的是sys_clock,80M.
	PCC->PCCn[PCC_FTM3_INDEX] &= ~PCC_PCCn_CGC_MASK; //禁止FTM3时钟
	PCC->PCCn[PCC_FTM3_INDEX] |= PCC_PCCn_PCS(0) | PCC_PCCn_CGC_MASK;//选择了系统时钟80M,不使用外部时钟

	FTM3->MODE |= FTM_MODE_WPDIS_MASK; //写保护禁用
	FTM3->MODE |= FTM_MODE_FTMEN_MASK; //FTM使能 开始写寄存器
	FTM3->SC = 0;//清除状态寄存器
	FTM3->SC |= FTM_SC_TOIE_MASK | FTM_SC_PS(0b111);//  计数器溢出中断 分频因子选择 111b Divide by 128
	FTM3->SC &= ~FTM_SC_TOF_MASK;//清除计数器溢出标志

	FTM3->COMBINE = 0x00000000;//DECAPENx, MCOMBINEx, COMBINEx=0
	FTM3->POL = 0x00000000; //设置通道输出的极性
	FTM3->CNTIN = 0;
	FTM3->MOD = 625-1;//2560 - 1; //设置计数器终止值 配置中断时间  10ms
	/* FTM3 Period = MOD-CNTIN+0x0001 ~= 2 ctr clks  */

	FTM3->SC |= FTM_SC_CLKS(FTM_IN_CLOCK);//选择时钟源

 	pr_NVIC_ISER[122/32]=0x01<<(122%32);


}


void HardFault_Handler(void)
{
	static char conti=1;
	while(conti);
}


void FTM3_Ovf_Reload_IRQHandler (void)
{
	FTM3->SC &= ~FTM_SC_TOF_MASK; //清除中断标志
	OS_sys_count_add();
	OS_readyToSwitch();
}



void taska(void) 
{
	while (1) 
	{
		task_blink_red();
		//Dlyms(2000);
		OStaskDelay(2000);
	}

}

void taskb(void) 
{
    while (1) 
	{
		//Dlyms(3000);
		OStaskDelay(3000);
		task_blink_green();
		
    }
}
void taskc(void) 
{
    while (1) 
	{
		//Dlyms(4000);
		OStaskDelay(4000);
		task_blink_blue();
		//Dlyms(1000);
		OStaskDelay(1000);
    }
}




int main(void) 
{

	DISABLE_INTERRUPTS();


	WDOG_disable();
	SOSC_init_8MHz();       /* Initialize system oscillator for 8 MHz xtal */
	SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	FTM3_init_40MHZ();
	PORT_init();             /* Configure ports */



	mdos_create_task(&taskA, taska, &taskC_Stk[TASKA_STK_SIZE - 1]);  
	mdos_create_task(&taskB, taskb, &taskD_Stk[TASKA_STK_SIZE - 1]);  
	mdos_create_task(&taskC, taskc, &taskE_Stk[TASKA_STK_SIZE - 1]);



  ENABLE_INTERRUPTS();
  mdos_start();


  int i=0;

  
  for (;;) {                        /* Loop: if a msg is received, transmit a msg */

	i=OSTest(i);
	if(i!=0)
	{

	}

  }
}


















