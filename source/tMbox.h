#ifndef _TMBOX_H
#define _TMBOX_H

#include "tinyOS.h"


#define tMBOXSendNormal  	0x00  // һ�����ȼ���Ϣ
#define tMBOXSendFront  	0x01  // �����ȼ���Ϣ

// ��������
typedef struct _tMbox
{
    // �ýṹ������ŵ���ʼ������ʵ��tSemͬʱ��һ��tEvent��Ŀ��
    tEvent event; // �¼����ƿ�
	 
    uint32_t count; // ��ǰ����Ϣ����
	   
    uint32_t read;  // ��ȡ��Ϣ������ ������
	   
    uint32_t write;  // д��Ϣ������  д����
	    
    uint32_t maxCount; // ����������ɵ���Ϣ����
	
    void ** msgBuffer; // ��Ϣ�洢������
} tMbox;

// ����״̬����
typedef struct _tMboxInfo 
{
    uint32_t count;  // ��ǰ����Ϣ����
    
    uint32_t maxCount;  // ����������ɵ���Ϣ����
    
    uint32_t taskCount;  // ��ǰ�ȴ���������
} tMboxInfo;


/* �����ʼ�� */
void tMboxInit(tMbox * mbox, void ** msgBuffer, uint32_t maxCount);

/* ��ȡ��Ϣ����������Ҫ�������ȡ��Ϣ�� */
uint32_t tMboxWait(tMbox * mbox, void ** msg, uint32_t waitTicks);

/* ��ȡ��Ϣ���������ȴ������û����Ϣ��ֱ���˳��� */
uint32_t tMboxNoWaitGet(tMbox * mbox, void ** msg);

/* ��Ϣ֪ͨ��������ϢҪ���͸����䣩
@ notifyOption�����ѡ���ʾ��Ϣ�Ƿ��Ǹ������ȼ���Ϣ���ǣ���ᱻ���ȴ洢*/
uint32_t tMboxNotify(tMbox * mbox, void * msg, uint32_t notifyOption);

/* ������亯��(��ջ�����) */
void tMboxFlush(tMbox * mbox);

/* ɾ�����亯��(�Ƴ��ȴ�����������) */
uint32_t tMboxDestroy(tMbox * mbox);

/* ������Ϣ��ѯ���� */
void tMboxGetInfo(tMbox * mbox, tMboxInfo * info);

#endif


