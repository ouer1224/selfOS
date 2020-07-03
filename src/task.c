


/*selfos�ĺ�������ȤʹȻ��os*/

#include "null.h"
#include "cortex_m4_register.h"
#include "link_list.h"
#include "task.h"


#if 1

extern volatile struct selfos_task_struct taskA;
extern volatile struct selfos_task_struct taskB;
extern volatile struct selfos_task_struct taskC;
#endif




volatile uint32_t gOS_sys_time=0;


volatile struct selfos_slp_info		sInfo_slp_task={0xffffffff,NULL};
volatile struct selfos_spd_info		sInfo_spd_task={0xffffffff,NULL};



//static struct __link_list shead_run_link={NULL,NULL};//run����Ĺ���
static struct __link_list shead_suspend_link={NULL,NULL};//��̬ͣ����Ĺ���
static struct __link_list shead_sleep_link={NULL,NULL};//��������Ĺ���

static struct __link_list *spr_tail_spd_link=&shead_suspend_link;
static struct __link_list *spr_tail_slp_link=&shead_sleep_link;




/*
����һ������,���ȼ�Ϊ0,��Ϊ��������ı�ͷ.��������Զ����ȥִ��.��Ϊ��ȡ��һ����Ҫ���е�����ʱ,
��Զʹ������ͷ��next��λ��.
����Ĭ�ϴ���idle������,�������ͷ��nextλ�ñض���������һ�������.
*/
struct selfos_task_struct task_topest={.priority=0};

static struct __link_list *spr_head_task_link=&(task_topest.link);

volatile struct  selfos_task_struct  *gp_selfos_cur_task=NULL;




/*�������ȼ�����Ľṹ,ÿ���ڵ���ȥ����һ����ͬ���ȼ�����������*/
struct slefos_prio_struct sos_prio_list[MAX_NUM_PRIORITY];

struct slefos_prio_struct *spr_os_prio_list=sos_prio_list;




/*idle���񴴽��Ļ���*/
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
	gp_selfos_cur_task->wake_time=get_OS_sys_count()+dly;	//��Ҫ�жϼ������������


	if(gp_selfos_cur_task->wake_time<sInfo_slp_task.recent_wake)
	{
		sInfo_slp_task.recent_wake=gp_selfos_cur_task->wake_time;
	}






}

void OS_setCurInfoSpdTask(uint32_t source,uint32_t dly)
{


	gp_selfos_cur_task->state=OS_SUSPEND;
	gp_selfos_cur_task->wake_time=get_OS_sys_count()+dly; //��Ҫ�жϼ������������
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
		
		if((pr_link_next==NULL)) //������δ��ʼ��
		{
			break;
		}
		
		pr_link=pr_link_next;//��ȡ��ǰ��Ҫ����Ľڵ��λ��
		pr_link_next=pr_link->next;//�����¸��ڵ��λ��.���⵱ǰ�ڵ�ɾ����,�Ҳ����¸��ڵ�
	
		pr=container_of(pr_link,struct selfos_task_struct,link);
		
		if(pr->spd_source==source)
		{
			pr->state=OS_RUN;
			pr->wake_time=0xffffffff;
			pr->spd_source=os_spd_init;
			list_del(&(pr->link));

#if 0
			list_add_before(&(pr->link),&(gp_selfos_cur_task->link));	
#else
			put_task_into_certain_state(pr, OS_RUN);
#endif

	
			continue;
		}
	
		/*�ҳ��µ�����Ļ���ʱ��*/
		if(sInfo_spd_task.recent_wake>pr->wake_time)
		{
			sInfo_spd_task.recent_wake=pr->wake_time;	
			sInfo_spd_task.recent_task=pr;
		}
		
	}


}
/*�����������޵ȴ�ʱ�Ĵ����ʩ*/
/*���޵ȴ��Ĺ���,����������tcb�����һ�����޵ȴ��ı�־ȥʵ��.*/
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

	//���idle����
	OS__selfos_create_task(&task_idle,taskidle,&task_idle_Stk[SIZE_STACK_TASK_IDLE - 1],255);

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

	__asm volatile	//��psp����Ϊ0,��ʾ�ǵ�һ���л�������.
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


uint32_t OS__selfos_create_task(struct selfos_task_struct * tcb, selfos_task task, uint32_t * stk ,uint32_t priority)
{
	uint32_t  *pstk;
	void *pr=NULL;
	uint32_t i=0;
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
    *(--pstk) = (uint32_t)selfos_distroy_task; // R14 (LR)
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

#if 0
	/*��������ӵ�������*/
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
		put_task_into_certain_state(tcb, OS_RUN);	//�������ȼ�����������.
	}
