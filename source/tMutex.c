#include "tinyOS.h"

#if TINYOS_ENABLE_MUTEX== 1

/* �����ź����ĳ�ʼ�� */
void tMutexInit(tMutex * mutex)
{
    // ��ʼ�������ź������¼����ƿ飬�����¼�����Ϊ�����ź�������
    tEventInit(&mutex->event, tEventTypeMutex);
	
    // ���ѱ������Ĵ�����ʼ��Ϊ0
    mutex->lockedCount = 0;
	
    // ��ӵ����ָ���ʼ��Ϊ��ָ��
    mutex->owner = (tTask *)0;
	
    // ��ӵ����ԭʼ���ȼ�����ΪTINY_PRIO_COUNT��һ���ض��ĳ���ֵ��
    mutex->ownerOriginalPrio = TINY_PRIO_COUNT;
}


/* �ȴ������ź������� */
uint32_t tMutexWait(tMutex * mutex, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();
	
    if (mutex->lockedCount <= 0)   // ��������ź���δ��������lockedCount <= 0��
    {
        mutex->owner = currentTask;  // ���õ�ǰ����ռ�и��ź���

        mutex->ownerOriginalPrio = currentTask->prio; // ��¼��ǰ�����ԭʼ���ȼ�

        mutex->lockedCount++; // ����������1
			
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else  // ����Ѿ�������
    {
        if (mutex->owner == currentTask)  // ������Լ�������
        {
            mutex->lockedCount++;  // ����������1

            tTaskExitCritical(status);
            return tErrorNoError;
        }
        else // ��������Լ�������
        {
            if (currentTask->prio < mutex->owner->prio)  // ����ǵ����ȼ����������ģ�ע������ԽС�����ȼ�Խ�ߣ�
            {
                tTask * owner = mutex->owner;
                
                if (owner->state == TINYOS_TASK_STATE_RDY) // ���ӵ���������ھ���״̬
                {
                    tTaskSchedUnRdy(owner); // ��ӵ�������������Ƴ���������
                   
                    owner->prio = currentTask->prio; // ��ӵ������������ȼ�����Ϊ��ǰ��������ȼ���������ȼ���ʹ�価����ɣ�
                   
                    tTaskSchedRdy(owner); // Ȼ��ӵ�����������¼����������
                }
                else
                {
                    // ��������ڷǾ���״̬��������ʱ���ȴ���״̬��ֱ�Ӹ��������ȼ�����
                    owner->prio = currentTask->prio;
                }
            }
           
            tEventWait(&mutex->event, currentTask, (void *)0, tEventTypeMutex, waitTicks); // ����ǰ�������ȴ�������

            tTaskExitCritical(status);
           
            tTaskSched(); // ִ���¼����ȣ��л�����������
						
            // �л��ظ�����ʱ�����ص�ǰ����ĵȴ��¼����
            return currentTask->waitEventResult;
        }
    }
}

/* �����ź����޵ȴ���ȡ������Ҳ���������� */
uint32_t tMutexNoWaitGet(tMutex * mutex)
{
    uint32_t status = tTaskEnterCritical();
	
    if (mutex->lockedCount <= 0) // ��������ź���δ��������lockedCount <= 0��
    {
        mutex->owner = currentTask; // ���õ�ǰ����ռ�и��ź���
       
        mutex->ownerOriginalPrio = currentTask->prio; // ��¼��ǰ�����ԭʼ���ȼ�

        mutex->lockedCount++; // ����������1

        tTaskExitCritical(status);

        return tErrorNoError;
    }
    else // ����Ѿ�������
    {
        if (mutex->owner == currentTask) // ������Լ�������
        {
           
            mutex->lockedCount++; // ����������1
           
            tTaskExitCritical(status);

            return tErrorNoError;
        }
				
        // ��������Լ������ģ�ֱ���˳�
        tTaskExitCritical(status);
				
        // ������Դ�����ô����ʶ
        return tErrorResourceUnavalable;
    }
}


/* ֪ͨ/�ͷŻ����ź������� */
uint32_t tMutexNotify(tMutex * mutex)
{
    uint32_t status = tTaskEnterCritical();
    if (mutex->lockedCount <= 0) // ��������ź���δ��������ֱ���˳�
    {
        tTaskExitCritical(status);
        return tErrorNoError;
    }
		
    if (mutex->owner!= currentTask) // ��������ɵ�ǰ�����򷵻ش�����Ϊ�����ź���ֻ��������ռ�к��ͷ�
    {
        tTaskExitCritical(status);
        return tErrorOwner;
    }
   
    if (--mutex->lockedCount > 0) // ����������1������Դ����㣬���˳�
    {
        tTaskExitCritical(status);
        return tErrorNoError;
    }
		
		// �������0������Ҫ�ͷ�ռ����
		// ���ռ���ߵ�ԭʼ���ȼ��͵�ǰ���ȼ���һ�£���˵���������ȼ��̳У���Ҫ�ָ�ԭ���ȼ�
    if (mutex->ownerOriginalPrio!= mutex->owner->prio) 
    {
        if (mutex->owner->state == TINYOS_TASK_STATE_RDY) // ���ռ���������ھ���״̬
        {
            tTaskSchedUnRdy(mutex->owner); // ��ռ���������Ƴ���������
          
            currentTask->prio = mutex->ownerOriginalPrio;  // ��ռ������������ȼ��ָ�Ϊԭʼ���ȼ�
           
            tTaskSchedRdy(mutex->owner); // ��ռ�����������¼����������
        }
        else  // ���ռ���������ڷǾ���״̬
        {
            // ֱ�ӽ�ռ������������ȼ��ָ�Ϊԭʼ���ȼ�
            currentTask->prio = mutex->ownerOriginalPrio;
        }
    }
		
    // ����������ڵȴ��û����ź���
    if (tEventWaitCount(&mutex->event) > 0)
    {
        tTask * task = tEventWakeUp(&mutex->event, (void *)0, tErrorNoError); // ���ѵȴ������еĵ�һ�����񣨿ɸĽ���
			
        mutex->owner = task;  // �����µ�ӵ����Ϊ�����ѵ�����
       
        mutex->ownerOriginalPrio = task->prio; // ��¼��ӵ���ߵ����ȼ�
      
        mutex->lockedCount++;  // ����������1
       
        if (task->prio < currentTask->prio) // �����������������ȼ����ڵ�ǰ�������ȼ�
        {
            tTaskSched(); // �л��������ȼ�����
        }
    }
    tTaskExitCritical(status);
    return tErrorNoError;
}

/* ɾ�������ź������� */
uint32_t tMutexDestroy(tMutex * mutex)
{
    uint32_t count = 0;
    uint32_t status = tTaskEnterCritical();
   
    if (mutex->lockedCount > 0) // ����ź����Ƿ��ѱ�����
    {
        // �Ƿ��з������ȼ��̳У������������Ҫ�ָ�ӵ���ߵ�ԭ���ȼ�
        if (mutex->ownerOriginalPrio!= mutex->owner->prio)
        {
            if (mutex->owner->state == TINYOS_TASK_STATE_RDY)  // ��������ھ���״̬
            {
                // �����ھ���״̬ʱ�����������ھ������е�λ��
                tTaskSchedUnRdy(mutex->owner);
                currentTask->prio = mutex->ownerOriginalPrio;
                tTaskSchedRdy(mutex->owner);
            }
            else
            {
                // ����״̬��ֻҪ�޸����ȼ�
                currentTask->prio = mutex->ownerOriginalPrio;
            }
        }
        // ����¼����ƿ�ȴ������е����񣬷���ɾ������������
        count = tEventRemoveAll(&mutex->event, (void *)0, tErrorDel);
				
        // ��չ����п��������������ִ��һ�ε���
        if (count > 0)
        {
            tTaskSched();
        }
    }
    tTaskExitCritical(status);
    return count;
}

/* �����ź���״̬��ѯ */
void tMutexGetInfo(tMutex * mutex, tMutexInfo * info)
{
    uint32_t status = tTaskEnterCritical();
	
    // ������Ҫ����Ϣ
    info->taskCount = tEventWaitCount(&mutex->event);
    info->ownerPrio = mutex->ownerOriginalPrio;
	
		if(mutex->owner != (tTask *)0)  // �жϵ�ǰ��û��ӵ����
		{
			info->inheritedPrio = mutex->owner->prio;  // �У���������¼
		}
		else
		{
			info->inheritedPrio = TINY_PRIO_COUNT; // û�У������÷Ƿ�ֵ
		}
    
    info->owner = mutex->owner;
    info->lockedCount = mutex->lockedCount;
	
    tTaskExitCritical(status);
}

#endif

