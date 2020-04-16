
#ifndef __task_h__
#define __task_h__

#include <stdint.h>

#define uint32	uint32_t

#define NULL	(void *)0

struct mdos_task_struct {
    uint32 *pTopOfStack;   /* ’ª∂•µÿ÷∑ */
	void * next;
};
typedef void(*mdos_task)(void);


extern volatile struct mdos_task_struct *gp_mdos_cur_task;
extern volatile struct mdos_task_struct *gp_mdos_next_task;

void mdos_start(void);
void mdos_context_switch(void);
void mdos_pendsv_handler(void);

void mdos_create_task(struct mdos_task_struct *tcb, mdos_task task, uint32 *stk);
void mdos_distroy_task(void);


#endif



