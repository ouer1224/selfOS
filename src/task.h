
#ifndef __task_h__
#define __task_h__

#include <stdint.h>

#define uint32	uint32_t


struct xtos_task_struct {
    uint32 *pTopOfStack;   /* ’ª∂•µÿ÷∑ */
	void * next;
};
typedef void(*xtos_task)(void);


extern volatile struct xtos_task_struct *gp_xtos_cur_task;
extern volatile struct xtos_task_struct *gp_xtos_next_task;

void xtos_start(void);
void xtos_context_switch(void);
void xtos_pendsv_handler(void);

void xtos_create_task(struct xtos_task_struct *tcb, xtos_task task, uint32 *stk);
void xtos_distroy_task(void);


#endif



