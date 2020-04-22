

#ifndef __MEM_MANAGE_H__
#define __MEM_MANAGE_H__

#include <stdint.h>
#include "link_list.h"

#define MAX_MEM_DEEP		256
#define NUM_MEM_POOL_LIST	(MAX_MEM_DEEP/8+1)		

typedef struct
{
	uint32_t len;
	uint32_t deep;
	uint8_t free[NUM_MEM_POOL_LIST];
	uint8_t allocated[NUM_MEM_POOL_LIST];
	void *pr_end;
	void *pr_start;
	struct __link_list list;
}mem_pool;


uint32_t os_memcpy(void * dest,void * source,int32_t len);

uint32_t os_memset(void *dest,uint8_t ch,int32_t len);
uint32_t free_mem_to_pool(void **pr_free);
void * get_mem_from_pool(mem_pool *pr_pool,uint32_t len);
uint32_t creat_mem_pool(mem_pool *pr_pool,void * pr,uint32_t len,uint32_t deep);





#endif




