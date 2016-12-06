










#include "binheap.h"
#include "../path/pathnode.h"
#include "../utils.h"

BinHeap::BinHeap()
{
#if 0
	heap = NULL;
	allocsz = 0;
	nelements = 0;
#endif
	//comparefunc = comparef;
}

BinHeap::~BinHeap()
{
	free();
}

//todo get rid
void BinHeap::alloc(int32_t ncells)
{
#if 0
	freemem();

	heap = new PathNode* [ ncells ];
	allocsz = ncells;
	nelements = 0;
#endif
}

void BinHeap::free()
{
#if 0
	if(heap)
	{
		delete [] heap;
		heap = NULL;
	}

	allocsz = 0;
	nelements = 0;
#else
	heap.clear();

	//total = used = 0;
#endif
}

//this is part of the underlying memory pool, not the BinHeap
#if 0
void BinHeap::resetelems()
{
#if 0
	nelements = 0;
#else
	heap.clear();
#endif
}
#endif

bool BinHeap::add(void* element)
{
#if 0

	if(nelements >= allocsz)
		return false;

	heap[nelements] = element;
	nelements++;
	heapifyup(nelements - 1);
#else
	heap.push_back(element);
	heapifyup(heap.size() - 1);
#endif

	//total++;

	return true;
}

bool BinHeap::hasmore()
{
#if 0
	return nelements > 0;
#else
	//return heap.size() > 0;
	return (bool)heap.size();
#endif
}

//todo macro BINHEAP_HASMORE for c
//PRIOQUEUE_HASMORE

void* BinHeap::delmin()
{
#if 0
	PathNode* pmin = heap[0];
	heap[0] = heap[nelements - 1];
	nelements--;
	heapifydown(0);
	return pmin;
#else
	void* pmin = heap.front();
	heap[0] = heap.at(heap.size() - 1);
	heap.pop_back();
	heapifydown(0);
	
	//used++;

	return pmin;
#endif
}

void BinHeap::heapify(void* element, HeapKey* oldhk)
{
#if 1
	if(heap.size() <= 0)
		return;

	if(element)
	{
		bool found = false;
		int32_t i = 0;

		for(std::vector<void*>::iterator iter = heap.begin(); iter != heap.end(); iter++, i++)
			if(*iter == element)
			{
				found = true;
				break;
			}

		if(found)
		{
			heapifydown(i);
			heapifydown(i);
		}
		return;
	}
	for(int32_t i = (int32_t)(heap.size()/2); i; i--)
	{
		heapifydown(i);
	}
	return;
#endif
}

#if 0
void BinHeap::print()
{
	std::vector<int32_t>::iterator pos = heap.begin();
	cout << "BinHeap = ";
	while ( pos != heap.end() )
	{
		cout << *pos << " ";
		++pos;
	}
	cout << std::endl;
}
#endif

void BinHeap::heapifyup(int32_t index)
{
	//cout << "index=" << index << std::endl;
	//cout << "parent(index)=" << parent(index) << std::endl;
	//cout << "heap[parent(index)]=" << heap[parent(index)] << std::endl;
	//cout << "heap[index]=" << heap[index] << std::endl;

	int32_t p = parent(index);
	
	//while ( p < heap.size() && ( index > 0 ) && ( parent(index) >= 0 ) && comparefunc(heap[parent(index)], heap[index])
	while ( p < heap.size() && ( index > 0 ) && ( p >= 0 ) && HEAPCOMPARE( HEAPKEY(heap[p]), HEAPKEY(heap[index]) )
	                /* ( heap[parent(index)]->cost > heap[index]->cost ) */ )
	{
		void* tmp = heap[p];
		heap[p] = heap[index];
		heap[index] = tmp;
		index = p;

		p = parent(index);
	}
}

void BinHeap::heapifydown(int32_t index)
{
	//cout << "index=" << index << std::endl;
	//cout << "left(index)=" << left(index) << std::endl;
	//cout << "right(index)=" << right(index) << std::endl;
	int32_t child = left(index);
	//todo is child > 0 always?
	//todo do error checking here like left < heap.size() && right < heap.size() so that they don't have ot be done in left() and right()

	int32_t r = right(index);

	//if ( ( child > 0 ) && ( right(index) > 0 ) && comparefunc(heap[child], heap[right(index)])	
	if ( ( child < heap.size() ) && ( r < heap.size() ) && HEAPCOMPARE( HEAPKEY(heap[child]), HEAPKEY(heap[r]) )
	              /*  ( heap[child]->cost > heap[right(index)]->cost ) */ )
	{
		child = r;
	}
	//if ( child > 0 )
	if ( child < heap.size() && child > 0 )
	{
		void* tmp = heap[index];
		heap[index] = heap[child];
		heap[child] = tmp;
		heapifydown(child);
	}
}

int32_t BinHeap::left(int32_t parent)
{
	//int32_t i = ( parent * 2 ) + 1; // 2 * parent + 1
#if 0
	return ( i < nelements ) ? i : -1;
#else
	//return ( i < heap.size() ) ? i : -1;
	return ( parent << 2 ) + 1;
#endif
}

int32_t BinHeap::right(int32_t parent)
{
	//todo <<1
	//todo no error checking
	//int32_t i = ( parent * 2 ) + 2; // 2 * parent + 2
#if 0
	return ( i < nelements ) ? i : -1;
#else
	//return ( i < heap.size() ) ? i : -1;
	return ( parent << 2 ) + 2;
#endif
}

int32_t BinHeap::parent(int32_t child)
{
	//if (child != 0)
	if (child)
	{
		//int32_t i = (child - 1) / 2;
		//return i;
		return (child - 1) >> 2;
	}
	return -1;
}

#if 0
int32_t main()
{
	// Create the heap
	BinHeap* myheap = new BinHeap();
	myheap->insert(700);
	myheap->print();
	myheap->insert(500);
	myheap->print();
	myheap->insert(100);
	myheap->print();
	myheap->insert(800);
	myheap->print();
	myheap->insert(200);
	myheap->print();
	myheap->insert(400);
	myheap->print();
	myheap->insert(900);
	myheap->print();
	myheap->insert(1000);
	myheap->print();
	myheap->insert(300);
	myheap->print();
	myheap->insert(600);
	myheap->print();

	// Get priority element from the heap
	int32_t heapSize = myheap->size();
	for ( int32_t i = 0; i < heapSize; i++ )
		cout << "Get std::min element = " << myheap->delmin() << std::endl;

	// Cleanup
	delete myheap;
}
#endif
