#ifndef _THOOKS_H
#define _THOOKS_H

#include "tinyOS.h"

/* 钩子函数 */
void tHooksCpuIdle(void);  // 空闲任务运行时

void tHooksSysTick(void);  // 时钟节拍中断发生时

void tHooksTaskSwitch(tTask * from, tTask * to); // 任务切换时

void tHooksTaskInit(tTask * task);  // 任务初始化时

#endif


