#include "tinyOS.h"
#include "tTask.h"

/* �¼����ƿ��ʼ������ */
void tEventInit(tEvent * event, tEventType type)
{
	event->type = tEventTypeUnknown;  // ��ʼ���¼�����
	tList_init(&(event->waitList));  // ��ʼ���ȴ�����
}


/* �¼����ƿ�ȴ��ӿ� 
@ msg: ���ڴ���������ͺ��յ�������
@ state: �ȴ���״̬
@ timeout: ��ʱʱ��
*/
void tEventWait(tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout)
{
	uint32_t status = tTaskEnterCritical();
	
	task->state |= state << 16;  // �ȴ�״̬(��16λ��ŵȴ��¼�״̬)
	task->waitEvent = event; // �ȴ��¼�
	task->eventMsg = msg;  // �ȴ��¼���Ϣ
	task->waitEventResult = tErrorNoError;  // �ȴ����
	
	tTaskSchedUnRdy(task);  // �Ӿ����������Ƴ�
	// ע����ʱ���к͵ȴ����в�һ������ʱ��������delay,�ȴ��������ڵȴ�ĳ���¼�
	// �����ȵȴ��������ȱ����ѣ����Բ������β��
	tList_addLast(&event->waitList, &task->linkNode); // ����ȴ����е�β��
	
	
	if(timeout)  // ��������˳�ʱʱ�䣬�����������ʱ����
	{
		tTimeTaskWait(task, timeout);
	}
	
	tTaskExitCritical(status);
}

/* ��������¼����ƿ��л��� */
tTask * tEventWakeUp(tEvent * event, void * msg, uint32_t result)
{
	tNode * node;
	tTask * task = (tTask * )0;
	uint32_t status = tTaskEnterCritical();
	
	
	// ȡ���¼����ƿ�ĵȴ������еĵ�һ�����񣬲������Ƴ��ö���
	if((node = tList_removeFirst(&event->waitList)) != (tNode *)0)
	{
		task = tNode_parent(node, tTask, linkNode);
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;  // �ȴ��¼���Ϣ
		task->waitEventResult = result;  // �ȴ����
		task->state &=  ~TINYOS_TASK_WAIT_MASK; // �����16λ�����ȴ���־λ
		
		if(task->delayTicks != 0)
		{
			tTimeTaskWakeUp(task); // �����������ʱ����ǿ�ƽ������ʱ������ɾ��
		}
		
		tTaskSchedRdy(task);  // ��������뵽����������
		
	}
	
	tTaskExitCritical(status);
	
	return task;
}

/* ��ָ��������¼����ƿ��л��� */
tTask * tEventWakeUpTask(tEvent * event, tTask * task, void * msg, uint32_t result)
{
//	tNode * node;
	uint32_t status = tTaskEnterCritical();
	
	tList_remove(&event->waitList, &task->linkNode);
	task->waitEvent = (tEvent *)0;
	task->eventMsg = msg;  // �ȴ��¼���Ϣ
	task->waitEventResult = result;  // �ȴ����
	task->state &=  ~TINYOS_TASK_WAIT_MASK; // �����16λ�����ȴ���־λ
	
	if(task->delayTicks != 0)
	{
		tTimeTaskWakeUp(task); // �����������ʱ����ǿ�ƽ������ʱ������ɾ��
	}
	
	tTaskSchedRdy(task);  // ��������뵽����������
	
	
	tTaskExitCritical(status);
	
	return task;
}


/* ��������¼����ƿ���ǿ���Ƴ� */
void tEventRemoveTask(tTask * task, void * msg, uint32_t result)
{
		uint32_t status = tTaskEnterCritical();
	
		tList_remove(&task->waitEvent->waitList, &task->linkNode);
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;  // �ȴ��¼���Ϣ
		task->waitEventResult = result;  // �ȴ����
		task->state &=  ~TINYOS_TASK_WAIT_MASK; // �����16λ�����ȴ���־λ
		
	
		tTaskExitCritical(status);
}


/* ����¼����ƿ�ȴ����У�����ɾ��ǰ�ȴ��������ж������� */
uint32_t tEventRemoveAll(tEvent * event, void * msg, uint32_t result)
{
	tNode * node;
	
	uint32_t cnt = 0;
	
	uint32_t status = tTaskEnterCritical();
	
	cnt = tList_count(&event->waitList);  // ��¼�ȴ������е���������
	
	while((node = tList_removeFirst(&event->waitList)) != (tNode *)0) // �����Ƴ������е�ͷ������
	{
		tTask * task = tNode_parent(node, tTask, linkNode);
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;  // �ȴ��¼���Ϣ
		task->waitEventResult = result;  // �ȴ����
		task->state &=  ~TINYOS_TASK_WAIT_MASK; // �����16λ�����ȴ���־λ
		
		if(task->delayTicks != 0)
		{
			tTimeTaskWakeUp(task);  // �����������ʱ�������Ƴ���ʱ����
		}
		
		tTaskSchedRdy(task);  // �����������
	}
	
	tTaskExitCritical(status);
	
	return cnt;
	
}

/* ��ȡ�¼����ƿ��еȴ����������� */
uint32_t tEventWaitCount(tEvent * event)
{
	uint32_t cnt = 0;
	
	uint32_t status = tTaskEnterCritical();
	
	cnt = tList_count(&event->waitList);
	
	tTaskExitCritical(status);
	
	return cnt; 
}






