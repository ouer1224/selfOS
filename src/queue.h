
#ifndef __QUEUE_h__
#define __QUEUE_h__


#include <stdint.h>


/*queue control block*/
typedef struct
{
	uint32_t num_cur;
	uint32_t num_max;
	uint32_t send;
	uint32_t inser;
	uint32_t * mem;	//每次只能向队列中放入1个int型数据或者一个指针,推荐使用指针的方式进行数据的传递
	
}QueueCB;


uint32_t queue_creat(QueueCB *pr_q,void * mem,uint32_t deep);
uint32_t put_dat_to_queue(QueueCB *pr_q,void * pr_dat,uint32_t delay,uint32_t pos);
uint32_t get_dat_from_queue(QueueCB *pr_q,void ** pr_dat,uint32_t delay,uint32_t pos);


#endif





