
#ifndef __task_h__
#define __task_h__

#include <stdint.h>

#include "link_list.h"




extern volatile uint32_t gOS_sys_time;



enum _State_Task
{
	OS_RUN=0,
	OS_SUSPEND,
	OS_SLEEP
};


struct mdos_task_struct {
	uint32_t *pTopOfStack;   /* ’ª∂•µÿ÷∑ */
	enum _State_Task state;
	uint32_t wake_time;
	struct __link_list link;
};



#define OS_sys_count_add()	do{gOS_sys_time++;}while(0)
#define get_OS_sys_count()		gOS_sys_time

#define Sys_readyToSwitch()		do{	OS_sys_count_add();OS_readyToSwitch();}while(0)




typedef void(*mdos_task)(void);


extern volatile struct mdos_task_struct *gp_mdos_cur_task;
extern volatile struct mdos_task_struct *gp_mdos_next_task;

void mdos_start(void);
void mdos_context_switch(void);
void mdos_pendsv_handler(void);

void mdos_create_task(struct mdos_task_struct *tcb, mdos_task task, uint32_t *stk);
void mdos_distroy_task(void);

uint8_t OS_readyToSwitch(void);


void TaskDelay(uint32_t dly);

#endif



