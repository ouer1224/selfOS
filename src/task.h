
#ifndef __task_h__
#define __task_h__

#include <stdint.h>



#define NULL	(void *)0

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
	void * next;
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

void OStaskDelay(uint32_t dly);
uint8_t OS_readyToSwitch(void);


#endif



