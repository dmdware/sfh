










#include "conduit.h"
#include "../render/heightmap.h"
#include "building.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "player.h"
#include "../sim/selection.h"
#include "../gui/gui.h"
#include "../gui/widgets/spez/cstrview.h"
#include "../gui/layouts/appviewport.h"
#include "../gui/layouts/appgui.h"
#include "../math/hmapmath.h"
#include "../render/foliage.h"
#include "../render/transaction.h"
#include "unit.h"
#include "job.h"
#include "../gui/layouts/chattext.h"
#include "map.h"
#include "../render/drawsort.h"
#include "../render/fogofwar.h"
#include "../net/client.h"
#include "../net/lockstep.h"
#include "../path/collidertile.h"
#include "simdef.h"
#include "border.h"

//not engine
#include "../app/appmain.h"
#include "../tool/rendersprite.h"

//int32_t g_ncdtypes = 0;
CdType g_cdtype[CD_TYPES];

/*
TODO make sure when bl, cd, fol, or u destroyed, that associated Depthable in g_drawlist is erased.
*/

void ClearCdPlans(unsigned char ctype)
{
	//if(!get)
	//	return;

	for(int32_t x=0; x<g_mapsz.x; x++)
		for(int32_t y=0; y<g_mapsz.y; y++)
		{
			CdTile* ctile = GetCd(ctype, x, y, true);

			if(!ctile->on)
				continue;

			ctile->on = false;

			std::list<Depthable>::iterator dit=g_drawlist.begin();
			while(dit!=g_drawlist.end())
			{
				if(&*dit != ctile->depth &&
					( dit->dtype != DEPTH_CD ||
					dit->cdtype != ctype ||
					dit->index != y * g_mapsz.x + x ||
					dit->plan != true) )
				{
					dit++;
					continue;
				}

				dit = g_drawlist.erase(dit);
			}

			ctile->depth = NULL;
		}
}

void ResetNetw(unsigned char ctype)
{
	CdType* ct = &g_cdtype[ctype];

	for(int32_t i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(ct->blconduct)
		{
			int16_t& netw = *(int16_t*)(((char*)b)+ct->netwoff);
			//int16_t& netw = b->netw[ctype];
			netw = -1;
		}
		else
		{
			std::list<int16_t>& netw = *(std::list<int16_t>*)(((char*)b)+ct->netwoff);
			//std::list<int16_t>& netw = b->netwlist[ctype];
			netw.clear();
		}
	}

	//corpc fix
	int32_t lastnetw = BUILDINGS+1;	//don't interfere with network range

	for(int32_t x=0; x<g_mapsz.x; x++)
		for(int32_t y=0; y<g_mapsz.y; y++)
		{
			CdTile* ctile = GetCd(ctype, x, y, false);

			if(!ctile->on)
				continue;

			if(!ctile->finished)
				continue;

			//corpc fix
			//reqsource indicates if being connected to the grid 
			//requires connection to a source building, or just the network.
			if(!ct->reqsource)
				ctile->netw = lastnetw++;
			else
				ctile->netw = -1;
		}
}

// If buildings conduct the resource,
// merge the networks of touching buildings.
// Return true if there was a change.
bool ReNetwB(unsigned char ctype)
{
	CdType* ct = &g_cdtype[ctype];

	if(!ct->blconduct)
		return false;

	bool change = false;

	Building* b;
	Building* b2;

	for(int32_t i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
		int16_t& netw = *(int16_t*)(((char*)b)+ct->netwoff);
		//int16_t& netw = b->netw[ctype];

		if(!b->on)
			continue;

		for(int32_t j=i+1; j<BUILDINGS; j++)
		{
			b2 = &g_building[j];

			if(!b2->on)
				continue;

			if(!BlAdj(i, j))
				continue;

			int16_t& netw2 = *(int16_t*)(((char*)b2)+ct->netwoff);
			//int16_t& netw2 = b2->netw[ctype];

			if(netw < 0 && netw2 >= 0)
			{
				netw = netw2;
				change = true;
			}
			else if(netw2 < 0 && netw >= 0)
			{
				netw2 = netw;
				change = true;
			}
			else if(netw >= 0 && netw2 >= 0 && netw != netw2)
			{
				MergeNetw(ctype, netw, netw2);
				change = true;
			}
			//corpc fix
			//reqsource indicates if being connected to the grid 
			//requires connection to a source building, or just the network.
			else if(netw2 < 0 && netw2 < 0 && ct->reqsource)
			{
				BlType* bt = &g_bltype[b->type];
				BlType* bt2 = &g_bltype[b2->type];

				//check if bl is source
				for(int32_t ri=0; ri<RESOURCES; ri++)
				{
					Resource* r = &g_resource[ri];

					if(r->conduit != ctype)
						continue;

					if(bt->output[ri] > 0)
					{
						//we have a winner
						int16_t bi = b - g_building;
						netw = bi;
						netw2 = bi;
						change = true;
					}
					else if(bt2->output[ri] > 0)
					{
						//we have a winner
						int16_t bi = b2 - g_building;
						netw = bi;
						netw2 = bi;
						change = true;
					}
				}
			}
		}
	}

	return change;
}

// Merge two networks that have been found to be touching.
void MergeNetw(unsigned char ctype, int32_t A, int32_t B)
{
	int32_t mini = imin(A, B);
	int32_t maxi = imax(A, B);

	CdType* ct = &g_cdtype[ctype];

	if(ct->blconduct)
		for(int32_t i=0; i<BUILDINGS; i++)
		{
			Building* b = &g_building[i];

			if(!b->on)
				continue;

			int16_t& netw = *(int16_t*)(((char*)b)+ct->netwoff);
			//int16_t& netw = b->netw[ctype];

			if(netw == maxi)
				netw = mini;
		}
	else
		for(int32_t i=0; i<BUILDINGS; i++)
		{
			Building* b = &g_building[i];

			if(!b->on)
				continue;

			bool found = false;
			std::list<int16_t>& bnetw = *(std::list<int16_t>*)(((char*)b)+ct->netwoff);
			//std::list<int16_t>& bnetw = b->netwlist[ctype];
			std::list<int16_t>::iterator netwiter = bnetw.begin();

			while(netwiter != bnetw.end())
			{
				if(*netwiter == maxi)
				{
					if(!found)
					{
						*netwiter = mini;
						found = true;
					}
					else
					{
						netwiter = bnetw.erase( netwiter );
						continue;
					}
				}

				netwiter++;
			}
		}

		for(int32_t x=0; x<g_mapsz.x; x++)
			for(int32_t y=0; y<g_mapsz.y; y++)
			{
				CdTile* ctile = GetCd(ctype, x, y, false);

				if(!ctile->on)
					continue;

				if(!ctile->finished)
					continue;

				if(ctile->netw == maxi)
					ctile->netw = mini;
			}
}

bool CompareCo(unsigned char ctype, CdTile* ctile, int32_t tx, int32_t ty)
{
	CdTile* ctile2 = GetCd(ctype, tx, ty, false);

	if(!ctile2->on)
		return false;

	if(!ctile2->finished)
		return false;

	if(ctile2->netw < 0 && ctile->netw >= 0)
	{
		ctile2->netw = ctile->netw;
		return true;
	}
	else if(ctile->netw < 0 && ctile2->netw >= 0)
	{
		ctile->netw = ctile2->netw;
		return true;
	}
	else if(ctile->netw >= 0 && ctile2->netw >= 0 && ctile->netw != ctile2->netw)
	{
		MergeNetw(ctype, ctile->netw, ctile2->netw);
		return true;
	}

	return false;
}

