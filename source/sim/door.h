

#ifndef DOOR_H
#define DOOR_H

#include "../math/vec3f.h"
#include "../math/3dmath.h"
//#include "../bsp/brushside.h"
//#include "../bsp/brush.h"

class BrushSide;

class EdDoor
{
public:
	Vec3f axis;
	Vec3f point;
	float opendeg;	//show degrees
	bool startopen;
	
	//Closed state brush sides
	int m_nsides;
	BrushSide* m_sides;

	EdDoor();
	~EdDoor();
	EdDoor& operator=(const EdDoor& original);
	EdDoor(const EdDoor& original);
};

class Door
{
public:
	Vec3f axis;
	Vec3f point;
	float opendeg;	//degrees
	float openness;	//ratio of how show it is
	bool opening;

	//Brush* brushp;
	//Brush closedstate;
};

#endif