

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






#endif




