#include "tHooks.h"

#if TINYOS_ENABLE_HOOKS == 1

void tHooksCpuIdle(void)  // 空闲任务运行时
{

}

void tHooksSysTick(void)  // 时钟节拍中断发生时
{

}

void tHooksTaskSwitch(tTask * from, tTask * to) // 任务切换时
{

}

void tHooksTaskInit(tTask * task)  // 任务初始化时
{

}

#endif


