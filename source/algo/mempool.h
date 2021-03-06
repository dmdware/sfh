










#ifndef MEMPOOL_H
#define MEMPOOL_H

#include "../platform.h"

// http://www.codeproject.com/Articles/27487/Why-to-use-memory-pool-and-how-to-implement-it
// http://www.ibm.com/developerworks/aix/tutorials/au-memorymanager/au-memorymanager-pdf.pdf


class MemPool
{
private:
	struct MemUnit							//The type of the node of linkedlist.
	{
		struct MemUnit *pPrev, *pNext;
	};

	void*			m_pMemBlock;			//The address of memory pool.

	//Manage all unit with two linkedlist.
	struct MemUnit*	m_pAllocatedMemBlock;	//Head pointer to Allocated linkedlist.
	struct MemUnit*	m_pFreeMemBlock;		//Head pointer to Free linkedlist.

	int32_t	m_ulUnitSize;			//Memory unit size. There are much unit in memory pool.
	int32_t	m_ulBlockSize;			//Memory pool size. Memory pool is make of memory unit.
	int32_t m_nUnits;

public:
	MemPool();
	~MemPool();

	void			allocsys(int32_t nunits, int32_t unitsz);
	void*           alloc();	//Allocate memory unit
	void            freeunit( void* p );										//Free memory unit
	void			resetunits();
	void			freesysmem();
};

#endif