// Building adjacent?
bool BAdj(unsigned char ctype, int32_t i, int32_t tx, int32_t ty)
{
	Building* b = &g_building[i];
	BlType* bt = &g_bltype[b->type];

	Vec2i btp = b->tpos;

	//Building min/max positions
	int32_t bcmminx = (btp.x - bt->width.x/2)*TILE_SIZE;
	int32_t bcmminy = (btp.y - bt->width.y/2)*TILE_SIZE;
	int32_t bcmmaxx = bcmminx + bt->width.x*TILE_SIZE;
	int32_t bcmmaxy = bcmminy + bt->width.y*TILE_SIZE;

	CdType* ct = &g_cdtype[ctype];
	//Vec3i p2 = RoadPhysPos(x, y);
	Vec2i ccmp = Vec2i(tx, ty)*TILE_SIZE + ct->physoff;

	//Conduit min/max positions
	const int32_t hwx2 = TILE_SIZE/2;
	const int32_t hwz2 = TILE_SIZE/2;
	int32_t ccmminx = ccmp.x - hwx2;
	int32_t ccmminy = ccmp.y - hwz2;
	int32_t ccmmaxx = ccmminx + TILE_SIZE;
	int32_t ccmmaxy = ccmminy + TILE_SIZE;

	if(bcmmaxx >= ccmminx && bcmmaxy >= ccmminy && bcmminx <= ccmmaxx && bcmminy <= ccmmaxy)
		return true;

	return false;
}

bool CompareB(unsigned char ctype, Building* b, CdTile* ctile)
{
	CdType* ct = &g_cdtype[ctype];

	//if buildings don't conduct this
	if(!ct->blconduct)
	{
		//we can have several disjoined networks connected to bl (like roads)
		std::list<int16_t>& bnetw = *(std::list<int16_t>*)(((char*)b)+ct->netwoff);
		//std::list<int16_t>& bnetw = b->netwlist[ctype];

		if(bnetw.size() <= 0 && ctile->netw >= 0)
		{
			bnetw.push_back(ctile->netw);
			return true;
		}/*
		 else if(r->netw < 0 && b->roadnetw >= 0)
		 {
		 pow->netw = b->pownetw;
		 return true;
		 }
		 else if(pow->netw >= 0 && b->pownetw >= 0 && pow->netw != b->pownetw)
		 {
		 MergePow(pow->netw, b->pownetw);
		 return true;
		 }*/
		else if(bnetw.size() > 0 && ctile->netw >= 0)
		{
			bool found = false;
			for(std::list<int16_t>::iterator netwiter = bnetw.begin(); netwiter != bnetw.end(); netwiter++)
			{
				if(*netwiter == ctile->netw)
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				bnetw.push_back(ctile->netw);
				return true;	//corpc fix
			}
		}
		//neither bl nor cd tile has source	
		//corpc fix
		//reqsource indicates if being connected to the grid 
		//requires connection to a source building, or just the network.
		else if(ctile->netw < 0 && bnetw.size() <= 0 && ct->reqsource)
		{
			BlType* bt = &g_bltype[b->type];

			//check if bl is source
			for(int32_t ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(r->conduit != ctype)
					continue;

				if(bt->output[ri] <= 0)
					continue;

				//we have a winner

				int16_t bi = b - g_building;
				ctile->netw = bi;
				bnetw.push_back(bi);

				return true;
			}
		}

		return false;
	}
	else
	{
		int16_t& bnetw = *(int16_t*)(((char*)b)+ct->netwoff);
		//int16_t& bnetw = b->netw[ctype];

		if(bnetw < 0 && ctile->netw >= 0)
		{
			bnetw = ctile->netw;
			return true;
		}
		else if(ctile->netw < 0 && bnetw >= 0)
		{
			ctile->netw = bnetw;
			return true;
		}
		else if(ctile->netw >= 0 && bnetw >= 0 && ctile->netw != bnetw)
		{
			MergeNetw(ctype, ctile->netw, bnetw);
			return true;
		}
		//neither bl nor cd tile has source	
		//corpc fix
		//reqsource indicates if being connected to the grid 
		//requires connection to a source building, or just the network.
		else if(ctile->netw < 0 && bnetw < 0 && ct->reqsource)
		{
			BlType* bt = &g_bltype[b->type];

			//check if bl is source
			for(int32_t ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(r->conduit != ctype)
					continue;

				if(bt->output[ri] <= 0)
					continue;

				//we have a winner

				int16_t bi = b - g_building;
				ctile->netw = bi;
				bnetw = bi;

				return true;
			}
		}

		return false;
	}

	return false;
}

// Called by conduit network update function.
// Returns true if there was a change.
bool ReNetwTl(unsigned char ctype)
{
	bool change = false;

	for(int32_t x=0; x<g_mapsz.x; x++)
		for(int32_t y=0; y<g_mapsz.y; y++)
		{
			CdTile* ctile = GetCd(ctype, x, y, false);

			if(!ctile->on)
				continue;

			if(!ctile->finished)
				continue;

			if(x > 0 && CompareCo(ctype, ctile, x-1, y))
				change = true;
			if(x < g_mapsz.x-1 && CompareCo(ctype, ctile, x+1, y))
				change = true;
			if(y > 0 && CompareCo(ctype, ctile, x, y-1))
				change = true;
			if(y < g_mapsz.y-1 && CompareCo(ctype, ctile, x, y+1))
				change = true;

			for(int32_t i=0; i<BUILDINGS; i++)
			{
				Building* b = &g_building[i];

				if(!b->on)
					continue;

				if(!BAdj(ctype, i, x, y))
					continue;

				//compare networks of bl and cd tile and merge
				if(CompareB(ctype, b, ctile))
					change = true;
			}
		}

		return change;
}

void ReNetw(unsigned char ctype)
{
	ResetNetw(ctype);

	bool change;

	do
	{
		change = false;

		if(ReNetwB(ctype))
			change = true;

		if(ReNetwTl(ctype))
			change = true;
	}
	while(change);

#if 0
	CheckRoadAccess();
#endif
}

