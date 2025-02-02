#ifndef _TMUTEX_H
#define _TMUTEX_H

#include "tEvent.h"

/* �����ź����ṹ���� */ 
typedef struct _tMutex
{
	// �¼����ƿ�
	tEvent event;
	
	// ����������
	uint32_t lockedCount;
	
	// ������
	tTask * owner;
	
	// �����ߵ�ԭʼ���ȼ�
	uint32_t ownerOriginalPrio;
	
}tMutex;	


/* �����ź�����ѯ�ṹ���� */ 
typedef struct _tMutexInfo
{
    // �ȴ�����������
    uint32_t taskCount;
    // ӵ������������ȼ�
    uint32_t ownerPrio;
    // �̳����ȼ�(��ǰ���ȼ�)
    uint32_t inheritedPrio;
    // ��ǰ�ź�����ӵ����
    tTask * owner;
    // ��������
    uint32_t lockedCount;
} tMutexInfo;


/* �����ź�����ʼ������ */
void tMutexInit(tMutex * mutex);

/* �ȴ������ź������� */
uint32_t tMutexWait(tMutex * mutex, uint32_t waitTicks);

/* �����ź����޵ȴ���ȡ������Ҳ���������� */
uint32_t tMutexNoWaitGet(tMutex * mutex);

/* ֪ͨ/�ͷŻ����ź������� */
uint32_t tMutexNotify(tMutex * mutex);

/* ɾ�������ź������� */
uint32_t tMutexDestroy(tMutex * mutex);

/* �����ź���״̬��ѯ */
void tMutexGetInfo(tMutex * mutex, tMutexInfo * info);

#endif
