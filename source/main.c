/*
	部署
*/

#include "tinyOS.h"

tTask *currentTask; 	// 指向当前任务的指针
tTask *nextTask;		// 指向下一任务的指针
tTask *idleTask;  // 空闲任务指针


tBitmap taskPrioBitmap; // 就绪位图
tList taskTable[TINY_PRIO_COUNT]; // 任务列表(就绪表)
tList tTaskDelayList;  // 延时队列

uint8_t schedLockCount;  // 调度锁计数器


/* 用于统计CPU Usage的计数值 */
uint32_t idleCount; 		// 过程计数值（递增）
uint32_t idleMaxCount;  // 最大计数值
uint32_t tickCount;  // 时钟节拍的发生次数

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

/* CPU使用率测量 初始化函数 */
static void initCpuUsageState(void);
	
/* 时钟节拍中断中调用的 CPU使用率统计函数，完成记录、统计、计算的功能 */
static void checkCpuUsage(void);
	
/* 实现空闲任务与时钟同步的函数 */
static void cpuUsageSyncWithSystick(void);

#endif



/* 在优先级位图中查找最高优先级（即可以立刻运行）任务函数 */
tTask * tTaskHighestReady(void)
{
	uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
	
	tNode * node = tList_first(&taskTable[highestPrio]);  // 取出同优先级队列的第一个节点
	
	tTask * task = tNode_parent(node, tTask, linkNode);  // 依据节点，找出其父结构即对应任务的地址
	
	return task;  // 返回任务
}


/* 调度器初始化函数 */
void tTaskSchedInit(void)
{
	int i;
	schedLockCount = 0;  // 调度锁初始化
	tBitmapInit(&taskPrioBitmap); // 优先级位图进行初始化
	for(i = 0; i < TINY_PRIO_COUNT; ++i)  // 初始化就绪表中的各个优先级列表
	{
		tList_init(&taskTable[i]);
	}
}

/* 上锁函数（禁止调度） */
void tTaskSchedDisable(void)
{
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount < 255)
	{
		schedLockCount++;
	}
	tTaskExitCritical(status);
}

/* 解锁函数（使能调度） */
void tTaskSchedEnable(void)
{
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount > 0)
	{
		if(--schedLockCount == 0)
		{
			tTaskSched();  // 如果调度锁计数变为0，执行调度
		}
	}
	tTaskExitCritical(status);
}

/*
任务插入就绪列表（任务延时完成后）
*/
void tTaskSchedRdy(tTask * task)
{
	tList_addFirst(&taskTable[task->prio], &(task->linkNode)); // 将任务按照其优先级插入就绪表中的对应优先级队列
	tBitmapSet(&taskPrioBitmap, task->prio); // 将位图中对应位置置1，表示任务就绪
}

