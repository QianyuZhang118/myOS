#ifndef _TTIMER_H
#define _TTIMER_H

#include "tEvent.h"

#define TIMER_CONFIG_TYPE_HARD		(1 << 0)  // 在中断服务中处理的定时器
#define TIMER_CONFIG_TYPE_SOFT			(0 << 0)  // 在任务中处理的定时器

/* 定时器状态枚举值 */
typedef enum _tTimerState
{
	tTimerCreated, //已创建
	tTimerStarted,  // 已启动
	tTimerRunning,  // 正在运行
	tTimerStopped,  // 已停止
	tTimerDestroyed,  // 已销毁
}tTimerState;

/* 定时器状态查询结构定义 */
typedef struct _tTimerInfo
{
    uint32_t startDelayTicks; // 初次启动延迟后的ticks数
   
    uint32_t durationTicks; // 周期定时时的周期tick数
   
    void (*timerFunc)(void * arg); // 定时回调函数
   
    void * arg; // 传递给回调函数的参数
   
    uint32_t config; // 定时器配置参数
   
    tTimerState state; // 定时器状态
} tTimerInfo;


/* 定时器结构体定义 */
typedef struct _tTimer
{
    tNode linkNode; // 链表结点
   
    uint32_t startDelayTicks; // 初始延时tick数
   
    uint32_t durationTicks; // 周期tick数
   
    uint32_t delayTicks; // 当前定时计数值
   
    void (*timerFunc)(void * arg); // 定时回调函数
   
    void * arg; // 传递给回调函数的参数

    uint32_t config; // 定时器配置参数
   
    tTimerState state; // 定时器状态
} tTimer;


/* 定义定时器任务函数 */
static void tTimerSoftTask(void * param);

/* 定时器队列处理函数，遍历列表中的各个定时器 */
static void tTimerCallFuncList(tList * timerList);

/* 在中断中调用，通知tTimerTickSem信号量，相应中断已经发生，tTimerSoftTask 函数得以继续执行*/
void tTimerModuleTickNotify(void);

/* 定时器模块初始化 */
void tTimerModuleInit(void);

/* 定时器任务初始化函数 （为实现CPU Usage功能，该函数从tTimerModuleInit中独立出来） */
void tTimerTaskInit(void);

/* 定时器初始化函数 */
void tTimerInit(tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                void (*timerFunc)(void * arg), void * arg, uint32_t config);

/* 定时器启动函数，加入对应的定时器列表 */
void tTimerStart(tTimer * timer);

/* 定时器停止函数 */
void tTimerStop(tTimer * timer);

/* 定时器删除函数 */
void tTimerDestroy(tTimer * timer);

/* 定时器状态查询函数 */
void tTimerGetInfo(tTimer * timer, tTimerInfo * info);
#endif
