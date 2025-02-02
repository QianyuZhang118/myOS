#include "tinyOS.h"

#if TINYOS_ENABLE_MEMBLOCK == 1


/* 存储块初始化函数 */ 
// memBlock: 指向要初始化的tMemBlock结构体的指针
// memStart: 存储块区域的起始地址
// blockSize: 每个存储块的大小
// blockCnt: 存储块的总数
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt)
{
    // 将传入的内存起始地址转换为uint8_t类型指针，方便后续按字节操作
    uint8_t * memBlockStart = (uint8_t *)memStart;
	
    // 计算存储块区域的结束地址，通过起始地址加上所有存储块的总大小得到
    uint8_t * memBlockEnd = memBlockStart + blockSize * blockCnt;

    // 每个存储块需要放置链接指针（用于构建存储块列表等数据结构相关操作），所以其空间至少要比tNode大
    // 这样做不会减少实际用户可用空间，只是为了满足内部数据结构存储需求
    // 如果当前设置的存储块大小小于tNode类型的大小，无法满足要求，直接返回不进行后续初始化
    if (blockSize < sizeof(tNode))
    {
        return;
    }

    // 初始化存储块的事件控制块
    tEventInit(&memBlock->event, tEventTypeMemBlock);
		
    // 初始化存储块的起始地址
    memBlock->memStart = memStart;
		
    // 初始化每个存储块的的大小
    memBlock->blockSize = blockSize;
		
    // 初始化存储块的数量上限
    memBlock->maxCount = blockCnt;

    // 初始化存储块列表，创建一个空的列表结构，用于后续存储块的组织管理
    tList_init(&memBlock->blockList);
		
    // 遍历存储块区域，对每个存储块进行初始化操作
    while (memBlockStart < memBlockEnd)
    {
        // 将每一个存储块的开始区域作为一个节点，对其初始化
        tNode_init((tNode *)memBlockStart);
			
        // 将初始化后的存储块节点插入存储块列表的末尾
        tList_addLast(&memBlock->blockList, (tNode *)memBlockStart);
			
        // 将指针向后移动一个存储块的大小，指向下一个待初始化的存储块
        memBlockStart += blockSize;
    }
}


/* 获取存储块函数 */
uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (tList_count(&memBlock->blockList) > 0)
    {
        // 如果有的话，取出一个
        *mem = (uint8_t *)tList_removeFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else  
    {
        // 如果没有空闲存储块，则将任务插入等待队列中
        tEventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        // 当切换回来时，从tTask中取出获得的消息
        *mem = currentTask->eventMsg;

        // 取出等待结果
        return currentTask->waitEventResult;
    }
}


/* 获取存储块函数, 无等待 */
uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, uint8_t ** mem)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (tList_count(&memBlock->blockList) > 0)
    {
        // 如果有的话，取出一个
        *mem = (uint8_t *)tList_removeFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else  
    {
        // 如果没有空闲存储块，直接退出
        tTaskExitCritical(status);

				// 返回资源不可用
        return tErrorResourceUnavalable;
    }
}

/* 释放存储块函数 */
void tMemBlockNotify(tMemBlock * memBlock, uint8_t * mem)
{
    uint32_t status = tTaskEnterCritical();

    // 检查是否有任务等待
    if (tEventWaitCount(&memBlock->event) > 0)
    {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask * task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    else
    {
        // 如果没有任务等待的话，将存储块插入到队列中
        tList_addLast(&memBlock->blockList, (tNode *)mem);
    }
    tTaskExitCritical(status);
}

/* 删除存储块函数 */
uint32_t tMemBlockDestroy(tMemBlock * memBlock)
{
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务，返回被移除的任务数量
    uint32_t count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);
    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，若有则执行一次任务调度
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}

#endif
