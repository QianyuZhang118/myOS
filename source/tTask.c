/* ������ش��� */

#include "tinyOS.h"

/*
	�����ʼ������
param:
1. ����ṹ��ָ��
2. ������ں�����ַ
3. ������������ַ
4. �������ȼ�
5. �����ջ��ַ(ʵ�ֶ�ջ��С����ǰ��Ϊ��ջ������ַ��ʵ�ֺ󣬺����Ϊ��ջ��ʼ��ַ)
6. ��ջ��С��ʵ�ֶ�ջ��С����������
*/
void tTaskInit(tTask * task, void(*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, uint32_t size)
{
	uint32_t * stackTop;  // ָ���ջĩ�˵�ָ��
	task->stackBase = stack;
	task->stackSize = size;
	memset(stack, 0, size);
	
	stackTop = stack + size / sizeof(tTaskStack);
	
	
	//��ʼ���������ֳ����浽ջ��
	/* ����PendSVʱ��Ӳ���Զ�����ͻָ��Ĳ��� */
	*(--stackTop) = (unsigned long)(1 << 24);  //ʹ��Thumbָ�
	*(--stackTop) = (unsigned long)entry;  		//PC
	*(--stackTop) = (unsigned long)0x14;       //LR
	*(--stackTop) = (unsigned long)0x12;       //R12
	*(--stackTop) = (unsigned long)0x03;       //R3
	*(--stackTop) = (unsigned long)0x02;       //R2
	*(--stackTop) = (unsigned long)0x01;       //R1
	*(--stackTop) = (unsigned long)param;      //R0  ������ں�������
	
	/* ��Ҫ�ֶ�����ͻָ��Ĳ��� */
	//R4~R11
	*(--stackTop) = (unsigned long)0x11;       
	*(--stackTop) = (unsigned long)0x10;       
	*(--stackTop) = (unsigned long)0x9;     
	*(--stackTop) = (unsigned long)0x8;
	*(--stackTop) = (unsigned long)0x7;
	*(--stackTop) = (unsigned long)0x6;
	*(--stackTop) = (unsigned long)0x5;
	*(--stackTop) = (unsigned long)0x4;
	
	task->slice = TINY_SLICE_MAX; // ��ʼ��ʱ��Ƭ����
	task->stack = stackTop;
	task->delayTicks = 0;
	task->prio = prio;
	task->state = TINYOS_TASK_STATE_RDY;
	task->spspendCount = 0;
	task->clean = (void (*) (void *))0;
	task->cleanParam = (void *)0;
	task->requestDeleteFlag = 0;
	
	tNode_init(&(task->delayNode));  // �ڵ��ʼ��
	tNode_init(&(task->linkNode));
	
	tTaskSchedRdy(task);  // �����������
	
#if TINYOS_ENABLE_HOOKS == 1

	tHooksTaskInit(task);  // ���Ӻ���
	
#endif

}

/*
���������
*/
void tTaskSuspend(tTask * task)
{
	uint32_t status = tTaskEnterCritical();  // �����ٽ���
	
	if(!(task->state & TINYOS_TASK_STATE_DELAY))  // ������񲻴�����ʱ״̬
	{
		if(++task->spspendCount <= 1) //��������ǵ�һ�ι���
		{
			task->state |= TINYOS_TASK_STATE_SUSPEND;  // ��������Ϊ����״̬
			tTaskSchedUnRdy(task);  // �������Ƴ������б�
			if(task == currentTask)  // 
			{
				tTaskSched();
			}
		}
		
		tTaskExitCritical(status);
	}
}

/*
�����Ѻ���
*/
void tTaskWakeUp(tTask * task)
{
	uint32_t status = tTaskEnterCritical();  // �����ٽ���
	
	if(task->state & TINYOS_TASK_STATE_SUSPEND)  // ��������ڹ���״̬
	{
		if(--task->spspendCount == 0) // ���������һ
		{
			task->state &= ~TINYOS_TASK_STATE_SUSPEND;  // ������������״̬���
			tTaskSchedRdy(task);  // �������������б�
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
}


/*
�������������ص�����
*/
void tTaskSetCleanCallFunc(tTask * task, void (* clean)(void * param), void * param)
{
	task->clean = clean;
	task->cleanParam = param;
}

/*
ǿ��ɾ������
*/
void tTaskForceDelete(tTask * task)
{
	uint32_t status = tTaskEnterCritical();
	
	// �����������ʱ״̬�����������ʱ����ɾ��
	if(task->state & TINYOS_TASK_STATE_DELAY)
	{
		tTimeTaskRemove(task);
	}
	
	// ������񲻴��ڹ���״̬��������Ӿ���������ɾ��(ע��������elseif)
	else if(!(task->state & TINYOS_TASK_STATE_SUSPEND))  
	{
		tTaskSchedRemove(task);
	}
	
	if(task->clean)
	{
		task->clean(task->cleanParam); // ����������������,����������
	}
	
	if(currentTask == task) // �����ǰ����ΪҪɾ����������ִ�������л�
	{
		tTaskSched();
	}
	
	tTaskExitCritical(status);
}

/*
����ɾ�������ӿڣ���������ɾ����ǣ�
*/
void tTaskRequestDelete(tTask * task)
{
	uint32_t status = tTaskEnterCritical();
	
	task->requestDeleteFlag = 1;
	
	tTaskExitCritical(status);
}

/*
��鵱ǰ�����Ƿ�����ɾ�� ��������������ɾ����ǣ�
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
ɾ�������Լ�
*/
void tTaskDeleteSelf(void)
{
	uint32_t status = tTaskEnterCritical();
	
	tTaskSchedRemove(currentTask); // �Ӿ��������Ƴ�
	
	if(currentTask->clean)
	{
		currentTask->clean(currentTask->cleanParam);  // ����������
	}
	
	tTaskExitCritical(status);
	
	tTaskSched();
}

/*
����״̬��ѯ
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
	
	Info->stackSize = task->stackSize;  // ��ջʹ����
	
	Info->stackFree = 0;
	stackEnd = task->stackBase; // �ڸ�OS�У���ջ�Ӹߵ�ַ��͵�ַ����
	while((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(tTaskStack)))
	{
		Info->stackFree++;  // �Ӷ�ջĩ�˿�ʼ������ջ����ʼ��ַ��ͳ�Ƶ����㵥Ԫ�ĸ���
	}
	
	Info->stackFree *= sizeof(sizeof(tTaskStack));  // ����ջ��Ԫ����ת��Ϊ�ֽ���

	tTaskExitCritical(status);
}










