#ifndef _MYTASK_H
#define _MYTASK_H

#include "tinyOS.h"

// LED��ʼ��
void LedInit(void);

// ���ڳ�ʼ��
void UartInit(void);

void USART5_IRQHandler(void);

// ���ڴ�ӡ���
int myPrintf(uint32_t usart_periph, uint8_t data);

void delay(uint32_t ms);

#endif

