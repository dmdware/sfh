











#ifndef FOLIAGE_H
#define FOLIAGE_H

#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../math/vec2s.h"
#include "../math/vec3i.h"
#include "../math/matrix.h"
#include "vertexarray.h"
#include "sprite.h"
#include "depthable.h"

class FlType
{
public:
	char name[64];
	Vec2s size;
	//uint32_t sprite;
	uint32_t splist;
};


#define FL_SPRUCE1			0
#define FL_SPRUCE2			1
#define FL_SPRUCE3			2
#define FL_EUHB1			3
#define FL_TYPES			3

extern FlType g_fltype[FL_TYPES];

// byte-align structures
#pragma pack(push, 1)
class Foliage
{
public:
	Depthable* depth;
	bool on;
	unsigned char type;
	Vec2i cmpos;
	Vec3f drawpos;
	float yaw;
	unsigned char lastdraw;	//used for preventing repeats

	Foliage();
	void fillcollider();
	void freecollider();
	void destroy();
};
#pragma pack(pop)

//#define FOLIAGES	128
//#define FOLIAGES	1024
//#define FOLIAGES	2048
#define FOLIAGES	6000
//#define FOLIAGES	10000
//#define FOLIAGES	30000
//#define FOLIAGES	60000	//ushort limit
//#define FOLIAGES	240000


//__declspec(dllexport) asd();

extern Foliage g_foliage[FOLIAGES];

void DefF(int32_t type, const char* sprel, /* Vec3f scale, Vec3f translate,*/ Vec2s size);
bool PlaceFol(int32_t type, Vec3i cmpos);
void DrawFol(Foliage* f, float rendz, uint32_t renderdepthtex=0, uint32_t renderfb=0);
void ClearFol(int32_t cmminx, int32_t cmminy, int32_t cmmaxx, int32_t cmmaxy);
void FreeFol();
void FillForest(uint32_t r);
#endif
