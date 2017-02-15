/******************************************************************************

   Copyright (C), 2005-2015, CVTE.

 ******************************************************************************
  File Name     : system_tick.c
  Version       : Initial Draft
  Author        : qiuweibo
  Created       : 2016/11/19
  Last Modified :
  Description   : system tick
  Function List :
              IsOnTime
              IsOverTime
              SysTick_Compare
              SysTick_Get
              SysTick_Incremental
  History       :
  1.Date        : 2016/11/19
    Author      : qiuweibo
    Modification: Created file

******************************************************************************/
#include "system_tick.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/
static vu32 System_Tick = 0;

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
void SysTick_Init(void)
{
      /* Setup SysTick Timer for 1 msec interrupts */
      if (SysTick_Config(SystemCoreClock / 1000))
      { 
          /* Capture error */ 
          while (1);
        } 
      SysTick_Start();
}

void SysTick_Start(void)
{
    System_Tick = 0;
}

void SysTick_Incremental(void)
{
    System_Tick++;
}

/*********************************************************************************************************
** Function name:           SysTick_Get
** Descriptions:            SysTick_Get
** input parameters:        none
** output parameters:       none
** Returned value:          none
*********************************************************************************************************/
u32 SysTick_Get(void)
{
    return System_Tick;
}

/*********************************************************************************************************
** Function name:           SysTick_Compare
** Descriptions:            SysTick_Compare
** input parameters:        none
** output parameters:       none
** Returned value:          none
*********************************************************************************************************/
s32 SysTick_Compare(u32 u32Tick1, u32 u32Tick2) // Interval must less than 0x80000000
{
    if(u32Tick1 == u32Tick2)
    {
        return 0;       // ==
    }
    else if(u32Tick1 < u32Tick2)
    {
        if((u32Tick2 - u32Tick1) < 0x80000000)
        {
            return -1;  // <
        }
        else            // Tick Out
        {
            return 1;   // >
        }
    }
    else    // u32Tick1 > u32Tick2
    {
        if((u32Tick1 - u32Tick2) < 0x80000000)
        {
            return 1;   // >
        }
        else            // Tick Out
        {
            return -1;  // <
        }
    }
}

/*********************************************************************************************************
** Function name:           IsOnTime
** Descriptions:            IsOnTime
** input parameters:        none
** output parameters:       none
** Returned value:          0: less than target time     
                            1: on time or over time with target
*********************************************************************************************************/
u32 IsOnTime(const u32 u32Target)
{
    if(SysTick_Compare(SysTick_Get(), u32Target) == -1) 
    {
        return 0;
    }

    return 1;
}

/*********************************************************************************************************
** Function name:           IsOverTime
** Descriptions:            IsOverTime
** input parameters:        none
** output parameters:       none
** Returned value:          0: less than target time
                            1: over the target time
*********************************************************************************************************/
u32 IsOverTime(const u32 u32Base, const u32 u32Duration)    
{
    u32 u32Target;

    u32Target = u32Base + u32Duration;

    if(SysTick_Compare(SysTick_Get(), u32Target) == 1)
    {
        return 1;
    }

    return 0;
}



/**************************** (C) COPYRIGHT CVTE *****************************/


