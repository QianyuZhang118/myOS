#ifndef _TMUTEX_H
#define _TMUTEX_H

#include "tEvent.h"

/* 互斥信号量结构定义 */ 
typedef struct _tMutex
{
	// 事件控制块
	tEvent event;
	
	// 锁定计数器
	uint32_t lockedCount;
	
	// 所有者
	tTask * owner;
	
	// 所有者的原始优先级
	uint32_t ownerOriginalPrio;
	
}tMutex;	


/* 互斥信号量查询结构定义 */ 
typedef struct _tMutexInfo
{
    // 等待的任务数量
    uint32_t taskCount;
    // 拥有者任务的优先级
    uint32_t ownerPrio;
    // 继承优先级(当前优先级)
    uint32_t inheritedPrio;
    // 当前信号量的拥有者
    tTask * owner;
    // 锁定次数
    uint32_t lockedCount;
} tMutexInfo;


/* 互斥信号量初始化函数 */
void tMutexInit(tMutex * mutex);

/* 等待互斥信号量函数 */
uint32_t tMutexWait(tMutex * mutex, uint32_t waitTicks);

/* 互斥信号量无等待获取函数（也叫无阻塞） */
uint32_t tMutexNoWaitGet(tMutex * mutex);

/* 通知/释放互斥信号量函数 */
uint32_t tMutexNotify(tMutex * mutex);

/* 删除互斥信号量函数 */
uint32_t tMutexDestroy(tMutex * mutex);

/* 互斥信号量状态查询 */
void tMutexGetInfo(tMutex * mutex, tMutexInfo * info);

#endif
