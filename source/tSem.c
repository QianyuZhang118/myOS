#include "tSem.h"
#include "tinyOS.h"

#if TINYOS_ENABLE_SEM == 1

/* �ź�����ʼ�� */
void tSemInit(tSem * sem, uint32_t startCount, uint32_t maxCount)
{
	tEventInit(&sem->event, tEventTypeSem);
	
	sem->maxCount = maxCount;
	
	if(maxCount == 0)  // ������ֵΪ0��˵������ֵû������
	{
		sem->count = startCount;
	}
	else
	{
		sem->count = (startCount > maxCount) ? maxCount : startCount;
	}
}


/* ����tSemWait���ڵȴ��ź��� */
// sem��ָ���ź����ṹ���ָ��
// waitTicks���ȴ���ʱ��������
uint32_t tSemWait(tSem * sem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ���ź��������Ƿ����0
    if (sem->count > 0)
    {
        // �������0�Ļ������ĵ�һ����Ȼ�������˳�
        --sem->count;
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else // �����������0�����������ȴ�״̬
    {
        // Ȼ����������¼�������
        tEventWait(&sem->event, currentTask, (void *)0, tEventTypeSem, waitTicks);
        tTaskExitCritical(status);

        // �����ִ��һ���¼����ȣ��Ա����л�����������
        tTaskSched();

        // �����ڵȴ���ʱ���߼�������ʱ��ִ�л᷵�ص����Ȼ��ȡ���ȴ��ṹ
        return currentTask->waitEventResult;
    }
}


/* ����tSemNotify����֪ͨ�ź��� */
void tSemNotify(tSem * sem)
{
    uint32_t status = tTaskEnterCritical();

    // ����Ƿ�������ȴ�
    if (tEventWaitCount(&sem->event) > 0)
    {
        // ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
        tTask * task = tEventWakeUp(&sem->event, (void *)0, tErrorNoError);

        // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    else
    {
        // ���û������ȴ��Ļ������Ӽ���
        ++sem->count;

        // �����������������������ļ�����������Ϊ���ֵ
        if ((sem->maxCount != 0) && (sem->count > sem->maxCount))
        {
            sem->count = sem->maxCount;
        }
    }
    tTaskExitCritical(status);
}

/* tSemNoWaitGet �ź������޵ȴ���ȡ����
����tSemWait���𣺵�����Ϊ��ʱ��ֱ���˳��������ǽ��������ȴ����У� */
uint32_t tSemNoWaitGet(tSem * sem)
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ���ź��������Ƿ����0
    if (sem->count > 0)
    {
        // �������0�Ļ������ĵ�һ����Ȼ�������˳�
        --sem->count;
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else
    {
        // ���򣬷�����Դ������
        tTaskExitCritical(status);
        return tErrorResourceUnavalable;
    }
}


/* ����tSemDestroy���������ź��� */ 
uint32_t tSemDestroy(tSem * sem)
{
    uint32_t status = tTaskEnterCritical();

    // ����¼����ƿ��е�����
    uint32_t count = tEventRemoveAll(&sem->event, (void *)0, tErrorDel);
    tTaskExitCritical(status);

    // ��չ����п��������������ִ��һ�ε���
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}


/* ״̬��ѯ�ӿ� ����tSemGetInfo���ڻ�ȡ�ź�������Ϣ */ 
void tSemGetInfo(tSem * sem, tSemInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    // ������Ҫ����Ϣ
    info->count = sem->count;  // ������ǰ�ź�������
    info->maxCount = sem->maxCount;  // ����������
    info->taskCount = tEventWaitCount(&sem->event);  // �ȴ�������

    tTaskExitCritical(status);
}

#endif

