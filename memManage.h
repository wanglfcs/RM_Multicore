#ifndef MEMMANAGE_H
#define MEMMANAGE_H

#define MMNODE 100

extern void * memPool;
extern long memPoolSize;

typedef struct freeUnit{
	void *start;
	long length;
	struct freeUnit *next;
	int used;
}freeUnit;

typedef struct assignMemTable{
	void *addr;
	long length;
	int used;
}assignMemTable;

void* mMalloc(int size);
void mFree(void *ptr);
void initMemManage(void *memPool,long memPoolSize);
freeUnit *findMmNode();

assignMemTable *findAssNode();

#endif
