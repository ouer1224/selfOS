/* main.c                              Copyright NXP 2016




 */

#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "FlexCAN.h"
#include "clocks_and_modes.h"

#include <stdint.h>
#include "task.h"
#include "cortex_m4_register.h"
#include "null.h"

#include "mem_manage.h"
#include "queue.h"
#include "sem.h"



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


volatile struct selfos_task_struct taskA;
volatile struct selfos_task_struct taskB;
volatile struct selfos_task_struct taskC;



/*---mem pool---*/
#define LEN_TASKA_MEM	12
#define DEEP_TASKA_MEM	15

static mem_pool smem_test;
static uint8_t buf_mem[LEN_TASKA_MEM*DEEP_TASKA_MEM];



/*----queue---*/
#define DEEP_QUEUE_TASKB	12
static uint32_t queue_mem_taskb[DEEP_QUEUE_TASKB];
QueueCB queue_taskb;

/*---sem---*/

SemCB sem_prevent_taskc;





/*-----函数---*/
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
	
	Sys_readyToSwitch();

}



void taska(void) 
{
	uint32_t rc=0;
	uint32_t count=0;
	uint32_t i=0;
	uint8_t buf_a[LEN_TASKA_MEM+4];
	uint8_t *pr_send=NULL;
	TaskDelay(500);

	rc=creat_mem_pool(&smem_test,buf_mem,LEN_TASKA_MEM,DEEP_TASKA_MEM);
	if(rc!=os_true)
	{
		while(1);
	}	
	while (1) 
	{
		TaskDelay(1000);
		task_blink_red();
		for(i=0;i<LEN_TASKA_MEM;i++)
		{
			*((uint8_t *)buf_a+i)=i+count;
		}
		count++;
#if 1
		pr_send=get_mem_from_pool(&smem_test,LEN_TASKA_MEM);
		if(pr_send!=NULL)
		{
			os_memcpy(pr_send,buf_a,LEN_TASKA_MEM);
			rc=put_dat_to_queue(&queue_taskb,pr_send,2000,0);
			if(rc!=os_true)
			{
				;
			}
		}
#endif

	}

}

void taskb(void) 
{
	uint8_t buf_b[24];
	uint32_t rc=0;
	uint8_t *pr_rcv=NULL;
	TaskDelay(1000);	
    while (1) 
	{
#if 1
		rc=get_dat_from_queue(&queue_taskb, &pr_rcv,3000, 0);
		if(rc==os_true)
		{
			os_memcpy(buf_b,pr_rcv,LEN_TASKA_MEM);
			free_mem_to_pool(&pr_rcv);
			
			task_blink_green();

			sem_release(&sem_prevent_taskc);
		}	
#else

	TaskDelay((uint32_t)(-1));

#endif


    }
}
void taskc(void) 
{

	uint32_t rc=0;

	TaskDelay(1500);	
    while (1) 
	{
		TaskDelay(200);
#if 1
		rc=sem_acquire(&sem_prevent_taskc,4000);
		if(rc==os_true)
		{
			task_blink_blue();
		}
#else
		task_blink_blue();
		TaskDelay((uint32_t)(-1));


#endif

		
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



	selfos_create_task(&taskA, taska, &taskC_Stk[TASKA_STK_SIZE - 1],4);  
	selfos_create_task(&taskB, taskb, &taskD_Stk[TASKA_STK_SIZE - 1],1);  
	selfos_create_task(&taskC, taskc, &taskE_Stk[TASKA_STK_SIZE - 1],3);


	/*创建队列*/
	queue_creat(&queue_taskb,queue_mem_taskb,DEEP_QUEUE_TASKB);


	/*创建信号量*/
	sem_creat(&sem_prevent_taskc,1, 1);




  ENABLE_INTERRUPTS();
  selfos_start();


  int i=0;

  
  for (;;) {                        /* Loop: if a msg is received, transmit a msg */

	i=OSTest(i);
	if(i!=0)
	{

	}

  }
}


