/*
将任务移出就绪列表(将任务设置为非就绪状态)
*/
void tTaskSchedUnRdy(tTask * task)
{
	tList_remove(&taskTable[task->prio], &(task->linkNode));  // 将任务按照其优先级从就绪表中的对应优先级队列中删除
	
	if(tList_count(&taskTable[task->prio]) == 0)  // 当对应优先级队列中没有就绪任务时，相应位图清零
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

/*
将任务移出优先级队列(用于任务删除)
*/
void tTaskSchedRemove(tTask * task)
{
	tList_remove(&taskTable[task->prio], &(task->linkNode));
	
	if(tList_count(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}



/* 任务调度函数 */
void tTaskSched() 
{
	tTask * tempTask;
	uint32_t status = tTaskEnterCritical(); // 进入临界区保护
	
	if(schedLockCount > 0)  // 如果调度锁上锁，则禁止调度，直接返回
	{
		tTaskExitCritical(status);
		return;
	}
	
	tempTask = tTaskHighestReady();  // 从位图中得到当前最高优先级的就绪任务
	if(tempTask != currentTask)
	{
		nextTask = tempTask; // 指定该任务的下一个要运行的任务
		// tTaskSwitch();
		
#if TINYOS_ENABLE_HOOKS == 1

	tHooksTaskSwitch(currentTask, nextTask);  // 钩子函数
	
#endif
		
	}

	tTaskExitCritical(status);  // 退出临界区保护
	
	tTaskSwitch(); // 触发PendSV异常，切换任务
	
	/* tTaskSwitch()放在临界区保护内和外均可，这是因为：
		当中断重新使能时，如果 PendSV 标志位仍然被设置，处理器会在下一个合适的时机进入 PendSV 中断处理程序。 */
	 
	
}


/*
任务队列初始化函数
*/
void tTaskDelayListInit(void)
{
	tList_init(&tTaskDelayList);
}

/*
任务延时 函数（将要延时的任务插入延时队列）
*/
void tTimeTaskWait(tTask * task, uint32_t ticks)
{
	task->delayTicks = ticks;
	tList_addLast(&tTaskDelayList, &(task->delayNode));
	task->state |= TINYOS_TASK_STATE_DELAY;
}

/*
任务退出延时 函数（将任务移除延时队列）
*/
void tTimeTaskWakeUp(tTask * task)
{
	tList_remove(&tTaskDelayList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAY;
}

/*
将任务从延时队列中删除（用于任务删除）
*/
void tTimeTaskRemove(tTask * task)
{
	tList_remove(&tTaskDelayList, &(task->delayNode));
}

/* 时钟节拍计数值tickCount初始化 */
void tTimeTickInit()
{
	tickCount = 0;
}

/*
时钟节拍中断处理函数 
用于递减各个任务中的延时计数器
*/
void tTaskSystemTickHandler()
{
	tNode * node;
	
	//int i;
	uint32_t status = tTaskEnterCritical();
	
	// 扫描延时队列
	for(node = tTaskDelayList.headNode.nextNode; node!=&(tTaskDelayList.headNode); node = node->nextNode)
  {
		tTask * task = tNode_parent(node, tTask, delayNode);
		if(--task->delayTicks == 0)
		{
			if(task->waitEvent) // 检测任务是否在等待事件，这是因为在设置等待事件超时的情况下，任务会被加入延时队列
			{
				tEventRemoveTask(task, (void *)0, tErrorTimeOut);  // 当等待事件超时时，将任务从事件控制块的等待队列中强制移除
			}
			tTimeTaskWakeUp(task);  // 如果延时结束，移除延时队列，切换任务状态
			tTaskSchedRdy(task); // 将其插入就绪表
		}
	}
	
	if(--currentTask->slice == 0)  // 如果当前任务的时间片为零，则切换任务
	{
		if(tList_count(&taskTable[currentTask->prio]) > 0)  // 如果当前优先级还有就绪任务，则同优先级切换
		{
			tList_removeFirst(&taskTable[currentTask->prio]);  // 切换时，将优先级队列的第一个任务（当前任务）移除
			tList_addLast(&taskTable[currentTask->prio], &(currentTask->linkNode));  // 重新插入到优先级队列末尾
			
			currentTask->slice = TINY_SLICE_MAX; // 重新设置当前任务（已被放在队列最后）的时间片
		}
	}
	
	tickCount++; // 时钟节拍计数递增
	
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	checkCpuUsage();  // 检查CPU使用率
#endif
	
	tTaskExitCritical(status);

#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleTickNotify();  // 通知定时器
#endif
	
#if TINYOS_ENABLE_HOOKS == 1
	tHooksSysTick();  // 钩子函数
#endif
	
	tTaskSched();
}


#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

static float cpuUsage;  							// CPU使用率
static uint32_t enableCpuUsageState;  // 时钟同步标志位 0未同步 1已同步

/* CPU使用率测量 初始化函数 */
static void initCpuUsageState(void)
{
	idleCount = 0;
	idleMaxCount = 0;
	cpuUsage = 0.0f;  // 后缀f指定这个常量是 float 类型
	enableCpuUsageState = 0;
}

/* CPU使用率统计函数 时钟节拍中断中调用，完成记录、统计、计算的功能 */
static void checkCpuUsage(void)
{
	if(enableCpuUsageState == 0)  // 如果未同步
	{
		enableCpuUsageState = 1;  // 设置为已同步
		
		tickCount = 0;  // 时钟节拍计数归零
		
		return;
	}
	
	// 系统运行的前1s，全部运行空闲任务，以获得最大计数值。
	// 1s后，任务调度启动。
	
	if(tickCount == TICKS_PER_SEC)  // 如果从系统开始运行到此时，时间过去了1s
	{
		idleMaxCount = idleCount;  // 将当前计数值记为最大计数值
		
		idleCount = 0;  // 当前计数值归零
		
		tTaskSchedEnable();  // 完成最大计数值记录后，即可运行其他任务，开启调度锁
	}
	else if(tickCount % TICKS_PER_SEC == 0)  // 此后每隔1s，统计一次CPU Usage
	{
		cpuUsage = 100.0 - 100.0 * idleCount / idleMaxCount;  // 计算CPU使用率
		
		idleCount = 0;  // 当前计数值归零
	}
}

/* 实现空闲任务与时钟同步的函数 */
static void cpuUsageSyncWithSystick(void)
{
	while(enableCpuUsageState == 0)  // 如果标志位为0，原地等待
	{
		;;
	}
	// 时钟节拍发生时，enableCpuUsageState置为1 ,该函数退出，空闲任务才能继续向下运行
}

/* CPU使用率获取的接口函数 */
float tCpuUsageGet()
{
	float usage = 0; 
	
	uint32_t status = tTaskEnterCritical();
	
	usage = cpuUsage;  // 取值
	
	tTaskExitCritical(status);
	
	return usage;  // 返回
}

#endif

/* 空闲任务定义 */
tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

/* 空闲任务函数 */
void idleTaskEntry(void *param)
{
	tTaskSchedDisable();  // 为防止切换到其他任务，关闭调度锁（用于统计CPU Usage）
	
	tInitApp();  // 任务初始化 

#if TINYOS_ENABLE_TIMER == 1
	tTimerTaskInit();  // 定时器任务初始化
#endif
	
	SysTick_init(TINYOS_SYSTICK_MS);  // 时钟节拍初始化
	
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	cpuUsageSyncWithSystick();  // 任务与时钟节拍同步
#endif
	
	LedInit(); // LED初始化
	
	UartInit(); // 串口初始化
	
	for(;;)
	{
		uint32_t status = tTaskEnterCritical();
		
		idleCount++;  // 计数值加一
		
		tTaskExitCritical(status);
		
#if TINYOS_ENABLE_HOOKS == 1

	tHooksCpuIdle();  // 钩子函数
	
#endif
	}
}



int main()
{
	
	/* 调度锁初始化 */
	tTaskSchedInit();
	
	/* 延时列表初始化 */
	tList_init(&tTaskDelayList);
	
	/* 定时器模块初始化 */
#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleInit();
#endif
	
	/* CPU使用率测量初始化 */
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	initCpuUsageState();
#endif
	
	/* 时钟节拍计数值tickCount初始化 */
	tTimeTickInit();

	
//	taskTable[0] = &tTask1;
//	taskTable[1] = &tTask2;
	
	
	/*空闲任务初始化*/
	// 空闲任务的优先级应设置为最低
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINY_PRIO_COUNT - 1, idleTaskEnv, sizeof(idleTaskEnv));
	idleTask = &tTaskIdle;

	
	nextTask = tTaskHighestReady();
	
	tTaskRunFirst();  // 执行后是永远不会返回的
	
	return 0;

}


	