// Is the tile level for a conduit? Take into account the direction which the conduit is leading and coming from
// (e.g., forward incline may be greater than sideways incline).
bool CdLevel(unsigned char ctype, float iterx, float itery, float testx, float testy, float dx, float dy, int32_t i, float d, bool plantoo)
{
	//return true;

	bool n = false, e = false, s = false, w = false;
	int32_t ix, iy;

	// Check which neighbours should have conduits too based on
	// the dragged starting and ending position (actually the loop
	// variables for the drag line).

	if(i > 0)
	{
		float x = iterx - dx;
		float y = itery - dy;

		ix = x;
		iy = y;

		if(ix == testx)
		{
			if(iy == testy+1)	n = true;
			else if(iy == testy-1)	s = true;
		}
		else if(iy == testy)
		{
			if(ix == testx+1)	w = true;
			else if(ix == testx-1)	e = true;
		}

		float prevx = x - dx;
		float prevy = y - dy;

		//diagonal case? fill in extra ones so there's a continuous line
		if((int32_t)x != prevx && (int32_t)y != prevy)
		{
			ix = prevx;

			if(ix == testx)
			{
				if(iy == testy+1)	n = true;
				else if(iy == testy-1)	s = true;
			}
			else if(iy == testy)
			{
				if(ix == testx+1)	w = true;
				else if(ix == testx-1)	e = true;
			}
		}
	}

	if(i < d)
	{
		float x = iterx + dx;
		float y = itery + dy;

		ix = x;
		iy = y;

		if(ix == testx)
		{
			if(iy == testy+1)	n = true;
			else if(iy == testy-1)	s = true;
		}
		else if(iy == testy)
		{
			if(ix == testx+1)	w = true;
			else if(ix == testx-1)	e = true;
		}

		if(i > 0)
		{
			float prevx = x - dx;
			float prevy = y - dy;

			//diagonal case? fill in extra ones so there's a continuous line
			if((int32_t)x != prevx && (int32_t)y != prevy)
			{
				ix = prevx;

				if(ix == testx)
				{
					if(iy == testy+1)	n = true;
					else if(iy == testy-1)	s = true;
				}
				else if(iy == testy)
				{
					if(ix == testx+1)	w = true;
					else if(ix == testx-1)	e = true;
				}
			}
		}
	}

	CdType* ct = &g_cdtype[ctype];

	// Check for water

	if(!ct->cornerpl)
	{
		ix = testx;
		iy = testy;

		if(g_hmap.getheight(ix, iy) <= WATER_LEVEL)		return false;
		if(g_hmap.getheight(ix+1, iy) <= WATER_LEVEL)	return false;
		if(g_hmap.getheight(ix, iy+1) <= WATER_LEVEL)	return false;
		if(g_hmap.getheight(ix+1, iy+1) <= WATER_LEVEL)	return false;
	}
	else
	{
		//ix = (testx * TILE_SIZE + physoff.x)/TILE_SIZE;
		//iy = (testy * TILE_SIZE + physoff.y)/TILE_SIZE;
		ix = testx;
		iy = testy;

		if(g_hmap.getheight(ix, iy) <= WATER_LEVEL)		return false;
	}

	// Check which neighbours have conduits too

	if(ix > 0)
	{
		if(GetCd(ctype, ix-1, iy, false)->on)	w = true;
		//if(RoadPlanAt(ix-1, iy)->on)	w = true;
		if(plantoo)
			if(GetCd(ctype, ix-1, iy, true)->on) w = true;
	}

	if(ix < g_mapsz.x-1)
	{
		if(GetCd(ctype, ix+1, iy, false)->on)	e = true;
		//if(RoadPlanAt(ix+1, iy)->on)	e = true;
		if(plantoo)
			if(GetCd(ctype, ix+1, iy, true)->on) e = true;
	}

	if(iy > 0)
	{
		if(GetCd(ctype, ix, iy-1, false)->on)	s = true;
		//if(RoadPlanAt(ix, iy-1)->on)	s = true;
		if(plantoo)
			if(GetCd(ctype, ix, iy-1, true)->on) s= true;
	}

	if(iy < g_mapsz.y-1)
	{
		if(GetCd(ctype, ix, iy+1, false)->on)	n = true;
		//if(RoadPlanAt(ix, iy+1)->on)	n = true;
		if(plantoo)
			if(GetCd(ctype, ix, iy+1, true)->on) n = true;
	}
#if 0
	Log("level? ix"<<ix<<","<<iy);

#endif

	//Log("nesw? "<<(int32_t)n<<","<<(int32_t)e<<","<<(int32_t)s<<","<<(int32_t)w);

	// Check forward and sideways incline depending on the connection type

	// 4- or 3-way connections
	if((n && e && s && w) || (n && e && s && !w) || (n && e && !s && w) || (n && e && !s && !w) || (n && !e && s && w)
		|| (n && !e && !s && w) || (!n && e && s && !w) || (!n && !e && s && w) || (!n && !e && !s && !w) || (!n && e && s && w))
	{
		float compare = g_hmap.getheight(ix, iy);
		if(iabs(g_hmap.getheight(ix+1, iy) - compare) > ct->maxsideincl)	return false;
		if(iabs(g_hmap.getheight(ix, iy+1) - compare) > ct->maxsideincl)	return false;
		if(iabs(g_hmap.getheight(ix+1, iy+1) - compare) > ct->maxsideincl)	return false;
	}

	// straight-through 2-way connection or 1-way connection
	else if((n && !e && s && !w) || (n && !e && !s && !w) || (!n && !e && s && !w))
	{
		if(iabs(g_hmap.getheight(ix, iy) - g_hmap.getheight(ix+1, iy)) > ct->maxsideincl)	return false;
		if(iabs(g_hmap.getheight(ix, iy+1) - g_hmap.getheight(ix+1, iy+1)) > ct->maxsideincl)	return false;
		if(iabs(g_hmap.getheight(ix, iy) - g_hmap.getheight(ix, iy+1)) > ct->maxforwincl)	return false;
		if(iabs(g_hmap.getheight(ix+1, iy) - g_hmap.getheight(ix+1, iy+1)) > ct->maxforwincl)	return false;
	}

	// straight-through 2-way connection or 1-way connection
	else if((!n && e && !s && w) || (!n && e && !s && !w) || (!n && !e && !s && w))
	{
		if(iabs(g_hmap.getheight(ix, iy) - g_hmap.getheight(ix+1, iy)) > ct->maxforwincl)	return false;
		if(iabs(g_hmap.getheight(ix, iy+1) - g_hmap.getheight(ix+1, iy+1)) > ct->maxforwincl)	return false;
		if(iabs(g_hmap.getheight(ix, iy) - g_hmap.getheight(ix, iy+1)) > ct->maxsideincl)	return false;
		if(iabs(g_hmap.getheight(ix+1, iy) - g_hmap.getheight(ix+1, iy+1)) > ct->maxsideincl)	return false;
	}

#if 0
	Log("level yes! ix"<<ix<<","<<iy);

#endif

	return true;
}

