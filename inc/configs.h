/******************************************************************************

   Copyright (C), 2005-2016, CVTE.

 ******************************************************************************
  File Name     : configs.h
  Version       : Initial Draft
  Author        : qiuweibo
  Created       : 2016/12/14
  Last Modified :
  Description   : configs.h header file
  Function List :
  History       :
  1.Date        : 2016/12/14
    Author      : qiuweibo
    Modification: Created file

******************************************************************************/

#ifndef __CONFIGS_H__
#define __CONFIGS_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define CFG_ON          1
#define CFG_OFF         0

#define CFG_UART1_PRINTF            CFG_ON
#define CFG_UART1_TXRX_DMA          CFG_OFF /* Bug */
#define CFG_UART1_TIM9_HANDLE       CFG_OFF /* Bug */

// Params Define ...
#define CFG_PRINTF_BUF              1024

// Debug Infomation ....
#define CFG_USB_DBG_INFO            CFG_ON


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CONFIGS_H__ */

