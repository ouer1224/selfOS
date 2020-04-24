

#include "null.h"
#include "os_def.h"
#include "os_errCode.h"
#include "queue.h"


#ifdef queue_null_pr
#error	queue_null_pr shuld not been defined by user
#endif

#ifdef queue_true
#error	queue_true shuld not been defined by user
#endif

#ifdef queue_false
#error	queue_false shuld not been defined by user
#endif



#define __queue_empty__		2
#define __queue_full__		3
#define __queue_unused_		4
#define __queue_err__		0x10



#define queue_null_pr		os_kernel_val(queue,os_null_pr)
#define queue_true			os_kernel_val(queue,os_true)
#define queue_false			os_kernel_val(queue,os_false)
#define queue_empty			os_kernel_val(queue,__queue_empty__)
#define queue_full			os_kernel_val(queue,__queue_full__)
#define queue_unused		os_kernel_val(queue,__queue_unused_)
#define queue_err			os_kernel_val(queue,__queue_err__)



#define QUEUE_MAX_DEEP		0xffff
#define QUEUE_MIN_DEEP		1


extern void OStaskDelay(uint32_t dly);


uint32_t queue_creat(QueueCB *pr_q,void * mem,uint32_t deep)
{
	if((pr_q==NULL)||(mem==NULL))
	{
		return queue_null_pr;
	}
	if(deep>QUEUE_MAX_DEEP)
	{
		return queue_false;
	}
	if(deep<QUEUE_MIN_DEEP)
	{
		return queue_false;
	}

	pr_q->mem=mem;
	pr_q->num_cur=0;		//当前队列中可用数据的数量
	pr_q->num_max=deep;		//用于保护

	pr_q->send=0;	//起始位置是和inser位置相同.其实当send和inser的位置相同时,就表示数据是空的了.
	pr_q->inser=0;	//从位置0开始,放入数据

	return queue_true;
}

/*插入一个数据到指定的队列中*/
uint32_t __inser_dat_to_queue(QueueCB *pr_q,void * pr_dat,uint32_t pos)
{
	if((pr_q==NULL)||(pr_dat==NULL))
	{
		return queue_null_pr;
	}
	

	if(pr_q->num_cur<pr_q->num_max)
	{

		pr_q->mem[pr_q->inser]=(uint32_t)(pr_dat);
		
		pr_q->num_cur++;
		
		pr_q->inser++;	//指向下一个位置
		pr_q->inser=pr_q->inser%pr_q->num_max;

	}
	else
	{
		if((pr_q->send!=pr_q->inser))	//队列满的时候,inser是和send相同的位置
		{
			return queue_err;	//发生了严重的err
		}
	
		pr_q->num_cur=pr_q->num_max;

		return queue_full;
	}

	return queue_true;
}


/*从指定队列中获取一个数据或指针*/
/*需要考虑,当获取不到数据的时候,进入等待队列,该功能在外包函数中实现*/
uint32_t __acquire_dat_from_queue(QueueCB *pr_q,void ** pr_dat,uint32_t pos)
{
	if((pr_q==NULL)||(pr_dat==NULL))
	{
		return queue_null_pr;
	}
	if(pr_q->num_cur<=0)	//队列为空的时候
	{
		if(pr_q->inser!=pr_q->send)
		{
			return queue_err;
		}
	
		return queue_empty;
	}

	*pr_dat=(void *)(pr_q->mem[pr_q->send]);
	pr_q->send++;
	pr_q->send=pr_q->send%pr_q->num_max;

	pr_q->num_cur--;

	return queue_true;

}


/*加入临界区保护和延时的队列写函数*/
uint32_t put_dat_to_queue(QueueCB *pr_q,void * pr_dat,uint32_t delay,uint32_t pos)
{
	uint32_t rc=os_false;
	
	if((pr_q==NULL)||(pr_dat==NULL))
	{
		return queue_null_pr;
	}

	do{

		input_critical_area();
		rc=__inser_dat_to_queue(pr_q,pr_dat,pos);

		if(rc==os_true)
		{
			//放入成功
			delay=0;
		}
		else if(rc==__queue_full__)
		{
			//满了,需要等待
		}
		else	//其他错误
		{
			delay=0;
		}

		exit_critical_area();

		if(delay>0)
		{
			OStaskDelay(1);
			delay--;
		}
	}while(delay>0);



	return rc;
}


uint32_t get_dat_from_queue(QueueCB *pr_q,void ** pr_dat,uint32_t delay,uint32_t pos)
{
	uint32_t rc=os_false;

	if((pr_q==NULL)||(pr_dat==NULL))
	{
		return queue_null_pr;
	}
	
	do
	{
		input_critical_area();
		rc=__acquire_dat_from_queue(pr_q,pr_dat,pos);
		if(rc==os_true)
		{
			delay=0;
		}
		else if(rc==os_false)
		{
			delay=0;
		}
		else if(rc==__queue_empty__)
		{
			//队列中没有数据,需要等待
		}
		else 
		{
			delay=0;
		}
		exit_critical_area();

		
		if(delay>0)
		{
			OStaskDelay(1);
			delay--;
		}

	}while(delay>0);


	return rc;
}

