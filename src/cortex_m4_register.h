
#ifndef __CORTEX_M4_REGISTER_H__
#define __CORTEX_M4_REGISTER_H__

#include <stdint.h>

#define SYSHND_CTRL (*(volatile uint32_t*)  (0xE000ED24u))
// Memory Management Fault Status Register
#define NVIC_MFSR   (*(volatile uint8_t*) (0xE000ED28u))
// Bus Fault Status Register
#define NVIC_BFSR   (*(volatile uint8_t*) (0xE000ED29u))
// Usage Fault Status Register
#define NVIC_UFSR   (*(volatile uint16_t*)(0xE000ED2Au))
// Hard Fault Status Register
#define NVIC_HFSR   (*(volatile uint32_t*)  (0xE000ED2Cu))
// Debug Fault Status Register
#define NVIC_DFSR   (*(volatile uint32_t*)  (0xE000ED30u))
// Bus Fault Manage Address Register
#define NVIC_BFAR   (*(volatile uint32_t*)  (0xE000ED38u))
// Auxiliary Fault Status Register
#define NVIC_AFSR   (*(volatile uint32_t*)  (0xE000ED3Cu))

#define SCB_ICSR   (*(volatile uint32_t*)  (0xE000ED04))
#define SCB_SHPR3	(*(volatile uint32_t*)  (0xE000ED20))



#define SYST_CSR	(*(volatile uint32_t*)  (0xE000E010))	//control state
#define SYST_RVR	(*(volatile uint32_t*)  (0xE000E014))	//reload val
#define SYST_CVR	(*(volatile uint32_t*)  (0xE000E018))	//current val


#define pr_NVIC_ISER   ((volatile uint32_t*)  (0xE000E100))
#define pr_NVIC_ICER   ((volatile uint32_t*)  (0xE000E180))

#define pr_NVIC_ISPR   ((volatile uint32_t*)  (0xE000E200))
#define pr_NVIC_ICPR   ((volatile uint32_t*)  (0xE000E280))

#define pr_NVIC_IABR   ((volatile uint32_t*)  (0xE000E300))

#define pr_NVIC_IPR   ((volatile uint32_t*)  (0xE000E400))



#endif