#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tlib.h"
#include "tconfig.h"
#include "tEvent.h"
#include "tTask.h"
#include "tSem.h"
#include "tMbox.h"
#include "tMemBlock.h"
#include "tFlagGroup.h"
#include "tMutex.h"
#include "tTimer.h"
#include "tHooks.h"
#include "myTask.h"


//  1s 的tick数量
#define TICKS_PER_SEC (1000 / TINYOS_SYSTICK_MS)

/* 错误码枚举结构 */
typedef enum _error
{
    tErrorNoError = 0,
		tErrorTimeOut,
		tErrorResourceUnavalable, 
		tErrorDel, 
		tErrorResourceFull, 
		tErrorOwner, 
}tError;

extern tTask *currentTask; 	// 指向当前任务的指针
extern tTask *nextTask;		// 指向下一任务的指针

void tTaskRunFirst(void);
void tTaskSwitch(void);


/* 临界区保护相关函数（开关中断） */
uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);


/* 调度器相关函数 */
void tTaskSchedInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);
void tTaskSchedRdy(tTask * task);
void tTaskSchedUnRdy(tTask * task);
void tTaskSched(void); 


/* 延时相关函数 */
void tTimeTaskWait(tTask * task, uint32_t ticks);
void tTimeTaskWakeUp(tTask * task);
void tTaskSystemTickHandler(void);
void tTaskDelay(uint32_t delay);


/* SysTick初始化函数, 设置系统时钟节拍 */
void SysTick_init(uint32_t ms);


/* 任务删除 */
void tTimeTaskRemove(tTask * task);  // 将任务从延时队列中删除
void tTaskSchedRemove(tTask * task); // 将任务从优先级队列中删除

/* CPU使用率获取的接口函数 */
float tCpuUsageGet(void);


#endif
