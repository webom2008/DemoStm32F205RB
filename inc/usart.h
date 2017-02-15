#ifndef _USART_H
#define _USART_H

#include "stm32f2xx.h"

extern void USART_Configuration(void);
extern void USART_NVIC_Config(void);
extern void Uart_Server(void);
extern void Uart1_SendByte(char byte);
extern void Uart1_SendString(char *pString);

#endif /*_USART_H*/
