/*
	����
*/

#include "tinyOS.h"

tTask *currentTask; 	// ָ��ǰ�����ָ��
tTask *nextTask;		// ָ����һ�����ָ��
tTask *idleTask;  // ��������ָ��


tBitmap taskPrioBitmap; // ����λͼ
tList taskTable[TINY_PRIO_COUNT]; // �����б�(������)
tList tTaskDelayList;  // ��ʱ����

uint8_t schedLockCount;  // ������������


/* ����ͳ��CPU Usage�ļ���ֵ */
uint32_t idleCount; 		// ���̼���ֵ��������
uint32_t idleMaxCount;  // ������ֵ
uint32_t tickCount;  // ʱ�ӽ��ĵķ�������

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

/* CPUʹ���ʲ��� ��ʼ������ */
static void initCpuUsageState(void);
	
/* ʱ�ӽ����ж��е��õ� CPUʹ����ͳ�ƺ�������ɼ�¼��ͳ�ơ�����Ĺ��� */
static void checkCpuUsage(void);
	
/* ʵ�ֿ���������ʱ��ͬ���ĺ��� */
static void cpuUsageSyncWithSystick(void);

#endif



/* �����ȼ�λͼ�в���������ȼ����������������У������� */
tTask * tTaskHighestReady(void)
{
	uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
	
	tNode * node = tList_first(&taskTable[highestPrio]);  // ȡ��ͬ���ȼ����еĵ�һ���ڵ�
	
	tTask * task = tNode_parent(node, tTask, linkNode);  // ���ݽڵ㣬�ҳ��丸�ṹ����Ӧ����ĵ�ַ
	
	return task;  // ��������
}


/* ��������ʼ������ */
void tTaskSchedInit(void)
{
	int i;
	schedLockCount = 0;  // ��������ʼ��
	tBitmapInit(&taskPrioBitmap); // ���ȼ�λͼ���г�ʼ��
	for(i = 0; i < TINY_PRIO_COUNT; ++i)  // ��ʼ���������еĸ������ȼ��б�
	{
		tList_init(&taskTable[i]);
	}
}

/* ������������ֹ���ȣ� */
void tTaskSchedDisable(void)
{
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount < 255)
	{
		schedLockCount++;
	}
	tTaskExitCritical(status);
}

/* ����������ʹ�ܵ��ȣ� */
void tTaskSchedEnable(void)
{
	uint32_t status = tTaskEnterCritical();
	if(schedLockCount > 0)
	{
		if(--schedLockCount == 0)
		{
			tTaskSched();  // ���������������Ϊ0��ִ�е���
		}
	}
	tTaskExitCritical(status);
}

/*
�����������б�������ʱ��ɺ�
*/
void tTaskSchedRdy(tTask * task)
{
	tList_addFirst(&taskTable[task->prio], &(task->linkNode)); // �������������ȼ�����������еĶ�Ӧ���ȼ�����
	tBitmapSet(&taskPrioBitmap, task->prio); // ��λͼ�ж�Ӧλ����1����ʾ�������
}

