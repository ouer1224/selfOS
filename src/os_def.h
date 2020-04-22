
#ifndef __OS_DEF_H__
#define __OS_DEF_H__

#include "os_errCode.h"
#include "task.h"

#define os_true 			1
#define os_false			0
#define os_null_pr			0xffffffff

#define os_kernel_save_line(member)			(gOS_kernel_err_code.member.line=__LINE__)
#define os_kernel_save_err(member,val)		((gOS_kernel_err_code.member.err=val)?val:0)
#define os_kernel_save_err_line(member,val)	((os_kernel_save_line(member))?((os_kernel_save_err(member,val))):0)
#define os_kernel_save_time(member)			(gOS_kernel_err_code.member.time=get_OS_sys_count())
#define os_kernel_err(member,val) 			((os_kernel_save_time(member))?((os_kernel_save_err_line(member,val))):0)
#define os_kernel_val(member,val)			((val==os_true)?os_true:(os_kernel_err(member,val)))


#define __isnbitseted_array(array,N)		((((uint8_t *)array)[(N)/8]&(0x01<<(N%8)))?1:0)				
#define __clearnbit_array(array,N)			(((uint8_t *)array)[(N)/8]&=(~(0x01<<(N%8))))
#define __setnbit_array(array,N)			(((uint8_t *)array)[(N)/8]|=((0x01<<(N%8))))


#define close_all_interruct() 	do{__asm volatile ("cpsid i" : : : "memory");}while(0)
#define open_all_interruct() 	do{__asm volatile ("cpsie i" : : : "memory");}while(0)

#define input_critical_area()		close_all_interruct()
#define exit_critical_area()		open_all_interruct()


#endif








