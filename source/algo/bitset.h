

#ifndef BITSET_H
#define BITSET_H

#include "../platform.h"

class BitSet
{

public:
	BitSet() : m_bits(0), m_size(0) {}

	~BitSet()
	{
		if (m_bits)
		{
			delete [] m_bits;	//[]
			m_bits = NULL;
		}

		m_size = 0;
	}

	void resize(int32_t count)
	{
		m_size = count / 32 + 1;

		if (m_bits)
		{
			delete [] m_bits;	//[]
			m_bits = 0;
		}

		m_bits = new uint32_t[m_size];
		clearall();
	}

	void set(int32_t i)
	{
		m_bits[i >> 5] |= (1 << (i & 31));
		//m_bits[i / 32] |= (1 << (i % 32));
	}

#define BITSET_ON(b,i)	(b[i >> 5] & (1 << (i & 31)))

	int32_t on(int32_t i)
	{
		return m_bits[i >> 5] & (1 << (i & 31));
	}

	void clear(int32_t i)
	{
		m_bits[i >> 5] &= ~(1 << (i & 31));
	}

	void clearall()
	{
		memset(m_bits, 0, sizeof(uint32_t) * m_size);
	}

//private:

	uint32_t *m_bits;
	int32_t m_size;
};

#endif