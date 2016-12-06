












#ifndef DEPTHABLE_H
#define DEPTHABLE_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../platform.h"

class Depthable
{
public:
	//used to do topological depth sort with other items:
	Vec3i cmmin;
	Vec3i cmmax;
	//used for picking what to send to render (pixel min/max):
	Vec2i pixmin;
	Vec2i pixmax;
	uint16_t index;
	unsigned char dtype;
	unsigned char cdtype;
	bool plan;
	std::list<Depthable*> behind;
	bool visited;
	uint32_t pathnode;	//index to pathnode, with vertical/z layers, used in draw order sorting
	int32_t rendz;	//depth for per-pixel depth writing
};

#define DEPTH_U		0	//unit
#define DEPTH_BL	1	//building
#define DEPTH_FOL	2	//foliage
#define DEPTH_CD	3	//conduit

class Unit;
class Building;
class Foliage;
class CdTile;

//update drawabilities
void UpDraw(Unit* u);
void UpDraw(Building* b);
void UpDraw(Foliage* f);
void UpDraw(CdTile* c, unsigned char ctype, int32_t tx, int32_t ty);

#endif
