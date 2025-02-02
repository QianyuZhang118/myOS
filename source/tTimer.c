#include "tinyOS.h"

#if TINYOS_ENABLE_TIMER == 1

static tList tTimerHardList; // ���жϷ��������Ķ�ʱ���б�
static tList tTimerSoftList;  // �ɶ�ʱ��������Ķ�ʱ���б�

static tSem tTimerProtectSem;  // ��ʱ��/�����������tTimerSoftList ���ź���
static tSem tTimerTickSem;   // �������жϷ��������֪ͨ��ʱ���������жϷ����� �ź���


static tTask tTimerTask;  // ���嶨ʱ������
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE]; // ���嶨ʱ����ջ

/* ��ʱ�����д������������б��еĸ�����ʱ�� */
static void tTimerCallFuncList(tList * timerList)
{
    tNode * node;
    // ������ʱ���б�
    for (node = timerList->headNode.nextNode; node!= &(timerList->headNode); node = node->nextNode)
    {
        tTimer * timer = tNode_parent(node, tTimer, linkNode);
				
				 // ��ʱ���ĵݼ�����Ϊ0������ʱ������,��ʱ��Ҫ����һ�ζ�ʱ���ص�����
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0)) 
        {
            timer->state = tTimerRunning; // ���ö�ʱ��״̬Ϊ��������
           
            timer->timerFunc(timer->arg); // ���ö�ʱ���Ļص����������������
           
            timer->state = tTimerStarted; // ���ö�ʱ��״̬Ϊ������
            
            if (timer->durationTicks > 0) // �жϸö�ʱ���Ƿ�����Ҫ�����Դ���
            {
                timer->delayTicks = timer->durationTicks; // �����ӳټ���ֵΪ����ֵ
            }
            else
            {
                // �����һ���Զ�ʱ���Ӷ�ʱ���б����Ƴ��ö�ʱ��
                tList_remove(timerList, &timer->linkNode);
               
                timer->state = tTimerStopped; // ���ö�ʱ��״̬Ϊ��ֹͣ
            }
        }
    }
}


/* ���嶨ʱ�������� */
static void tTimerSoftTask(void * param)
{
	for(;;)
	{
		tSemWait(&tTimerTickSem, 0);  // �����ȴ�����ϵͳʱ�ӽ��Ĵ���������tTimerTickSem������֪ͨ
		
		tSemWait(&tTimerProtectSem, 0); // �ȴ�tTimerSoftList�ź����������tTimerSoftList�ķ���Ȩ
		
		tTimerCallFuncList(&tTimerSoftList); // ���ö�ʱ�����д�����������tTimerSoftList��ʱ���б�
		
		tSemNotify(&tTimerProtectSem); // �ͷ��ź���
	}
}


/* ���ж��е��ã�֪ͨtTimerTickSem�ź�������Ӧ�ж��Ѿ�������tTimerSoftTask �������Լ���ִ��*/
void tTimerModuleTickNotify(void)
{
    uint32_t status = tTaskEnterCritical();
	
		// ���ڸú������ж��е��ã����Կ���ͬʱ����tTimerHardList��ʱ���б�
    tTimerCallFuncList(&tTimerHardList); 

    tTaskExitCritical(status);
   
    tSemNotify(&tTimerTickSem); // ֪ͨ/�ͷ��ź���
}


/* ��ʱ��ģ���ʼ�� */
void tTimerModuleInit(void)
{
    // ��ʼ����ʱ���б�
    tList_init(&tTimerHardList);
    tList_init(&tTimerSoftList);
	
    // ��ʼ���ź���
    tSemInit(&tTimerTickSem, 0, 0);
    tSemInit(&tTimerProtectSem, 1, 1);
}

/* ��ʱ�������ʼ������ ��Ϊʵ��CPU Usage���ܣ��ú�����tTimerModuleInit�ж��������� */
void tTimerTaskInit(void)
{
    // ���ԣ���ʱ����������ȼ�������� (TINYOS_PRO_COUNT - 1)
    // �������������������ʱ�ᱨ��
#if TINYOS_TIMERTASK_PRIO >= (TINY_PRIO_COUNT - 1)
		#error "The priority of timer task must be greater than (TINYOS_PRO_COUNT - 1)"
#endif
	
    // ��ʼ����ʱ������
    tTaskInit(&tTimerTask, tTimerSoftTask, (void *)0, TINYOS_TIMERTASK_PRIO, tTimerTaskStack, sizeof(tTimerTaskStack));
}


