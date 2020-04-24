
#ifndef __SEM_H__
#define __SEM_H__

#include "os_def.h"
#include "null.h"


/*sem control block*/
typedef struct
{
	uint32_t maxVal;
	uint32_t curVal;
}SemCB;


uint32_t sem_creat(SemCB * pr,uint32_t maxVal,uint32_t initVal);
uint32_t sem_acquire(SemCB *pr,uint32_t delay);
void sem_release(SemCB *pr);





#endif

