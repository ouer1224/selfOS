/* main.c                              Copyright NXP 2016
 * Description: Simple CAN 2.0 transmit / receive at 500 K baud
 *              for S32K144
 * 2016 Jul 22 S. Mihalik - Initial version
 * 2016 Sep 12 SM - Updated with SBC init, Node A - B communication
 * 2016 Oct 31 SM- Clocks adjusted for 160 MHz SPLL, updated loop logic
 * 2017 Jul 03 SM- Removed code for: MC33903 on obsolete EVB,
 *                 initial transmit for node B, tx_msg_count
 */

#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "FlexCAN.h"
#include "clocks_and_modes.h"

#include <stdint.h>
#include "task.h"


#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i" : : : "memory")
#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i" : : : "memory")



#define TASKA_STK_SIZE 1024
#define TASKB_STK_SIZE 1024
//static unsigned int taskA_Stk[TASKA_STK_SIZE];
static unsigned int taskB_Stk[TASKB_STK_SIZE];
static unsigned int taskC_Stk[TASKB_STK_SIZE];
static unsigned int taskD_Stk[TASKB_STK_SIZE];


uint32_t *pr_taskA=taskB_Stk+TASKA_STK_SIZE-1;
uint32_t count_sp=0;

static  volatile struct xtos_task_struct taskA;
static volatile struct xtos_task_struct taskB;

void task_switch() {
    if (gp_xtos_cur_task == &taskA)
        gp_xtos_next_task = &taskB;
    else
        gp_xtos_next_task = &taskA;

}




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


}
void Dlyms(int tick) {
    int i,j,k=0;

    for (i=0;i<tick;i++) {
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

//	Dlyms(4000);

}

void task_blink_green(void)
{

	PTD->PTOR |= 1<<16;

//	Dlyms(8000);

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


    S32_NVIC->ISER[(uint32_t)(122) >> 5U] = (uint32_t)(1UL << ((uint32_t)(122) & (uint32_t)0x1FU));


}

int self_add(void)
{
	static char tmp=0;
	char i=0;
	__asm volatile(
	"mov r1,%0\n"
	"add r1,r1,#1\n"
	"mov %0,r1\n"
	:"+r"(tmp)

	);

	return tmp;
}



void FTM3_Ovf_Reload_IRQHandler (void)
{

	FTM3->SC &= ~FTM_SC_TOF_MASK; //清除中断标志

if(gp_xtos_cur_task->saved==1)
{
	__asm volatile
	(
	"mrs r0,psp\n"
	
	"subs r0,r0,#0x04\n"
	"str r11,[r0]\n"


	"subs r0,r0,#0x04\n"
	"str r10,[r0]\n"

	"subs r0,r0,#0x04\n"
	"str r9,[r0]\n"

	"subs r0,r0,#0x04\n"
	"str r8,[r0]\n"

	"subs r0,r0,#0x04\n"
	"str r7,[r0]\n"

	"subs r0,r0,#0x04\n"
	"str r6,[r0]\n"


	"subs r0,r0,#0x04\n"
	"str r5,[r0]\n"

	"subs r0,r0,#0x04\n"
	"str r4,[r0]\n"

	);

	__asm volatile
	(

	"mov %0,r0\n"
	:"+r"(gp_xtos_cur_task->pTopOfStack)
	);

    if (gp_xtos_cur_task == &taskA)
    {
        gp_xtos_cur_task = &taskB;
    }
    else
    {
        gp_xtos_cur_task = &taskA;
    }
	if(taskA.pTopOfStack!=taskB.pTopOfStack)
	{
		count_sp++;
	}

}
else
{

	taskA.saved=1;
	taskB.saved=1;

}


#if 1
__asm volatile
(
"mov r0,%0\n"
:"+r"(gp_xtos_cur_task->pTopOfStack)
);


__asm volatile
(
	"ldr r4,[r0]\n"
	"add r0,r0,0x04\n"

	"ldr r5,[r0]\n"
	"add r0,r0,0x04\n"

	"ldr r6,[r0]\n"
	"add r0,r0,0x04\n"

//	"ldr r7,[r0]\n"
	"add r0,r0,0x04\n"


	"ldr r8,[r0]\n"
	"add r0,r0,0x04\n"


	"ldr r9,[r0]\n"
	"add r0,r0,0x04\n"


	"ldr r10,[r0]\n"
	"add r0,r0,0x04\n"


	"ldr r11,[r0]\n"
	"add r0,r0,0x04\n"


	"msr psp,r0\n"			//更新psp栈指针
//	"mov r7,r0\n"
	"orr lr,lr,#0x04\n"

);


__asm volatile
(
"nop\n"
);


#endif

	
}



void taska() {
    while (1) {

	
   task_blink_red();
	Dlyms(2000);

	

    }
}

void taskb() {
    while (1) {
        task_blink_green();
		Dlyms(2000);
    }
}

int test_asm(char x,char y,char z)
{
	char valx=0;
	char valy=0,valz=0;
	int buf[4];
	int *pr=buf;
	uint32_t val_pr;

	val_pr=(int)pr;
#if 1
		__asm volatile(
		"str r0,[%0]\n"
		"add %0,%0,0x04\n"

		"str r1,[%0]\n"
		"add %0,%0,0x04\n"

		"str r2,[%0]\n"
		"add %0,%0,0x04\n"


		:"+r"(pr)
		);
	
#endif

			return 0xff;
}


int main(void) 
{


	__asm volatile
	(
	"msr psp,%0\n"
	"mrs r0,control\n"
	"orr r0,r0,#0x02\n"
	"msr control,r0\n"
	:"+r"(pr_taskA)
	);


  DISABLE_INTERRUPTS();
  WDOG_disable();
  SOSC_init_8MHz();       /* Initialize system oscillator for 8 MHz xtal */
  SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
  NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
  FTM3_init_40MHZ();
  PORT_init();             /* Configure ports */
  

  


  xtos_create_task(&taskA, taska, &taskC_Stk[TASKA_STK_SIZE - 1]);  
  xtos_create_task(&taskB, taskb, &taskD_Stk[TASKA_STK_SIZE - 1]);  

  gp_xtos_cur_task = &taskA;
	
  ENABLE_INTERRUPTS();
  xtos_start();

	int i=0;
  
  for (;;) {                        /* Loop: if a msg is received, transmit a msg */


#if 1
 // i=test_asm(3,4,5);
	  i++;
	if(i!=0)
	{

	}
#endif

  }
}


















