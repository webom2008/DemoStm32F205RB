/**
  ******************************************************************************
  * @file    app.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   This file provides all the Application firmware functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/ 
#include "includes.h"
#include "system_tick.h"

#define UNUSED(x)           ((void)(x))

#define DBG_APP_INFO
#ifdef DBG_APP_INFO
#define DBG(fmt, arg...)        udprintf("\r\n[USB]App.c:"fmt, ##arg)
#else
#define DBG(fmt, arg...)        do{}while(0)
#endif

/** @defgroup APP_HID_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */ 

/** @defgroup APP_HID_Private_Functions
  * @{
  */ 
static void uart_Task(void *pvParameters)
{
    int count = 0;
    UNUSED(pvParameters);

    while(1)
    {
        DBG("count=%d", count++);
        vTaskDelay(500/portTICK_RATE_MS);
    }
}

/**
  * @brief  Program entry point
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
  this is done through SystemInit() function which is called from startup
  file (startup_stm32fxxx_xx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32fxxx.c file
  */

    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4  );

    USART_Configuration();

    xTaskCreate( uart_Task, "UART", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL );
    vTaskStartScheduler();
    return 0;
} 

#ifdef USE_FULL_ASSERT
/**
* @brief  assert_failed
*         Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  File: pointer to the source file name
* @param  Line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  DBG("\r\nWrong parameters value: file %s on line %d", file, line);
  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