#endif


	/*��ʼ�����ȼ�������*/
	if(sos_prio_list[0].link.next==NULL)
	{
		sos_prio_list[0].prio=0;
		sos_prio_list[0].link.next=&(sos_prio_list[0].link);
		sos_prio_list[0].link.pre=&(sos_prio_list[0].link);
	}

	/*������������ȼ�,ѡ����ʵ����ȼ��Ľڵ�,��������*/
	/*�ڵ�0�����κ�����,����Ϊ����ͷʹ��*/
	for(i=1;i<MAX_NUM_PRIORITY;i++)
	{
		if(tcb->priority==sos_prio_list[i].prio)
		{
			list_add_behind(&(sos_prio_list[i].pr_task_list->link),&(tcb->link));
			tcb->proi_node_link=&(sos_prio_list[i].link);
			i=0;
			break;
		}
		else if(sos_prio_list[i].prio==0)	//��Ҫ����һ���µ����ȼ��ڵ�
		{
			tcb->link.next=&(tcb->link);	//����һ���յ����ȼ�������ʱ,Ҫ�����ӵ�������������һ�γ�ʼ��.
			tcb->link.pre=&(tcb->link);
			sos_prio_list[i].prio=tcb->priority;
			sos_prio_list[i].pr_task_list=tcb;

			//�����ȼ�����Ľڵ�,���뵽���ȼ���������.���մ�С�����˳��.
			//���ȼ���ֵԽС,���ȼ�Խ��
			add_list_base_para(&(sos_prio_list[0].link),&(sos_prio_list[i].link),\
								__offsetof(struct slefos_prio_struct,link),\
								__offsetof(struct slefos_prio_struct,prio));
			tcb->proi_node_link=&(sos_prio_list[i].link);
			i=0;
			break;
			
		}


	}

	if(i==0)
	{
		return os_true;
	}
	else
	{
		return os_false;
	}



}


