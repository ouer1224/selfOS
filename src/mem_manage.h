

#ifndef __MEM_MANAGE_H__
#defind __MEM_MANAGE_H__

#include "stdint.h"


typedef struct
{
	uint32_t len;
	uint32_t deep;
	void *pr_end;
	void *pr_start;
}mem_pool;






#endif




