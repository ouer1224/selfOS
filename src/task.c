


/*selfos的含义是兴趣使然的os*/

#include "null.h"
#include "cortex_m4_register.h"
#include "link_list.h"
#include "task.h"








volatile struct  selfos_task_struct  *gp_selfos_cur_task=NULL;
volatile struct  selfos_task_struct  *gp_selfos_next_task=NULL;

volatile uint32_t gOS_sys_time=0;



static struct __link_list shead_suspend_link={NULL,NULL};//该链表暂时无用
static struct __link_list shead_sleep_link={NULL,NULL};//存放需要休眠的任务

static struct __link_list *spr_tail_spd_link=&shead_suspend_link;




/*idle任务创建的环境*/
#define SIZE_STACK_TASK_IDLE	48
volatile struct selfos_task_struct task_idle;
static unsigned int task_idle_Stk[SIZE_STACK_TASK_IDLE];




#define head_asm_frame		__asm volatile(
#define tail_asm_frame		);




uint8_t OS_readyToSwitch(void)
{	
	SCB_ICSR |= 0x01<<28;

	return 1;
}

/*比如设置无限等待时的处理措施*/
/*无限等待的功能,可以听过在tcb中添加一个无限等待的标志去实现.*/
void OStaskDelay(uint32_t dly)
{

	if(dly<=0)
	{
		return;
	}

	input_critical_area();
	gp_selfos_cur_task->state=OS_SUSPEND;
	gp_selfos_cur_task->wake_time=get_OS_sys_count()+dly;	//需要判断计数溢出的问题
	exit_critical_area();
	
	OS_readyToSwitch();
	while(gp_selfos_cur_task->state==OS_SUSPEND);
	
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



void selfos_start (void)
{

	//添加idle任务
	selfos_create_task(&task_idle,taskidle,&task_idle_Stk[SIZE_STACK_TASK_IDLE - 1]);

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


void selfos_distroy_task() {

    while(1){
    	__asm
		("nop\n");
    }
}


void selfos_create_task(struct selfos_task_struct * tcb, selfos_task task, uint32_t * stk)
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
    *(--pstk) = (uint32_t)selfos_distroy_task;; // R14 (LR)
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


	/*将任务添加到链表中*/
	if(gp_selfos_cur_task==NULL)
	{
		gp_selfos_cur_task=tcb;
		gp_selfos_cur_task->link.pre=&(gp_selfos_cur_task->link);
		gp_selfos_cur_task->link.next=&(gp_selfos_cur_task->link);

	}
	else
	{
		list_add_behind(&(tcb->link),&(gp_selfos_cur_task->link));
		gp_selfos_cur_task=tcb;
	}



}

/*获取下一个任务的tcb指针*/
void get_next_TCB(void)
{
	struct selfos_task_struct *pr=NULL;
	struct __link_list *pr_spd=NULL,*pr_spd_next=NULL;

#if 0
	do
	{
		gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
	
		if(gp_selfos_cur_task->state==OS_SUSPEND)
		{
			if(gp_selfos_cur_task->wake_time<=get_OS_sys_count())
			{
				gp_selfos_cur_task->state=OS_RUN;
			}
		}
	}while(gp_selfos_cur_task->state!=OS_RUN);
#else
	do
	{
		if(gp_selfos_cur_task->state==OS_SUSPEND)
		{
			pr=gp_selfos_cur_task;
			gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
			
			list_del(&(pr->link));

			if(spr_tail_spd_link->next==NULL)
			{
				spr_tail_spd_link->next=&(pr->link);
				spr_tail_spd_link->pre=&(pr->link);

				pr->link.pre=spr_tail_spd_link;
				pr->link.next=spr_tail_spd_link;
			}
			else
			{
				list_add_behind(&(pr->link), spr_tail_spd_link);
			}
			
		}
		else
		{
			gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
		}
	}
	while(gp_selfos_cur_task->state!=OS_RUN);


	pr_spd=&shead_suspend_link;
	pr_spd_next=pr_spd->next;
	while((pr_spd_next!=(&shead_suspend_link)))
	{
		
		if((pr_spd_next==NULL))	//链表尚未初始化
		{
			break;
		}
		
		pr_spd=pr_spd_next;//获取当前需要处理的节点的位置
		pr_spd_next=pr_spd->next;//保存下个节点的位置.避免当前节点删除后,找不到下个节点
	
		pr=container_of(pr_spd,struct selfos_task_struct,link);
		
		if(pr->wake_time<=get_OS_sys_count())
		{
			pr->state=OS_RUN;
			list_del(&(pr->link));

			list_add_before(&(pr->link),&(gp_selfos_cur_task->link));
			
		}
	}


#endif
	
}






