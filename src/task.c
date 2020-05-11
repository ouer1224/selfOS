


/*selfos的含义是兴趣使然的os*/

#include "null.h"
#include "cortex_m4_register.h"
#include "link_list.h"
#include "task.h"











volatile uint32_t gOS_sys_time=0;


volatile struct selfos_slp_info		sInfo_slp_task={0xffffffff,NULL};
volatile struct selfos_spd_info		sInfo_spd_task={0xffffffff,NULL};



//static struct __link_list shead_run_link={NULL,NULL};//run任务的归宿
static struct __link_list shead_suspend_link={NULL,NULL};//暂停态任务的归宿
static struct __link_list shead_sleep_link={NULL,NULL};//休眠任务的归宿

static struct __link_list *spr_tail_spd_link=&shead_suspend_link;
static struct __link_list *spr_tail_slp_link=&shead_sleep_link;




/*
创建一个任务,优先级为0,作为任务链表的表头.该任务永远不会去执行.因为获取下一个需要运行的任务时,
永远使用链表头的next的位置.
由于默认创建idle的任务,因此链表头的next位置必定是有至少一个任务的.
*/
strcut selfos_task_struct task_topest={.priority=0};
static struct __link_list *spr_head_task_link=&(task_topest.link);

volatile struct  selfos_task_struct  *gp_selfos_cur_task=NULL;






/*idle任务创建的环境*/
#define SIZE_STACK_TASK_IDLE	64
volatile struct selfos_task_struct task_idle;
static unsigned int task_idle_Stk[SIZE_STACK_TASK_IDLE];




#define head_asm_frame		__asm volatile(
#define tail_asm_frame		);


#define IfTaskNeedUnSlp()		((sInfo_slp_task.recent_wake<=get_OS_sys_count())?1:0)

#define IfTaskNeedUnSpd()		((sInfo_spd_task.recent_wake<=get_OS_sys_count())?1:0)






uint8_t OS_readyToSwitch(void)
{	
	SCB_ICSR |= 0x01<<28;

	return 1;
}

void OS_setCurInfoSlpTask(uint32_t dly)
{

	gp_selfos_cur_task->state=OS_SLEEP;
	gp_selfos_cur_task->wake_time=get_OS_sys_count()+dly;	//需要判断计数溢出的问题


	if(gp_selfos_cur_task->wake_time<sInfo_slp_task.recent_wake)
	{
		sInfo_slp_task.recent_wake=gp_selfos_cur_task->wake_time;
	}






}

void OS_setCurInfoSpdTask(uint32_t source,uint32_t dly)
{


	gp_selfos_cur_task->state=OS_SUSPEND;
	gp_selfos_cur_task->wake_time=get_OS_sys_count()+dly; //需要判断计数溢出的问题
	gp_selfos_cur_task->spd_source=source;

	if(gp_selfos_cur_task->wake_time<sInfo_spd_task.recent_wake)
	{
		sInfo_spd_task.recent_wake=gp_selfos_cur_task->wake_time;
	}
}


void OS_relSpdTask(uint32_t source)
{
	struct selfos_task_struct *pr=NULL;
	struct __link_list *pr_link=NULL,*pr_link_next=NULL;

	pr_link=&shead_suspend_link;
	pr_link_next=pr_link->next;
	while((pr_link_next!=(&shead_suspend_link)))
	{
		
		if((pr_link_next==NULL)) //链表尚未初始化
		{
			break;
		}
		
		pr_link=pr_link_next;//获取当前需要处理的节点的位置
		pr_link_next=pr_link->next;//保存下个节点的位置.避免当前节点删除后,找不到下个节点
	
		pr=container_of(pr_link,struct selfos_task_struct,link);
		
		if(pr->spd_source==source)
		{
			pr->state=OS_RUN;
			pr->wake_time=0xffffffff;
			pr->spd_source=os_spd_init;
			list_del(&(pr->link));
	
			list_add_before(&(pr->link),&(gp_selfos_cur_task->link));	
	
			continue;
		}
	
		/*找出新的最近的唤醒时间*/
		if(sInfo_spd_task.recent_wake>pr->wake_time)
		{
			sInfo_spd_task.recent_wake=pr->wake_time;	
			sInfo_spd_task.recent_task=pr;
		}
		
	}


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
	OS_setCurInfoSlpTask(dly);
	exit_critical_area();
	
	OS_readyToSwitch();
	while(gp_selfos_cur_task->state==OS_SLEEP);
	
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
	__selfos_create_task(&task_idle,taskidle,&task_idle_Stk[SIZE_STACK_TASK_IDLE - 1],255);

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


uint32_t __selfos_create_task(struct selfos_task_struct * tcb, selfos_task task, uint32_t * stk ,uint32_t priority)
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
	tcb->priority=priority;

	/*将任务添加到链表中*/
	if(gp_selfos_cur_task==NULL)
	{
		gp_selfos_cur_task=tcb;

		spr_head_task_link->pre=&(gp_selfos_cur_task->link);
		spr_head_task_link->next=&(gp_selfos_cur_task->link);
		
		gp_selfos_cur_task->link.next=spr_head_task_link;
		gp_selfos_cur_task->link.pre=spr_head_task_link;

	}
	else
	{
		put_task_into_certain_state(tcb, OS_RUN);	//根据优先级放入链表中.
	}

	return os_true;

}


