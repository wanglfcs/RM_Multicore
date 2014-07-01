#include"memManage.h"
#include<string.h>
freeUnit *freeHead;
freeUnit mmNode[MMNODE];
assignMemTable assTable[MMNODE*2];

void initMemManage(void *memPool,long memPoolSize)
{
	freeHead=mmNode;
	memset(mmNode,0,sizeof(mmNode));
	memset(assTable,0,sizeof(assTable));
	freeHead->start=memPool;
	freeHead->length=memPoolSize;
	freeHead->next=0;
	freeHead->used=1;
}


void* mMalloc(int size)
{
	freeUnit *fptr=freeHead,*res=0;
	assignMemTable *assNodePtr=0;
	int diff=-1;
	while(fptr)
	{
		if(fptr->length>=size)
		{
			if(diff<0||((diff>0)&&(diff>fptr->length-size)))
			{
				diff=fptr->length-size;
				res=fptr;
			}
		}
		fptr=fptr->next;
	}
	if(res)
	{
		fptr=freeHead;
		assNodePtr=findAssNode();
		if(assNodePtr==0)
			return 0;
		assNodePtr->used=1;
		if(size==res->length)
		{
			if(res==freeHead)
			{
				freeHead->used=0;
				freeHead=freeHead->next;
			}
			else
			{
				while(fptr->next!=res)
					fptr=fptr->next;
				fptr->next=res->next;
			}
			res=res->start;
		}
		else
		{
			res->length-=size;
			res->start=res->start+size;
			res=res->start-size;
		}
		assNodePtr->addr=res;
		assNodePtr->length=size;
	}
	return res;
}

freeUnit *findMmNode()
{
	int i;
	for(i=0;i<MMNODE;i++)
	{
		if(mmNode[i].used==0)
			return mmNode+i;
	}
	return 0;
}

assignMemTable *findAssNode()
{
	int i;
	for(i=0;i<2*MMNODE;i++)
		if(assTable[i].used==0)
			return assTable+i;
	return 0;
}

void mFree(void *ptr)
{
	int size=0,i;
	freeUnit *fptr,*tptr;
	for(i=0;i<2*MMNODE;i++)
		if(assTable[i].used&&(assTable[i].addr==ptr))
		{
			size=assTable[i].length;
			assTable[i].used=0;
			break;
		}
	fptr=freeHead;
	while(fptr)
	{
		if((fptr->start+fptr->length)==ptr)
		{
			fptr->length+=size;

			if(freeHead->start==(ptr+size))
			{
				fptr->length+=freeHead->length;
				freeHead->used=0;
				freeHead=freeHead->next;
				return ;
			}
			tptr=freeHead;
			while(tptr->next)
			{
				if(tptr->next->start==(ptr+size))
				{
					fptr->length+=tptr->next->length;
					tptr->next->used=0;
					tptr->next=tptr->next->next;
					return ;
				}
				tptr=tptr->next;
			}
			return ;
		}
		else
			if(fptr->start==(ptr+size))
			{
				fptr->start=ptr;
				fptr->length+=size;
				return ;
			}
		fptr=fptr->next;
	}
	tptr=findMmNode();
	tptr->used=1;
	tptr->length=size;
	tptr->start=ptr;
	tptr->next=freeHead;
	freeHead=tptr;
}
