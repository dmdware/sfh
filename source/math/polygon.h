










#ifndef POLYGON_H
#define POLYGON_H

#include "../platform.h"
#include "vec3f.h"

class Polyg	//name shortened due to naming conflict on Windows
{
public:
	std::list<Vec3f> m_edv;	//used for constructing the polygon on-the-fly
	Vec3f* m_drawoutva;		//used for drawing outline

	Polyg();
	~Polyg();
	Polyg(const Polyg& original);
	Polyg& operator=(const Polyg& original);
	void makeva();
	void freeva();
};

bool InsidePoly(Vec3f vIntersection, Vec3f Poly[], int32_t verticeCount);
bool InterPoly(Vec3f vPoly[], Vec3f vLine[], int32_t verticeCount, Vec3f* vIntersection=NULL);

#endif
