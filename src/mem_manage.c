


/*�ڴ�صĴ���
˼·����:
ʹ������˫������.free+allocated
δʹ�õ��ڴ����allocated������.�������,�����������ɾ��,Ȼ����뵽free��.
��ʹ�����,����free����ʱ,���ڴ���free������ɾ��,���뵽allocated������.
���pool�Ľṹ����Ҫ���������������Ϣ.
*/


#include "os_def.h"
#include "mem_manage.h"
#include "link_list.h"

/*len����һ���ڴ��len,deepΪ��������ĸ���*/
uint32_t creat_mem_pool(mem_pool *pr_pool,void * pr,uin32_t len,uint32_t deep)
{

	if(pr==NULL)
	{
		return os_null_pr;
	}
	
	(mem_pool *)pr_pool->deep=deep;
	(mem_pool *)pr_pool->len=len;
	(mem_pool *)pr_pool->pr_start=pr+sizeof(mem_pool);
	(mem_pool *)pr_pool->pr_end=pr+len*deep;
	


	return os_true;
}


