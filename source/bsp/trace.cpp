#include "../path/collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../math/3dmath.h"
#include "../phys/collision.h"
#include "../math/vec2i.h"
#include "../math/plane2i.h"
#include "../path/pathnode.h"

#define EPSILON_I		1

//experimental, so far, haven't found a way to use fixmath instead of floats for pass ratios
bool PassUnits(Vec2i vstart, Vec2i vend,
               int cmminx, int cmminy, int cmmaxx, int cmmaxy,
               Unit* u2)
{
	float startratio = -1.0f;
	float endratio = 1.0f;
	bool startsout = false;

	UType* u2t = &g_utype[u2->type];

	int cmminx2 = u2->cmpos.x - u2t->size.x/2;
	int cmminy2 = u2->cmpos.y - u2t->size.x/2;
	int cmmaxx2 = cmminx2 + u2t->size.x;
	int cmmaxy2 = cmminy2 + u2t->size.x;

	Plane2i planes[4];
	planes[0] = Plane2i(-1, 0, PlaneDistance(Vec2i(-1, 0), Vec2i(cmminx2, cmminy2)));
	planes[1] = Plane2i(1, 0, PlaneDistance(Vec2i(1, 0), Vec2i(cmmaxx2, cmminy2)));
	planes[2] = Plane2i(0, -1, PlaneDistance(Vec2i(0, -1), Vec2i(cmminx2, cmminy2)));
	planes[3] = Plane2i(0, 1, PlaneDistance(Vec2i(0, 1), Vec2i(cmminx2, cmmaxy2)));

	for(int i = 0; i < 4; i++)
	{
		Plane2i* p = &planes[i];

		float offset = 0;

#if 0
		if(tj->type == TRACE_SPHERE)
			offset = tj->radius;

		float startdistance = Dot(tj->absstart, p->m_normal) + (p->m_d + offset);
		float enddistance = Dot(tj->absend, p->m_normal) + (p->m_d + offset);
#endif
		int startdistance;
		int enddistance;

		Vec2i voffset(0,0);
#if 0
		if(tj->type == TRACE_BOX)
#endif
		{

			voffset.x = (p->m_normal.x < 0) ? cmmaxx : cmminx;
			voffset.y = (p->m_normal.y < 0) ? cmmaxy : cmminy;
#if 0
			voffset.z = (p->m_normal.z < 0) ? tj->vmax.z : tj->vmin.z;
#endif

			startdistance = Dot(vstart + voffset, p->m_normal) + p->m_d;
			enddistance = Dot(vend + voffset, p->m_normal) + p->m_d;
		}

		if(startdistance > 0)	startsout = true;

		if(startdistance > 0 && enddistance > 0)
			return true;

		if(startdistance <= 0 && enddistance <= 0)
			continue;

		if(startdistance > enddistance)
		{
			float ratio1 = (startdistance - EPSILON_I) / (float)(startdistance - enddistance);

			if(ratio1 > startratio)
			{
				startratio = ratio1;
#if 0

				tw->collisionnormal = p->m_normal;

				if((tj->start.x != tj->end.x || tj->start.z != tj->end.z) && p->m_normal.y != 1 && p->m_normal.y >= 0.0f)
					//if((tj->start.x != tj->end.x || tj->start.z != tj->end.z))
				{
					tw->trytostep = true;

					//if(debugb)
					//	InfoMess("asd", "try step");
				}

				if(tw->collisionnormal.y > 0.2f)
					tw->onground = true;
#endif
			}
		}
		else
		{
			float ratio = (float)( (startdistance + EPSILON_I) / (startdistance - enddistance) );

			if(ratio < endratio)
				endratio = ratio;
		}
	}

#if 0
	tw->collided = true;

	Texture* ptex = &g_texture[b->m_texture];

	if(ptex->ladder)
		tw->atladder = true;
#endif

	if(startsout == false)
	{
#if 0
		tw->stuck = true;
#endif
		return false;
	}

	if(startratio < endratio)
	{
		//if(startratio > -1 && startratio < tw->traceratio)
		if(startratio > -1)
		{
			if(startratio < 0)
				startratio = 0;

#if 0
			g_selB.push_back(b);

			tw->traceratio = startratio;
#endif
			return false;
		}
	}

	return true;
}
