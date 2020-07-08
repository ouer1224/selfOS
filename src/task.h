
#ifndef __task_h__
#define __task_h__

#include <stdint.h>

#include "link_list.h"
#include "os_def.h"


#define MAX_NUM_PRIORITY	12		/*最多同时存在N中优先级.*/


#define os_spd_timeout		0xffffffff	
#define os_spd_init			0


extern volatile uint32_t gOS_sys_time;



enum _State_Task
{
	OS_RUN=0,
	OS_SUSPEND,
	OS_SLEEP
};


struct selfos_task_struct 
{
	uint32_t *pTopOfStack;   /* 栈顶地址 */
	enum _State_Task state;
	uint32_t priority:8;
	uint32_t :24;
	uint32_t wake_time;
	uint32_t spd_source;
	struct __link_list *proi_node_link;
	struct __link_list link;
};

/*用于*/
struct slefos_prio_struct
{
	uint32_t prio;	//优先级
	struct selfos_task_struct *pr_task_list;	//链接相同优先级的任务的链表,指向其链表的第一个任务.
	struct __link_list link;
};




struct selfos_slp_info
{
	uint32_t recent_wake;
	struct selfos_task_struct * recent_task;
};


struct selfos_spd_info
{
	uint32_t recent_wake;
	struct selfos_task_struct * recent_task;
};





#define OS_sys_count_add()	do{gOS_sys_time++;}while(0)
#define get_OS_sys_count()		gOS_sys_time

#define Sys_readyToSwitch()		do{	OS_sys_count_add();OS_readyToSwitch();}while(0)




typedef void(*selfos_task)(void);


extern volatile struct selfos_task_struct *gp_selfos_cur_task;
extern volatile struct selfos_task_struct *gp_selfos_next_task;

void selfos_start(void);
//void selfos_context_switch(void);
//void selfos_pendsv_handler(void);
uint32_t OS__selfos_create_task(struct selfos_task_struct * tcb, selfos_task task, uint32_t * stk ,uint32_t priority);
uint32_t selfos_create_task(struct selfos_task_struct *tcb, selfos_task task, uint32_t *stk,uint32_t priority);
void selfos_distroy_task(void);

uint8_t OS_readyToSwitch(void);

void OS_setCurInfoSlpTask(uint32_t dly);
void OS_setCurInfoSpdTask(uint32_t source,uint32_t dly);

void OS_relSpdTask(uint32_t source);



uint32_t put_task_into_other_state(struct __link_list **pr_tail,struct selfos_task_struct *pr_task);

uint32_t put_task_into_certain_state(struct selfos_task_struct *pr_task,uint32_t flag);

void __add_list_base_para(struct __link_list * head_list,
						struct __link_list * inser_list,
						uint32_t list_offset,
						uint32_t para_offset,uint8_t order);


#define add_list_base_para(head_list,inser_list,list_offset, para_offset)		__add_list_base_para(head_list,inser_list,list_offset, para_offset,0)


void TaskDelay(uint32_t dly);


uint8_t checkInAppOrInterr(void);
void __input_critical_area(void);
void __exit_critical_area(void);

#define close_all_interruct() 	do{__asm volatile ("cpsid i" : : : "memory");}while(0)
#define open_all_interruct() 	do{__asm volatile ("cpsie i" : : : "memory");}while(0)



#endif