uint32_t selfos_create_task(struct selfos_task_struct * tcb, selfos_task task, uint32_t * stk ,uint32_t priority)
{
	if((priority==0)||(priority==255))
	{
		return os_false;
	}

	__selfos_create_task(tcb,task,stk,priority);

	return os_true;
}
/*获取下一个任务的tcb指针*/
void get_next_TCB(void)
{
	struct selfos_task_struct *pr=NULL;
	struct __link_list *pr_link=NULL,*pr_link_next=NULL;


	
	/*检查sleep的链表*/
	
		if(IfTaskNeedUnSlp()==1)
		{
			sInfo_slp_task.recent_wake=0xffffffff;
	
			pr_link=&shead_sleep_link;
			pr_link_next=pr_link->next;
			while((pr_link_next!=(&shead_sleep_link)))
			{
				
				if((pr_link_next==NULL))	//链表尚未初始化
				{
					break;
				}
				
				pr_link=pr_link_next;//获取当前需要处理的节点的位置
				pr_link_next=pr_link->next;//保存下个节点的位置.避免当前节点删除后,找不到下个节点
			
				pr=container_of(pr_link,struct selfos_task_struct,link);
				
				if(pr->wake_time<=get_OS_sys_count())
				{
					pr->state=OS_RUN;
					pr->wake_time=0xffffffff;
					list_del(&(pr->link));
	
					//list_add_before(&(pr->link),&(gp_selfos_cur_task->link)); 
					put_task_into_certain_state(pr, OS_RUN);
	
					continue;
				}
	
				/*找出新的最近的唤醒时间*/
				if(sInfo_slp_task.recent_wake>pr->wake_time)
				{
					sInfo_slp_task.recent_wake=pr->wake_time;	
					sInfo_slp_task.recent_task=pr;
				}
	
				
			}
	
		}
	
	
	/*检查suspend的链表*/
		if(IfTaskNeedUnSpd()==1)
		{
			sInfo_spd_task.recent_wake=0xffffffff;
	
			pr_link=&shead_suspend_link;
			pr_link_next=pr_link->next;
			while((pr_link_next!=(&shead_suspend_link)))
			{
				
				if((pr_link_next==NULL))	//链表尚未初始化
				{
					break;
				}
				
				pr_link=pr_link_next;//获取当前需要处理的节点的位置
				pr_link_next=pr_link->next;//保存下个节点的位置.避免当前节点删除后,找不到下个节点
			
				pr=container_of(pr_link,struct selfos_task_struct,link);
				
				if(pr->wake_time<=get_OS_sys_count())
				{
					pr->state=OS_RUN;
					pr->wake_time=0xffffffff;
					pr->spd_source=os_spd_timeout;
					list_del(&(pr->link));
	
					//list_add_before(&(pr->link),&(gp_selfos_cur_task->link)); 
					put_task_into_certain_state(pr, OS_RUN);
	
					continue;
				}
	
				/*找出新的最近的唤醒时间*/
				if(sInfo_spd_task.recent_wake>pr->wake_time)
				{
					sInfo_spd_task.recent_wake=pr->wake_time;	
					sInfo_spd_task.recent_task=pr;
				}
	
				
			}
	
		}

	/*在将sleep和suspend的任务处理完毕,将run链表进行重新排序后,在进行run的轮训*/
	/*对run链表进行轮训,获取下一个需要运行的任务*/
	do
	{
		if(gp_selfos_cur_task->state==OS_SLEEP)
		{
			pr=gp_selfos_cur_task;
			gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
			list_del(&(pr->link));	

			put_task_into_other_state(&spr_tail_slp_link,pr);
			
		}
		else if(gp_selfos_cur_task->state==OS_SUSPEND)
		{
			pr=gp_selfos_cur_task;
			gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
			
			list_del(&(pr->link));

			put_task_into_other_state(&spr_tail_spd_link,pr);

		}
		else
		{
			//gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
			gp_selfos_cur_task=container_of(spr_head_task_link->link.next,struct selfos_task_struct,link);
		}
	}
	while(gp_selfos_cur_task->state!=OS_RUN);






#endif
	
}


uint32_t put_task_into_other_state(struct __link_list **pr_tail,struct selfos_task_struct *pr_task)
{
	
	if((*pr_tail)->next==NULL)	//初始化
	{
		(*pr_tail)->next=&(pr_task->link);
		(*pr_tail)->pre=&(pr_task->link);

		pr_task->link.pre=(*pr_tail);
		pr_task->link.next=(*pr_tail);
	}
	else
	{
		return list_add_behind(&(pr_task->link), (*pr_tail));
	}

	return os_true;
}

uint32_t put_task_into_run_state(struct __link_list **pr_head,\
								struct selfos_task_struct *pr_task)
{
	struct __link_list *pr_link=NULL;
	struct selfos_task_struct *pr_task_tmp=NULL;
	struct selfos_task_struct *pr_task_tmp_next=NULL;

	if((*pr_head)->next==NULL)	//初始化
	{
		(*pr_head)->next=&(pr_task->link);
		(*pr_head)->pre=&(pr_task->link);

		pr_task->link.pre=(*pr_head);
		pr_task->link.next=(*pr_head);
	}
	else
	{
		pr_link=pr_head;
		pr_task_tmp=container_of(pr_link,struct selfos_task_struct,link);
		pr_task_tmp_next=container_of(pr_link->next,struct selfos_task_struct,link);
		
		while((pr_link->next!=pr_head)||(pr_task->priority>=pr_task_tmp->priority)&&(pr_task->priority<pr_task_tmp_next->priority))
		{

		}
		list_add_behind(&(pr_task->link), (*pr_head));
	}

	return os_true;
}

uint32_t put_task_into_certain_state(struct selfos_task_struct *pr_task,\
									enum _State_Task flag)
{
	switch(flag)
	{
		case OS_RUN:
		{
			put_task_into_run_state(&spr_head_task_link,pr_task);
		}
		break;

		defualt:
			return os_false;
			break;
	}

	return os_true;
}



