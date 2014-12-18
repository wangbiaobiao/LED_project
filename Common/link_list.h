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

/*����ڵ�����*/
typedef struct Node
{
	struct _strategy_Timetable data;		/*������*/
	PNode previous; /*ָ��ǰ��*/
	PNode next;		/*ָ����*/
}Node;
/*������������*/
typedef struct _DList
{
	PNode head;		/*ָ��ͷ�ڵ�*/
	PNode tail;		/*ָ��β�ڵ�*/
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

