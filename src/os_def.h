
#ifndef __OS_DEF_H__
#define __OS_DEF_H__

#include "os_errCode.h"
#include "task.h"

#define os_true 			1
#define os_false			0
#define os_null_pr			0xffffffff

#if 0

#define os_fun_save_err(member,val)		((gOS_err_code.member.err=val)?val:0)
#define os_fun_save_time(member)		(gOS_err_code.member.time=get_OS_sys_count())	

#define os_fun_err(member,val) 			((os_fun_save_time(member))?((os_fun_save_err(member,val))):0)





#define os_fun_val(member,val)	(val==os_true)?os_true:(os_fun_err(member,val))

#endif



#define close_all_interruct() 	do{__asm volatile ("cpsid i" : : : "memory");}while(0)
#define open_all_interruct() 	do{__asm volatile ("cpsie i" : : : "memory");}while(0)

#define input_critical_area()		close_all_interruct()
#define exit_critical_area()		open_all_interruct()


#endif








