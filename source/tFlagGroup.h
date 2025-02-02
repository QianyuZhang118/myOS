#ifndef _TFLAGGROUP_H
#define _TFLAGGROUP_H

#include "tEvent.h"


/* 宏定义：任务等待哪种事件设置类型 */ 
#define TFLAGGROUP_CLEAR		(0x0 << 0)  // 清除
#define TFLAGGROUP_SET			(0x1 << 0)  // 设置
#define TFLAGGROUP_ANY			(0x0 << 1)  // 任意标志位设置或清零
#define TFLAGGROUP_ALL			(0x1 << 1)  // 所有标志位设置或清零

#define TFLAGGROUP_SET_ALL		(TFLAGGROUP_SET | TFLAGGROUP_ALL)  // 所有标志位设置
#define TFLAGGROUP_SET_ANY		(TFLAGGROUP_SET | TFLAGGROUP_ANY)  // 任意标志位设置
#define TFLAGGROUP_CLEAR_ALL		(TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)  // 所有标志位清零
#define TFLAGGROUP_CLEAR_ANY		(TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)  // 任意标志位清零

#define TFLAGGROUP_CONSUME		(1 << 7)  // 当任务等待到对应的事件组时，是否需要将对应位清零或置1


/* 事件标志组结构定义 */
typedef struct _tFlagGroup
{
	tEvent event;  // 事件控制块
	
	uint32_t flags;  // 32个事件标志位
	
}tFlagGroup;

/* 事件标志组信息结构体 */
typedef struct _tFlagGroupInfo
{
    // 当前的事件标志
    uint32_t flags;
    // 当前等待的任务计数
    uint32_t taskCount;
} tFlagGroupInfo;

/* 事件标志组初始化 */ 
void tFlagGroupInit(tFlagGroup * flagGroup, uint32_t flags);

/* 事件标志组等待操作 */
uint32_t tFlagGroupWait(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag,
                        uint32_t * resultFlag, uint32_t waitTicks);

/* 事件标志组获取/检查操作 */
uint32_t tFlagGroupNoWaitGet(tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag);

/* 通知标志组事件 */
void tFlagGroupNotify(tFlagGroup * flagGroup, uint8_t isSet, uint32_t flags);

#endif

