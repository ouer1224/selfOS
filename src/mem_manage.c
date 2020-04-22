

/*内存池的创建
思路如下:
使用两个双向链表.free+allocated
未使用的内存放在allocated链表中.当分配后,将其从链表中删除,然后加入到free中.
当使用完毕,调用free函数时,将内存块从free链表中删除,加入到allocated链表中.
因此pool的结构体中要加入两个链表的信息.
*/

#include "null.h"
#include "os_def.h"
#include "mem_manage.h"
#include "link_list.h"
#include "task.h"


#ifdef mempool_null_pr
#error	mempool_null_pr shuld not been defined by user
#endif

#ifdef mempool_true
#error	mempool_true shuld not been defined by user
#endif

#ifdef mempool_false
#error	mempool_false shuld not been defined by user
#endif



#define mempool_null_pr		os_kernel_val(memPool,os_null_pr)
#define mempool_true		os_kernel_val(memPool,os_true)
#define mempool_false		os_kernel_val(memPool,os_false)
#define mempool_empty		os_kernel_val(memPool,os_false)
#define mempool_unused		os_kernel_val(memPool,os_false)




static struct __link_list	*spr_head_pool=NULL;
static struct __link_list	*spr_tail_pool=NULL;



uint32_t os_memcpy(void * dest,void * source,int32_t len)
{	
	if((dest==NULL)||(source==NULL))
	{
		return mempool_null_pr;
	}
	if(len<=0)
	{
		return mempool_false;
	}

	while(len>0)
	{
		len--;
		((char *)dest)[len]=((char *)source)[len];
	}


	return mempool_true;
}

uint32_t os_memset(void *dest,uint8_t ch,int32_t len)
{

	if(dest==NULL)
	{
		return mempool_null_pr;
	}
	if(len<=0)
	{
		return mempool_false;
	}

	while(len>0)
	{
		len--;
		((char *)dest)[len]=ch;
	}

	return mempool_true;

}





/*len申请一次内存的len,deep为可以申请的个数*/
uint32_t creat_mem_pool(mem_pool *pr_pool,void * pr,uint32_t len,uint32_t deep)
{
	if(pr_pool==NULL)
	{
		return mempool_null_pr;
	}
	if(deep>256)
	{
		return mempool_false;
	}

	pr_pool->deep=deep;
	pr_pool->len=len;

	pr_pool->pr_start=(void *)pr;
	pr_pool->pr_end=(void *)pr+(len*deep)-1;

	os_memset(pr_pool->free,0,NUM_MEM_POOL_LIST);
	os_memset(pr_pool->allocated,0,NUM_MEM_POOL_LIST);

	/*必须先进行余数部分的位的赋值*/
	while(deep%8>0)
	{
		pr_pool->free[deep/8]=(pr_pool->free[deep/8]<<1)|(0x01);
	}
	
	while(deep/8>0)
	{
		pr_pool->free[deep/8-1]=0xff;
		deep-=8;
	}

	if(spr_head_pool==NULL)
	{
		spr_head_pool=&pr_pool->list;
		spr_head_pool->next=spr_head_pool;
		spr_head_pool->pre=spr_head_pool;
		spr_tail_pool=spr_head_pool;
	}
	else
	{
		
		list_add_behind(&(pr_pool->list),spr_tail_pool);
	}


	
	return mempool_true;
}

void * get_mem_from_pool(mem_pool *pr_pool,uint32_t len)
{
	uint32_t i=0;
	if(pr_pool==NULL)
	{
		mempool_null_pr;
		return NULL;
	}
	if(len>pr_pool->len)
	{	
		mempool_false;
		return NULL;
	}

	for(i=0;i<pr_pool->deep;i++)
	{	
		/*寻找尚未使用的内存块*/
		if(__isnbitseted_array(pr_pool->free,i)==1)
		{
			//将内存地址return,并将对应的free位清0
			__clearnbit_array(pr_pool->free,i);
			
			return ((void *)(pr_pool->pr_start))+len*i;
			
			break;
		}
	}
	mempool_empty;
	return NULL;
}

uint32_t free_mem_to_pool(void *pr)
{
	struct __link_list *cur=NULL;	
	mem_pool * pr_pool=NULL;
	uint32_t deep=0;
	if(pr==NULL)
	{
		return mempool_null_pr;
	}
	if(spr_head_pool==NULL)
	{
		return mempool_unused;
	}

	cur=spr_head_pool;
	do
	{
		pr_pool=container_of(cur,mem_pool,list);
		if((pr>=pr_pool->pr_start)&&(pr<=pr_pool->pr_end))
		{
			break;			
		}
		pr_pool=NULL;
		cur=cur->next;	//指向下一个结构体
	}while(cur!=spr_head_pool);	//轮训完毕
	
	if(pr_pool==NULL)
	{
		return mempool_false;
	}

	/*找出需要释放内存在pool中的深度*/
	for(deep=0;deep<pr_pool->deep;deep++)
	{	
		if((pr>=(pr_pool->pr_start+deep*pr_pool->len))&&(pr<(pr_pool->pr_start+(deep+1)*pr_pool->len)))
		{
			__setnbit_array(pr_pool->free,deep);
			break;
		}
	}

	return mempool_true;
}

/*不给外界使用*/
#undef mempool_null_pr
#undef mempool_true
#undef mempool_false



