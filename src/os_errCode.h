#ifndef __OS_ERRCODE_H__
#define __OS_ERRCODE_H__

#include <stdint.h>

typedef struct
{
	uint32_t err;
	uint32_t time;
	uint32_t line;
}__ErrCode;


typedef struct
{
	__ErrCode fun;
	__ErrCode memPool;
	__ErrCode task;
	__ErrCode list;
}ErrCode;

extern ErrCode gOS_kernel_err_code;



#endif
