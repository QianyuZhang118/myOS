#include "tinyOS.h"

#if TINYOS_ENABLE_TIMER == 1

static tList tTimerHardList; // 由中断服务程序处理的定时器列表
static tList tTimerSoftList;  // 由定时器任务处理的定时器列表

static tSem tTimerProtectSem;  // 定时器/其他任务访问tTimerSoftList 的信号量
static tSem tTimerTickSem;   // 用于在中断服务程序中通知定时器任务有中断发生的 信号量


static tTask tTimerTask;  // 定义定时器任务
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE]; // 定义定时器堆栈

/* 定时器队列处理函数，遍历列表中的各个定时器 */
static void tTimerCallFuncList(tList * timerList)
{
    tNode * node;
    // 遍历定时器列表
    for (node = timerList->headNode.nextNode; node!= &(timerList->headNode); node = node->nextNode)
    {
        tTimer * timer = tNode_parent(node, tTimer, linkNode);
				
				 // 定时器的递减计数为0，即定时到达了,此时需要调用一次定时器回调函数
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0)) 
        {
            timer->state = tTimerRunning; // 设置定时器状态为正在运行
           
            timer->timerFunc(timer->arg); // 调用定时器的回调函数，并传入参数
           
            timer->state = tTimerStarted; // 设置定时器状态为已启动
            
            if (timer->durationTicks > 0) // 判断该定时器是否是需要周期性处理
            {
                timer->delayTicks = timer->durationTicks; // 重置延迟计数值为周期值
            }
            else
            {
                // 如果是一次性定时，从定时器列表中移除该定时器
                tList_remove(timerList, &timer->linkNode);
               
                timer->state = tTimerStopped; // 设置定时器状态为已停止
            }
        }
    }
}


/* 定义定时器任务函数 */
static void tTimerSoftTask(void * param)
{
	for(;;)
	{
		tSemWait(&tTimerTickSem, 0);  // 阻塞等待来自系统时钟节拍处理函数中向tTimerTickSem发出的通知
		
		tSemWait(&tTimerProtectSem, 0); // 等待tTimerSoftList信号量，申请对tTimerSoftList的访问权
		
		tTimerCallFuncList(&tTimerSoftList); // 调用定时器队列处理函数，遍历tTimerSoftList定时器列表
		
		tSemNotify(&tTimerProtectSem); // 释放信号量
	}
}


/* 在中断中调用，通知tTimerTickSem信号量，相应中断已经发生，tTimerSoftTask 函数得以继续执行*/
void tTimerModuleTickNotify(void)
{
    uint32_t status = tTaskEnterCritical();
	
		// 由于该函数在中断中调用，所以可以同时处理tTimerHardList定时器列表
    tTimerCallFuncList(&tTimerHardList); 

    tTaskExitCritical(status);
   
    tSemNotify(&tTimerTickSem); // 通知/释放信号量
}


/* 定时器模块初始化 */
void tTimerModuleInit(void)
{
    // 初始化定时器列表
    tList_init(&tTimerHardList);
    tList_init(&tTimerSoftList);
	
    // 初始化信号量
    tSemInit(&tTimerTickSem, 0, 0);
    tSemInit(&tTimerProtectSem, 1, 1);
}

/* 定时器任务初始化函数 （为实现CPU Usage功能，该函数从tTimerModuleInit中独立出来） */
void tTimerTaskInit(void)
{
    // 断言：定时器任务的优先级必须大于 (TINYOS_PRO_COUNT - 1)
    // 如果不满足条件，编译时会报错
#if TINYOS_TIMERTASK_PRIO >= (TINY_PRIO_COUNT - 1)
		#error "The priority of timer task must be greater than (TINYOS_PRO_COUNT - 1)"
#endif
	
    // 初始化定时器任务
    tTaskInit(&tTimerTask, tTimerSoftTask, (void *)0, TINYOS_TIMERTASK_PRIO, tTimerTaskStack, sizeof(tTimerTaskStack));
}


