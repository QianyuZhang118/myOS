#include "tlib.h"

// �õ�������
uint32_t tList_count (tList * list)
{
	return list->nodeCount;
}

// �ڵ��ʼ��
void tNode_init(tNode * node)
{
   node->nextNode = node;
	 node->preNode =  node;
}

// �����ʼ����˫������
void tList_init(tList * list)
{
    list->headNode.nextNode = &(list->headNode);
	  list->headNode.preNode  = &(list->headNode);
    list->nodeCount = 0;
}

// �����һ���ڵ�
tNode * tList_first(tList * list)
{
	  tNode * node = (tNode *)0;
    if(list->nodeCount != 0)    //�������򷵻�0
		{
		   node = list->headNode.nextNode;
		}
    return node;
}

// �������һ���ڵ�
tNode * tList_last(tList * list)
{
    tNode * node = (tNode *)0;
    if(list->nodeCount != 0)    //�������򷵻�0
		{
		   node = list->headNode.preNode;
		}
    return node;
}

// ָ���ڵ��ǰһ���ڵ�
tNode * tList_pre(tList * list,tNode * node)
{
	  if(node->preNode == node)
			return (tNode *)0;
		
		return node->preNode;
}

// ָ���ڵ�ĺ�һ���ڵ�
tNode * tList_next(tList * list,tNode * node)
{
	  if(node->nextNode == node)
			return (tNode *)0;
		
		return node->nextNode;
}

// ɾ�����нڵ�
void tList_removeAll(tList * list)
{
    uint32_t count;
	  tNode * nextNode;
	  
	  nextNode = list->headNode.nextNode;
	  //��һ����㿪ʼ  ǰ����ָ���Լ�
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

// ͷ������ڵ�
void tList_addFirst(tList * list,tNode * node)
{
    node->preNode = list->headNode.nextNode->preNode;
    node->nextNode = list->headNode.nextNode;
	
	  list->headNode.nextNode->preNode = node;
	  list->headNode.nextNode = node;
	
	  list->nodeCount++;

}

// β������ڵ�
void tList_addLast(tList * list,tNode * node)
{
    node->nextNode = &(list->headNode);
    node->preNode =  list->headNode.preNode;

    list->headNode.preNode->nextNode = node;
    list->headNode.preNode = node;
	
	  list->nodeCount++;
}

// ɾ����һ���ڵ�
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

// ��ָ���ڵ������µĽڵ�
void tList_insertAfter(tList * list,tNode * nodeAfter,tNode * nodetoInsert)
{
		nodetoInsert->preNode = nodeAfter;
	  nodetoInsert->nextNode = nodeAfter->nextNode;
	
	  nodeAfter->nextNode->preNode = nodetoInsert;
    nodeAfter->nextNode = nodetoInsert;

    list->nodeCount++;
}

// ɾ��ָ���ڵ�
void tList_remove(tList * list,tNode * node)
{
    node->preNode->nextNode = node->nextNode;
	  node->nextNode->preNode = node->preNode;
	  list->nodeCount--;
}
