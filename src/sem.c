
#include "null.h"
#include "stdint.h"
#include "sem.h"



extern void OStaskDelay(uint32_t dly);


uint32_t sem_creat(SemCB * pr,uint32_t maxVal,uint32_t initVal)
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
uint32_t __sem_acquire(SemCB *pr)
{
	uint32_t rt=os_false;
	if(pr==NULL)
	{
		return os_null_pr;
	}
	

	if(pr->curVal>0)
	{
		pr->curVal--;
		rt=os_true;
	}
	else
	{
		rt=os_false;
	}


	return rt;

}

/*增加了延时等待和临界区保护的信号量获取函数*/
uint32_t sem_acquire(SemCB *pr,uint32_t delay)
{
	uint32_t rc=0;
	
	if(pr==NULL)
	{
		return os_null_pr;
	}


		input_critical_area();
		rc=__sem_acquire(pr);
		if(rc==os_true)
		{
			delay=0;
		}
		else if(rc==os_false)
		{

		}
		else if(rc==os_null_pr)
		{
			delay=0;
		}
		else
		{
			delay=0;
		}

		if(delay>0)
		{

			OS_setCurInfoSpdTask((uint32_t)pr,delay);

		}

		exit_critical_area();


		rc=os_true;
		if(delay>0)
		{
			OS_readyToSwitch();
			while(gp_selfos_cur_task->state==OS_SUSPEND);
			
			if(gp_selfos_cur_task->spd_source==os_spd_timeout)
			{
				gp_selfos_cur_task->spd_source=os_spd_init;
				rc=os_false;
			}
			else
			{
				input_critical_area();
				rc=__sem_acquire(pr);
				exit_critical_area();

			}
		}


	return rc;
}

/*释放信号量*/
void sem_release(SemCB *pr)
{
	uint8_t need_rel=0;
	if(pr==NULL)
	{
		return ;
	}
	input_critical_area();

	if(pr->curVal==0)
	{

	}

	if(pr->curVal<pr->maxVal)
	{
		pr->curVal++;
	}
	else
	{
		;
	}

	need_rel=1;

	
	if(need_rel==1)
	{
		OS_relSpdTask((uint32_t)pr);
	}


	
	exit_critical_area();

	return;
}































