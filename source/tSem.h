#ifndef _TSEM_H
#define _TSEM_H

#include "tEvent.h"

/* �ź����ṹ���� */
typedef struct _tSem
{
	tEvent event;  // tSemͬʱ��һ��tEvent
	uint32_t count; // ��ǰ����ֵ
	uint32_t maxCount;  // ������ֵ
}tSem;

/* �ź�����״̬�ṹ���� */
typedef struct _tSemInfo
{
    // ��ǰ�ź����ļ���
    uint32_t count;
    // �ź��������������
    uint32_t maxCount;
    // ��ǰ�ȴ���������
    uint32_t taskCount;
} tSemInfo;

/* �ź�����ʼ�� */
void tSemInit(tSem * sem, uint32_t startCount, uint32_t maxCount);

/* ����tSemWait���ڵȴ��ź��� */
uint32_t tSemWait(tSem * sem, uint32_t waitTicks);


/* ����tSemNotify����֪ͨ�ź��� */
void tSemNotify(tSem * sem);

/* tSemNoWaitGet �ź������޵ȴ���ȡ����  */
uint32_t tSemNoWaitGet(tSem * sem);

/* ״̬��ѯ�ӿ� ����tSemGetInfo���ڻ�ȡ�ź�������Ϣ */ 
void tSemGetInfo(tSem * sem, tSemInfo * info);

/* ����tSemDestroy���������ź��� */ 
uint32_t tSemDestroy(tSem * sem);


#endif

