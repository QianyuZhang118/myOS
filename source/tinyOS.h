#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>
#include "tlib.h"
#include "tconfig.h"
#include "tEvent.h"
#include "tTask.h"
#include "tSem.h"
#include "tMbox.h"
#include "tMemBlock.h"
#include "tFlagGroup.h"
#include "tMutex.h"
#include "tTimer.h"
#include "tHooks.h"
#include "myTask.h"


//  1s ��tick����
#define TICKS_PER_SEC (1000 / TINYOS_SYSTICK_MS)

/* ������ö�ٽṹ */
typedef enum _error
{
    tErrorNoError = 0,
		tErrorTimeOut,
		tErrorResourceUnavalable, 
		tErrorDel, 
		tErrorResourceFull, 
		tErrorOwner, 
}tError;

extern tTask *currentTask; 	// ָ��ǰ�����ָ��
extern tTask *nextTask;		// ָ����һ�����ָ��

void tTaskRunFirst(void);
void tTaskSwitch(void);


/* �ٽ���������غ����������жϣ� */
uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);


/* ��������غ��� */
void tTaskSchedInit(void);
void tTaskSchedDisable(void);
void tTaskSchedEnable(void);
void tTaskSchedRdy(tTask * task);
void tTaskSchedUnRdy(tTask * task);
void tTaskSched(void); 


/* ��ʱ��غ��� */
void tTimeTaskWait(tTask * task, uint32_t ticks);
void tTimeTaskWakeUp(tTask * task);
void tTaskSystemTickHandler(void);
void tTaskDelay(uint32_t delay);


/* SysTick��ʼ������, ����ϵͳʱ�ӽ��� */
void SysTick_init(uint32_t ms);


/* ����ɾ�� */
void tTimeTaskRemove(tTask * task);  // ���������ʱ������ɾ��
void tTaskSchedRemove(tTask * task); // ����������ȼ�������ɾ��

/* CPUʹ���ʻ�ȡ�Ľӿں��� */
float tCpuUsageGet(void);


#endif
