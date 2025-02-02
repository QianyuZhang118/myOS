#ifndef _TEVENT_H
#define _TEVENT_H

#include "tlib.h"
#include "tTask.h"

/* 事件类型枚举结构 */
typedef enum _tEventType
{
    tEventTypeUnknown, 
		tEventTypeSem, 
		tEventTypeMbox, 
		tEventTypeMemBlock, 
		tEventTypeFlagGroup, 
		tEventTypeMutex,
}tEventType;


/* 事件控制块结构 */
typedef struct _tEvent
{
	tEventType type;
	
	tList waitList;  // 等待队列
}tEvent;


/* 事件控制块初始化函数 */
void tEventInit(tEvent * event, tEventType type);

/* 事件控制块等待接口 */
void tEventWait(tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout);

/* 将任务从事件控制块中唤醒 */
tTask * tEventWakeUp(tEvent * event, void * msg, uint32_t result);

/* 将指定任务从事件控制块中唤醒 */
tTask * tEventWakeUpTask(tEvent * event, tTask * task, void * msg, uint32_t result);

/* 将任务从事件控制块中强制移除 */
void tEventRemoveTask(tTask * task, void * msg, uint32_t result);


/* 清空事件控制块等待队列（返回删除前等待队列中有多少任务） */
uint32_t tEventRemoveAll(tEvent * event, void * msg, uint32_t result);

/* 获取事件控制块中等待的任务数量 */
uint32_t tEventWaitCount(tEvent * event);

#endif



