#include "tinyOS.h"

#if TINYOS_ENABLE_FLAGGROUP == 1

/* ����tFlagGroupInit���ڳ�ʼ����־�� */ 
void tFlagGroupInit(tFlagGroup * flagGroup, uint32_t flags)
{
    // ��ʼ���¼����ƿ飬�����¼�����Ϊ��־������
    tEventInit(&flagGroup->event, tEventTypeFlagGroup);
	
    // ������ı�־ֵ������־��ĵ�ǰ�¼���־��Ա
    flagGroup->flags = flags;
}

/* ���������������Ӧ���¼���־�Ƿ����㣬������㣬�������Ƿ����Ķ�Ӧ��־ 
@ type���ȴ����¼����ͣ���tFlagGroup.h�е�һϵ�к궨�壩
@ flag������ʱΪ����ϣ���ȴ����¼���־������ʱΪ�������¼���־
*/
static uint32_t tFlagGroupCheckAndConsume(tFlagGroup * flagGroup, uint32_t type, uint32_t * flag)
{
	uint32_t srcFlag = *flag;
	
	// ȷ������ȴ����������¼�����
	uint32_t isSet = type & TFLAGGROUP_SET;
	uint32_t isAll = type & TFLAGGROUP_ALL;
	uint32_t isConsume = type & TFLAGGROUP_CONSUME;
	
	// ����ʵ�ʵ��¼���־
	uint32_t calFlag = isSet ? (flagGroup->flags & srcFlag) : (~flagGroup->flags & srcFlag);
	
	//����������������¼���־�Ա�
	if(((isAll != 0) && (calFlag == srcFlag)) || ((isAll == 0) && (calFlag != 0)) )
	{
		// ƥ��ɹ�
		
			if(isConsume)  // �Ƿ�Ҫ������Ӧ��־
			{
				if(isSet)
				{
					flagGroup->flags &= ~srcFlag; // ��Ӧ��־λ����
				}
				else
				{
					flagGroup->flags |= srcFlag; // ��Ӧ��־λ����
				}
			}
			
			*flag = calFlag;  // ����ʵ�ʱ�־λ
			return tErrorNoError;
	}
	
	*flag = calFlag;
	return tErrorResourceUnavalable;
}

/* �¼���־��ȴ����� 
@ flagGroup ��ָ���¼���־���ָ��
@ waittype ���ȴ����¼�����
@ requestFlag���ȴ���Щ��־
@ resultFlag���ȴ������¼���־
@ waitTicks���ȴ���ʱ
*/
uint32_t tFlagGroupWait(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag,
                        uint32_t * resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;
    uint32_t status = tTaskEnterCritical();
	
    // ��鲢���ѱ�־�����ؽ��
    result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    // ����¼���־����������
    if (result!= tErrorNoError)
    {
        // ���õ�ǰ����ĵȴ���־����
        currentTask->waitFlagsType = waitType;
			
        // ���õ�ǰ����ĵȴ��¼���־
        currentTask->eventFlags = requestFlag;
			
        // ��������뵽�ȴ�������
        tEventWait(&flagGroup->event, currentTask, (void *)0, tEventTypeFlagGroup, waitTicks);
			
        tTaskExitCritical(status);
			
        tTaskSched();
			
        // ��ȡ�����Ѻ���¼���־
        *resultFlag = currentTask->eventFlags;
			
        // ��ȡ����ȴ����
        result = currentTask->waitEventResult;
    }
    else
    {
        // ����¼���־�������������ý����־
        *resultFlag = flags;
		
        tTaskExitCritical(status);
    }
    // ���ؽ����������
    return result;
}

/* �¼���־���ȡ/������ */
uint32_t tFlagGroupNoWaitGet(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag)
{
    uint32_t flags = requestFlag;
    uint32_t status = tTaskEnterCritical();
	
    // ��鲢���ѱ�־�����ؽ��
    uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);

    tTaskExitCritical(status);
	
    // ���ý����־
    *resultFlag = flags;
	
    // ���ؽ��
    return result;
}

/* ֪ͨ��־���¼� */
void tFlagGroupNotify(tFlagGroup * flagGroup, uint8_t isSet, uint32_t flags)
{
    tList * waitList;
    tNode * node;
    tNode * nextNode;
    uint8_t sched = 0;

    uint32_t status = tTaskEnterCritical();
	
    // ����isSet��ֵ���û������־��ı�־
    if (isSet)
    {
        flagGroup->flags |= flags; // ��1�¼�
    }
    else
    {
        flagGroup->flags &= ~flags; // ��0�¼�
    }
		
    // ��ȡ�ȴ��б�ĵ�ַ
    waitList = &flagGroup->event.waitList;
		
    // �����ȴ��б�
    for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
    {
        tTask * task = tNode_parent(node, tTask, linkNode); // ��ȡ�����ַ
			
        uint32_t tFlags = task->eventFlags; // ��ȡ����ĵȴ��¼���־
			
        uint32_t result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &tFlags); // ��鵱ǰ��־�Ƿ�����
			
				nextNode = node->nextNode;
			
        // �����ǰ��־��������
        if (result == tErrorNoError)
        {
            // ����������¼���־
            task->eventFlags = flags;
					
            // ��������
            tEventWakeUpTask(&flagGroup->event, task, (void *)0, tErrorNoError);
            sched = 1;
        }
    }
    // ��������������ִ��һ�ε���
    if (sched)
    {
        tTaskSched();
    }
    // �˳��ٽ���
    tTaskExitCritical(status);
}

/* �¼���־��ɾ�� */
uint32_t tFlagGroupDestroy(tFlagGroup * flagGroup)
{
    // �����ٽ�������ֹ�����񻷾��µĲ�����ͻ
    uint32_t status = tTaskEnterCritical();
    // ��ձ�־���Ӧ���¼����ƿ��е��������񣬷��ر��Ƴ�����������
    uint32_t count = tEventRemoveAll(&flagGroup->event, (void *)0, tErrorDel);
    // �˳��ٽ���
    tTaskExitCritical(status);
    // �������չ����������������ִ��һ���������
    if (count > 0)
    {
        tTaskSched();
    }
    // ���ر��Ƴ�����������
    return count;
}

/* �¼���־��״̬��ѯ���� */
void tFlagGroupGetInfo(tFlagGroup * flagGroup, tFlagGroupInfo * info)
{
    // �����ٽ���
    uint32_t status = tTaskEnterCritical();
    // ������ǰ���¼���־��info�ṹ����
    info->flags = flagGroup->flags;
    // ��ȡ��ǰ�ȴ��ڸñ�־���¼��ϵ�������������������info�ṹ����
    info->taskCount = tEventWaitCount(&flagGroup->event);
    // �˳��ٽ���
    tTaskExitCritical(status);
}

#endif

