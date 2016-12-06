










#ifndef BINHEAP_H
#define BINHEAP_H

#include "../platform.h"
#include "mempool.h"
#include "heapkey.h"

class PathNode;
struct HeapKey;

// http://www.sourcetricks.com/2011/06/c-heaps.html

class BinHeap
{
public:
	BinHeap();
	~BinHeap();
	bool add(void* element);
	void* delmin();
	bool hasmore();
	void alloc(int32_t ncells);
	void free();
	//void resetelems();
	void heapify(void* element, HeapKey* oldhk);
#if 0
	void print();
#endif
//private:
	int32_t left(int32_t parent);
	int32_t right(int32_t parent);
	int32_t parent(int32_t child);
	void heapifyup(int32_t index);
	void heapifydown(int32_t index);
//private:
#if 0
	PathNode** heap;
	int32_t nelements;
	int32_t allocsz;
#else
	std::vector<void*> heap;
	//bool (*comparefunc)(void* a, void* b);
#endif

	
	void setbase(HeapKey basecost){}

	//int total;
	//int used;
};

#endif
