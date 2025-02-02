#ifndef _MYTASK_H
#define _MYTASK_H

#include "tinyOS.h"

// LED初始化
void LedInit(void);

// 串口初始化
void UartInit(void);

void USART5_IRQHandler(void);

// 串口打印输出
int myPrintf(uint32_t usart_periph, uint8_t data);

void delay(uint32_t ms);

#endif