uint32_t selfos_create_task(struct selfos_task_struct * tcb, selfos_task task, uint32_t * stk ,uint32_t priority)
{
	if((priority==0)||(priority==255))
	{
		return os_false;
	}

	OS__selfos_create_task(tcb,task,stk,priority);

	return os_true;
}
/*��ȡ��һ�������tcbָ��*/
void get_next_TCB(void)
{
	struct selfos_task_struct *pr=NULL;
	struct __link_list *pr_link=NULL,*pr_link_next=NULL;
	struct slefos_prio_struct *pr_proity=NULL;
	void *pr_tmp=NULL;

	
	/*���sleep������*/
	
		if(IfTaskNeedUnSlp()==1)
		{
			sInfo_slp_task.recent_wake=0xffffffff;
	
			pr_link=&shead_sleep_link;
			pr_link_next=pr_link->next;
			while((pr_link_next!=(&shead_sleep_link)))
			{
				
				if((pr_link_next==NULL))	//������δ��ʼ��
				{
					break;
				}
				
				pr_link=pr_link_next;//��ȡ��ǰ��Ҫ����Ľڵ��λ��
				pr_link_next=pr_link->next;//�����¸��ڵ��λ��.���⵱ǰ�ڵ�ɾ����,�Ҳ����¸��ڵ�
			
				pr=container_of(pr_link,struct selfos_task_struct,link);
				
				if(pr->wake_time<=get_OS_sys_count())
				{

#if 1
				if(pr==&taskA)
				{
					__asm volatile
					(
						"nop"
					);
				}
				
#endif
					pr->state=OS_RUN;
					pr->wake_time=0xffffffff;
					list_del(&(pr->link));
	
					//list_add_before(&(pr->link),&(gp_selfos_cur_task->link)); 
					put_task_into_certain_state(pr, OS_RUN);
	
					continue;
				}
	
				/*�ҳ��µ�����Ļ���ʱ��*/
				if(sInfo_slp_task.recent_wake>pr->wake_time)
				{
					sInfo_slp_task.recent_wake=pr->wake_time;	
					sInfo_slp_task.recent_task=pr;
				}
	
				
			}
	
		}
	
	
	/*���suspend������*/
		if(IfTaskNeedUnSpd()==1)
		{
			sInfo_spd_task.recent_wake=0xffffffff;
	
			pr_link=&shead_suspend_link;
			pr_link_next=pr_link->next;
			while((pr_link_next!=(&shead_suspend_link)))
			{
				
				if((pr_link_next==NULL))	//������δ��ʼ��
				{
					break;
				}
				
				pr_link=pr_link_next;//��ȡ��ǰ��Ҫ����Ľڵ��λ��
				pr_link_next=pr_link->next;//�����¸��ڵ��λ��.���⵱ǰ�ڵ�ɾ����,�Ҳ����¸��ڵ�
			
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
	
				/*�ҳ��µ�����Ļ���ʱ��*/
				if(sInfo_spd_task.recent_wake>pr->wake_time)
				{
					sInfo_spd_task.recent_wake=pr->wake_time;	
					sInfo_spd_task.recent_task=pr;
				}
	
				
			}
	
		}

	/*�ڽ�sleep��suspend�����������,��run����������������,�ڽ���run����ѵ*/

	do
	{
		if(gp_selfos_cur_task==NULL)
		{
			break;
		}

		if(gp_selfos_cur_task->state==OS_SLEEP)
		{
			pr=gp_selfos_cur_task;

			if(pr==&taskA)
			{
				__asm volatile
				(
					"nop"
				);
			}
			
			//gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
			if(pr->link.next==&(pr->link))	//�Ѿ��ǵ�ǰ���ȼ������һ����
			{
				//�����ȼ�����ɾ����ǰ���ȼ��ڵ�
				
				pr_tmp=container_of((pr->proi_node_link),struct slefos_prio_struct,link);
				
				list_del(&(((struct slefos_prio_struct *)pr_tmp)->link));
				((struct slefos_prio_struct *)pr_tmp)->prio=NULL;
				((struct slefos_prio_struct *)pr_tmp)->pr_task_list=NULL;
			}
			else
			{
				list_del(&(pr->link));	
			}
			
			put_task_into_other_state(&spr_tail_slp_link,pr);
			pr->proi_node_link=NULL;
			
			gp_selfos_cur_task=NULL;

		}
		else if(gp_selfos_cur_task->state==OS_SUSPEND)
		{
			pr=gp_selfos_cur_task;
			//gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
			if(pr->link.next==&(pr->link))	//�Ѿ��ǵ�ǰ���ȼ������һ����
			{
				//�����ȼ�����ɾ����ǰ���ȼ��ڵ�
				
				pr_tmp=container_of((pr->proi_node_link),struct slefos_prio_struct,link);
				
				list_del(&(((struct slefos_prio_struct *)pr_tmp)->link));
				((struct slefos_prio_struct *)pr_tmp)->prio=NULL;
				((struct slefos_prio_struct *)pr_tmp)->pr_task_list=NULL;
			}
			else
			{
				list_del(&(pr->link));	
			}

			put_task_into_other_state(&spr_tail_spd_link,pr);
			pr->proi_node_link=NULL;
			
			gp_selfos_cur_task=NULL;


		}
		else if(gp_selfos_cur_task->state==OS_RUN)
		{
			//gp_selfos_cur_task=container_of(gp_selfos_cur_task->link.next,struct selfos_task_struct,link);
			//gp_selfos_cur_task=container_of(spr_head_task_link->next,struct selfos_task_struct,link);

#if 0
			gp_selfos_cur_task=pr_proity->pr_task_list;
			gp_selfos_cur_task->proi_node_link=&(pr_proity->link);

			pr_proity->pr_task_list=container_of(pr_proity->pr_task_list->link.next,struct selfos_task_struct,link);

#endif

			pr_link=gp_selfos_cur_task->proi_node_link;
			pr_tmp=container_of((pr_link),struct slefos_prio_struct,link);

			((struct slefos_prio_struct *)pr_tmp)->pr_task_list=container_of((gp_selfos_cur_task->link.next),struct selfos_task_struct,link);

		}
		else
		{
				break;
		}
	}
	while(gp_selfos_cur_task->state!=OS_RUN);

/*��sos_prio_list�����в����Ѿ���������߼�������*/
/*Ҳ����sos_prio_list0ָ�����һ���ṹ����ָ�������������*/
	pr_proity=container_of((sos_prio_list[0].link.next), struct slefos_prio_struct, link);
	
	gp_selfos_cur_task=pr_proity->pr_task_list;

/*����idle����ض������,��һֱ����run̬,��˱ض�����һ��sos_prio_list���͵Ľṹ��,��ṹ��ָ��idle����
������ֿյ����*/
	
}


