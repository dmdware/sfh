








/*
simplified memory pool
given that
we keep allocating to the end of a growing pool
and only free the elements altogether, or back up to a certain point
*/

#ifndef STACKPOOL_H
#define STACKPOOL_H

#include "../platform.h"

// http://www.codeproject.com/Articles/27487/Why-to-use-memory-pool-and-how-to-implement-it
// http://www.ibm.com/developerworks/aix/tutorials/au-memorymanager/au-memorymanager-pdf.pdf


class StackPool
{
public:
	void*			m_pMemBlock;			//The address of memory pool.

	int32_t m_totalsz;	//	MAXPATHN * sizeof(PathNode)
	int32_t m_freestart;

	StackPool();
	~StackPool();

	void			allocsys(int32_t totalsz);
	void*           alloc(int32_t unitsz);	//Allocate memory unit
	void            freeunit( void* p );	//Free memory unit
	void			resetunits();
	void			freesysmem();
};

#endif
