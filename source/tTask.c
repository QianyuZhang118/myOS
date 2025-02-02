/* 任务相关代码 */

#include "tinyOS.h"

/*
	任务初始化函数
param:
1. 任务结构体指针
2. 任务入口函数地址
3. 任务函数参数地址
4. 任务优先级
5. 任务堆栈地址(实现堆栈大小测量前，为堆栈结束地址；实现后，含义改为堆栈起始地址)
6. 堆栈大小（实现堆栈大小测量新增）
*/
void tTaskInit(tTask * task, void(*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, uint32_t size)
{
	uint32_t * stackTop;  // 指向堆栈末端的指针
	task->stackBase = stack;
	task->stackSize = size;
	memset(stack, 0, size);
	
	stackTop = stack + size / sizeof(tTaskStack);
	
	
	//初始化把任务现场保存到栈中
	/* 进出PendSV时，硬件自动保存和恢复的部分 */
	*(--stackTop) = (unsigned long)(1 << 24);  //使用Thumb指令集
	*(--stackTop) = (unsigned long)entry;  		//PC
	*(--stackTop) = (unsigned long)0x14;       //LR
	*(--stackTop) = (unsigned long)0x12;       //R12
	*(--stackTop) = (unsigned long)0x03;       //R3
	*(--stackTop) = (unsigned long)0x02;       //R2
	*(--stackTop) = (unsigned long)0x01;       //R1
	*(--stackTop) = (unsigned long)param;      //R0  任务入口函数参数
	
	/* 需要手动保存和恢复的部分 */
	//R4~R11
	*(--stackTop) = (unsigned long)0x11;       
	*(--stackTop) = (unsigned long)0x10;       
	*(--stackTop) = (unsigned long)0x9;     
	*(--stackTop) = (unsigned long)0x8;
	*(--stackTop) = (unsigned long)0x7;
	*(--stackTop) = (unsigned long)0x6;
	*(--stackTop) = (unsigned long)0x5;
	*(--stackTop) = (unsigned long)0x4;
	
	task->slice = TINY_SLICE_MAX; // 初始化时间片计数
	task->stack = stackTop;
	task->delayTicks = 0;
	task->prio = prio;
	task->state = TINYOS_TASK_STATE_RDY;
	task->spspendCount = 0;
	task->clean = (void (*) (void *))0;
	task->cleanParam = (void *)0;
	task->requestDeleteFlag = 0;
	
	tNode_init(&(task->delayNode));  // 节点初始化
	tNode_init(&(task->linkNode));
	
	tTaskSchedRdy(task);  // 加入就绪队列
	
#if TINYOS_ENABLE_HOOKS == 1

	tHooksTaskInit(task);  // 钩子函数
	
#endif

}

/*
任务挂起函数
*/
void tTaskSuspend(tTask * task)
{
	uint32_t status = tTaskEnterCritical();  // 进入临界区
	
	if(!(task->state & TINYOS_TASK_STATE_DELAY))  // 如果任务不处于延时状态
	{
		if(++task->spspendCount <= 1) //如果任务是第一次挂起
		{
			task->state |= TINYOS_TASK_STATE_SUSPEND;  // 将任务标记为挂起状态
			tTaskSchedUnRdy(task);  // 将任务移出就绪列表
			if(task == currentTask)  // 
			{
				tTaskSched();
			}
		}
		
		tTaskExitCritical(status);
	}
}

/*
任务唤醒函数
*/
void tTaskWakeUp(tTask * task)
{
	uint32_t status = tTaskEnterCritical();  // 进入临界区
	
	if(task->state & TINYOS_TASK_STATE_SUSPEND)  // 如果任务处于挂起状态
	{
		if(--task->spspendCount == 0) // 挂起计数减一
		{
			task->state &= ~TINYOS_TASK_STATE_SUSPEND;  // 将任务解除挂起状态标记
			tTaskSchedRdy(task);  // 将任务加入就绪列表
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
}


/*
设置任务的清理回调函数
*/
void tTaskSetCleanCallFunc(tTask * task, void (* clean)(void * param), void * param)
{
	task->clean = clean;
	task->cleanParam = param;
}

/*
强制删除函数
*/
void tTaskForceDelete(tTask * task)
{
	uint32_t status = tTaskEnterCritical();
	
	// 如果任务处于延时状态，则将任务从延时队列删除
	if(task->state & TINYOS_TASK_STATE_DELAY)
	{
		tTimeTaskRemove(task);
	}
	
	// 如果任务不处于挂起状态，则将任务从就绪队列中删除(注意这里是elseif)
	else if(!(task->state & TINYOS_TASK_STATE_SUSPEND))  
	{
		tTaskSchedRemove(task);
	}
	
	if(task->clean)
	{
		task->clean(task->cleanParam); // 将参数传入清理函数,调用清理函数
	}
	
	if(currentTask == task) // 如果当前任务为要删除的任务，则执行任务切换
	{
		tTaskSched();
	}
	
	tTaskExitCritical(status);
}

/*
请求删除函数接口（设置请求删除标记）
*/
void tTaskRequestDelete(tTask * task)
{
	uint32_t status = tTaskEnterCritical();
	
	task->requestDeleteFlag = 1;
	
	tTaskExitCritical(status);
}

/*
检查当前任务是否被请求删除 函数（返回请求删除标记）
*/
uint8_t tTaskIsRequestDelete(void)
{
	uint8_t dele;
	
	uint32_t status = tTaskEnterCritical();
	
	dele = currentTask->requestDeleteFlag;
	
	tTaskExitCritical(status);
	
	return dele;
}

/*
删除任务自己
*/
void tTaskDeleteSelf(void)
{
	uint32_t status = tTaskEnterCritical();
	
	tTaskSchedRemove(currentTask); // 从就绪表中移除
	
	if(currentTask->clean)
	{
		currentTask->clean(currentTask->cleanParam);  // 调用清理函数
	}
	
	tTaskExitCritical(status);
	
	tTaskSched();
}

/*
任务状态查询
*/
void tTaskGetInfo(tTask * task, tTaskInfo * Info)
{
	uint32_t * stackEnd;
	uint32_t status = tTaskEnterCritical();
	
	Info->delayTicks = task->delayTicks;
	Info->prio = task->prio;
	Info->slice = task->slice;
	Info->state = task->state;
	Info->suspendCount = task->spspendCount;
	
	Info->stackSize = task->stackSize;  // 堆栈使用量
	
	Info->stackFree = 0;
	stackEnd = task->stackBase; // 在该OS中，堆栈从高地址向低地址延伸
	while((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(tTaskStack)))
	{
		Info->stackFree++;  // 从堆栈末端开始，到堆栈的起始地址，统计等于零单元的个数
	}
	
	Info->stackFree *= sizeof(sizeof(tTaskStack));  // 将堆栈单元个数转换为字节数

	tTaskExitCritical(status);
}










