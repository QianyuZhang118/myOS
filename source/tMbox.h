#ifndef _TMBOX_H
#define _TMBOX_H

#include "tinyOS.h"


#define tMBOXSendNormal  	0x00  // 一般优先级消息
#define tMBOXSendFront  	0x01  // 高优先级消息

// 邮箱类型
typedef struct _tMbox
{
    // 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
    tEvent event; // 事件控制块
	 
    uint32_t count; // 当前的消息数量
	   
    uint32_t read;  // 读取消息的索引 读索引
	   
    uint32_t write;  // 写消息的索引  写索引
	    
    uint32_t maxCount; // 最大允许容纳的消息数量
	
    void ** msgBuffer; // 消息存储缓冲区
} tMbox;

// 邮箱状态类型
typedef struct _tMboxInfo 
{
    uint32_t count;  // 当前的消息数量
    
    uint32_t maxCount;  // 最大允许容纳的消息数量
    
    uint32_t taskCount;  // 当前等待的任务数
} tMboxInfo;


/* 邮箱初始化 */
void tMboxInit(tMbox * mbox, void ** msgBuffer, uint32_t maxCount);

/* 获取消息函数（任务要从邮箱获取消息） */
uint32_t tMboxWait(tMbox * mbox, void ** msg, uint32_t waitTicks);

/* 获取消息函数（不等待，如果没有消息则直接退出） */
uint32_t tMboxNoWaitGet(tMbox * mbox, void ** msg);

/* 消息通知函数（消息要发送给邮箱）
@ notifyOption：这个选项表示消息是否是更高优先级消息。是，则会被优先存储*/
uint32_t tMboxNotify(tMbox * mbox, void * msg, uint32_t notifyOption);

/* 清空邮箱函数(清空缓存区) */
void tMboxFlush(tMbox * mbox);

/* 删除邮箱函数(移出等待的所有任务) */
uint32_t tMboxDestroy(tMbox * mbox);

/* 邮箱信息查询函数 */
void tMboxGetInfo(tMbox * mbox, tMboxInfo * info);

#endif


