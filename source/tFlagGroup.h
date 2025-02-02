#ifndef _TFLAGGROUP_H
#define _TFLAGGROUP_H

#include "tEvent.h"


/* �궨�壺����ȴ������¼��������� */ 
#define TFLAGGROUP_CLEAR		(0x0 << 0)  // ���
#define TFLAGGROUP_SET			(0x1 << 0)  // ����
#define TFLAGGROUP_ANY			(0x0 << 1)  // �����־λ���û�����
#define TFLAGGROUP_ALL			(0x1 << 1)  // ���б�־λ���û�����

#define TFLAGGROUP_SET_ALL		(TFLAGGROUP_SET | TFLAGGROUP_ALL)  // ���б�־λ����
#define TFLAGGROUP_SET_ANY		(TFLAGGROUP_SET | TFLAGGROUP_ANY)  // �����־λ����
#define TFLAGGROUP_CLEAR_ALL		(TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)  // ���б�־λ����
#define TFLAGGROUP_CLEAR_ANY		(TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)  // �����־λ����

#define TFLAGGROUP_CONSUME		(1 << 7)  // ������ȴ�����Ӧ���¼���ʱ���Ƿ���Ҫ����Ӧλ�������1


/* �¼���־��ṹ���� */
typedef struct _tFlagGroup
{
	tEvent event;  // �¼����ƿ�
	
	uint32_t flags;  // 32���¼���־λ
	
}tFlagGroup;

/* �¼���־����Ϣ�ṹ�� */
typedef struct _tFlagGroupInfo
{
    // ��ǰ���¼���־
    uint32_t flags;
    // ��ǰ�ȴ����������
    uint32_t taskCount;
} tFlagGroupInfo;

/* �¼���־���ʼ�� */ 
void tFlagGroupInit(tFlagGroup * flagGroup, uint32_t flags);

/* �¼���־��ȴ����� */
uint32_t tFlagGroupWait(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag,
                        uint32_t * resultFlag, uint32_t waitTicks);

/* �¼���־���ȡ/������ */
uint32_t tFlagGroupNoWaitGet(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag);

/* ֪ͨ��־���¼� */
void tFlagGroupNotify(tFlagGroup * flagGroup, uint8_t isSet, uint32_t flags);

#endif

