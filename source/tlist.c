#include "tlib.h"

// 得到链表长度
uint32_t tList_count (tList * list)
{
	return list->nodeCount;
}

// 节点初始化
void tNode_init(tNode * node)
{
   node->nextNode = node;
	 node->preNode =  node;
}

// 链表初始化（双向链表）
void tList_init(tList * list)
{
    list->headNode.nextNode = &(list->headNode);
	  list->headNode.preNode  = &(list->headNode);
    list->nodeCount = 0;
}

// 链表第一个节点
tNode * tList_first(tList * list)
{
	  tNode * node = (tNode *)0;
    if(list->nodeCount != 0)    //不存在则返回0
		{
		   node = list->headNode.nextNode;
		}
    return node;
}

// 链表最后一个节点
tNode * tList_last(tList * list)
{
    tNode * node = (tNode *)0;
    if(list->nodeCount != 0)    //不存在则返回0
		{
		   node = list->headNode.preNode;
		}
    return node;
}

// 指定节点的前一个节点
tNode * tList_pre(tList * list,tNode * node)
{
	  if(node->preNode == node)
			return (tNode *)0;
		
		return node->preNode;
}

// 指定节点的后一个节点
tNode * tList_next(tList * list,tNode * node)
{
	  if(node->nextNode == node)
			return (tNode *)0;
		
		return node->nextNode;
}

// 删除所有节点
void tList_removeAll(tList * list)
{
    uint32_t count;
	  tNode * nextNode;
	  
	  nextNode = list->headNode.nextNode;
	  //第一个结点开始  前后结点指向自己
	  for(count = list->nodeCount; count != 0; count--)
	  {
		     tNode * currentNode = nextNode;
			   nextNode = nextNode->nextNode;
			   
			   currentNode->nextNode = currentNode;
			   currentNode->preNode = currentNode;
		}
		
		list->headNode.nextNode = &(list->headNode);
		list->headNode.preNode  = &(list->headNode);
		list->nodeCount = 0;
}

// 头部插入节点
void tList_addFirst(tList * list,tNode * node)
{
    node->preNode = list->headNode.nextNode->preNode;
    node->nextNode = list->headNode.nextNode;
	
	  list->headNode.nextNode->preNode = node;
	  list->headNode.nextNode = node;
	
	  list->nodeCount++;

}

// 尾部插入节点
void tList_addLast(tList * list,tNode * node)
{
    node->nextNode = &(list->headNode);
    node->preNode =  list->headNode.preNode;

    list->headNode.preNode->nextNode = node;
    list->headNode.preNode = node;
	
	  list->nodeCount++;
}

// 删除第一个节点
tNode * tList_removeFirst(tList * list)
{
    tNode * node = (tNode *) 0;
	  if(list->nodeCount != 0)
		{
		    node = list->headNode.nextNode;
		    node->nextNode->preNode = &(list->headNode);
		    list->headNode.nextNode = node->nextNode;
			  list->nodeCount--;
		}
		return node;
}

// 在指定节点后插入新的节点
void tList_insertAfter(tList * list,tNode * nodeAfter,tNode * nodetoInsert)
{
		nodetoInsert->preNode = nodeAfter;
	  nodetoInsert->nextNode = nodeAfter->nextNode;
	
	  nodeAfter->nextNode->preNode = nodetoInsert;
    nodeAfter->nextNode = nodetoInsert;

    list->nodeCount++;
}

// 删除指定节点
void tList_remove(tList * list,tNode * node)
{
    node->preNode->nextNode = node->nextNode;
	  node->nextNode->preNode = node->preNode;
	  list->nodeCount--;
}
