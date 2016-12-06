

#ifndef HASHKEY_H
#define HASHKEY_H

#include "../platform.h"

// byte-align structures
#pragma pack(push, 1)

struct HashKey
{
	uint16_t nx;
	uint16_t ny;
};

#define HASHINT	uint32_t

#pragma pack(pop)

#define HASHKEY(a)			( &((PathNode*)a)->hashkey )
//#define HASHKEY(a)		( a )

#endif