void UpdCdPlans(unsigned char ctype, char owner, Vec3i start, Vec3i end)
{
	ClearCdPlans(ctype);

	CdType *ct = &g_cdtype[ctype];

	start.x -= ct->physoff.x;
	start.y -= ct->physoff.y;
	end.x -= ct->physoff.x;
	end.y -= ct->physoff.y;

	int32_t x1 = Clipi(start.x/TILE_SIZE, 0, g_mapsz.x-1);
	int32_t y1 = Clipi(start.y/TILE_SIZE, 0, g_mapsz.y-1);

	int32_t x2 = Clipi(end.x/TILE_SIZE, 0, g_mapsz.x-1);
	int32_t y2 = Clipi(end.y/TILE_SIZE, 0, g_mapsz.y-1);

#if 0
	Log("road plan "<<x1<<","<<y1<<"->"<<x2<<","<<y2);

#endif

	//PlacePowl(x1, y1, stateowner, true);
	//PlacePowl(x2, y2, stateowner, true);

	float dx = (float)(x2-x1);
	float dy = (float)(y2-y1);

	float d = sqrtf(dx*dx + dy*dy);

	dx /= d;
	dy /= d;

	int32_t prevx = x1;
	int32_t prevy = y1;

	int32_t i = 0;

	//PlaceRoad(x1, y1, g_localPlayer, true);
	//if(RoadLevel(x2, y2, dx, dy, i, d))
	//	PlaceRoad(x2, y2, g_localPlayer, true);

	for(float x=x1, y=(float)y1; i<=d; x+=dx, y+=dy, i++)
	{
		if(CdLevel(ctype, x, y, x, y, dx, dy, i, d, true))
		{
#if 0
			Log("place road urp "<<x<<","<<y);

#endif
			PlaceCd(ctype, x, y, owner, true);
		}

		if((int32_t)x != prevx && (int32_t)y != prevy)
		{
			if(CdLevel(ctype, x, y, prevx, y, dx, dy, i, d, true))
				PlaceCd(ctype, prevx, y, owner, true);
		}

		prevx = x;
		prevy = y;

#if 0
		Log("place road "<<x<<","<<y);

#endif
	}

	if((int32_t)x2 != prevx && (int32_t)y2 != prevy)
	{
		if(CdLevel(ctype, x2, y1, prevx, y2, dx, dy, i, d, true))
			PlaceCd(ctype, prevx, y2, owner, true);
	}
}

void Repossess(unsigned char ctype, int32_t tx, int32_t ty, int32_t owner)
{
	CdTile* ctile = GetCd(ctype, tx, ty, false);
	ctile->owner = (unsigned char)owner;
	ctile->allocate(ctype);
}

bool CdPlaceable(int32_t ctype, int32_t x, int32_t y)
{
	CdType* ct = &g_cdtype[ctype];
	Vec2i cmpos = Vec2i(x, y) * TILE_SIZE + ct->physoff;

	//if(TileUnclimable(cmpos.x, cmpos.y))
	//	return false;

	int32_t cmminx = cmpos.x - TILE_SIZE/2;
	int32_t cmminy = cmpos.y - TILE_SIZE/2;
	int32_t cmmaxx = cmminx + TILE_SIZE;
	int32_t cmmaxy = cmminy + TILE_SIZE;

	// Make sure construction resources can be transported
	// to this conduit tile/corner. E.g., powerlines and
	// above-ground pipelines need to be road-accessible.

#if 1	//Doesn't work yet?
	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(ct->conmat[ri] <= 0)
			continue;

		Resource* r = &g_resource[ri];

		if(r->conduit == CD_NONE)
			continue;

		if(r->conduit == ctype)
			continue;

		char reqctype = r->conduit;
		CdType* reqct = &g_cdtype[reqctype];

		Vec2i reqctiles[8];
		reqctiles[0] = Vec2i(cmminx,cmminy)/TILE_SIZE;
		reqctiles[1] = Vec2i(cmmaxx,cmminy)/TILE_SIZE;
		reqctiles[2] = Vec2i(cmmaxx,cmmaxy)/TILE_SIZE;
		reqctiles[3] = Vec2i(cmminx,cmmaxy)/TILE_SIZE;
		reqctiles[4] = Vec2i(cmminx-1,cmminy-1)/TILE_SIZE;
		reqctiles[5] = Vec2i(cmmaxx+1,cmminy-1)/TILE_SIZE;
		reqctiles[6] = Vec2i(cmmaxx+1,cmmaxy+1)/TILE_SIZE;
		reqctiles[7] = Vec2i(cmminx-1,cmmaxy+1)/TILE_SIZE;

		//NOTE: this won't work for non-corner-placed to non-corner-placed requisite conduits
		//For that, you need to also check all 8 tiles around. Currently it just checks 4, and perhaps some additional.

		bool found = false;

		for(int32_t ti=0; ti<8; ti++)
		{
			Vec2i tpos = reqctiles[ti];

			if(tpos.x < 0)
				continue;
			if(tpos.y < 0)
				continue;
			if(tpos.x >= g_mapsz.x)
				continue;
			if(tpos.y >= g_mapsz.y)
				continue;

			CdTile* reqc = GetCd(reqctype, tpos.x, tpos.y, false);

			if(!reqc->on)
				continue;
			//if(!reqc->finished)
			//	continue;

			Vec2i cmpos2 = Vec2i(x, y) * TILE_SIZE + reqct->physoff;

			int32_t cmminx2 = cmpos2.x - TILE_SIZE/2;
			int32_t cmminy2 = cmpos2.y - TILE_SIZE/2;
			int32_t cmmaxx2 = cmminx2 + TILE_SIZE;
			int32_t cmmaxy2 = cmminy2 + TILE_SIZE;

			if(cmminx > cmmaxx2)
				continue;

			if(cmminy > cmmaxy2)
				continue;

			if(cmmaxx < cmminx2)
				continue;

			if(cmmaxy < cmminy2)
				continue;

			found = true;
		}

		if(!found)
		{
			Player* py = &g_player[g_localP];

			//display error?
			if(g_build == BL_TYPES + ctype)
			{

			}

			return false;
		}
#if 0
		cmminx -= 1;
		cmminy -= 1;
		cmminx2 -= 1;
		cmminy2 -= 1;

		//if(GetCd(reqctype, tpos.x, tpos.y, false)->on)
		//	continue;

		if(tpos.x-1 >= 0 && tpos.y-1 >= 0 && GetCd(reqctype, tpos.x-1, tpos.y-1, false)->on)
			if(cmmaxx >= cmpos.x && cmmaxy >= cmpos.y && cmminx <= cmpos.x && cmminy <= cmpos.y)
				continue;

		if(tpos.x+1 < g_mapsz.x && tpos.y-1 >= 0 && GetCd(reqctype, tpos.x+1, tpos.y-1, false)->on)
			if(cmmaxx2 >= cmpos.x && cmmaxy >= cmpos.y && cmminx2 <= cmpos.x && cmminy <= cmpos.y)
				continue;

		if(tpos.x+1 < g_mapsz.x && tpos.y+1 < g_mapsz.y && GetCd(reqctype, tpos.x+1, tpos.y+1, false)->on)
			if(cmmaxx2 >= cmpos.x && cmmaxy2 >= cmpos.y && cmminx2 <= cmpos.x && cmminy2 <= cmpos.y)
				continue;

		if(tpos.x-1 >= 0 && tpos.y+1 < g_mapsz.y && GetCd(reqctype, tpos.x-1, tpos.y+1, false)->on)
			if(cmmaxx >= cmpos.x && cmmaxy2 >= cmpos.y && cmminx <= cmpos.x && cmminy2 <= cmpos.y)
				continue;

		return false;
#endif
	}
