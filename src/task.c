


/*mdos�ĺ�����module os,����ͨ���޸Ľӿڵķ�ʽ,�Ե����㷨,�ڴ����,��Ϣ�Ĵ���*/


#include <stdint.h>
#include "task.h"

#include "cortex_m4_register.h"



volatile struct  mdos_task_struct  *gp_mdos_cur_task=NULL;
volatile struct  mdos_task_struct  *gp_mdos_next_task=NULL;

volatile struct mdos_task_struct task_idle;
static unsigned int task_idle_Stk[128];



#define head_asm_frame		__asm volatile(
#define tail_asm_frame		);



void taskidle(void)
{

    while(1){
    	__asm
		("nop\n");
    }

}



void mdos_start (void)
{

	//���idle����
	mdos_create_task(&task_idle,taskidle,&task_idle_Stk[128 - 1]);




	SCB_SHPR3=0xff<<16;

	SYST_RVR=160000;
	SYST_CVR=0;

	SYST_CSR=(0x01<<2)|(0x01<<1);
	SYST_CSR=SYST_CSR&(~(0x01<<0));

	SCB_SHPR3=0x01<<24;

	SYST_CSR=SYST_CSR|0x01;

	__asm volatile	//��psp����Ϊ0,��ʾ�ǵ�һ���л�������.
	(
	"mov r0,#0\n"
	"msr psp,r0\n"
	"isb\n"
	);

}


void mdos_distroy_task() {

    while(1){
    	__asm
		("nop\n");
    }
}


void mdos_create_task(struct mdos_task_struct * tcb, mdos_task task, uint32 * stk) 
{
    uint32  *pstk;
    pstk = stk;
    pstk = (uint32 *)((uint32)(pstk) & 0xFFFFFFF8uL);

    *(--pstk) = (uint32)0x00000000L; //No Name Register  
    *(--pstk) = (uint32)0x00001000L; //FPSCR
    *(--pstk) = (uint32)0x00000015L; //s15
    *(--pstk) = (uint32)0x00000014L; //s14
    *(--pstk) = (uint32)0x00000013L; //s13
    *(--pstk) = (uint32)0x00000012L; //s12
    *(--pstk) = (uint32)0x00000011L; //s11
    *(--pstk) = (uint32)0x00000010L; //s10
    *(--pstk) = (uint32)0x00000009L; //s9
    *(--pstk) = (uint32)0x00000008L; //s8
    *(--pstk) = (uint32)0x00000007L; //s7
    *(--pstk) = (uint32)0x00000006L; //s6
    *(--pstk) = (uint32)0x00000005L; //s5
    *(--pstk) = (uint32)0x00000004L; //s4
    *(--pstk) = (uint32)0x00000003L; //s3
    *(--pstk) = (uint32)0x00000002L; //s2
    *(--pstk) = (uint32)0x00000001L; //s1
    *(--pstk) = (uint32)0x00000000L; //s0


    *(--pstk) = (uint32)0x01000000uL; // xPSR
    *(--pstk) = (uint32)task;         // Entry Point
    *(--pstk) = (uint32)mdos_distroy_task;; // R14 (LR)
    *(--pstk) = (uint32)12121212; // R12
    *(--pstk) = (uint32)03030303; // R3
    *(--pstk) = (uint32)02020202; // R2
    *(--pstk) = (uint32)01010101; // R1
    *(--pstk) = (uint32)0x00000000;  // R0

    *(--pstk) = (uint32)0x11111111uL; // R11
    *(--pstk) = (uint32)0x10101010uL; // R10
    *(--pstk) = (uint32)0x09090909uL; // R9
    *(--pstk) = (uint32)0x08080808uL; // R8
    *(--pstk) = (uint32)0x07070707uL; // R7
    *(--pstk) = (uint32)0x06060606uL; // R6
    *(--pstk) = (uint32)0x05050505uL; // R5
    *(--pstk) = (uint32)0x04040404uL; // R4

    tcb->pTopOfStack = pstk;

	if(gp_mdos_cur_task==NULL)
	{
		gp_mdos_cur_task=tcb;
		gp_mdos_cur_task->next=tcb;
	}
	else
	{
		tcb->next=gp_mdos_cur_task->next;
		gp_mdos_cur_task->next=tcb;
		gp_mdos_cur_task=tcb;
	}
	
}

/*��ȡ��һ�������tcbָ��*/
void get_next_TCB(void)
{
	gp_mdos_cur_task=gp_mdos_cur_task->next;
}






