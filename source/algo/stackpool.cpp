










#include "stackpool.h"
#include "../platform.h"
#include "../utils.h"

StackPool::StackPool()
{
	m_pMemBlock = NULL;
	m_freestart = 0;
	m_totalsz = 0;
}

/*==============================================================================
MemPool:
Constructor of this class. It allocate memory block from system and create
a static double linked std::list to manage all memory unit.

Parameters:
[in]ulUnitNum
The number of unit which is a part of memory block.

[in]ulUnitSize
The size of unit.
//=============================================================================
*/
void StackPool::allocsys(int32_t totalsz)
{
	freesysmem();

	m_totalsz = totalsz;
	m_freestart = 0;

	m_pMemBlock = malloc(totalsz);			//Allocate a memory block.

	if(!m_pMemBlock)
		OUTOFMEM();

	resetunits();
}

/*==============================================================================
~MemPool():
Destructor of this class. Its task is to free memory block.
//=============================================================================
*/
StackPool::~StackPool()
{
	freesysmem();
}


/*==============================================================================
Alloc:
To allocate a memory unit. If memory pool can`t provide proper memory unit,
will call system function.

Parameters:
[in]ulSize
Memory unit size.

[in]bUseMemPool
Whether use memory pool.

Return Values:
Return a pointer to a memory unit.
//=============================================================================
*/
void* StackPool::alloc(int32_t unitsz)
{
	const int32_t end = m_freestart + unitsz;

	if( /* m_pMemBlock == NULL || */ end >= m_totalsz)
	//if(true)
	{
#if 0
		return malloc(unitsz);
#else
		return NULL;
#endif
	}

	//will this cause alignment issues if trying to cast to a misaligned struct with alignment of eg 4 bytes?
	void* memunit = (void*)( ((char*)m_pMemBlock) + m_freestart);

	m_freestart = end;

	return memunit;
}


/*==============================================================================
Free:
To free a memory unit. If the pointer of parameter point to a memory unit,
then insert it to "Free linked std::list". Otherwise, call system function "free".

Parameters:
[in]p
It point to a memory unit and prepare to free it.

Return Values:
none
//=============================================================================
*/
void StackPool::freeunit( void* p )
{
#if 0
	if(m_pMemBlock<=p && p<(void*)( ((char*)m_pMemBlock) + m_totalsz) )
	{
	}
	else
	{
		free(p);
	}
#endif
}

void StackPool::resetunits()
{
	//if(!m_pMemBlock)
	//	return;

	m_freestart = 0;
}

void StackPool::freesysmem()
{
	if(m_pMemBlock)
	{
		free(m_pMemBlock);
		m_pMemBlock = NULL;
	}

	m_pMemBlock = NULL;
	m_totalsz = 0;
	m_freestart = 0;
}