#endif

	if(!ct->cornerpl)
	{
		if(CollidesWithBuildings(cmpos.x, cmpos.y, cmpos.x, cmpos.y))
			return false;
	}
	else
	{
		if(CollidesWithBuildings(cmpos.x+1, cmpos.y+1, cmpos.x-1, cmpos.y-1))
			return false;

		bool nw_occupied = false;
		bool se_occupied = false;
		bool sw_occupied = false;
		bool ne_occupied = false;

		Vec2i nw_tile_center = cmpos + Vec2i(-TILE_SIZE/2, -TILE_SIZE/2);
		Vec2i se_tile_center = cmpos + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
		Vec2i sw_tile_center = cmpos + Vec2i(-TILE_SIZE/2, TILE_SIZE/2);
		Vec2i ne_tile_center = cmpos + Vec2i(TILE_SIZE/2, -TILE_SIZE/2);

		//Make sure the conduit corner isn't surrounded by buildings or map edges

		if(x<=0 || y<=0)
			nw_occupied = true;
		else if(CollidesWithBuildings(nw_tile_center.x, nw_tile_center.y, nw_tile_center.x, nw_tile_center.y))
			nw_occupied = true;

		if(x<=0 || y>=g_mapsz.y-1)
			sw_occupied = true;
		else if(CollidesWithBuildings(sw_tile_center.x, sw_tile_center.y, sw_tile_center.x, sw_tile_center.y))
			sw_occupied = true;

		if(x>=g_mapsz.x-1 || y>=g_mapsz.y-1)
			se_occupied = true;
		else if(CollidesWithBuildings(se_tile_center.x, se_tile_center.y, se_tile_center.x, se_tile_center.y))
			se_occupied = true;

		if(x>=g_mapsz.x-1 || y<=0)
			ne_occupied = true;
		else if(CollidesWithBuildings(ne_tile_center.x, ne_tile_center.y, ne_tile_center.x, ne_tile_center.y))
			ne_occupied = true;

		if( nw_occupied && sw_occupied && se_occupied && ne_occupied )
			return false;
	}

	return true;
}

int32_t GetConn(unsigned char ctype, int32_t x, int32_t y, bool plan=false)
{
	bool n = false, e = false, s = false, w = false;

	if(x+1 < g_mapsz.x && GetCd(ctype, x+1, y, false)->on)
		e = true;
	if(x-1 >= 0 && GetCd(ctype, x-1, y, false)->on)
		w = true;

	if(y+1 < g_mapsz.y && GetCd(ctype, x, y+1, false)->on)
		s = true;
	if(y-1 >= 0 && GetCd(ctype, x, y-1, false)->on)
		n = true;

	if(plan)
	{
		if(x+1 < g_mapsz.x && GetCd(ctype, x+1, y, true)->on)
			e = true;
		if(x-1 >= 0 && GetCd(ctype, x-1, y, true)->on)
			w = true;

		if(y+1 < g_mapsz.y && GetCd(ctype, x, y+1, true)->on)
			s = true;
		if(y-1 >= 0 && GetCd(ctype, x, y-1, true)->on)
			n = true;
	}

	return ConnType(n, e, s, w);
}

void ConnectCd(unsigned char ctype, int32_t tx, int32_t ty, bool plan)
{
	CdTile* ctile = GetCd(ctype, tx, ty, plan);

	if(!ctile->on)
		return;

	ctile->conntype = GetConn(ctype, tx, ty, plan);
}

void ConnectCdAround(unsigned char ctype, int32_t x, int32_t y, bool plan)
{
	if(x+1 < g_mapsz.x)
		ConnectCd(ctype, x+1, y, plan);
	if(x-1 >= 0)
		ConnectCd(ctype, x-1, y, plan);

	if(y+1 < g_mapsz.y)
		ConnectCd(ctype, x, y+1, plan);
	if(y-1 >= 0)
		ConnectCd(ctype, x, y-1, plan);
}

void PlaceCd(unsigned char ctype, int32_t tx, int32_t ty, int32_t owner, bool plan)
{
	if(!plan && GetCd(ctype, tx, ty, false)->on)
	{
		Repossess(ctype, tx, ty, owner);
		return;
	}

	if(!CdPlaceable(ctype, tx, ty))
		return;

	//if(ctype == CD_CRPIPE)
	//	InfoMess("a,","a");

	CdTile* ctile = GetCd(ctype, tx, ty, plan);

	//if(owner >= PLAYERS)
	//	InfoMess("!","!");

	ctile->on = true;
	ctile->owner = (unsigned char)owner;
	//Zero(ctile->maxcost);
	ctile->conwage = DEFL_CSWAGE;
	ctile->selling = false;

	CdType* ct = &g_cdtype[ctype];

	//ctile->drawpos = Vec3f(tx*TILE_SIZE, 0, ty*TILE_SIZE) + ct->drawoff;
	
	int32_t cmminx = tx*TILE_SIZE;
	int32_t cmminy = ty*TILE_SIZE;
	int32_t cmmaxx = cmminx + 1*TILE_SIZE - 1;
	int32_t cmmaxy = cmminy + 1*TILE_SIZE - 1;

	ClearFol(cmminx, cmminy, cmmaxx, cmmaxy);

	//Log("placecd simf=%d netf=%d ctype=%d x,y=%d,%d", (int32_t)g_simframe, (int32_t)g_netframe, (int32_t)ctype, (int32_t)tx, (int32_t)ty);

	if(g_appmode == APPMODE_PLAY)
	{
		ctile->finished = false;

		if(!plan)
			ctile->allocate(ctype);
	}
	else if(g_appmode == APPMODE_EDITOR
		&& !plan)
	{
		ctile->finished = true;
		ReNetw(ctype);
		Vec2i cmpos = Vec2i(tx,ty)*TILE_SIZE + ct->physoff;
		int32_t cmminx = cmpos.x - TILE_SIZE/2;
		int32_t cmminy = cmpos.y - TILE_SIZE/2;
		int32_t cmmaxx = cmminx + TILE_SIZE-1;
		int32_t cmmaxy = cmminy + TILE_SIZE-1;
		ClearFol(cmminx, cmminy, cmmaxx, cmmaxy);
	}
	//if(plan || g_appmode == APPMODE_EDITOR)
	if(plan)
		ctile->finished = true;

	ConnectCd(ctype, tx, ty, plan);
	ConnectCdAround(ctype, tx, ty, plan);

	for(int32_t i=0; i<RESOURCES; i++)
		ctile->transporter[i] = -1;

	//if(!plan)
	//	ReRoadNetw

	g_drawlist.push_back(Depthable());
	Depthable* d = &*g_drawlist.rbegin();
	d->dtype = DEPTH_CD;
	d->cdtype = ctype;
	d->plan = plan;
	d->index = tx + ty * g_mapsz.x;
	ctile->depth = d;
	UpDraw(ctile, d->cdtype, tx, ty);
}

