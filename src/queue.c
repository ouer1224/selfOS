

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
	pr_q->num_cur=0;		//��ǰ�����п������ݵ�����
	pr_q->num_max=deep;		//���ڱ���

	pr_q->send=0;	//��ʼλ���Ǻ�inserλ����ͬ.��ʵ��send��inser��λ����ͬʱ,�ͱ�ʾ�����ǿյ���.
	pr_q->inser=0;	//��λ��0��ʼ,��������

	return queue_true;
}

/*����һ�����ݵ�ָ���Ķ�����*/
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
		
		pr_q->inser++;	//ָ����һ��λ��
		pr_q->inser=pr_q->inser%pr_q->num_max;

	}
	else
	{
		if((pr_q->send!=pr_q->inser))	//��������ʱ��,inser�Ǻ�send��ͬ��λ��
		{
			return queue_err;	//���������ص�err
		}
	
		pr_q->num_cur=pr_q->num_max;

		return queue_full;
	}

	return queue_true;
}


/*��ָ�������л�ȡһ�����ݻ�ָ��*/
/*��Ҫ����,����ȡ�������ݵ�ʱ��,����ȴ�����,�ù��������������ʵ��*/
uint32_t __acquire_dat_from_queue(QueueCB *pr_q,void ** pr_dat,uint32_t pos)
{
	if((pr_q==NULL)||(pr_dat==NULL))
	{
		return queue_null_pr;
	}
	if(pr_q->num_cur<=0)	//����Ϊ�յ�ʱ��
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


/*�����ٽ�����������ʱ�Ķ���д����*/
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
			//����ɹ�
			delay=0;
		}
		else if(rc==__queue_full__)
		{
			//����,��Ҫ�ȴ�
		}
		else	//��������
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
			//������û������,��Ҫ�ȴ�
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

