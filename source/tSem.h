#ifndef _TSEM_H
#define _TSEM_H

#include "tEvent.h"

/* 信号量结构定义 */
typedef struct _tSem
{
	tEvent event;  // tSem同时是一个tEvent
	uint32_t count; // 当前计数值
	uint32_t maxCount;  // 最大计数值
}tSem;

/* 信号量的状态结构定义 */
typedef struct _tSemInfo
{
    // 当前信号量的计数
    uint32_t count;
    // 信号量允许的最大计数
    uint32_t maxCount;
    // 当前等待的任务数
    uint32_t taskCount;
} tSemInfo;

/* 信号量初始化 */
void tSemInit(tSem * sem, uint32_t startCount, uint32_t maxCount);

/* 函数tSemWait用于等待信号量 */
uint32_t tSemWait(tSem * sem, uint32_t waitTicks);


/* 函数tSemNotify用于通知信号量 */
void tSemNotify(tSem * sem);

/* tSemNoWaitGet 信号量的无等待获取函数  */
uint32_t tSemNoWaitGet(tSem * sem);

/* 状态查询接口 函数tSemGetInfo用于获取信号量的信息 */ 
void tSemGetInfo(tSem * sem, tSemInfo * info);

/* 函数tSemDestroy用于销毁信号量 */ 
uint32_t tSemDestroy(tSem * sem);


#endif

