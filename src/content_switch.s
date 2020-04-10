


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
content_save:

	ldr r2,=gp_xtos_cur_task
	ldr r1,[r2]
	add r3,r1,#4	//saved flag address
	ldr r0,[r3]		//saved flag
	cbz r0,content_load

	
//	ldr r0,[r1]		//stack address

	mrs r0,psp		//read psp addr
	subs r0,r0,#0x20
	stm  r0,{r4-r11}

	str r0,[r1]		//save new stack addr


	//switch task tcb
	ldr r1,=gp_xtos_cur_task
	ldr	r3,[r1]
	ldr r2,=taskA
	sub r0,r3,r2
	cbz r0,slect_tasb
	str r2,[r1]
	b content_load
slect_tasb:
	ldr r2,=taskB
	str r2,[r1]

content_load:
	ldr r0,=taskA
	add r0,r0,#4
	mov r1,#1
	str r1,[r0]
	ldr r0,=taskB
	add r0,r0,#4
	mov r1,#1
	str r1,[r0]

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






