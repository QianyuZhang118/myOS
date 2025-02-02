#ifndef _TMEMBLOCK_H
#define _TMEMBLOCK_H

#include "tEvent.h"

/* �洢��ṹ�����Ͷ��� */
typedef struct _tMemBlock
{
    tEvent event;     // �¼����ƿ�

    void * memStart;    // �洢����׵�ַ

    uint32_t blockSize;    // ÿ���洢��Ĵ�С

    uint32_t maxCount;    // �洢��ĸ�������

    tList blockList;    // �洢���б�
} tMemBlock;

/* �洢����Ϣ�ṹ�嶨�� */
typedef struct _tMemBlockInfo
{
    uint32_t count;    // ��ǰ�洢��ļ���

    uint32_t maxCount;    // �����������

    uint32_t blockSize;    // ÿ���洢��Ĵ�С

    uint32_t taskCount;    // ��ǰ�ȴ����������
} tMemBlockInfo;

/* �洢���ʼ�� */
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);

/* ��ȡ�洢�麯�� */
uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);

/* ��ȡ�洢�麯��, �޵ȴ� */
uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, uint8_t ** mem);

/* �ͷŴ洢�麯�� */
void tMemBlockNotify(tMemBlock * memBlock, uint8_t * mem);

/* ɾ���洢�麯�� */
uint32_t tMemBlockDestroy(tMemBlock * memBlock);

#endif








