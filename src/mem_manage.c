


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


uint32_t os_memcpy(void * dest,void * source,int32_t len)
{	
	if((dest==NULL)||(source==NULL))
	{
		return os_null_pr;
	}
	if(len<=0)
	{
		return os_false;
	}

	while(len>0)
	{
		len--;
		((char *)dest)[len]=((char *)source)[len];
	}

	return os_true;
}

uint32_t os_memset(void *dest,uint8_t ch,int32_t len)
{

	if(dest==NULL)
	{
		return os_null_pr;
	}
	if(len<=0)
	{
		return os_false;
	}

	while(len>0)
	{
		len--;
		((char *)dest)[len]=ch;
	}

	return os_true;

}





/*len申请一次内存的len,deep为可以申请的个数*/
uint32_t creat_mem_pool(mem_pool *pr_pool,void * pr,uint32_t len,uint32_t deep)
{
	if(pr_pool==NULL)
	{
		return os_null_pr;
	}
	if(deep>256)
	{
		return os_false;
	}

	pr_pool->deep=deep;
	pr_pool->len=len;

	pr_pool->pr_start=(void *)pr;
	pr_pool->pr_end=(void *)pr+(len*deep);

	os_memset(pr_pool->free,0,NUM_MEM_POOL_LIST);
	os_memset(pr_pool->allocated,0,NUM_MEM_POOL_LIST);
	
	//while()


	
	return os_true;
}









