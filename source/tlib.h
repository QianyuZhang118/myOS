#ifndef _TLIB_H
#define _TLIB_H

#include <stdint.h>

typedef struct _bitmap
{
    uint32_t bitmap;
}tBitmap;

typedef struct _tnode  // 节点
{
   struct _tnode * preNode;
	 struct _tnode * nextNode;
}tNode;

typedef struct _tlist  // 链表
{
   tNode headNode;  // 链表头结点
	 uint32_t nodeCount;  // 节点数量
}tList;

// 宏定义：如何通过结构体内某个字段的地址，反推结构体的地址
#define tNode_parent(node,parent,name)  (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name) 

//----------位图---------------
void 		 	tBitmapInit(tBitmap * bitmap);
void 			tBitmapSet(tBitmap * bitmap,uint32_t pos);
void 			tBitmapClear(tBitmap * bitmap,uint32_t pos);
uint32_t 	tBitmapGetFirstSet(tBitmap * bitmap);
uint32_t 	tBitmapPosCount(void);

//----------链表---------------
void 			tNode_init(tNode * node);
uint32_t 	tList_count (tList * list);
void 			tList_init(tList * list);
tNode* 		tList_first(tList * list);
tNode* 		tList_last(tList * list);
tNode* 		tList_pre(tList * list,tNode * node);
tNode* 		tList_next(tList * list,tNode * node);
void 			tList_removeAll(tList * list);
void 			tList_addFirst(tList * list,tNode * node);
void 			tList_addLast(tList * list,tNode * node);
tNode* 		tList_removeFirst(tList * list);
void 			tList_insertAfter(tList * list,tNode * nodeAfter,tNode * nodetoInsert);
void 			tList_remove(tList * list,tNode * node);
//void 			tList_addPriorityList(Task* task);
//void 			tList_removePriorityList(Task * task);
//void 			tList_addDelayList(Task* task,uint32_t delay);
//void 			tList_removeDelayList(Task* task);


#endif
