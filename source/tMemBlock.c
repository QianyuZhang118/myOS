#include "tinyOS.h"

#if TINYOS_ENABLE_MEMBLOCK == 1


/* �洢���ʼ������ */ 
// memBlock: ָ��Ҫ��ʼ����tMemBlock�ṹ���ָ��
// memStart: �洢���������ʼ��ַ
// blockSize: ÿ���洢��Ĵ�С
// blockCnt: �洢�������
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt)
{
    // ��������ڴ���ʼ��ַת��Ϊuint8_t����ָ�룬����������ֽڲ���
    uint8_t * memBlockStart = (uint8_t *)memStart;
	
    // ����洢������Ľ�����ַ��ͨ����ʼ��ַ�������д洢����ܴ�С�õ�
    uint8_t * memBlockEnd = memBlockStart + blockSize * blockCnt;

    // ÿ���洢����Ҫ��������ָ�루���ڹ����洢���б�����ݽṹ��ز�������������ռ�����Ҫ��tNode��
    // �������������ʵ���û����ÿռ䣬ֻ��Ϊ�������ڲ����ݽṹ�洢����
    // �����ǰ���õĴ洢���СС��tNode���͵Ĵ�С���޷�����Ҫ��ֱ�ӷ��ز����к�����ʼ��
    if (blockSize < sizeof(tNode))
    {
        return;
    }

    // ��ʼ���洢����¼����ƿ�
    tEventInit(&memBlock->event, tEventTypeMemBlock);
		
    // ��ʼ���洢�����ʼ��ַ
    memBlock->memStart = memStart;
		
    // ��ʼ��ÿ���洢��ĵĴ�С
    memBlock->blockSize = blockSize;
		
    // ��ʼ���洢�����������
    memBlock->maxCount = blockCnt;

    // ��ʼ���洢���б�����һ���յ��б�ṹ�����ں����洢�����֯����
    tList_init(&memBlock->blockList);
		
    // �����洢�����򣬶�ÿ���洢����г�ʼ������
    while (memBlockStart < memBlockEnd)
    {
        // ��ÿһ���洢��Ŀ�ʼ������Ϊһ���ڵ㣬�����ʼ��
        tNode_init((tNode *)memBlockStart);
			
        // ����ʼ����Ĵ洢��ڵ����洢���б��ĩβ
        tList_addLast(&memBlock->blockList, (tNode *)memBlockStart);
			
        // ��ָ������ƶ�һ���洢��Ĵ�С��ָ����һ������ʼ���Ĵ洢��
        memBlockStart += blockSize;
    }
}


/* ��ȡ�洢�麯�� */
uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ���Ƿ��п��еĴ洢��
    if (tList_count(&memBlock->blockList) > 0)
    {
        // ����еĻ���ȡ��һ��
        *mem = (uint8_t *)tList_removeFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else  
    {
        // ���û�п��д洢�飬���������ȴ�������
        tEventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);

        // �����ִ��һ���¼����ȣ��Ա����л�����������
        tTaskSched();

        // ���л�����ʱ����tTask��ȡ����õ���Ϣ
        *mem = currentTask->eventMsg;

        // ȡ���ȴ����
        return currentTask->waitEventResult;
    }
}


/* ��ȡ�洢�麯��, �޵ȴ� */
uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, uint8_t ** mem)
{
    uint32_t status = tTaskEnterCritical();

    // ���ȼ���Ƿ��п��еĴ洢��
    if (tList_count(&memBlock->blockList) > 0)
    {
        // ����еĻ���ȡ��һ��
        *mem = (uint8_t *)tList_removeFirst(&memBlock->blockList);
        tTaskExitCritical(status);
        return tErrorNoError;
    }
    else  
    {
        // ���û�п��д洢�飬ֱ���˳�
        tTaskExitCritical(status);

				// ������Դ������
        return tErrorResourceUnavalable;
    }
}

/* �ͷŴ洢�麯�� */
void tMemBlockNotify(tMemBlock * memBlock, uint8_t * mem)
{
    uint32_t status = tTaskEnterCritical();

    // ����Ƿ�������ȴ�
    if (tEventWaitCount(&memBlock->event) > 0)
    {
        // ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
        tTask * task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);

        // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
        }
    }
    else
    {
        // ���û������ȴ��Ļ������洢����뵽������
        tList_addLast(&memBlock->blockList, (tNode *)mem);
    }
    tTaskExitCritical(status);
}

/* ɾ���洢�麯�� */
uint32_t tMemBlockDestroy(tMemBlock * memBlock)
{
    uint32_t status = tTaskEnterCritical();

    // ����¼����ƿ��е����񣬷��ر��Ƴ�����������
    uint32_t count = tEventRemoveAll(&memBlock->event, (void *)0, tErrorDel);
    tTaskExitCritical(status);

    // ��չ����п��������������������ִ��һ���������
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}

#endif