uint32_t put_task_into_other_state(struct __link_list **pr_tail,struct selfos_task_struct *pr_task)
{
	
	if((*pr_tail)->next==NULL)	//��ʼ��
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

uint32_t put_task_into_run_state(struct selfos_task_struct *tcb)
{
	uint32_t i=0;
#if 0
	tcb->link.next=&(tcb->link);
	tcb->link.pre=&(tcb->link);
#endif
	
	/*������������ȼ�,ѡ����ʵ����ȼ��Ľڵ�,��������*/
	for(i=1;i<MAX_NUM_PRIORITY;i++)
	{
		if(tcb->priority==sos_prio_list[i].prio)
		{
			list_add_behind(&(tcb->link),&(sos_prio_list[i].pr_task_list->link));
			tcb->proi_node_link=&(sos_prio_list[i].link);
			tcb->state=OS_RUN;
			return os_true;
		}
		else if(sos_prio_list[i].prio==0)	//��Ҫ����һ���µ����ȼ��ڵ�
		{
			tcb->link.next=&(tcb->link);	//����һ���յ����ȼ�������ʱ,Ҫ�����ӵ�������������һ�γ�ʼ��.
			tcb->link.pre=&(tcb->link);
			
			sos_prio_list[i].pr_task_list=tcb;
			sos_prio_list[i].prio=tcb->priority;
			//�����ȼ�����Ľڵ�,���뵽���ȼ���������.���մ�С�����˳��.

			add_list_base_para(&sos_prio_list[0].link,&(sos_prio_list[i].link),\
								__offsetof(struct slefos_prio_struct,link),\
								__offsetof(struct slefos_prio_struct,prio));
			
			tcb->proi_node_link=&(sos_prio_list[i].link);
			tcb->state=OS_RUN;

			return os_true;

		}

	}

	return os_false;

}



uint32_t put_task_into_certain_state(struct selfos_task_struct *pr_task,uint32_t flag)
{
	switch(flag)
	{
		case OS_RUN:
		{
			put_task_into_run_state(pr_task);
		}
		break;

		defualt:
			return os_false;
			break;
	}

	return os_true;
}



#define macro_prIsNull		0xff

#define CheckPrIsVail(pr)		do{if(pr==NULL){return macro_prIsNull;}}while(0)

uint8_t listToSlef(struct __link_list * pr_list)
{
	CheckPrIsVail(pr_list);
	
	pr_list->next=pr_list;
	pr_list->pre=pr_list;
		
	return 1;
}

uint8_t listIsNull(struct __link_list * head_list)
{
	CheckPrIsVail(head_list);
	if((head_list->pre==NULL)&&(head_list->next==NULL))	
	{
		return 1;	
	}	
	return 0;
}

uint8_t listReachEnd(struct __link_list * head_list,struct __link_list * cur_list)
{
	if(head_list->next==head_list)
	{
		return 0xff;	/*��ʾֻ��1��Ԫ��*/
	}	
	
	if(cur_list->next==head_list)
	{
		return 1;		/*��ʾ���������end*/ 
	}	
	
	return 0;
}

uint32_t ParaValOfStrcut(void * pr_struct,uint32_t para_offset) 
{
	return *((uint32_t *)(pr_struct+para_offset));
}




void __add_list_base_para(struct __link_list * head_list,
						struct __link_list * inser_list,
						uint32_t list_offset,
						uint32_t para_offset,uint8_t order)
{
	void * inser_node=NULL;
	void * head_node=NULL;
	void * cur_node=NULL;
	uint32_t para_node=0;
	uint32_t para_new_node=0;
	
	struct __link_list * pr_list=NULL;
	struct __link_list * pr_list_next=NULL;
	
	inser_node= (void *)inser_list - list_offset ;
	head_node= (void *)head_list - list_offset;
	
	para_node = *((uint32_t *)(head_node+para_offset));
	para_new_node = *((uint32_t *)(inser_node+para_offset));
	
	
	if(listIsNull(head_list)==1)		
	{
		listToSlef(head_list);
	}
	if(listReachEnd(head_list,head_list)==0xff)	/*ֻ��һ�����ݵ�ʱ��,ֱ�Ӳ���*/
	{
		list_add_behind(inser_list,head_list);
	}
	else
	{
		pr_list=head_list;
		pr_list_next=pr_list->next;
		
		do
		{
			cur_node = (void *)pr_list - list_offset ;	
			para_node = *((uint32_t *)(cur_node+para_offset));
			para_new_node = *((uint32_t *)(inser_node+para_offset));
			
			if(para_new_node > para_node)
			{
				
				if(listReachEnd(head_list,pr_list)==1)/*���������β��,��û���ҵ���inser_node�Ĳ������,��inser���뵽β��*/
				{
					list_add_before(inser_list,head_list); 
					break;
				}				
				
				/*loop*/
				pr_list=pr_list_next;
				pr_list_next=pr_list->next;

			}
			else
			{
				/*inser before cur*/
				list_add_before(inser_list,pr_list);
				break;
			}

		}
		while(1);	
	}	
}



