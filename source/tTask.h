#ifndef _TTASK_H
#define _TTASK_H

#include <stdint.h>
#include "tlib.h"

/* 低16位 */
#define TINYOS_TASK_STATE_RDY 				0
#define TINYOS_TASK_STATE_DELAY 			(1 << 1)  // 延时标志位
#define TINYOS_TASK_STATE_SUSPEND			(1 << 2)  // 挂起标志位
#define TINYOS_TASK_STATE_DELETED			(1 << 3)  // 删除标志位

/* 高16位 */
#define TINYOS_TASK_WAIT_MASK					(0xFF << 16)  // 等待标志位（掩码） 清除高16位值

/* 前向引用，用于解决tTask.h和tEvent.h相互依赖的问题 */
struct _tEvent;


/* 任务堆栈数据类型 */
typedef uint32_t tTaskStack;

/* 任务结构 */
typedef struct _tTask
{
	tTaskStack *stack;  // 任务栈指针
	uint32_t * stackBase;  // 堆栈起始地址
	uint32_t stackSize; // 堆栈总的大小
	
	
	uint32_t delayTicks;  // 软定时器计数（用于延时）
	tNode delayNode; // 用于将任务添加到延时队列中 的节点
	tNode linkNode; // 用于构成就绪表中同优先级队列 的节点
	uint32_t prio;  // 任务优先级字段
	uint32_t state; // 标志任务是否处于延时状态
	uint32_t slice;  // 时间片计数值
	uint32_t spspendCount; // 任务被挂起的次数
	
	void (*clean) (void * param);  // 删除任务的回调函数
	void * cleanParam;
	uint8_t requestDeleteFlag;  // 请求删除标志位
	
	struct _tEvent * waitEvent; // 任务正在等待哪一个事件控制块
	// (前向引用只适合与指针类型，因为编译器知道指针类型的大小，而结构体类型只靠声明无法确定大小)
	
	void * eventMsg; // 
	uint32_t waitEventResult; // 等待结果
	
	uint32_t waitFlagsType;  // 请求类型，用于事件标志组
	uint32_t eventFlags;  // 请求事件标志，用于事件标志组
	
}tTask;

/* 任务状态查询结构 */
typedef struct __tTaskInfo
{
		uint32_t prio;
		uint32_t delayTicks;
		uint32_t state;
		uint32_t slice;
		uint32_t suspendCount;
	
		uint32_t stackSize;  // 堆栈总的大小
		uint32_t stackFree;  // 堆栈空闲大小
}tTaskInfo;

/* 任务初始化函数 */
void tTaskInit(tTask * task, void(*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, uint32_t size);
void tInitApp(void);

/* 任务挂起与唤醒 */
void tTaskSuspend(tTask * task);
void tTaskWakeUp(tTask * task);

/* 任务删除 */
void tTaskSetCleanCallFunc(tTask * task, void (* clean)(void * param), void * param);  // 设置任务的清理回调函数
void tTaskForceDelete(tTask * task); // 强制删除函数
void tTaskRequestDelete(tTask * task);  // 请求删除函数接口（设置请求删除标记）
uint8_t tTaskIsRequestDelete(void);  // 检查当前任务请求删除函数（返回请求删除标记）
void tTaskDeleteSelf(void);  // 删除任务自己

/* 任务状态查询 */
void tTaskGetInfo(tTask * task, tTaskInfo * Info);

#endif

