#ifndef _TTASK_H
#define _TTASK_H

#include <stdint.h>
#include "tlib.h"

/* ��16λ */
#define TINYOS_TASK_STATE_RDY 				0
#define TINYOS_TASK_STATE_DELAY 			(1 << 1)  // ��ʱ��־λ
#define TINYOS_TASK_STATE_SUSPEND			(1 << 2)  // �����־λ
#define TINYOS_TASK_STATE_DELETED			(1 << 3)  // ɾ����־λ

/* ��16λ */
#define TINYOS_TASK_WAIT_MASK					(0xFF << 16)  // �ȴ���־λ�����룩 �����16λֵ

/* ǰ�����ã����ڽ��tTask.h��tEvent.h�໥���������� */
struct _tEvent;


/* �����ջ�������� */
typedef uint32_t tTaskStack;

/* ����ṹ */
typedef struct _tTask
{
	tTaskStack *stack;  // ����ջָ��
	uint32_t * stackBase;  // ��ջ��ʼ��ַ
	uint32_t stackSize; // ��ջ�ܵĴ�С
	
	
	uint32_t delayTicks;  // ��ʱ��������������ʱ��
	tNode delayNode; // ���ڽ�������ӵ���ʱ������ �Ľڵ�
	tNode linkNode; // ���ڹ��ɾ�������ͬ���ȼ����� �Ľڵ�
	uint32_t prio;  // �������ȼ��ֶ�
	uint32_t state; // ��־�����Ƿ�����ʱ״̬
	uint32_t slice;  // ʱ��Ƭ����ֵ
	uint32_t spspendCount; // ���񱻹���Ĵ���
	
	void (*clean) (void * param);  // ɾ������Ļص�����
	void * cleanParam;
	uint8_t requestDeleteFlag;  // ����ɾ����־λ
	
	struct _tEvent * waitEvent; // �������ڵȴ���һ���¼����ƿ�
	// (ǰ������ֻ�ʺ���ָ�����ͣ���Ϊ������֪��ָ�����͵Ĵ�С�����ṹ������ֻ�������޷�ȷ����С)
	
	void * eventMsg; // 
	uint32_t waitEventResult; // �ȴ����
	
	uint32_t waitFlagsType;  // �������ͣ������¼���־��
	uint32_t eventFlags;  // �����¼���־�������¼���־��
	
}tTask;

/* ����״̬��ѯ�ṹ */
typedef struct __tTaskInfo
{
		uint32_t prio;
		uint32_t delayTicks;
		uint32_t state;
		uint32_t slice;
		uint32_t suspendCount;
	
		uint32_t stackSize;  // ��ջ�ܵĴ�С
		uint32_t stackFree;  // ��ջ���д�С
}tTaskInfo;

/* �����ʼ������ */
void tTaskInit(tTask * task, void(*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, uint32_t size);
void tInitApp(void);

/* ��������뻽�� */
void tTaskSuspend(tTask * task);
void tTaskWakeUp(tTask * task);

/* ����ɾ�� */
void tTaskSetCleanCallFunc(tTask * task, void (* clean)(void * param), void * param);  // �������������ص�����
void tTaskForceDelete(tTask * task); // ǿ��ɾ������
void tTaskRequestDelete(tTask * task);  // ����ɾ�������ӿڣ���������ɾ����ǣ�
uint8_t tTaskIsRequestDelete(void);  // ��鵱ǰ��������ɾ����������������ɾ����ǣ�
void tTaskDeleteSelf(void);  // ɾ�������Լ�

/* ����״̬��ѯ */
void tTaskGetInfo(tTask * task, tTaskInfo * Info);

#endif

