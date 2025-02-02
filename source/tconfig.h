#ifndef _T_CONFIG_H
#define _T_CONFIG_H

#include <stdint.h>

#define TINY_PRIO_COUNT   32  // �������ȼ�����
#define TINY_SLICE_MAX    10  // 100ms
#define TINY_IDLETASK_STACK_SIZE   1024 // �����ջ��С

#define TINYOS_TIMERTASK_STACK_SIZE  1024 // ��ʱ�������ջ��С
#define TINYOS_TIMERTASK_PRIO  1 // ��ʱ���������ȼ�

#define TINYOS_SYSTICK_MS 10  // 1 ticks = 10 ms


/* �ں˲ü����� �궨�� */
#define TINYOS_ENABLE_SEM 						1
#define TINYOS_ENABLE_MUTEX 					1
#define TINYOS_ENABLE_FLAGGROUP 			1
#define TINYOS_ENABLE_MBOX 						1
#define TINYOS_ENABLE_MEMBLOCK 				1
#define TINYOS_ENABLE_TIMER 					1
#define TINYOS_ENABLE_CPUUSAGE_STAT 	1
#define TINYOS_ENABLE_HOOKS						1


#endif
