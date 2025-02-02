#include "tinyOS.h"
#include "tTask.h"

/* 事件控制块初始化函数 */
void tEventInit(tEvent * event, tEventType type)
{
	event->type = tEventTypeUnknown;  // 初始化事件类型
	tList_init(&(event->waitList));  // 初始化等待队列
}


/* 事件控制块等待接口 
@ msg: 用于存放数据类型和收到的数据
@ state: 等待的状态
@ timeout: 超时时间
*/
void tEventWait(tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout)
{
	uint32_t status = tTaskEnterCritical();
	
	task->state |= state << 16;  // 等待状态(高16位存放等待事件状态)
	task->waitEvent = event; // 等待事件
	task->eventMsg = msg;  // 等待事件消息
	task->waitEventResult = tErrorNoError;  // 等待结果
	
	tTaskSchedUnRdy(task);  // 从就绪队列中移除
	// 注意延时队列和等待队列不一样，延时队列用于delay,等待队列用于等待某个事件
	// 规则：先等待的任务先被唤醒，所以插入队列尾部
	tList_addLast(&event->waitList, &task->linkNode); // 插入等待队列的尾部
	
	
	if(timeout)  // 如果设置了超时时间，则将任务插入延时队列
	{
		tTimeTaskWait(task, timeout);
	}
	
	tTaskExitCritical(status);
}

/* 将任务从事件控制块中唤醒 */
tTask * tEventWakeUp(tEvent * event, void * msg, uint32_t result)
{
	tNode * node;
	tTask * task = (tTask * )0;
	uint32_t status = tTaskEnterCritical();
	
	
	// 取出事件控制块的等待队列中的第一个任务，并将其移出该队列
	if((node = tList_removeFirst(&event->waitList)) != (tNode *)0)
	{
		task = tNode_parent(node, tTask, linkNode);
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;  // 等待事件消息
		task->waitEventResult = result;  // 等待结果
		task->state &=  ~TINYOS_TASK_WAIT_MASK; // 清除高16位，即等待标志位
		
		if(task->delayTicks != 0)
		{
			tTimeTaskWakeUp(task); // 如果任务有延时，则强制将其从延时队列中删除
		}
		
		tTaskSchedRdy(task);  // 将任务插入到就绪队列中
		
	}
	
	tTaskExitCritical(status);
	
	return task;
}

/* 将指定任务从事件控制块中唤醒 */
tTask * tEventWakeUpTask(tEvent * event, tTask * task, void * msg, uint32_t result)
{
//	tNode * node;
	uint32_t status = tTaskEnterCritical();
	
	tList_remove(&event->waitList, &task->linkNode);
	task->waitEvent = (tEvent *)0;
	task->eventMsg = msg;  // 等待事件消息
	task->waitEventResult = result;  // 等待结果
	task->state &=  ~TINYOS_TASK_WAIT_MASK; // 清除高16位，即等待标志位
	
	if(task->delayTicks != 0)
	{
		tTimeTaskWakeUp(task); // 如果任务有延时，则强制将其从延时队列中删除
	}
	
	tTaskSchedRdy(task);  // 将任务插入到就绪队列中
	
	
	tTaskExitCritical(status);
	
	return task;
}


/* 将任务从事件控制块中强制移除 */
void tEventRemoveTask(tTask * task, void * msg, uint32_t result)
{
		uint32_t status = tTaskEnterCritical();
	
		tList_remove(&task->waitEvent->waitList, &task->linkNode);
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;  // 等待事件消息
		task->waitEventResult = result;  // 等待结果
		task->state &=  ~TINYOS_TASK_WAIT_MASK; // 清除高16位，即等待标志位
		
	
		tTaskExitCritical(status);
}


/* 清空事件控制块等待队列（返回删除前等待队列中有多少任务） */
uint32_t tEventRemoveAll(tEvent * event, void * msg, uint32_t result)
{
	tNode * node;
	
	uint32_t cnt = 0;
	
	uint32_t status = tTaskEnterCritical();
	
	cnt = tList_count(&event->waitList);  // 记录等待队列中的任务数量
	
	while((node = tList_removeFirst(&event->waitList)) != (tNode *)0) // 依次移除队列中的头部任务
	{
		tTask * task = tNode_parent(node, tTask, linkNode);
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;  // 等待事件消息
		task->waitEventResult = result;  // 等待结果
		task->state &=  ~TINYOS_TASK_WAIT_MASK; // 清除高16位，即等待标志位
		
		if(task->delayTicks != 0)
		{
			tTimeTaskWakeUp(task);  // 如果任务有延时，则将其移出延时队列
		}
		
		tTaskSchedRdy(task);  // 加入就绪队列
	}
	
	tTaskExitCritical(status);
	
	return cnt;
	
}

/* 获取事件控制块中等待的任务数量 */
uint32_t tEventWaitCount(tEvent * event)
{
	uint32_t cnt = 0;
	
	uint32_t status = tTaskEnterCritical();
	
	cnt = tList_count(&event->waitList);
	
	tTaskExitCritical(status);
	
	return cnt; 
}






