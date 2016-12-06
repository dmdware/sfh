











#ifndef BLYPE_H
#define BLYPE_H

#include "../math/vec3f.h"
#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../math/vec2uc.h"
#include "resources.h"
#include "../render/billboard.h"
#include "../render/particle.h"
#include "conduit.h"

#define MAX_B_EMITTERS	10

#define BLSND_PROD	0	//production sound
#define BLSND_FINI	1	//finished construction sound
#define BLSND_PLACE	2	//placed new construction project sound
#define BLSND_SEL	3	//selected
#define BLSND_CSEL	4	//selected construction
#define BL_SOUNDS	5

#define FLAG_BLOWN_STATE	1
#define FLAG_BLOWN_FIRM		2

class BlType
{
public:
	//bool on;

	// width in tiles
	Vec2uc width;

	Vec2i cmalign;	//align cm intervals
	Vec2i cmalignoff;	//offset within align interval

	int32_t nframes;
	int32_t cnframes;
	/*
	The sprites are pointers to arrays of frames for
	non-terrain hugging building sprites, or the set
	of inclination combinations for terrain hugging.
	In the future the terrain hugging ones might
	have frames.
	*/
	//uint32_t* sprite;
	//uint32_t* csprite;
	uint32_t splist;
	uint32_t csplist;
	uint32_t icontex;

	char name[256];
	std::string desc;

	int32_t foundation;

	int32_t input[RESOURCES];
	int32_t output[RESOURCES];

	int32_t stockingrate;

	int32_t conmat[RESOURCES];

	int32_t reqdeposit;

	EmitterPlace emitterpl[MAX_B_EMITTERS];

	bool hugterr;

	std::list<unsigned char> manuf;

	int16_t sound[BL_SOUNDS];
	int32_t maxhp;

	int16_t visrange;

	int32_t price[RESOURCES];
	int32_t opwage;

	unsigned char flags;
	int32_t prop;	//building proportion/rate number
	int32_t wprop;	//proportion to workers ratio divisible by RATIO_DENOM

	void free();
	BlType();
	~BlType();
};

//foundation
#define FD_LAND			0
#define FD_COAST		1
#define FD_SEA			2

#if 1
//building
#define BL_NONE				-1
#define BL_HOUSE1			0
#define BL_HOUSE2			1
#define BL_STORE			2
#define BL_TRFAC			3
#define BL_FARM				4
#define BL_SHMINE			5
#define BL_IRONSM			6
#define BL_OILWELL			7
#define BL_OILREF			8
#define BL_NUCPOW			9
#define BL_COALPOW			10
#define BL_CEMPL			11
#define BL_CHEMPL			12
//#define BL_ELECPL			12
//#define BL_HOUSE2			12
#define BL_GASST			13
//#define BL_BRK				13
#define BL_BANK				14
#define BL_TYPES			15

#define BL_ROAD				(BL_TYPES+CD_ROAD)
#define BL_POWL				(BL_TYPES+CD_POWL)
#define BL_CRPIPE			(BL_TYPES+CD_CRPIPE)
//#define BL_WATERPIPE		(BL_TYPES+4)

#define BL_END				BL_TYPES
#define BL_CD_BEG			BL_END
#define BL_CD_END			(BL_CD_BEG+CD_TYPES)

#define TOTAL_BUILDABLES	(BL_TYPES+CD_TYPES)
#else

#define BL_NONE				-1
#define BL_BEG				0
#define BL_TYPES		256
#define BL_END				BL_TYPES
#define BL_CD_BEG			BL_END
#define BL_CD_END			(BL_CD_BEG+CD_TYPES)

#endif

extern BlType g_bltype[BL_TYPES];

void DefB(int32_t type,
		  const char* name,
		  const char* iconrel,
		  Vec2i size,
		  bool hugterr,
		  const char* sprel,
		  int32_t nframes,
		  const char* csprel,
		  int32_t cnframes,
		  int32_t foundation,
		  int32_t reqdeposit,
		  int32_t maxhp,
		  int32_t visrange,
		  int32_t opwage,
		  unsigned char flags,
		  int32_t prop,
		  int32_t wprop,
		  int32_t srate);
void DefB(int32_t type,
		  const char* name,
		  const char* iconrel,
		  Vec2i size,
		  Vec2i cmalign,
		  Vec2i cmalignoff,
		  bool hugterr,
		  const char* sprel,
		  int32_t nframes,
		  const char* csprel,
		  int32_t cnframes,
		  int32_t foundation,
		  int32_t reqdeposit,
		  int32_t maxhp,
		  int32_t visrange,
		  int32_t opwage,
		  unsigned char flags,
		  int32_t prop,
		  int32_t wprop,
		  int32_t srate);

void BMat(int32_t type, int32_t res, int32_t amt);
void BIn(int32_t type, int32_t res, int32_t amt);
void BOut(int32_t type, int32_t res, int32_t amt, int32_t defl);
void BEmit(int32_t type, int32_t emitterindex, int32_t ptype, Vec3i offset);
void BDes(int32_t type, const char* desc);
void BSon(int32_t type, int32_t stype, const char* relative);
void BMan(int32_t type, unsigned char utype);

#define STOCKING_RATE		(RATIO_DENOM*1/20)

#endif
