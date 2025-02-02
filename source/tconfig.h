#ifndef _T_CONFIG_H
#define _T_CONFIG_H

#include <stdint.h>

#define TINY_PRIO_COUNT   32  // 任务优先级数量
#define TINY_SLICE_MAX    10  // 100ms
#define TINY_IDLETASK_STACK_SIZE   1024 // 任务堆栈大小

#define TINYOS_TIMERTASK_STACK_SIZE  1024 // 定时器任务堆栈大小
#define TINYOS_TIMERTASK_PRIO  1 // 定时器任务优先级

#define TINYOS_SYSTICK_MS 10  // 1 ticks = 10 ms


/* 内核裁剪控制 宏定义 */
#define TINYOS_ENABLE_SEM 						1
#define TINYOS_ENABLE_MUTEX 					1
#define TINYOS_ENABLE_FLAGGROUP 			1
#define TINYOS_ENABLE_MBOX 						1
#define TINYOS_ENABLE_MEMBLOCK 				1
#define TINYOS_ENABLE_TIMER 					1
#define TINYOS_ENABLE_CPUUSAGE_STAT 	1
#define TINYOS_ENABLE_HOOKS						1


#endif
