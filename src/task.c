
#include <stdint.h>
#include "task.h"

#include "cortex_m4_register.h"




volatile struct  xtos_task_struct  *gp_xtos_cur_task;
volatile struct  xtos_task_struct  *gp_xtos_next_task;

#define head_asm_frame		__asm volatile(
#define tail_asm_frame		);




void xtos_start (void)
{

	SCB_SHPR3=0xff<<16;

	SYST_RVR=160000;
	SYST_CVR=0;

	SYST_CSR=(0x01<<2)|(0x01<<1);
	SYST_CSR=SYST_CSR&(~(0x01<<0));

	SCB_SHPR3=0x01<<24;

	SYST_CSR=SYST_CSR|0x01;

}


void xtos_distroy_task() {

    while(1){}
}


void xtos_create_task(struct xtos_task_struct * tcb, xtos_task task, uint32 * stk) {
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
    *(--pstk) = (uint32)xtos_distroy_task;; // R14 (LR)
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
	tcb->saved=0;
}


void xtos_pendsv_handler(void)
{
	uint32_t psp_tmp=0;
	uint32_t *pr_psp;
	uint32_t val=0;
	uint32_t i=0;
	void * pr_task_tmp;
	
/*关中断*/
	head_asm_frame
	"cpsid i\n"
	tail_asm_frame
	
//获取当前的psp
	head_asm_frame
	"mrs r0,psp\n"
	"mov %0,r0\n"
	:"+r"(psp_tmp)
	tail_asm_frame

	pr_psp=(uint32_t *)psp_tmp;

	if(psp_tmp==1)//如果为0,则表示首次调用
	{
			pr_psp--;
			__asm(
			"mov %0,r4\n"
			 :"+r"(val)
			);
			*pr_psp=val;

			pr_psp--;
			__asm(
			"mov %0,r5\n"
			 :"+r"(val)
			);
			*pr_psp=val;

			pr_psp--;
			__asm(
			"mov %0,r6\n"
			 :"+r"(val)
			);
			*pr_psp=val;

			pr_psp--;
			__asm(
			"mov %0,r7\n"
			 :"+r"(val)
			);
			*pr_psp=val;


			pr_psp--;
			__asm(
			"mov %0,r8\n"
			 :"+r"(val)
			);
			*pr_psp=val;


			pr_psp--;
			__asm(
			"mov %0,r9\n"
			 :"+r"(val)
			);
			*pr_psp=val;


			pr_psp--;
			__asm(
			"mov %0,r10\n"
			 :"+r"(val)
			);
			*pr_psp=val;

			pr_psp--;
			__asm(
			"mov %0,r11\n"
			 :"+r"(val)
			);
			*pr_psp=val;	

			gp_xtos_cur_task->pTopOfStack=pr_psp;
	}


	
	gp_xtos_cur_task=gp_xtos_next_task;

	
	/*下一个任务的sp地址*/
	pr_psp=gp_xtos_cur_task->pTopOfStack;

	head_asm_frame
	"ldr r11,[%0]\n"
	"add %0,%0,0x04\n"

	"ldr r10,[%0]\n"
	"add %0,%0,0x04\n"

	"ldr r9,[%0]\n"
	"add %0,%0,0x04\n"

	"ldr r8,[%0]\n"
	"add %0,%0,0x04\n"

	"ldr r7,[%0]\n"
	"add %0,%0,0x04\n"

	"ldr r6,[%0]\n"
	"add %0,%0,0x04\n"

	"ldr r5,[%0]\n"
	"add %0,%0,0x04\n"

	"ldr r4,[%0]\n"
	"add %0,%0,0x04\n"

	"msr psp,%0\n"			//更新psp栈指针
	"orr lr,lr,#0x04\n"

	
	:"+r"(pr_psp)
	tail_asm_frame

		

	//开中断
	head_asm_frame
	"orr lr,lr,0x04\n"		//通过该设置,在退出中断后,使用psp指针
	"cpsie i\n"
	"bx lr\n"
	tail_asm_frame




}




