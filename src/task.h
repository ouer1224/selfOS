
#ifndef __task_h__
#define __task_h__

#include <stdint.h>

#include "link_list.h"
#include "os_def.h"


#define os_spd_timeout		0xffffffff	
#define os_spd_init			0


extern volatile uint32_t gOS_sys_time;



enum _State_Task
{
	OS_RUN=0,
	OS_SUSPEND,
	OS_SLEEP
};


struct selfos_task_struct {
	uint32_t *pTopOfStack;   /* ’ª∂•µÿ÷∑ */
	enum _State_Task state;
	uint32_t wake_time;
	uint32_t spd_source;
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

void selfos_create_task(struct selfos_task_struct *tcb, selfos_task task, uint32_t *stk);
void selfos_distroy_task(void);

uint8_t OS_readyToSwitch(void);

void OS_setCurInfoSlpTask(uint32_t dly);
void OS_setCurInfoSpdTask(uint32_t source,uint32_t dly);

void OS_relSpdTask(uint32_t source);



uint32_t put_task_into_other_state(struct __link_list **pr_tail,struct selfos_task_struct *pr_task);



void TaskDelay(uint32_t dly);

#endif



