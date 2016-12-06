#ifndef COLLISION_H
#define COLLISION_H

#include "../platform.h"

#define MAX_CLIMB_INCLINE		(TILE_SIZE)

#define COLLIDER_NONE		-1
#define COLLIDER_UNIT		0
#define COLLIDER_BUILDING	1
#define COLLIDER_TERRAIN	2
#define COLLIDER_NOROAD		3
#define COLLIDER_OTHER		4
#define COLLIDER_NOLAND		5
#define COLLIDER_NOSEA		6
#define COLLIDER_NOCOAST	7
#define COLLIDER_ROAD		8
#define COLLIDER_OFFMAP		9

extern int g_lastcollider;
extern int g_collidertype;
extern bool g_ignored;

class Unit;

bool BlAdj(int i, int j);
bool CoAdj(char ctype, int i, int x, int z);
bool CollidesWithTerr(__int32 cmminx, __int32 cmminy, __int32 cmmaxx, __int32 cmmaxy);
bool CollidesWithUnits(__int32 minx, __int32 miny, __int32 maxx, __int32 maxy, bool isunit, Unit* thisu, Unit* ignore);
bool OffMap(__int32 cmminx, __int32 cmminy, __int32 cmmaxx, __int32 cmmaxy);
bool CollidesWithBuildings(int minx, int miny, int maxx, int maxy, int ignore=-1);

#endif
