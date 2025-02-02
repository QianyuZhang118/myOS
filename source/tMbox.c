#include "tinyOS.h"

#if TINYOS_ENABLE_MBOX == 1

/* �����ʼ������ */
void tMboxInit(tMbox * mbox, void ** msgBuffer, uint32_t maxCount)
{
    tEventInit(&mbox->event, tEventTypeMbox);  // ��ʼ���¼����ƿ�
    mbox->msgBuffer = msgBuffer;  // ��Ϣ������ָ��
    mbox->maxCount = maxCount;  // �����Ϣ����
    mbox->read = 0; // ������
    mbox->write = 0;  // д����
    mbox->count = 0;  // ��ǰ��Ϣ����
}

/* ��ȡ��Ϣ����������Ҫ�������ȡ��Ϣ�� */
uint32_t tMboxWait(tMbox * mbox, void ** msg, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ����Ϣ�����Ƿ����0
    if (mbox->count > 0)
    {
        // �������0�Ļ���ȡ��һ����Ϣ
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
			
        // ͬʱ������ǰ�ƣ���������߽������
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
				
        return tErrorNoError;

    }
    else
    {
        // Ȼ����������¼�������
        tEventWait(&mbox->event, currentTask, (void *)0, tEventTypeSem, waitTicks);
        tTaskExitCritical(status);
        // �����ִ��һ���¼����ȣ��Ա����л�����������
        tTaskSched();
			
        // ���л�����ʱ����tTask��ȡ����õ���Ϣ
        *msg = currentTask->eventMsg;
			
        // ȡ���ȴ����
        return currentTask->waitEventResult;
    }
}


/* ��ȡ��Ϣ���������ȴ������û����Ϣ��ֱ���˳��� */
uint32_t tMboxNoWaitGet(tMbox * mbox, void ** msg)
{
		uint32_t status = tTaskEnterCritical();

    // ���ȼ����Ϣ�����Ƿ����0
    if (mbox->count > 0)
    {
        // �������0�Ļ���ȡ��һ����Ϣ
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
			
        // ͬʱ������ǰ�ƣ���������߽������
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        tTaskExitCritical(status);
				
        return tErrorNoError;
    }
    else  // ���û����Ϣ
    {
        tTaskExitCritical(status);
        // ������Դ������
        return tErrorResourceUnavalable;
    }
}


/* ��Ϣ֪ͨ��������ϢҪ���͸����䣩
@ notifyOption�����ѡ���ʾ��Ϣ�Ƿ��Ǹ������ȼ���Ϣ���ǣ���ᱻ���ȴ洢*/
uint32_t tMboxNotify(tMbox * mbox, void * msg, uint32_t notifyOption)
{
    uint32_t status = tTaskEnterCritical();

    // ����Ƿ�������ȴ�
    if (tEventWaitCount(&mbox->event) > 0)
    {
        // ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
        tTask * task = tEventWakeUp(&mbox->event, (void *)msg, tErrorNoError);

        // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    else
    {
        // �ж������Ƿ����������������˳�
        if (mbox->count >= mbox->maxCount)
        {
            tTaskExitCritical(status);
            return tErrorResourceFull;
        }
				
				// ���û������ȴ��Ļ�������Ϣ���뵽��������
        // ����ѡ����Ϣ���뵽ͷ���������������ȡ��ʱ�����Ȼ�ȡ����Ϣ
        if (notifyOption & tMBOXSendFront)  // �����ȼ���Ϣ
        {
            if (mbox->read <= 0)
            {
                mbox->read = mbox->maxCount - 1;
            }
            else
            {
                --mbox->read;
            }
            mbox->msgBuffer[mbox->read] = msg;  // ͨ��readָ��д��
        }
        else  // ��ͨ��Ϣ
        {
            mbox->msgBuffer[mbox->write++] = msg;  // д����write���ƣ���д��msg
            if (mbox->write >= mbox->maxCount)
            {
                mbox->write = 0;
            }
        }
        mbox->count++;  // ������Ϣ����
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}


/* ������亯��(��ջ�����) */
void tMboxFlush(tMbox * mbox)
{
    uint32_t status = tTaskEnterCritical();

    // ���������û������ȴ���˵�������������Ϣ����Ҫ���
    if (tEventWaitCount(&mbox->event) == 0)
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }
		// ����������ڵȴ����򻺴���һ���ǿյģ��������

    tTaskExitCritical(status);
}

/* ɾ�����亯��(�Ƴ��ȴ�����������) */
uint32_t tMboxDestroy(tMbox * mbox)
{
    uint32_t status = tTaskEnterCritical();

    // ����¼����ƿ��е�����
    uint32_t count = tEventRemoveAll(&mbox->event, (void *)0, tErrorDel);
    tTaskExitCritical(status);

    // ��չ����п��������������ִ��һ�ε���
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}


/* ������Ϣ��ѯ���� */
void tMboxGetInfo(tMbox * mbox, tMboxInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    // ������Ҫ����Ϣ
    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = tEventWaitCount(&mbox->event);

    tTaskExitCritical(status);
}

#endif