//scheduled lockstep command, exec's the actual action
void PlaceCd(PlaceCdPacket* pcp)
{
	if(pcp->ntiles <= 0)
		return;

	unsigned char ctype = pcp->cdtype;
	int16_t player = pcp->player;
	std::list<Vec2uc> plan;

	CdType* ct = &g_cdtype[ctype];

	Player* py = &g_player[player];
	RichText name = py->name;
	if(py->client >= 0)
	{
		Client* c = &g_client[py->client];
		name = c->name;
	}
	char cmess[128];
	sprintf(cmess, " %d %ss.", (int32_t)pcp->ntiles, ct->name);
	RichText mess = name + RichText(" ") + STRTABLE[STR_PLACED] + RichText(" ") + RichText(cmess);
	AddNotif(&mess);
	
	std::list<Vec2i>& csel = *(std::list<Vec2i>*)(((char*)&g_sel)+ct->seloff);
	//std::list<Vec2i>& csel = g_sel.cdtiles[ctype];

	for(int16_t pin=0; pin<pcp->ntiles; pin++)
	{
		Vec2uc* pit = &pcp->place[ pin ];
		CdTile* actual = GetCd(ctype, pit->x, pit->y, false);
		bool willchange = !actual->on;
		PlaceCd(ctype, pit->x, pit->y, player, false);

		//if(player >= PLAYERS)
		//	InfoMess("!","!");

		if(g_appmode == APPMODE_PLAY && 
			!actual->finished &&
			player == g_localP)
			csel.push_back(Vec2i(pit->x,pit->y));

		ConnectCdAround(ctype, pit->x, pit->y, false);

		//was successfully placed and wasn't placed there before?
		if(g_appmode == APPMODE_PLAY && willchange && actual->on)
		{
			NewJob(UMODE_GOCDJOB, pit->x, pit->y, ctype);
		}
	}

	//ClearCdPlans(ctype);
	ReNetw(ctype);

	if(g_appmode == APPMODE_PLAY
		&& player == g_localP)
	{
		Player* py = &g_player[g_localP];
		GUI* gui = &g_gui;

		if(csel.size() > 0)
		{
			CstrView* cv = (CstrView*)gui->get("cs view");
			cv->regen(&g_sel);
			gui->show("cs view");
		}
	}
}

//local command called after local player releases left mouse button after dragging cd
void PlaceCd(unsigned char ctype, int32_t ownerpy)
{
	if(g_appmode == APPMODE_PLAY)
		ClearSel(&g_sel);

	//send off to sv? lock command?

	std::list<Vec2uc> places;

	for(int32_t x=0; x<g_mapsz.x; x++)
	{
		for(int32_t y=0; y<g_mapsz.y; y++)
		{
			CdTile* plan = GetCd(ctype, x, y, true);

			if(plan->on)
			{
				CdTile* actual = GetCd(ctype, x, y, false);

				if(!actual->on || !actual->finished)
					places.push_back(Vec2uc((unsigned char)x,(unsigned char)y));
			}
		}
	}

	PlaceCdPacket* pcp = (PlaceCdPacket*)malloc(
		sizeof(PlaceCdPacket) + 
		sizeof(Vec2uc)*(int16_t)places.size() );
	pcp->header.type = PACKET_PLACECD;
	pcp->cdtype = ctype;
	pcp->player = ownerpy;
	pcp->ntiles = (int16_t)places.size();

	std::list<Vec2uc>::iterator pit = places.begin();
	for(int16_t pin = 0; pit != places.end(); pit++, pin++)
		pcp->place[pin] = *pit;

	if(g_appmode == APPMODE_PLAY)
		LockCmd((PacketHeader*)pcp);
	else if(g_appmode == APPMODE_EDITOR)
		PlaceCd(pcp);

	free(pcp);
	ClearCdPlans(ctype);

#if 0
	CdType* ct = &g_cdtype[ctype];
	std::list<Vec2i>& csel = *(std::list<Vec2i>*)(((char*)&g_sel)+ct->seloff);

	for(int32_t x=0; x<g_mapsz.x; x++)
	{
		for(int32_t y=0; y<g_mapsz.y; y++)
		{
			CdTile* plan = GetCd(ctype, x, y, true);

			if(plan->on)
			{
				CdTile* actual = GetCd(ctype, x, y, false);
				bool willchange = !actual->on;
				PlaceCd(ctype, x, y, plan->owner, false);

				if(g_appmode == APPMODE_PLAY && !actual->finished)
					csel.push_back(Vec2i(x,y));

				if(g_appmode == APPMODE_PLAY && willchange)
				{
					NewJob(UMODE_GOCDJOB, x, y, ctype);
				}
			}
		}
	}

	ClearCdPlans(ctype);
	ReNetw(ctype);

	if(g_appmode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_localP];
		GUI* gui = &g_gui;

		std::list<Vec2i>* csel = (std::list<Vec2i>*)((char*)(&g_sel)+ct->seloff);

		if(csel->size() > 0)
		{
			CstrView* cv = (CstrView*)gui->get("cs view");
			cv->regen(&g_sel);
			gui->show("cs view");
		}
	}
#endif
}

