#include "tinyOS.h"

#if TINYOS_ENABLE_MBOX == 1

/* 邮箱初始化函数 */
void tMboxInit(tMbox * mbox, void ** msgBuffer, uint32_t maxCount)
{
    tEventInit(&mbox->event, tEventTypeMbox);  // 初始化事件控制块
    mbox->msgBuffer = msgBuffer;  // 消息缓冲区指针
    mbox->maxCount = maxCount;  // 最大消息数量
    mbox->read = 0; // 读索引
    mbox->write = 0;  // 写索引
    mbox->count = 0;  // 当前消息数量
}

/* 获取消息函数（任务要从邮箱获取消息） */
uint32_t tMboxWait(tMbox * mbox, void ** msg, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查消息计数是否大于0
    if (mbox->count > 0)
    {
        // 如果大于0的话，取出一个消息
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
			
        // 同时读索引前移，如果超出边界则回绕
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
				
        return tErrorNoError;

    }
    else
    {
        // 然后将任务插入事件队列中
        tEventWait(&mbox->event, currentTask, (void *)0, tEventTypeSem, waitTicks);
        tTaskExitCritical(status);
        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();
			
        // 当切换回来时，从tTask中取出获得的消息
        *msg = currentTask->eventMsg;
			
        // 取出等待结果
        return currentTask->waitEventResult;
    }
}


/* 获取消息函数（不等待，如果没有消息则直接退出） */
uint32_t tMboxNoWaitGet(tMbox * mbox, void ** msg)
{
		uint32_t status = tTaskEnterCritical();

    // 首先检查消息计数是否大于0
    if (mbox->count > 0)
    {
        // 如果大于0的话，取出一个消息
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
			
        // 同时读索引前移，如果超出边界则回绕
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
				
        return tErrorNoError;
    }
    else  // 如果没有消息
    {
        tTaskExitCritical(status);
        // 返回资源不可用
        return tErrorResourceUnavalable;
    }
}


/* 消息通知函数（消息要发送给邮箱）
@ notifyOption：这个选项表示消息是否是更高优先级消息。是，则会被优先存储*/
uint32_t tMboxNotify(tMbox * mbox, void * msg, uint32_t notifyOption)
{
    uint32_t status = tTaskEnterCritical();

    // 检查是否有任务等待
    if (tEventWaitCount(&mbox->event) > 0)
    {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask * task = tEventWakeUp(&mbox->event, (void *)msg, tErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    else
    {
        // 判断邮箱是否已满，邮箱满则退出
        if (mbox->count >= mbox->maxCount)
        {
            tTaskExitCritical(status);
            return tErrorResourceFull;
        }
				
				// 如果没有任务等待的话，将消息插入到缓冲区中
        // 可以选择将消息插入到头，这样后面任务获取的时候，优先获取该消息
        if (notifyOption & tMBOXSendFront)  // 高优先级消息
        {
            if (mbox->read <= 0)
            {
                mbox->read = mbox->maxCount - 1;
            }
            else
            {
                --mbox->read;
            }
            mbox->msgBuffer[mbox->read] = msg;  // 通过read指针写入
        }
        else  // 普通消息
        {
            mbox->msgBuffer[mbox->write++] = msg;  // 写索引write后移，并写入msg
            if (mbox->write >= mbox->maxCount)
            {
                mbox->write = 0;
            }
        }
        mbox->count++;  // 增加消息计数
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}


/* 清空邮箱函数(清空缓存区) */
void tMboxFlush(tMbox * mbox)
{
    uint32_t status = tTaskEnterCritical();

    // 如果队列中没有任务等待，说明邮箱可能有消息，需要清空
    if (tEventWaitCount(&mbox->event) == 0)
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }
		// 如果有任务在等待，则缓存区一定是空的，无需清除

    tTaskExitCritical(status);
}

/* 删除邮箱函数(移出等待的所有任务) */
uint32_t tMboxDestroy(tMbox * mbox)
{
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&mbox->event, (void *)0, tErrorDel);
    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}


/* 邮箱信息查询函数 */
void tMboxGetInfo(tMbox * mbox, tMboxInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = tEventWaitCount(&mbox->event);

    tTaskExitCritical(status);
}

#endif