/* 定时器初始化函数 */
void tTimerInit(tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                void (*timerFunc)(void * arg), void * arg, uint32_t config)
{
    tNode_init(&timer->linkNode); // 初始化定时器的链表结点
   
    timer->startDelayTicks = delayTicks; // 设置初次启动延迟的ticks数
   
    timer->durationTicks = durationTicks; // 设置周期定时的周期tick数
  
    timer->timerFunc = timerFunc;  // 设置定时回调函数
   
    timer->arg = arg; // 设置传递给回调函数的参数
  
    timer->config = config;  // 设置定时器配置参数
	
    // 如果初始启动延迟为0，则使用周期值作为当前定时递减计数值
    if (delayTicks == 0)
    {
        timer->delayTicks = durationTicks;
    }
    else
    {
        // 否则，使用初次启动延迟值作为当前定时递减计数值
        timer->delayTicks = timer->startDelayTicks;
    }
    // 设置定时器状态为已创建
    timer->state = tTimerCreated;
}

/* 定时器启动函数，加入对应的定时器列表 */
void tTimerStart(tTimer * timer)
{
    switch (timer->state)  // 判断定时器状态
    {
			// 定时器只有处于已创建或已停止状态下，才允许启动，其他状态下直接退出
        case tTimerCreated:
        case tTimerStopped:
					
            // 设置定时器的当前定时递减计数值
            // 如果有初次启动延迟，则使用该值，否则使用周期值
            timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
           
            timer->state = tTimerStarted; // 设置定时器状态为已启动
				
            // 根据定时器类型加入相应的定时器列表
            if (timer->config & TIMER_CONFIG_TYPE_HARD) // 由中断服务程序处理的定时器
            {
                uint32_t status = tTaskEnterCritical();
                tList_addLast(&tTimerHardList, &timer->linkNode);  // 将定时器加入定时器列表
                tTaskExitCritical(status);
            }
            else  // 由定时器任务处理的定时器
            {
								// 由于此处代码足够简单，因此不需要互斥信号量的特性
								// tTimerSoftList被设定为只能由定时器任务或其他任务访问，中断无法访问，因此无需使用临界区保护
								// 所以，这里使用计数为1的信号量
                tSemWait(&tTimerProtectSem, 0);
                tList_addLast(&tTimerSoftList, &timer->linkNode);
                tSemNotify(&tTimerProtectSem);
							
								// 注意，这里假定tTimerStart和tTimerStop都是由任务调用的，
								// 如果想要由中断调用，则不能使用信号量，需使用临界区保护了
            }
            break;
						
        default:
            break;
    }
}


/* 定时器停止函数 */
void tTimerStop(tTimer * timer)
{
    // 根据定时器的当前状态进行操作
    switch (timer->state)
    {
			// 定时器只有处于已启动或正在运行状态下，才允许停止，其他状态下直接退出
        case tTimerStarted:
        case tTimerRunning:
            // 判断定时器类型，然后从相应的延时列表中移除
            if (timer->config & TIMER_CONFIG_TYPE_HARD)  // 由中断服务程序处理的定时器
            {
                uint32_t status = tTaskEnterCritical();
                tList_remove(&tTimerHardList, &timer->linkNode);   // 从定时器列表中移除定时器
                tTaskExitCritical(status);
            }
            else
            {
                tSemWait(&tTimerProtectSem, 0);  // 等待信号量，获取tTimerSoftList的占有权
                tList_remove(&tTimerSoftList, &timer->linkNode);
                tSemNotify(&tTimerProtectSem);
            }
           
            timer->state = tTimerStopped; // 设置定时器状态为已停止
            break;
        default:
            break;
    }
}

/* 定时器删除函数 */
void tTimerDestroy(tTimer * timer)
{
	tTimerStop(timer);  // 调用定时器停止函数即可
	
	timer->state = tTimerDestroyed;  // 状态标记为销毁
}

/* 定时器状态查询函数 */
void tTimerGetInfo(tTimer * timer, tTimerInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    info->startDelayTicks = timer->startDelayTicks;

    info->durationTicks = timer->durationTicks;

    info->timerFunc = timer->timerFunc;

    info->arg = timer->arg;

    info->config = timer->config;

    info->state = timer->state;

    tTaskExitCritical(status);
}

#endif
