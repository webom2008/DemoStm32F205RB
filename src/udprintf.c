/******************************************************************************

   Copyright (C), 2005-2015, CVTE.

 ******************************************************************************
  File Name     : udprintf.c
  Version       : Initial Draft
  Author        : qiuweibo
  Created       : 2016/7/19
  Last Modified :
  Description   : udprintf
  Function List :
  History       :
  1.Date        : 2016/7/19
    Author      : qiuweibo
    Modification: Created file

******************************************************************************/
#include "configs.h"

#include <string.h>
#ifdef USED_STDIO_PRINTF
#include <stdarg.h>
#include <stdio.h>
#else
#include <stdarg.h>
#endif


#define UDPRINTF_C
#include "udprintf.h"
#undef UDPRINTF_C


/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/
extern void Uart1_SendByte(char byte);
extern void Uart1_SendString(char *pString);

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define PRINTF_BUFFER_SIZE          128


#define DEV_SEND_BYTE(x)        Uart1_SendByte((x))
#define DEV_SEND_STRING(x)      Uart1_SendString((x))

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/
static va_list argptr;

#ifdef USED_STDIO_PRINTF
static char strbuf[PRINTF_BUFFER_SIZE];
#endif
/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#ifndef USED_STDIO_PRINTF

static const char sign[] = 
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
};

static void print_int(int num, int mode, int flag)
{
    if (0 == num)
    {
        if (0 == flag)
        {
            DEV_SEND_BYTE('0');
            return;
        }
        else
        {
            return;
        }
    }

    print_int(num / mode,mode,1);
    DEV_SEND_BYTE(sign[num%mode]);
}

static void print_float(float num)
{
    int part = (int)(num / 1);
    print_int(part, 10, 0);

    DEV_SEND_BYTE('.');

    part = (int)(num*1000000) - part*1000000;
    print_int(part, 10, 0);
}


static void check_type(const char type)
{
    switch(type)
    {
    case 'd':
    {
        int num = va_arg(argptr, int);
        if (num < 0)
        {
            DEV_SEND_BYTE('-');
            num = num * (-1);
        }
        print_int(num, 10, 0);
    }
    break;
    case 'c':
    {
        char ch = (char)va_arg(argptr, int);
        DEV_SEND_BYTE(ch);
    }
    break;
    case 's':
    {
        char *str = va_arg(argptr, char *);
        DEV_SEND_STRING(str);
    }
    break;
    case 'f':
    {
        float num = (float)va_arg(argptr, double);
        if (num < 0)
        {
            DEV_SEND_BYTE('-');
            num = num * (-1);
        }
        else
        {
            print_float(num);
        }
    }
    break;
    case 'x':
    {
        int num = va_arg(argptr, int);
        print_int(num, 16, 0);
    }
    break;
    default:
    {
        DEV_SEND_BYTE('%');
        DEV_SEND_BYTE(type);
    }
    break;
    }
}

#endif


void udprintf(const char* fmt, ...)
{
    if (NULL == fmt)
    {
        return;
    }
    
    va_start(argptr, fmt); 
#ifdef USED_STDIO_PRINTF
    memset(strbuf, 0x00, PRINTF_BUFFER_SIZE);
    
    vsnprintf(strbuf, sizeof(strbuf), fmt, argptr);
    va_end(argptr);
    
    DEV_SEND_STRING(strbuf);
#else
    while('\0' != *fmt)
    {
        while(('%' != *fmt) && ('\0' != *fmt))
        {
            DEV_SEND_BYTE(*fmt);
            fmt++;
        }
        
        if ('\0' != *fmt)
        {
            fmt++;

            if ('\0' != *fmt)
            {
                check_type(*fmt);
                fmt++;
            }
            else
            {
                DEV_SEND_BYTE(*(fmt - 1));
            }
        }
    }
    
    va_end(argptr);
#endif
}

/**************************** (C) COPYRIGHT CVTE *****************************/
