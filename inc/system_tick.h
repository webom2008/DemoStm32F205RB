#ifndef __SYSTEM_TICK_H_
#define __SYSTEM_TICK_H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f2xx.h"

extern void SysTick_Init(void);
extern void SysTick_Incremental(void);  
extern u32 SysTick_Get(void);
extern void SysTick_Start(void);
extern s32 SysTick_Compare(u32 u32Tick1, u32 u32Tick2);
extern u32 IsOnTime(const u32 u32Target);
extern u32 IsOverTime(const u32 u32Base, const u32 u32Duration);


#ifdef __cplusplus
}
#endif

#endif /*__TIMER_H*/

