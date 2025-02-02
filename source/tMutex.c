#include "tinyOS.h"

#if TINYOS_ENABLE_MUTEX== 1

/* 互斥信号量的初始化 */
void tMutexInit(tMutex * mutex)
{
    // 初始化互斥信号量的事件控制块，设置事件类型为互斥信号量类型
    tEventInit(&mutex->event, tEventTypeMutex);
	
    // 将已被锁定的次数初始化为0
    mutex->lockedCount = 0;
	
    // 将拥有者指针初始化为空指针
    mutex->owner = (tTask *)0;
	
    // 将拥有者原始优先级设置为TINY_PRIO_COUNT（一个特定的常量值）
    mutex->ownerOriginalPrio = TINY_PRIO_COUNT;
}


/* 等待互斥信号量函数 */
uint32_t tMutexWait(tMutex * mutex, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();
	
    if (mutex->lockedCount <= 0)   // 如果互斥信号量未被锁定（lockedCount <= 0）
    {
        mutex->owner = currentTask;  // 设置当前任务占有该信号量

        mutex->ownerOriginalPrio = currentTask->prio; // 记录当前任务的原始优先级

        mutex->lockedCount++; // 锁定次数加1
			
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else  // 如果已经被锁定
    {
        if (mutex->owner == currentTask)  // 如果是自己锁定的
        {
            mutex->lockedCount++;  // 锁定次数加1

            tTaskExitCritical(status);
            return tErrorNoError;
        }
        else // 如果不是自己锁定的
        {
            if (currentTask->prio < mutex->owner->prio)  // 如果是低优先级任务锁定的（注意数字越小，优先级越高）
            {
                tTask * owner = mutex->owner;
                
                if (owner->state == TINYOS_TASK_STATE_RDY) // 如果拥有者任务处于就绪状态
                {
                    tTaskSchedUnRdy(owner); // 将拥有者任务设置移出就绪队列
                   
                    owner->prio = currentTask->prio; // 将拥有者任务的优先级设置为当前任务的优先级（提高优先级，使其尽快完成）
                   
                    tTaskSchedRdy(owner); // 然后将拥有者任务重新加入就绪队列
                }
                else
                {
                    // 如果任务处于非就绪状态，比如延时、等待等状态，直接更改其优先级即可
                    owner->prio = currentTask->prio;
                }
            }
           
            tEventWait(&mutex->event, currentTask, (void *)0, tEventTypeMutex, waitTicks); // 将当前任务插入等待队列中

            tTaskExitCritical(status);
           
            tTaskSched(); // 执行事件调度，切换到其他任务
						
            // 切换回该任务时，返回当前任务的等待事件结果
            return currentTask->waitEventResult;
        }
    }
}

/* 互斥信号量无等待获取函数（也叫无阻塞） */
uint32_t tMutexNoWaitGet(tMutex * mutex)
{
    uint32_t status = tTaskEnterCritical();
	
    if (mutex->lockedCount <= 0) // 如果互斥信号量未被锁定（lockedCount <= 0）
    {
        mutex->owner = currentTask; // 设置当前任务占有该信号量
       
        mutex->ownerOriginalPrio = currentTask->prio; // 记录当前任务的原始优先级

        mutex->lockedCount++; // 锁定次数加1

        tTaskExitCritical(status);

        return tErrorNoError;
    }
    else // 如果已经被锁定
    {
        if (mutex->owner == currentTask) // 如果是自己锁定的
        {
           
            mutex->lockedCount++; // 锁定次数加1
           
            tTaskExitCritical(status);

            return tErrorNoError;
        }
				
        // 如果不是自己锁定的，直接退出
        tTaskExitCritical(status);
				
        // 返回资源不可用错误标识
        return tErrorResourceUnavalable;
    }
}


/* 通知/释放互斥信号量函数 */
uint32_t tMutexNotify(tMutex * mutex)
{
    uint32_t status = tTaskEnterCritical();
    if (mutex->lockedCount <= 0) // 如果互斥信号量未被锁定，直接退出
    {
        tTaskExitCritical(status);
        return tErrorNoError;
    }
		
    if (mutex->owner!= currentTask) // 如果不是由当前任务，则返回错误。因为互斥信号量只能由自身占有和释放
    {
        tTaskExitCritical(status);
        return tErrorOwner;
    }
   
    if (--mutex->lockedCount > 0) // 锁定次数减1，如果仍大于零，则退出
    {
        tTaskExitCritical(status);
        return tErrorNoError;
    }
		
		// 如果等于0，则需要释放占有者
		// 如果占有者的原始优先级和当前优先级不一致，则说明发生优先级继承，需要恢复原优先级
    if (mutex->ownerOriginalPrio!= mutex->owner->prio) 
    {
        if (mutex->owner->state == TINYOS_TASK_STATE_RDY) // 如果占有者任务处于就绪状态
        {
            tTaskSchedUnRdy(mutex->owner); // 将占有者任务移出就绪队列
          
            currentTask->prio = mutex->ownerOriginalPrio;  // 将占有者任务的优先级恢复为原始优先级
           
            tTaskSchedRdy(mutex->owner); // 将占有者任务重新加入就绪队列
        }
        else  // 如果占有者任务处于非就绪状态
        {
            // 直接将占有者任务的优先级恢复为原始优先级
            currentTask->prio = mutex->ownerOriginalPrio;
        }
    }
		
    // 如果有任务在等待该互斥信号量
    if (tEventWaitCount(&mutex->event) > 0)
    {
        tTask * task = tEventWakeUp(&mutex->event, (void *)0, tErrorNoError); // 唤醒等待队列中的第一个任务（可改进）
			
        mutex->owner = task;  // 设置新的拥有者为被唤醒的任务
       
        mutex->ownerOriginalPrio = task->prio; // 记录新拥有者的优先级
      
        mutex->lockedCount++;  // 锁定次数加1
       
        if (task->prio < currentTask->prio) // 如果被唤醒任务的优先级高于当前任务优先级
        {
            tTaskSched(); // 切换到高优先级任务
        }
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}

/* 删除互斥信号量函数 */
uint32_t tMutexDestroy(tMutex * mutex)
{
    uint32_t count = 0;
    uint32_t status = tTaskEnterCritical();
   
    if (mutex->lockedCount > 0) // 检查信号量是否已被锁定
    {
        // 是否有发生优先级继承，如果发生，需要恢复拥有者的原优先级
        if (mutex->ownerOriginalPrio!= mutex->owner->prio)
        {
            if (mutex->owner->state == TINYOS_TASK_STATE_RDY)  // 如果任务处于就绪状态
            {
                // 任务处于就绪状态时，更改任务在就绪表中的位置
                tTaskSchedUnRdy(mutex->owner);
                currentTask->prio = mutex->ownerOriginalPrio;
                tTaskSchedRdy(mutex->owner);
            }
            else
            {
                // 其它状态，只要修改优先级
                currentTask->prio = mutex->ownerOriginalPrio;
            }
        }
        // 清空事件控制块等待队列中的任务，返回删除的任务数量
        count = tEventRemoveAll(&mutex->event, (void *)0, tErrorDel);
				
        // 清空过程中可能有任务就绪，执行一次调度
        if (count > 0)
        {
            tTaskSched();
        }
    }
    tTaskExitCritical(status);
    return count;
}

/* 互斥信号量状态查询 */
void tMutexGetInfo(tMutex * mutex, tMutexInfo * info)
{
    uint32_t status = tTaskEnterCritical();
	
    // 拷贝需要的信息
    info->taskCount = tEventWaitCount(&mutex->event);
    info->ownerPrio = mutex->ownerOriginalPrio;
	
		if(mutex->owner != (tTask *)0)  // 判断当前有没有拥有者
		{
			info->inheritedPrio = mutex->owner->prio;  // 有，则正常记录
		}
		else
		{
			info->inheritedPrio = TINY_PRIO_COUNT; // 没有，则设置非法值
		}
    
    info->owner = mutex->owner;
    info->lockedCount = mutex->lockedCount;
	
    tTaskExitCritical(status);
}

#endif

