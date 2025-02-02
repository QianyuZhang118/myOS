#ifndef _TMEMBLOCK_H
#define _TMEMBLOCK_H

#include "tEvent.h"

/* 存储块结构体类型定义 */
typedef struct _tMemBlock
{
    tEvent event;     // 事件控制块

    void * memStart;    // 存储块的首地址

    uint32_t blockSize;    // 每个存储块的大小

    uint32_t maxCount;    // 存储块的个数上限

    tList blockList;    // 存储块列表
} tMemBlock;

/* 存储块信息结构体定义 */
typedef struct _tMemBlockInfo
{
    uint32_t count;    // 当前存储块的计数

    uint32_t maxCount;    // 允许的最大计数

    uint32_t blockSize;    // 每个存储块的大小

    uint32_t taskCount;    // 当前等待的任务计数
} tMemBlockInfo;

/* 存储块初始化 */
void tMemBlockInit(tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);

/* 获取存储块函数 */
uint32_t tMemBlockWait(tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks);

/* 获取存储块函数, 无等待 */
uint32_t tMemBlockNoWaitGet(tMemBlock * memBlock, uint8_t ** mem);

/* 释放存储块函数 */
void tMemBlockNotify(tMemBlock * memBlock, uint8_t * mem);

/* 删除存储块函数 */
uint32_t tMemBlockDestroy(tMemBlock * memBlock);

#endif








