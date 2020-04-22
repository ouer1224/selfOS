


/*�ڴ�صĴ���
˼·����:
ʹ������˫������.free+allocated
δʹ�õ��ڴ����allocated������.�������,�����������ɾ��,Ȼ����뵽free��.
��ʹ�����,����free����ʱ,���ڴ���free������ɾ��,���뵽allocated������.
���pool�Ľṹ����Ҫ���������������Ϣ.
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





/*len����һ���ڴ��len,deepΪ��������ĸ���*/
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









