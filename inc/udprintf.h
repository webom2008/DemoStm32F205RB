/******************************************************************************

   Copyright (C), 2005-2015, CVTE.

 ******************************************************************************
  File Name     : udprintf.h
  Version       : Initial Draft
  Author        : qiuweibo
  Created       : 2016/7/19
  Last Modified :
  Description   : udprintf.c header file
  Function List :
  History       :
  1.Date        : 2016/7/19
    Author      : qiuweibo
    Modification: Created file

******************************************************************************/
#ifndef __UDPRINTF_H__
#define __UDPRINTF_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifdef UDPRINTF_C
#define UDPRINT_EXT
#else
#define UDPRINT_EXT     extern
#endif

UDPRINT_EXT void udprintf(const char* fmt, ...);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UDPRINTF_H__ */
