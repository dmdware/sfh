











#ifndef UNITTYPE_H
#define UNITTYPE_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../math/vec2s.h"
#include "../render/heightmap.h"
#include "resources.h"
#include "../math/vec3f.h"
#include "../path/pathnode.h"

#define USND_SEL		0	//unit selected
#define U_SOUNDS		1

class UType
{
public:
#if 0
	uint32_t texindex;
	Vec2i bilbsize;
#endif
	//bool on;
	//uint32_t* sprite[DIRS];
	uint32_t splist;	//sprite list
	int32_t nframes;
	Vec2s size;
	char name[256];
	int32_t starthp;
	int32_t cmspeed;
	int32_t cost[RESOURCES];
	bool walker;
	bool landborne;
	bool roaded;
	bool seaborne;
	bool airborne;
	bool military;
	int16_t visrange;
	int16_t sound[U_SOUNDS];
	int32_t prop;	//proportion to buildings ratio out of RATIO_DENOM

	UType()
	{
#if 0
		//TODO move all bl, u, cd constructors to Init();
		for(int32_t s=0; s<DIRS; s++)
		{
			sprite[s] = NULL;
		}
#endif

		nframes = 0;
		//on = false;
	}

	~UType()
	{
		free();
	}

	void free()
	{
#if 0
		for(int32_t s=0; s<DIRS; s++)
		{
			if(!sprite[s])
				continue;

			delete [] sprite[s];
			sprite[s] = NULL;
		}
#endif

		nframes = 0;
		//on = false;
	}
};

#if 0
#define UNIT_LABOURER		0
#define UNIT_BATTLECOMP		1
#define UNIT_TRUCK			2
#define UNIT_CARLYLE		3
#define UNIT_TYPES			4
#else
#define UNIT_LABOURER		0
#define UNIT_TRUCK			1
#define UNIT_TYPES			2
#endif

//#define UNIT_TYPES		128

extern UType g_utype[UNIT_TYPES];

void DefU(int32_t type, const char* sprel, int32_t nframes,
	Vec2s size, const char* name,
	int32_t starthp,
	bool landborne, bool walker, bool roaded, bool seaborne, bool airborne,
	int32_t cmspeed, bool military,
	int32_t visrange,
	int32_t prop);
void UCost(int32_t type, int32_t res, int32_t amt);

#endif
