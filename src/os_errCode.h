#ifndef __OS_ERRCODE_H__
#define __OS_ERRCODE_H__

#include <stdint.h>

typedef struct
{
	uint32_t err;
	uint32_t time;
}__ErrCode;



typedef struct
{
	__ErrCode err_fun;
	__ErrCode err_mem_pool;
	__ErrCode err_task;
	__ErrCode err_list;
}ErrCode;

extern ErrCode gOS_err_code;



#endif
