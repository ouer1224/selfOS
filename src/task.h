
#ifndef __task_h__
#define __task_h__

#include <stdint.h>

#include "link_list.h"
#include "os_def.h"



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
	struct __link_list link;
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


void TaskDelay(uint32_t dly);

#endif



