#ifndef _TEVENT_H
#define _TEVENT_H

#include "tlib.h"
#include "tTask.h"

/* �¼�����ö�ٽṹ */
typedef enum _tEventType
{
    tEventTypeUnknown, 
		tEventTypeSem, 
		tEventTypeMbox, 
		tEventTypeMemBlock, 
		tEventTypeFlagGroup, 
		tEventTypeMutex,
}tEventType;


/* �¼����ƿ�ṹ */
typedef struct _tEvent
{
	tEventType type;
	
	tList waitList;  // �ȴ�����
}tEvent;


/* �¼����ƿ��ʼ������ */
void tEventInit(tEvent * event, tEventType type);

/* �¼����ƿ�ȴ��ӿ� */
void tEventWait(tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout);

/* ��������¼����ƿ��л��� */
tTask * tEventWakeUp(tEvent * event, void * msg, uint32_t result);

/* ��ָ��������¼����ƿ��л��� */
tTask * tEventWakeUpTask(tEvent * event, tTask * task, void * msg, uint32_t result);

/* ��������¼����ƿ���ǿ���Ƴ� */
void tEventRemoveTask(tTask * task, void * msg, uint32_t result);


/* ����¼����ƿ�ȴ����У�����ɾ��ǰ�ȴ��������ж������� */
uint32_t tEventRemoveAll(tEvent * event, void * msg, uint32_t result);

/* ��ȡ�¼����ƿ��еȴ����������� */
uint32_t tEventWaitCount(tEvent * event);

#endif



