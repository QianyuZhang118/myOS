#ifndef _THOOKS_H
#define _THOOKS_H

#include "tinyOS.h"

/* ���Ӻ��� */
void tHooksCpuIdle(void);  // ������������ʱ

void tHooksSysTick(void);  // ʱ�ӽ����жϷ���ʱ

void tHooksTaskSwitch(tTask * from, tTask * to); // �����л�ʱ

void tHooksTaskInit(tTask * task);  // �����ʼ��ʱ

#endif


