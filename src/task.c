


/*mdos的含义是module os,可以通过修改接口的方式,对调度算法,内存管理,消息的传递*/



#include "task.h"
#include "null.h"
#include "cortex_m4_register.h"



volatile struct  mdos_task_struct  *gp_mdos_cur_task=NULL;
volatile struct  mdos_task_struct  *gp_mdos_next_task=NULL;

volatile uint32_t gOS_sys_time=0;



#define SIZE_STACK_TASK_IDLE	48
volatile struct mdos_task_struct task_idle;
static unsigned int task_idle_Stk[SIZE_STACK_TASK_IDLE];



#define head_asm_frame		__asm volatile(
#define tail_asm_frame		);






uint8_t OS_readyToSwitch(void)
{	
	SCB_ICSR |= 0x01<<28;

	return 1;
}

/*比如设置无限等待时的处理措施*/
void OStaskDelay(uint32_t dly)
{

	gp_mdos_cur_task->state=OS_SUSPEND;
	gp_mdos_cur_task->wake_time=get_OS_sys_count()+dly;	//需要判断计数溢出的问题
	OS_readyToSwitch();
	while(gp_mdos_cur_task->state==OS_SUSPEND);
}

void TaskDelay(uint32_t dly)
{

	OStaskDelay(dly);

}


void taskidle(void)
{

    while(1){
    	__asm
		("nop\n");
    }

}



void mdos_start (void)
{

	//添加idle任务
	mdos_create_task(&task_idle,taskidle,&task_idle_Stk[SIZE_STACK_TASK_IDLE - 1]);

#if 0		//systick
	SCB_SHPR3=0xff<<16;

	SYST_RVR=160000;
	SYST_CVR=0;

	SYST_CSR=(0x01<<2)|(0x01<<1);
	SYST_CSR=SYST_CSR&(~(0x01<<0));

	SCB_SHPR3=0x01<<24;

	SYST_CSR=SYST_CSR|0x01;

#else
	//pendsv
	SCB_SHPR3=0xff<<16;


#endif

	__asm volatile	//将psp设置为0,表示是第一次切换上下文.
	(
	"mov r0,#0\n"
	"msr psp,r0\n"
	"isb\n"
	);

}


void mdos_distroy_task() {

    while(1){
    	__asm
		("nop\n");
    }
}


void mdos_create_task(struct mdos_task_struct * tcb, mdos_task task, uint32_t * stk)
{
	uint32_t  *pstk;
    pstk = stk;
    pstk = (uint32_t *)((uint32_t)(pstk) & 0xFFFFFFF8uL);

    *(--pstk) = (uint32_t)0x00000000L; //No Name Register
    *(--pstk) = (uint32_t)0x00001000L; //FPSCR
    *(--pstk) = (uint32_t)0x00000015L; //s15
    *(--pstk) = (uint32_t)0x00000014L; //s14
    *(--pstk) = (uint32_t)0x00000013L; //s13
    *(--pstk) = (uint32_t)0x00000012L; //s12
    *(--pstk) = (uint32_t)0x00000011L; //s11
    *(--pstk) = (uint32_t)0x00000010L; //s10
    *(--pstk) = (uint32_t)0x00000009L; //s9
    *(--pstk) = (uint32_t)0x00000008L; //s8
    *(--pstk) = (uint32_t)0x00000007L; //s7
    *(--pstk) = (uint32_t)0x00000006L; //s6
    *(--pstk) = (uint32_t)0x00000005L; //s5
    *(--pstk) = (uint32_t)0x00000004L; //s4
    *(--pstk) = (uint32_t)0x00000003L; //s3
    *(--pstk) = (uint32_t)0x00000002L; //s2
    *(--pstk) = (uint32_t)0x00000001L; //s1
    *(--pstk) = (uint32_t)0x00000000L; //s0


    *(--pstk) = (uint32_t)0x01000000uL; // xPSR
    *(--pstk) = (uint32_t)task;         // Entry Point
    *(--pstk) = (uint32_t)mdos_distroy_task;; // R14 (LR)
    *(--pstk) = (uint32_t)12121212; // R12
    *(--pstk) = (uint32_t)03030303; // R3
    *(--pstk) = (uint32_t)02020202; // R2
    *(--pstk) = (uint32_t)01010101; // R1
    *(--pstk) = (uint32_t)0x00000000;  // R0

    *(--pstk) = (uint32_t)0x11111111uL; // R11
    *(--pstk) = (uint32_t)0x10101010uL; // R10
    *(--pstk) = (uint32_t)0x09090909uL; // R9
    *(--pstk) = (uint32_t)0x08080808uL; // R8
    *(--pstk) = (uint32_t)0x07070707uL; // R7
    *(--pstk) = (uint32_t)0x06060606uL; // R6
    *(--pstk) = (uint32_t)0x05050505uL; // R5
    *(--pstk) = (uint32_t)0x04040404uL; // R4

    tcb->pTopOfStack = pstk;
	tcb->state=OS_RUN;
	tcb->wake_time=0;


#if 0
	if(gp_mdos_cur_task==NULL)
	{
		gp_mdos_cur_task=tcb;
		gp_mdos_cur_task->next=tcb;
	}
	else
	{
		tcb->next=gp_mdos_cur_task->next;
		gp_mdos_cur_task->next=tcb;
		gp_mdos_cur_task=tcb;
	}
#else
	if(gp_mdos_cur_task==NULL)
	{
		gp_mdos_cur_task=tcb;
		gp_mdos_cur_task->link.pre=&(gp_mdos_cur_task->link);
		gp_mdos_cur_task->link.next=&(gp_mdos_cur_task->link);

	}
	else
	{
		list_add_behind(&(tcb->link),&(gp_mdos_cur_task->link));
		gp_mdos_cur_task=tcb;
	}

#endif	
}

/*获取下一个任务的tcb指针*/
void get_next_TCB(void)
{
	do
	{

	
		gp_mdos_cur_task=container_of(gp_mdos_cur_task->link.next,struct mdos_task_struct,link);
	
//		gp_mdos_cur_task=gp_mdos_cur_task->next;
		if(gp_mdos_cur_task->state==OS_SUSPEND)
		{
			if(gp_mdos_cur_task->wake_time<=get_OS_sys_count())
			{
				gp_mdos_cur_task->state=OS_RUN;
			}
		}
	}while(gp_mdos_cur_task->state!=OS_RUN);
}






