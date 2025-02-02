#include "tinyOS.h"

#if TINYOS_ENABLE_FLAGGROUP == 1

/* 函数tFlagGroupInit用于初始化标志组 */ 
void tFlagGroupInit(tFlagGroup * flagGroup, uint32_t flags)
{
    // 初始化事件控制块，设置事件类型为标志组类型
    tEventInit(&flagGroup->event, tEventTypeFlagGroup);
	
    // 将传入的标志值赋给标志组的当前事件标志成员
    flagGroup->flags = flags;
}

/* 辅助函数：检查相应的事件标志是否满足，如果满足，可设置是否消耗对应标志 
@ type：等待的事件类型（即tFlagGroup.h中的一系列宏定义）
@ flag：传入时为任务希望等待的事件标志，返回时为检查出的事件标志
*/
static uint32_t tFlagGroupCheckAndConsume(tFlagGroup * flagGroup, uint32_t type, uint32_t * flag)
{
	uint32_t srcFlag = *flag;
	
	// 确定任务等待的是哪种事件类型
	uint32_t isSet = type & TFLAGGROUP_SET;
	uint32_t isAll = type & TFLAGGROUP_ALL;
	uint32_t isConsume = type & TFLAGGROUP_CONSUME;
	
	// 计算实际的事件标志
	uint32_t calFlag = isSet ? (flagGroup->flags & srcFlag) : (~flagGroup->flags & srcFlag);
	
	//将计算结果与请求的事件标志对比
	if(((isAll != 0) && (calFlag == srcFlag)) || ((isAll == 0) && (calFlag != 0)) )
	{
		// 匹配成功
		
			if(isConsume)  // 是否要消耗相应标志
			{
				if(isSet)
				{
					flagGroup->flags &= ~srcFlag; // 相应标志位置零
				}
				else
				{
					flagGroup->flags |= srcFlag; // 相应标志位置零
				}
			}
			
			*flag = calFlag;  // 返回实际标志位
			return tErrorNoError;
	}
	
	*flag = calFlag;
	return tErrorResourceUnavalable;
}

/* 事件标志组等待操作 
@ flagGroup ：指定事件标志组的指针
@ waittype ：等待的事件类型
@ requestFlag：等待哪些标志
@ resultFlag：等待到的事件标志
@ waitTicks：等待超时
*/
uint32_t tFlagGroupWait(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag,
                        uint32_t * resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;
    uint32_t status = tTaskEnterCritical();
	
    // 检查并消费标志，返回结果
    result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    // 如果事件标志不满足条件
    if (result!= tErrorNoError)
    {
        // 设置当前任务的等待标志类型
        currentTask->waitFlagsType = waitType;
			
        // 设置当前任务的等待事件标志
        currentTask->eventFlags = requestFlag;
			
        // 将任务插入到等待队列中
        tEventWait(&flagGroup->event, currentTask, (void *)0, tEventTypeFlagGroup, waitTicks);
			
        tTaskExitCritical(status);
			
        tTaskSched();
			
        // 获取任务唤醒后的事件标志
        *resultFlag = currentTask->eventFlags;
			
        // 获取任务等待结果
        result = currentTask->waitEventResult;
    }
    else
    {
        // 如果事件标志满足条件，设置结果标志
        *resultFlag = flags;
		
        tTaskExitCritical(status);
    }
    // 返回结果，错误码
    return result;
}

/* 事件标志组获取/检查操作 */
uint32_t tFlagGroupNoWaitGet(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag)
{
    uint32_t flags = requestFlag;
    uint32_t status = tTaskEnterCritical();
	
    // 检查并消费标志，返回结果
    uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);

    tTaskExitCritical(status);
	
    // 设置结果标志
    *resultFlag = flags;
	
    // 返回结果
    return result;
}

/* 通知标志组事件 */
void tFlagGroupNotify(tFlagGroup * flagGroup, uint8_t isSet, uint32_t flags)
{
    tList * waitList;
    tNode * node;
    tNode * nextNode;
    uint8_t sched = 0;

    uint32_t status = tTaskEnterCritical();
	
    // 根据isSet的值设置或清除标志组的标志
    if (isSet)
    {
        flagGroup->flags |= flags; // 置1事件
    }
    else
    {
        flagGroup->flags &= ~flags; // 清0事件
    }
		
    // 获取等待列表的地址
    waitList = &flagGroup->event.waitList;
		
    // 遍历等待列表
    for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
    {
        tTask * task = tNode_parent(node, tTask, linkNode); // 获取任务地址
			
        uint32_t tFlags = task->eventFlags; // 获取任务的等待事件标志
			
        uint32_t result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &tFlags); // 检查当前标志是否满足
			
				nextNode = node->nextNode;
			
        // 如果当前标志满足条件
        if (result == tErrorNoError)
        {
            // 设置任务的事件标志
            task->eventFlags = flags;
					
            // 唤醒任务
            tEventWakeUpTask(&flagGroup->event, task, (void *)0, tErrorNoError);
            sched = 1;
        }
    }
    // 如果有任务就绪，执行一次调度
    if (sched)
    {
        tTaskSched();
    }
    // 退出临界区
    tTaskExitCritical(status);
}

/* 事件标志组删除 */
uint32_t tFlagGroupDestroy(tFlagGroup * flagGroup)
{
    // 进入临界区，防止多任务环境下的并发冲突
    uint32_t status = tTaskEnterCritical();
    // 清空标志组对应的事件控制块中的所有任务，返回被移除的任务数量
    uint32_t count = tEventRemoveAll(&flagGroup->event, (void *)0, tErrorDel);
    // 退出临界区
    tTaskExitCritical(status);
    // 如果在清空过程中有任务就绪，执行一次任务调度
    if (count > 0)
    {
        tTaskSched();
    }
    // 返回被移除的任务数量
    return count;
}

/* 事件标志组状态查询函数 */
void tFlagGroupGetInfo(tFlagGroup * flagGroup, tFlagGroupInfo * info)
{
    // 进入临界区
    uint32_t status = tTaskEnterCritical();
    // 拷贝当前的事件标志到info结构体中
    info->flags = flagGroup->flags;
    // 获取当前等待在该标志组事件上的任务数量，并拷贝到info结构体中
    info->taskCount = tEventWaitCount(&flagGroup->event);
    // 退出临界区
    tTaskExitCritical(status);
}

#endif

