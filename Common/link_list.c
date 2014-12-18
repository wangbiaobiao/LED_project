#include "link_list.h"
#include <malloc.h>
#include <stdlib.h>
/*分配值为i的节点，并返回节点地址*/
Position MakeNode(Item* i)
{
	PNode p = NULL; 
	p = (PNode)malloc(sizeof(Node));
	if(p != NULL)
	{
		(p->data).startMoment = i->startMoment;
		(p->data).endMoment = i->endMoment;
		p->previous = NULL;
		p->next = NULL;
	}	
	return p;
}
/*释放p所指的节点*/
void FreeNode(PNode p)
{
	 free(p);
}
/*构造一个空的双向链表*/
DList * InitList()
{
	DList *plist = (DList *)malloc(sizeof(DList));
	strategy_Timetable t_node;
	t_node.startMoment = 0;
	t_node.endMoment = 0;
	PNode head = MakeNode(&t_node); 
	if(plist!=NULL)
	{
		if(head!=NULL)
		{
			plist->head = head;
			plist->tail = head;
			plist->size = 0;
		}
		else
			return NULL;
	}
	return plist;
}

/*摧毁一个双向链表*/
void DestroyList(DList *plist)
{
	ClearList(plist);
	free(GetHead(plist));
	free(plist);
}

/*判断链表是否为空表*/
int IsEmpty(DList *plist)
{
	if(GetSize(plist)==0&&GetTail(plist)==GetHead(plist))
		return 1;
	else
		return 0;
}
/*将一个链表置为空表，释放原链表节点空间*/
void ClearList(DList *plist)
{
	PNode temp,p;
	p = GetTail(plist);
	while(!IsEmpty(plist))
	{	
		temp = GetPrevious(p);
		FreeNode(p);
		p = temp;
		plist->tail = temp;
		plist->size--;
	}
}

/*返回头节点地址*/
Position GetHead(DList *plist)
{
	return plist->head;
}

/*返回尾节点地址*/
Position GetTail(DList *plist)
{
	return plist->tail;
}

/*返回链表大小*/
int GetSize(DList *plist)
{
	return plist->size;
}

/*返回p的直接后继位置*/
Position GetNext(Position p)
{
	return p->next;
}

/*返回p的直接前驱位置*/
Position GetPrevious(Position p)
{
	return p->previous;
}

/*将pnode所指节点插入第一个节点之前*/
PNode InsFirst(DList *plist,PNode pnode)
{
	Position head = GetHead(plist);

	if(IsEmpty(plist))
		plist->tail = pnode;
	plist->size++;

	pnode->next = head->next;
	pnode->previous = head;

	if(head->next!=NULL)
		head->next->previous = pnode;
	head->next = pnode;
	
	return pnode; 
}

/*将链表第一个节点删除,返回该节点的地址*/
PNode DelFirst(DList *plist)
{
	Position head = GetHead(plist);
	Position p=head->next;
	if(p!=NULL)
	{
		if(p==GetTail(plist))
			plist->tail = p->previous;
		head->next = p->next;
		head->next->previous = head;
		plist->size--;
		
	}	
	return p;
}


/*删除链表中的尾节点并返回地址，改变链表的尾指针指向新的尾节点*/
PNode Remove(DList *plist)
{
	Position p=NULL;
	if(IsEmpty(plist))
		return NULL;
	else
	{
		p = GetTail(plist);
		p->previous->next = p->next;
		plist->tail = p->previous;
		plist->size--;
		return p;
	}
}
/*在链表中p位置之前插入新节点s*/
PNode InsBefore(DList *plist,Position p,PNode s)
{
	s->previous = p->previous;
	s->next = p;
	p->previous->next = s;	
	p->previous = s;

	plist->size++;
	return s;
}
/*在链表中p位置之后插入新节点s*/
void InsAfter(DList *plist, Position p, PNode s)
{
	s->next = p->next;
	s->previous = p;
	
	if(p->next != NULL)
		p->next->previous = s;
	p->next = s;
	
	if(p = GetTail(plist))
		plist->tail = s;
	
	plist->size++;
}

/*返回在链表中第i个节点的位置*/
PNode LocatePos(DList *plist,int i)
{
	int cnt = 0;
	Position p = GetHead(plist);
	if(i>GetSize(plist)||i<1)
		return NULL;

	while(++cnt<=i)
	{
		p=p->next;
	}

	return p;
}

/*依次对链表中每个元素调用函数visit()*/
Node* ListTraverse(DList *plist, Position p, int data)
{
	//Position p = GetHead(plist);
	if(IsEmpty(plist))
		return GetHead(plist);
	else
	{	
		while(p->next != NULL)
		{
			p = p->next;
			if(p->data.startMoment > data)
//				if(p->previous == NULL)
//					return GetHead(plist);
				return p->previous;
		}	
		return p;
	}
}