/* ��ʱ����ʼ������ */
void tTimerInit(tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                void (*timerFunc)(void * arg), void * arg, uint32_t config)
{
    tNode_init(&timer->linkNode); // ��ʼ����ʱ����������
   
    timer->startDelayTicks = delayTicks; // ���ó��������ӳٵ�ticks��
   
    timer->durationTicks = durationTicks; // �������ڶ�ʱ������tick��
  
    timer->timerFunc = timerFunc;  // ���ö�ʱ�ص�����
   
    timer->arg = arg; // ���ô��ݸ��ص������Ĳ���
  
    timer->config = config;  // ���ö�ʱ�����ò���
	
    // �����ʼ�����ӳ�Ϊ0����ʹ������ֵ��Ϊ��ǰ��ʱ�ݼ�����ֵ
    if (delayTicks == 0)
    {
        timer->delayTicks = durationTicks;
    }
    else
    {
        // ����ʹ�ó��������ӳ�ֵ��Ϊ��ǰ��ʱ�ݼ�����ֵ
        timer->delayTicks = timer->startDelayTicks;
    }
    // ���ö�ʱ��״̬Ϊ�Ѵ���
    timer->state = tTimerCreated;
}

/* ��ʱ�����������������Ӧ�Ķ�ʱ���б� */
void tTimerStart(tTimer * timer)
{
    switch (timer->state)  // �ж϶�ʱ��״̬
    {
			// ��ʱ��ֻ�д����Ѵ�������ֹͣ״̬�£�����������������״̬��ֱ���˳�
        case tTimerCreated:
        case tTimerStopped:
					
            // ���ö�ʱ���ĵ�ǰ��ʱ�ݼ�����ֵ
            // ����г��������ӳ٣���ʹ�ø�ֵ������ʹ������ֵ
            timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
           
            timer->state = tTimerStarted; // ���ö�ʱ��״̬Ϊ������
				
            // ���ݶ�ʱ�����ͼ�����Ӧ�Ķ�ʱ���б�
            if (timer->config & TIMER_CONFIG_TYPE_HARD) // ���жϷ��������Ķ�ʱ��
            {
                uint32_t status = tTaskEnterCritical();
                tList_addLast(&tTimerHardList, &timer->linkNode);  // ����ʱ�����붨ʱ���б�
                tTaskExitCritical(status);
            }
            else  // �ɶ�ʱ��������Ķ�ʱ��
            {
								// ���ڴ˴������㹻�򵥣���˲���Ҫ�����ź���������
								// tTimerSoftList���趨Ϊֻ���ɶ�ʱ�����������������ʣ��ж��޷����ʣ��������ʹ���ٽ�������
								// ���ԣ�����ʹ�ü���Ϊ1���ź���
                tSemWait(&tTimerProtectSem, 0);
                tList_addLast(&tTimerSoftList, &timer->linkNode);
                tSemNotify(&tTimerProtectSem);
							
								// ע�⣬����ٶ�tTimerStart��tTimerStop������������õģ�
								// �����Ҫ���жϵ��ã�����ʹ���ź�������ʹ���ٽ���������
            }
            break;
						
        default:
            break;
    }
}


/* ��ʱ��ֹͣ���� */
void tTimerStop(tTimer * timer)
{
    // ���ݶ�ʱ���ĵ�ǰ״̬���в���
    switch (timer->state)
    {
			// ��ʱ��ֻ�д�������������������״̬�£�������ֹͣ������״̬��ֱ���˳�
        case tTimerStarted:
        case tTimerRunning:
            // �ж϶�ʱ�����ͣ�Ȼ�����Ӧ����ʱ�б����Ƴ�
            if (timer->config & TIMER_CONFIG_TYPE_HARD)  // ���жϷ��������Ķ�ʱ��
            {
                uint32_t status = tTaskEnterCritical();
                tList_remove(&tTimerHardList, &timer->linkNode);   // �Ӷ�ʱ���б����Ƴ���ʱ��
                tTaskExitCritical(status);
            }
            else
            {
                tSemWait(&tTimerProtectSem, 0);  // �ȴ��ź�������ȡtTimerSoftList��ռ��Ȩ
                tList_remove(&tTimerSoftList, &timer->linkNode);
                tSemNotify(&tTimerProtectSem);
            }
           
            timer->state = tTimerStopped; // ���ö�ʱ��״̬Ϊ��ֹͣ
            break;
        default:
            break;
    }
}

/* ��ʱ��ɾ������ */
void tTimerDestroy(tTimer * timer)
{
	tTimerStop(timer);  // ���ö�ʱ��ֹͣ��������
	
	timer->state = tTimerDestroyed;  // ״̬���Ϊ����
}

/* ��ʱ��״̬��ѯ���� */
void tTimerGetInfo(tTimer * timer, tTimerInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    info->startDelayTicks = timer->startDelayTicks;

    info->durationTicks = timer->durationTicks;

    info->timerFunc = timer->timerFunc;

    info->arg = timer->arg;

    info->config = timer->config;

    info->state = timer->state;

    tTaskExitCritical(status);
}

#endif
