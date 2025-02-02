#include "tSem.h"
#include "tinyOS.h"

#if TINYOS_ENABLE_SEM == 1

/* 信号量初始化 */
void tSemInit(tSem * sem, uint32_t startCount, uint32_t maxCount)
{
	tEventInit(&sem->event, tEventTypeSem);
	
	sem->maxCount = maxCount;
	
	if(maxCount == 0)  // 最大计数值为0，说明计数值没有限制
	{
		sem->count = startCount;
	}
	else
	{
		sem->count = (startCount > maxCount) ? maxCount : startCount;
	}
}


/* 函数tSemWait用于等待信号量 */
// sem：指向信号量结构体的指针
// waitTicks：等待的时钟周期数
uint32_t tSemWait(tSem * sem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查信号量计数是否大于0
    if (sem->count > 0)
    {
        // 如果大于0的话，消耗掉一个，然后正常退出
        --sem->count;
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else // 如果计数等于0，则任务进入等待状态
    {
        // 然后将任务插入事件队列中
        tEventWait(&sem->event, currentTask, (void *)0, tEventTypeSem, waitTicks);
        tTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        // 当由于等待超时或者计数可用时，执行会返回到这里，然后取出等待结构
        return currentTask->waitEventResult;
    }
}


/* 函数tSemNotify用于通知信号量 */
void tSemNotify(tSem * sem)
{
    uint32_t status = tTaskEnterCritical();

    // 检查是否有任务等待
    if (tEventWaitCount(&sem->event) > 0)
    {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask * task = tEventWakeUp(&sem->event, (void *)0, tErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    else
    {
        // 如果没有任务等待的话，增加计数
        ++sem->count;

        // 如果这个计数超过了最大允许的计数，则限制为最大值
        if ((sem->maxCount != 0) && (sem->count > sem->maxCount))
        {
            sem->count = sem->maxCount;
        }
    }
    tTaskExitCritical(status);
}

/* tSemNoWaitGet 信号量的无等待获取函数
（与tSemWait区别：当计数为零时，直接退出，而不是将任务插入等待队列） */
uint32_t tSemNoWaitGet(tSem * sem)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查信号量计数是否大于0
    if (sem->count > 0)
    {
        // 如果大于0的话，消耗掉一个，然后正常退出
        --sem->count;
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        // 否则，返回资源不可用
        tTaskExitCritical(status);
        return tErrorResourceUnavalable;
    }
}


/* 函数tSemDestroy用于销毁信号量 */ 
uint32_t tSemDestroy(tSem * sem)
{
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&sem->event, (void *)0, tErrorDel);
    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}


/* 状态查询接口 函数tSemGetInfo用于获取信号量的信息 */ 
void tSemGetInfo(tSem * sem, tSemInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->count = sem->count;  // 拷贝当前信号量计数
    info->maxCount = sem->maxCount;  // 拷贝最大计数
    info->taskCount = tEventWaitCount(&sem->event);  // 等待任务数

    tTaskExitCritical(status);
}

#endif

