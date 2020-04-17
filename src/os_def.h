
#ifndef __OS_DEF_H__
#define __OS_DEF_H__


#define os_true 			1
#define os_false			0
#define os_null_pr			0xffffffff


#define close_all_interruct() 	do{__asm volatile ("cpsid i" : : : "memory");}while(0)
#define open_all_interruct() 	do{__asm volatile ("cpsie i" : : : "memory");}while(0)

#define input_critical_area()		close_all_interruct()
#define exit_critical_area()		open_all_interruct()


#endif








