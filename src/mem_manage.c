


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





/*len����һ���ڴ��len,deepΪ��������ĸ���*/
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
	pr_pool->pr_end=(void *)pr+(len*deep);

	os_memset(pr_pool->free,0,NUM_MEM_POOL_LIST);
	os_memset(pr_pool->allocated,0,NUM_MEM_POOL_LIST);
	
	//while()


	
	return mempool_true;
}





/*�������ʹ��*/
#undef mempool_null_pr
#undef mempool_true
#undef mempool_false



