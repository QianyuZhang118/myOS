#ifndef _TTIMER_H
#define _TTIMER_H

#include "tEvent.h"

#define TIMER_CONFIG_TYPE_HARD		(1 << 0)  // ���жϷ����д���Ķ�ʱ��
#define TIMER_CONFIG_TYPE_SOFT			(0 << 0)  // �������д���Ķ�ʱ��

/* ��ʱ��״̬ö��ֵ */
typedef enum _tTimerState
{
	tTimerCreated, //�Ѵ���
	tTimerStarted,  // ������
	tTimerRunning,  // ��������
	tTimerStopped,  // ��ֹͣ
	tTimerDestroyed,  // ������
}tTimerState;

/* ��ʱ��״̬��ѯ�ṹ���� */
typedef struct _tTimerInfo
{
    uint32_t startDelayTicks; // ���������ӳٺ��ticks��
   
    uint32_t durationTicks; // ���ڶ�ʱʱ������tick��
   
    void (*timerFunc)(void * arg); // ��ʱ�ص�����
   
    void * arg; // ���ݸ��ص������Ĳ���
   
    uint32_t config; // ��ʱ�����ò���
   
    tTimerState state; // ��ʱ��״̬
} tTimerInfo;


/* ��ʱ���ṹ�嶨�� */
typedef struct _tTimer
{
    tNode linkNode; // ������
   
    uint32_t startDelayTicks; // ��ʼ��ʱtick��
   
    uint32_t durationTicks; // ����tick��
   
    uint32_t delayTicks; // ��ǰ��ʱ����ֵ
   
    void (*timerFunc)(void * arg); // ��ʱ�ص�����
   
    void * arg; // ���ݸ��ص������Ĳ���

    uint32_t config; // ��ʱ�����ò���
   
    tTimerState state; // ��ʱ��״̬
} tTimer;


/* ���嶨ʱ�������� */
static void tTimerSoftTask(void * param);

/* ��ʱ�����д������������б��еĸ�����ʱ�� */
static void tTimerCallFuncList(tList * timerList);

/* ���ж��е��ã�֪ͨtTimerTickSem�ź�������Ӧ�ж��Ѿ�������tTimerSoftTask �������Լ���ִ��*/
void tTimerModuleTickNotify(void);

/* ��ʱ��ģ���ʼ�� */
void tTimerModuleInit(void);

/* ��ʱ�������ʼ������ ��Ϊʵ��CPU Usage���ܣ��ú�����tTimerModuleInit�ж��������� */
void tTimerTaskInit(void);

/* ��ʱ����ʼ������ */
void tTimerInit(tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                void (*timerFunc)(void * arg), void * arg, uint32_t config);

/* ��ʱ�����������������Ӧ�Ķ�ʱ���б� */
void tTimerStart(tTimer * timer);

/* ��ʱ��ֹͣ���� */
void tTimerStop(tTimer * timer);

/* ��ʱ��ɾ������ */
void tTimerDestroy(tTimer * timer);

/* ��ʱ��״̬��ѯ���� */
void tTimerGetInfo(tTimer * timer, tTimerInfo * info);
#endif