/*
�������Ƴ������б�(����������Ϊ�Ǿ���״̬)
*/
void tTaskSchedUnRdy(tTask * task)
{
	tList_remove(&taskTable[task->prio], &(task->linkNode));  // �������������ȼ��Ӿ������еĶ�Ӧ���ȼ�������ɾ��
	
	if(tList_count(&taskTable[task->prio]) == 0)  // ����Ӧ���ȼ�������û�о�������ʱ����Ӧλͼ����
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

/*
�������Ƴ����ȼ�����(��������ɾ��)
*/
void tTaskSchedRemove(tTask * task)
{
	tList_remove(&taskTable[task->prio], &(task->linkNode));
	
	if(tList_count(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}



/* ������Ⱥ��� */
void tTaskSched() 
{
	tTask * tempTask;
	uint32_t status = tTaskEnterCritical(); // �����ٽ�������
	
	if(schedLockCount > 0)  // ������������������ֹ���ȣ�ֱ�ӷ���
	{
		tTaskExitCritical(status);
		return;
	}
	
	tempTask = tTaskHighestReady();  // ��λͼ�еõ���ǰ������ȼ��ľ�������
	if(tempTask != currentTask)
	{
		nextTask = tempTask; // ָ�����������һ��Ҫ���е�����
		// tTaskSwitch();
		
#if TINYOS_ENABLE_HOOKS == 1

	tHooksTaskSwitch(currentTask, nextTask);  // ���Ӻ���
	
#endif
		
	}

	tTaskExitCritical(status);  // �˳��ٽ�������
	
	tTaskSwitch(); // ����PendSV�쳣���л�����
	
	/* tTaskSwitch()�����ٽ��������ں�����ɣ�������Ϊ��
		���ж�����ʹ��ʱ����� PendSV ��־λ��Ȼ�����ã�������������һ�����ʵ�ʱ������ PendSV �жϴ������ */
	 
	
}


/*
������г�ʼ������
*/
void tTaskDelayListInit(void)
{
	tList_init(&tTaskDelayList);
}

/*
������ʱ ��������Ҫ��ʱ�����������ʱ���У�
*/
void tTimeTaskWait(tTask * task, uint32_t ticks)
{
	task->delayTicks = ticks;
	tList_addLast(&tTaskDelayList, &(task->delayNode));
	task->state |= TINYOS_TASK_STATE_DELAY;
}

/*
�����˳���ʱ �������������Ƴ���ʱ���У�
*/
void tTimeTaskWakeUp(tTask * task)
{
	tList_remove(&tTaskDelayList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAY;
}

/*
���������ʱ������ɾ������������ɾ����
*/
void tTimeTaskRemove(tTask * task)
{
	tList_remove(&tTaskDelayList, &(task->delayNode));
}

/* ʱ�ӽ��ļ���ֵtickCount��ʼ�� */
void tTimeTickInit()
{
	tickCount = 0;
}

/*
ʱ�ӽ����жϴ����� 
���ڵݼ����������е���ʱ������
*/
void tTaskSystemTickHandler()
{
	tNode * node;
	
	//int i;
	uint32_t status = tTaskEnterCritical();
	
	// ɨ����ʱ����
	for(node = tTaskDelayList.headNode.nextNode; node!=&(tTaskDelayList.headNode); node = node->nextNode)
  {
		tTask * task = tNode_parent(node, tTask, delayNode);
		if(--task->delayTicks == 0)
		{
			if(task->waitEvent) // ��������Ƿ��ڵȴ��¼���������Ϊ�����õȴ��¼���ʱ������£�����ᱻ������ʱ����
			{
				tEventRemoveTask(task, (void *)0, tErrorTimeOut);  // ���ȴ��¼���ʱʱ����������¼����ƿ�ĵȴ�������ǿ���Ƴ�
			}
			tTimeTaskWakeUp(task);  // �����ʱ�������Ƴ���ʱ���У��л�����״̬
			tTaskSchedRdy(task); // ������������
		}
	}
	
	if(--currentTask->slice == 0)  // �����ǰ�����ʱ��ƬΪ�㣬���л�����
	{
		if(tList_count(&taskTable[currentTask->prio]) > 0)  // �����ǰ���ȼ����о���������ͬ���ȼ��л�
		{
			tList_removeFirst(&taskTable[currentTask->prio]);  // �л�ʱ�������ȼ����еĵ�һ�����񣨵�ǰ�����Ƴ�
			tList_addLast(&taskTable[currentTask->prio], &(currentTask->linkNode));  // ���²��뵽���ȼ�����ĩβ
			
			currentTask->slice = TINY_SLICE_MAX; // �������õ�ǰ�����ѱ����ڶ�����󣩵�ʱ��Ƭ
		}
	}
	
	tickCount++; // ʱ�ӽ��ļ�������
	
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	checkCpuUsage();  // ���CPUʹ����
#endif
	
	tTaskExitCritical(status);

#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleTickNotify();  // ֪ͨ��ʱ��
#endif
	
#if TINYOS_ENABLE_HOOKS == 1
	tHooksSysTick();  // ���Ӻ���
#endif
	
	tTaskSched();
}


#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

static float cpuUsage;  							// CPUʹ����
static uint32_t enableCpuUsageState;  // ʱ��ͬ����־λ 0δͬ�� 1��ͬ��

/* CPUʹ���ʲ��� ��ʼ������ */
static void initCpuUsageState(void)
{
	idleCount = 0;
	idleMaxCount = 0;
	cpuUsage = 0.0f;  // ��׺fָ����������� float ����
	enableCpuUsageState = 0;
}

/* CPUʹ����ͳ�ƺ��� ʱ�ӽ����ж��е��ã���ɼ�¼��ͳ�ơ�����Ĺ��� */
static void checkCpuUsage(void)
{
	if(enableCpuUsageState == 0)  // ���δͬ��
	{
		enableCpuUsageState = 1;  // ����Ϊ��ͬ��
		
		tickCount = 0;  // ʱ�ӽ��ļ�������
		
		return;
	}
	
	// ϵͳ���е�ǰ1s��ȫ�����п��������Ի��������ֵ��
	// 1s���������������
	
	if(tickCount == TICKS_PER_SEC)  // �����ϵͳ��ʼ���е���ʱ��ʱ���ȥ��1s
	{
		idleMaxCount = idleCount;  // ����ǰ����ֵ��Ϊ������ֵ
		
		idleCount = 0;  // ��ǰ����ֵ����
		
		tTaskSchedEnable();  // ���������ֵ��¼�󣬼��������������񣬿���������
	}
	else if(tickCount % TICKS_PER_SEC == 0)  // �˺�ÿ��1s��ͳ��һ��CPU Usage
	{
		cpuUsage = 100.0 - 100.0 * idleCount / idleMaxCount;  // ����CPUʹ����
		
		idleCount = 0;  // ��ǰ����ֵ����
	}
}

/* ʵ�ֿ���������ʱ��ͬ���ĺ��� */
static void cpuUsageSyncWithSystick(void)
{
	while(enableCpuUsageState == 0)  // �����־λΪ0��ԭ�صȴ�
	{
		;;
	}
	// ʱ�ӽ��ķ���ʱ��enableCpuUsageState��Ϊ1 ,�ú����˳�������������ܼ�����������
}

/* CPUʹ���ʻ�ȡ�Ľӿں��� */
float tCpuUsageGet()
{
	float usage = 0; 
	
	uint32_t status = tTaskEnterCritical();
	
	usage = cpuUsage;  // ȡֵ
	
	tTaskExitCritical(status);
	
	return usage;  // ����
}

#endif

/* ���������� */
tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

/* ���������� */
void idleTaskEntry(void *param)
{
	tTaskSchedDisable();  // Ϊ��ֹ�л����������񣬹رյ�����������ͳ��CPU Usage��
	
	tInitApp();  // �����ʼ�� 

#if TINYOS_ENABLE_TIMER == 1
	tTimerTaskInit();  // ��ʱ�������ʼ��
#endif
	
	SysTick_init(TINYOS_SYSTICK_MS);  // ʱ�ӽ��ĳ�ʼ��
	
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	cpuUsageSyncWithSystick();  // ������ʱ�ӽ���ͬ��
#endif
	
	LedInit(); // LED��ʼ��
	
	UartInit(); // ���ڳ�ʼ��
	
	for(;;)
	{
		uint32_t status = tTaskEnterCritical();
		
		idleCount++;  // ����ֵ��һ
		
		tTaskExitCritical(status);
		
#if TINYOS_ENABLE_HOOKS == 1

	tHooksCpuIdle();  // ���Ӻ���
	
#endif
	}
}



int main()
{
	
	/* ��������ʼ�� */
	tTaskSchedInit();
	
	/* ��ʱ�б��ʼ�� */
	tList_init(&tTaskDelayList);
	
	/* ��ʱ��ģ���ʼ�� */
#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleInit();
#endif
	
	/* CPUʹ���ʲ�����ʼ�� */
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	initCpuUsageState();
#endif
	
	/* ʱ�ӽ��ļ���ֵtickCount��ʼ�� */
	tTimeTickInit();

	
//	taskTable[0] = &tTask1;
//	taskTable[1] = &tTask2;
	
	
	/*���������ʼ��*/
	// ������������ȼ�Ӧ����Ϊ���
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINY_PRIO_COUNT - 1, idleTaskEnv, sizeof(idleTaskEnv));
	idleTask = &tTaskIdle;

	
	nextTask = tTaskHighestReady();
	
	tTaskRunFirst();  // ִ�к�����Զ���᷵�ص�
	
	return 0;

}


	
