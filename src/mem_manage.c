


/*内存池的创建
思路如下:
使用两个双向链表.free+allocated
未使用的内存放在allocated链表中.当分配后,将其从链表中删除,然后加入到free中.
当使用完毕,调用free函数时,将内存块从free链表中删除,加入到allocated链表中.
因此pool的结构体中要加入两个链表的信息.
*/


#include "os_def.h"
#include "mem_manage.h"
#include "link_list.h"

/*len申请一次内存的len,deep为可以申请的个数*/
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


