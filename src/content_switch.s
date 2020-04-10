


/*-----------------------------------------------------------------------------------------------------*/

.global OSTest
.global SysTick_Handler

.type   OSTest, function
.type   SysTick_Handler, function


 .extern OSTest
 .extern SysTick_Handler


.text
.balign 4
.thumb
.syntax unified



.thumb_func


SysTick_Handler:

	ldr r0,=#0xE000ED04
	ldr r1,=#0x1000000
	str r1,[r0]

	mrs r0,psp
	cbnz r0,content_save
	
	mrs r1,control	//设置psp和msp
	orr r1,r1,#0x02
	msr control,r1
	
content_save:

	mrs r0,psp
	cbz r0,content_load


	mrs r0,psp		//read psp addr
	subs r0,r0,#0x20
	stm  r0,{r4-r11}

	ldr r2,=gp_xtos_cur_task
	ldr r1,[r2]		//获取到当前任务放栈地址的空间地址

	str r0,[r1]		//save new stack addr

/*在中断中,lr代表的不是返回地址.由于调用c函数,在进入子函数后,lr会发生变化,变为当前的位置.即使从子函数退出后,也不会变回来.因此在进入子函数前,lr需要保存一下*/
	mov r0,lr
	ldr r1,=get_next_TCB
	blx r1
	mov lr,r0

content_load:

	ldr r0,=gp_xtos_cur_task
	ldr r0,[r0]
	ldr r0,[r0]		//get the cur task stack

	ldm r0,{r4-r11}
	adds r0,r0,#0x20

	msr psp,r0
	orr lr,lr,#0x04

	cpsie i

	
	bx lr

	nop



OSTest:
	nop
	nop
	add r0,r0,#1
	bx lr

	.end






