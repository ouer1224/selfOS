

#include "stdint.h"
#include "sem.h"


typedef struct
{
	uint32_t maxVal;
	uint32_t curVal;
}SEM;



uint32_t sem_creat(SEM * pr,uint32_t maxVal,uint32_t initVal)
{
	if(pr==NULL)
	{
		return os_null_pr;
	}
	if(initVal>maxVal)
	{
		return os_false;
	}

	input_critical_area();
	pr->maxVal=maxVal;
	pr->curVal=initVal;
	exit_critical_area();

	return os_true;
}

/*获取信号量*/
uint32_t sem_acquire(SEM *pr)
{
	uint32_t rt=os_false;
	if(pr==NULL)
	{
		return os_null_pr;
	}
	input_critical_area();

	if(pr->curVal>0)
	{
		pr->curVal--;
		rt=os_true;
	}
	else
	{
		rt=os_false;
	}
	exit_critical_area();

	return rt;

}

/*获取信号量*/
uint32_t sem_take(SEM *pr)
{
	uint32_t rt=os_true;
	if(pr==NULL)
	{
		return os_null_pr;
	}
	input_critical_area();

	if(pr->curVal<pr->maxVal)
	{
		pr->curVal++;
	}
	else
	{
		;
	}
	exit_critical_area();

	return rt;

}