void DrawCd(CdTile* ctile, unsigned char x, unsigned char y, unsigned char cdtype, bool plan, float rendz,
			uint32_t renderdepthtex, uint32_t renderfb)
{
	//return;
	StartTimer(TIMER_DRAWCD);

	Player* py = &g_player[g_localP];
	CdType* ct = &g_cdtype[cdtype];
	Shader* s = &g_shader[g_curS];

#if 0
	int32_t x;
	int32_t y;

	Depthable* d = ctile->depth;

	CdXY(d->cdtype, ctile, d->plan, x, y);
#endif

	float alpha = 1.0f;

	if(plan)
		//glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
			alpha = 0.5f;

	int16_t tx = x;
	int16_t ty = y;

	if(IsTileVis(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, alpha);
	else if(Explored(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, alpha);
	else
	{
		StopTimer(TIMER_DRAWCD);
		return;
	}

	//if(!ctile->on)
	//	return;

	Tile& tile = SurfTile(x, y, &g_hmap);

	const float* owncol = g_player[ctile->owner].color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

	//Log("draw ctile "<<x<<","<<y<<": conntype"<<ctile->conntype<<", finished:"<<(int32_t)ctile->finished<<" incltype:"<<tile.incltype);

#if 1
	int32_t sli = ct->splist[ctile->conntype][ctile->finished ? 1 : 0];
	SpList* sl = &g_splist[ sli ];
	int32_t ci = SpriteRef(sl, 0, tile.incltype, 0, 0, 0, 0, 0);
	int32_t spi = sl->sprites[ ci ];
	Sprite* sp = &g_sprite[ spi ];
#else
	int32_t sli = ct->splist[ CONNECTION_EAST ][ 1 ];
	SpList* sl = &g_splist[ sli ];
	int32_t ci = SpriteRef(sl, 0, IN_0000, 0, 0, 0);
	int32_t spi = sl->sprites[ ci ];
	Sprite* sp = &g_sprite[ spi ];
#endif

	Vec3i cmpos = Vec3i( x * TILE_SIZE + TILE_SIZE/2, y * TILE_SIZE + TILE_SIZE/2, tile.elev * TILE_RISE );

	//Log("cmpos1 = "<<cmpos.x<<","<<cmpos.y);

	//cmpos.y = Bilerp(&g_hmap, (float)(cmpos.x + ct->drawoff.x), (float)(cmpos.y + ct->drawoff.y)) * TILE_RISE + ct->drawoff.y;
	Vec2i screenpos = CartToIso(cmpos) - g_scroll;

	//Log("cmpos2 = "<<(float)(cmpos.x + ct->drawoff.x)<<","<<(float)(cmpos.y + ct->drawoff.y)););

	//Log("cmpos.y/TILE_RISE = "<<(cmpos.y / TILE_RISE));

	Texture* difftex = &g_texture[ sp->difftexi ];
	Texture* depthtex = &g_texture[ sp->depthtexi ];
	Texture* teamtex = &g_texture[ sp->teamtexi ];
	Texture* elevtex = &g_texture[sp->elevtexi];
	
	glUniform1f(s->slot[SSLOT_BASEELEV], cmpos.z);

#if 0
	DrawImage(difftex->texname,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->cropoff[0]/(float)difftex->width, sp->cropoff[1]/(float)difftex->height, 
		sp->cropoff[2]/(float)difftex->width, sp->cropoff[3]/(float)difftex->height,
		g_gui.m_crop);
#elif 0
	DrawDeep(difftex->texname, depthtex->texname, rendz,
		(float)screenpos.x + sp->offset[0],
		(float)screenpos.y + sp->offset[1],
		(float)screenpos.x + sp->offset[2],
		(float)screenpos.y + sp->offset[3],
		0, 0, 1, 1);
#else
	DrawDeep2(difftex->texname, depthtex->texname, teamtex->texname, elevtex->texname, 
		renderdepthtex, renderfb,
		rendz, cmpos.z,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->crop[0], sp->crop[1],
		sp->crop[2], sp->crop[3]);
#endif

	//glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1.0f);
	StopTimer(TIMER_DRAWCD);
}

#if 0
void DrawCd(unsigned char ctype)
{
	//StartTimer(TIMER_DRAWROADS);

	Player* py = &g_player[g_localP];
	CdType* ct = &g_cdtype[ctype];
	Shader* s = &g_shader[g_curS];

	for(int32_t x=0; x<g_mapsz.x; x++)
		for(int32_t y=0; y<g_mapsz.y; y++)
		{
			CdTile* ctile = GetCd(ctype, x, y, false);

			if(!ctile->on)
				continue;

			Tile& tile = SurfTile(x, y);

			const float* owncol = g_player[ctile->owner].color;
			glUniform4f(s->slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

			//Log("draw ctile "<<x<<","<<y<<": conntype"<<ctile->conntype<<", finished:"<<(int32_t)ctile->finished<<" incltype:"<<tile.incltype);
			
			int32_t sli = ct->splist[ctile->conntype][(int32_t)ctile->finished];
			SpList* sl = &g_splist[ sli ];
			int32_t ci = SpriteRef(sl, 0, tile.incltype, 0, 0, 0);
			Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

			Vec3i cmpos = Vec3i( x * TILE_SIZE + TILE_SIZE/2, y * TILE_SIZE + TILE_SIZE/2, tile.elev * TILE_RISE );

			//cmpos.y = Bilerp(&g_hmap, (float)(cmpos.x + ct->drawoff.x), (float)(cmpos.y + ct->drawoff.y)) * TILE_RISE + ct->drawoff.y;
			Vec2i screenpos = CartToIso(cmpos) - g_scroll;

			//Log("cmpos.y/TILE_RISE = "<<(cmpos.y / TILE_RISE));

			Texture* tex = &g_texture[ sp->difftexi ];

			DrawImage(tex->texname,
				(float)screenpos.x + sp->offset[0],
				(float)screenpos.y + sp->offset[1],
				(float)screenpos.x + sp->offset[2],
				(float)screenpos.y + sp->offset[3], 
				0,0,1,1, g_gui.m_crop);
		}

#if 0
		int32_t spi = ct->sprite[CONNECTION_NORTHWEST][1][IN_0000];
		//Log("spi "<<spi);
		Sprite* sp = &g_sprite[ spi ];

		//Vec3i cmpos = Vec3i( x * TILE_SIZE + ct->drawoff.x, y * TILE_SIZE + ct->drawoff.y, tile.tpos.y * TILE_RISE + ct->drawoff.y );
		Vec2i screenpos = Vec2i(100,100);

		Texture* tex = &g_texture[ sp->difftexi ];

		DrawImage(tex->texname,
			(float)screenpos.x + sp->offset[0],
			(float)screenpos.y + sp->offset[1],
			(float)screenpos.x + sp->offset[2],
			(float)screenpos.y + sp->offset[3]);
#endif

		if(g_build != BL_TYPES + ctype)
			return;

		glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 0.5f);

		for(int32_t x=0; x<g_mapsz.x; x++)
			for(int32_t y=0; y<g_mapsz.y; y++)
			{
				CdTile* ctile = GetCd(ctype, x, y, true);

				if(!ctile->on)
					continue;

				const float* owncol = g_player[ctile->owner].color;
				glUniform4f(s->slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

				Tile& tile = SurfTile(x, y);

				int32_t spi = ct->sprite[ctile->conntype][(int32_t)ctile->finished][tile.incltype];
				Sprite* sp = &g_sprite[ spi ];

				Vec3i cmpos = Vec3i( x * TILE_SIZE + TILE_SIZE/2, y * TILE_SIZE + TILE_SIZE/2, tile.elev * TILE_RISE );
				//cmpos.y = Bilerp(&g_hmap, (float)(cmpos.x + ct->drawoff.x), (float)(cmpos.y + ct->drawoff.y)) * TILE_RISE + ct->drawoff.y;
				Vec2i screenpos = CartToIso(cmpos) - g_scroll;

				Texture* tex = &g_texture[ sp->difftexi ];

				DrawImage(tex->texname,
					(float)screenpos.x + sp->offset[0],
					(float)screenpos.y + sp->offset[1],
					(float)screenpos.x + sp->offset[2],
					(float)screenpos.y + sp->offset[3], 
					0,0,1,1, g_gui.m_crop);
			}

			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);

			//StopTimer(TIMER_DRAWROADS);
}
#endif

CdTile::CdTile()
{
	on = false;
	finished = false;
	Zero(conmat);
	netw = -1;
	depth = NULL;

	for(char ri=0; ri<RESOURCES; ri++)
		transporter[ri] = -1;
}

CdTile::~CdTile()
{
}

#if 0
unsigned char CdTile::cdtype()
{
	return CD_NONE;
}
#endif

int32_t CdTile::netreq(int32_t res, unsigned char cdtype)
{
	int32_t netrq = 0;

	if(!finished)
	{
		CdType* ct = &g_cdtype[cdtype];
		netrq = ct->conmat[res] - conmat[res];
	}

	return netrq;
}

void CdTile::destroy()
{
	Zero(conmat);
	on = false;
	finished = false;
	netw = -1;

	//corpc fix TODO
	//if(g_collider.m_size && on)
	if(on && g_pathnode)
		freecollider();

	for(std::list<Depthable>::iterator qit=g_drawlist.begin(); qit!=g_drawlist.end(); qit++)
	{
		if(&*qit == depth)
		{
			g_drawlist.erase(qit);
			depth = NULL;
			break;
		}
	}
}

void CdTile::allocate(unsigned char cdtype)
{
#if 1
	Player* py = &g_player[owner];
	CdType* ct = &g_cdtype[cdtype];
	RichText transx;

	for(int32_t i=0; i<RESOURCES; i++)
	{
		if(ct->conmat[i] <= 0)
			continue;

		if(i == RES_LABOUR)
			continue;
		//InfoMess("a","a1");

		int32_t alloc = ct->conmat[i] - conmat[i];

		if(py->global[i] < alloc)
			alloc = py->global[i];

		conmat[i] += alloc;
		py->global[i] -= alloc;

		if(i == RES_DOLLARS)
			py->gnp += alloc;

		if(alloc > 0)
		{
			//InfoMess("a","a");
			Resource* r = &g_resource[i];
			transx.m_part.push_back(RichPart(RICH_ICON, r->icon));
			char cstr1[32];
			sprintf(cstr1, "-%d  ", alloc);
			transx.m_part.push_back(RichPart(cstr1));
		}
	}

	if(transx.m_part.size() > 0
#ifdef LOCAL_TRANSX
		&& owner == g_localP
#endif
		)
	{
		int32_t x, y;
		CdXY(cdtype, this, false, x, y);
		NewTransx(Vec2i(x*TILE_SIZE + ct->physoff.x, y*TILE_SIZE + ct->physoff.y), &transx);
		//InfoMess("a","a");
	}

	checkconstruction(cdtype);
#endif
}

bool CdTile::checkconstruction(unsigned char cdtype)
{
	CdType* ct = &g_cdtype[cdtype];

	for(int32_t i=0; i<RESOURCES; i++)
		if(conmat[i] < ct->conmat[i])
			return false;

#if 1
	if(owner == g_localP)
	{
		char inform[128];
		sprintf(inform, "%s ", ct->name);
		RichText rt = RichText(inform) + STRTABLE[STR_CONCOMP];
		AddNotif(&rt);
		//ConCom();
	}

	finished = true;
	fillcollider();

	int32_t x, y;
	CdXY(cdtype, this, false, x, y);
	ReNetw(cdtype);

	Vec2i cmpos = Vec2i(x,y)*TILE_SIZE + ct->physoff;
	int32_t cmminx = cmpos.x - TILE_SIZE/2;
	int32_t cmminy = cmpos.y - TILE_SIZE/2;
	int32_t cmmaxx = cmminx + TILE_SIZE-1;
	int32_t cmmaxy = cmminy + TILE_SIZE-1;
	ClearFol(cmminx, cmminy, cmmaxx, cmmaxy);

	Zero(conmat);
	//if(owner == g_localP)
	//	OnFinishedB(ROAD);

	//MarkTerr(this);
#endif

	return true;
}

void CdTile::fillcollider()
{
}

void CdTile::freecollider()
{
}

void DefConn(unsigned char conduittype,
			 unsigned char connectiontype,
			 bool finished,
			 const char* sprel,
			 int rendtype)
{
	CdType* ct = &g_cdtype[conduittype];
	uint32_t* spl = &ct->splist[connectiontype][finished ? 1 : 0];
	//QueueModel(tm, modelfile, scale, transl);

	//for(int32_t i=0; i<INCLINES; i++)
	{
		//char specific[WF_MAX_PATH+1];
		//sprintf(specific, "%s_inc%s", sprel, INCLINENAME[i]);
		//QueueSprite(specific, &tm[i], true, true);
#if 11
		if(!LoadSpriteList(sprel, spl, true, true, true))
		{
			char m[128];
			sprintf(m, "Failed to load sprite list %s", sprel);
			ErrMess("Error", m);
		}
#else
		QueueRend(sprel, rendtype,
			   spl,
				   false,true,false,false,
				   1,1,
				   true,true,true, true,
				   1);
#endif
	}
}

int32_t CdWorkers(unsigned char ctype, int32_t tx, int32_t ty)
{
	CdType* ct = &g_cdtype[ctype];
	CdTile* tilesarr = ct->cdtiles[(int32_t)false];
	CdTile* ctile = &tilesarr[ tx + ty*(g_mapsz.x+0) ];

	int32_t c = 0;

	for(int32_t ui=0; ui<UNITS; ++ui)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(u->type != UNIT_LABOURER)
			continue;

		if(//u->mode != UMODE_GOCDJOB &&
			u->mode != UMODE_CDJOB)
			continue;

		if(u->target != tx)
			continue;

		if(u->target2 != ty)
			continue;

		if(u->cdtype != ctype)
			continue;

		++c;
	}

	return c;
}

void DefCd(unsigned char ctype,
		   const char* name,
		   const char* iconrel,
		   /* */
		   uint16_t netwoff,
		   uint16_t seloff,
		   /* */
		   bool blconduct,
		   bool cornerpl,
		   Vec2i physoff,
		   Vec3i drawoff,
		   const char* lacktex,
		   uint16_t maxsideincl,
		   uint16_t maxforwincl,
		   bool reqsource,
		   unsigned char flags)
{
	CdType* ct = &g_cdtype[ctype];
	//ct->on = true;
	strcpy(ct->name, name);
	QueueTex(&ct->icontex, iconrel, true, false);
	/* */
	ct->netwoff = netwoff;
	ct->seloff = seloff;
	/* */
	ct->physoff = physoff;
	ct->drawoff = drawoff;
	ct->cornerpl = cornerpl;
	ct->blconduct = blconduct;
	ct->maxsideincl = maxsideincl;
	ct->maxforwincl = maxforwincl;
	ct->reqsource = reqsource;
	ct->flags = flags;
	QueueTex(&ct->lacktex, lacktex, true, false);
}

void CdDes(unsigned char ctype, const char* desc)
{
	CdType* ct = &g_cdtype[ctype];
	ct->desc = desc;
}

void CdMat(unsigned char ctype, unsigned char rtype, int16_t ramt)
{
	CdType* ct = &g_cdtype[ctype];
	ct->conmat[rtype] = ramt;
}

void CdXY(unsigned char ctype, CdTile* ctile, bool plan, int32_t& tx, int32_t& ty)
{
	CdType* ct = &g_cdtype[ctype];
	CdTile* tilearr = ct->cdtiles[(int32_t)plan];
	int32_t off = ctile - tilearr;
	ty = off / g_mapsz.x;
	tx = off % g_mapsz.x;
}

void PruneCd(unsigned char ctype)
{
	CdType* ct = &g_cdtype[ctype];

	for(int32_t x=0; x<g_mapsz.x; x++)
		for(int32_t y=0; y<g_mapsz.y; y++)
			if(GetCd(ctype, x, y, false)->on && !CdPlaceable(ctype, x, y))
			{
				CdTile* ctile = GetCd(ctype, x, y, false);
				ctile->on = false;

				for(int32_t i=0; i<UNITS; i++)
				{
					Unit* u = &g_unit[i];

					if(!u->on)
						continue;

					switch(u->mode)
					{
					case UMODE_GOCDJOB:
					case UMODE_CDJOB:
					case UMODE_GODEMCD:
					case UMODE_ATDEMCD:
						if(u->target == x &&
							u->target == y &&
							u->cdtype == ctype)
							ResetMode(u);
						break;
					case UMODE_GOSUP:
					case UMODE_ATSUP:
					case UMODE_GOREFUEL:
					case UMODE_REFUELING:
						if(u->targtype == TARG_CD &&
							u->target == x &&
							u->target == y &&
							u->cdtype == ctype)
							ResetMode(u);
						break;
					};
				}

				ConnectCdAround(ctype, x, y, false);

				std::list<Depthable>::iterator qit = g_drawlist.begin();
				while(qit != g_drawlist.end())
				{
					if(&*qit != ctile->depth)
					{
						qit++;
						continue;
					}

					qit = g_drawlist.erase(qit);
				}

				ctile->depth = NULL;

			}
}
