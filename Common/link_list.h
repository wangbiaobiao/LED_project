#ifndef _LINK_LIST_H_
#define _LINK_LIST_H_
#include "xml_parse.h"

#define Item strategy_Timetable
typedef struct _strategy_Timetable
{
        int startMoment;
        int endMoment;
}strategy_Timetable;

typedef struct Node* PNode;
typedef PNode Position;

/*定义节点类型*/
typedef struct Node
{
	struct _strategy_Timetable data;		/*数据域*/
	PNode previous; /*指向前驱*/
	PNode next;		/*指向后继*/
}Node;
/*定义链表类型*/
typedef struct _DList
{
	PNode head;		/*指向头节点*/
	PNode tail;		/*指向尾节点*/
	int size;
}DList;

Position MakeNode(Item* i);

void FreeNode(PNode p);

DList* InitList();

void DestroyList(DList *plist);

void ClearList(DList *plist);

Position GetHead(DList *plist);

Position GetTail(DList *plist);

int GetSize(DList *plist);

Position GetNext(Position p);

Position GetPrevious(Position p);

PNode InsFirst(DList *plist,PNode pnode);

PNode DelFirst(DList *plist);

PNode Remove(DList *plist);

PNode InsBefore(DList *plist,Position p,PNode s);

void InsAfter(DList *plist,Position p,PNode s);

PNode LocatePos(DList *plist,int i);

Node* ListTraverse(DList *plist, Position p, int data);

#endif

