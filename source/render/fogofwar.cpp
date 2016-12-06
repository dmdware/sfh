













#include "fogofwar.h"
#include "../sim/utype.h"
#include "../sim/unit.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "heightmap.h"
#include "../utils.h"
#include "../math/fixmath.h"
#include "../app/appmain.h"
#include "../gui/layouts/messbox.h"

VisTile* g_vistile = NULL;


void AddVis(Unit* u)
{
	//return;

	//if(u->owner < 0)
	//	return;
#if 0
	if(u->owner >= PLAYERS)
		return;
#endif

	UType* ut = &g_utype[u->type];
	int32_t vr = ut->visrange;
	int32_t cmminx = u->cmpos.x - vr;
	int32_t cmminy = u->cmpos.y - vr;
	int32_t cmmaxx = u->cmpos.x + vr;
	int32_t cmmaxy = u->cmpos.y + vr;
	int32_t tminx = imax(0, cmminx / TILE_SIZE);
	int32_t tminy = imax(0, cmminy / TILE_SIZE);
	int32_t tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int32_t tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	///Log("cymmp "<<u->cmpos.y<<","<<vr);
	//Log("cymm "<<cmminy<<","<<cmmaxy);
	//Log("tymm "<<tminy<<","<<tmaxy);

	for(int32_t tx=tminx; tx<=tmaxx; tx++)
		for(int32_t ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int32_t dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int32_t dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int32_t dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int32_t dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int32_t dcmx = imin(dcmx1, dcmx2);
			int32_t dcmy = imin(dcmy1, dcmy2);
#else
			int32_t dcmx = tx * TILE_SIZE + TILE_SIZE/2 - u->cmpos.x;
			int32_t dcmy = ty * TILE_SIZE + TILE_SIZE/2 - u->cmpos.y;
#endif

#if 0
			int32_t d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int32_t d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			int32_t ui = u - g_unit;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			//v->uvis.push_back( ui );
			v->vis[u->owner] = 255;

			//Log("add vis "<<tx<<","<<ty);
		}
}


void UpdVis()
{
	//return;

#if 1
	for(unsigned char ty=0; ty<g_mapsz.y; ++ty)
	{
		for(unsigned char tx=0; tx<g_mapsz.x; ++tx)
		{
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];

			for(unsigned char pi=0; pi<PLAYERS; ++pi)
			{
				if(v->vis[pi] == 0)
					continue;
				v->vis[pi] --;
			}
		}
	}
#endif
}

void RemVis(Unit* u)
{
	//return;

#if 0
	UType* ut = &g_utype[u->type];
	int32_t vr = ut->visrange;
	int32_t cmminx = u->cmpos.x - vr;
	int32_t cmminy = u->cmpos.y - vr;
	int32_t cmmaxx = u->cmpos.x + vr;
	int32_t cmmaxy = u->cmpos.y + vr;
	int32_t tminx = imax(0, cmminx / TILE_SIZE);
	int32_t tminy = imax(0, cmminy / TILE_SIZE);
	int32_t tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int32_t tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	for(int32_t tx=tminx; tx<=tmaxx; tx++)
		for(int32_t ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int32_t dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int32_t dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int32_t dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int32_t dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int32_t dcmx = imin(dcmx1, dcmx2);
			int32_t dcmy = imin(dcmy1, dcmy2);
#else
			int32_t dcmx = tx * TILE_SIZE + TILE_SIZE/2 - u->cmpos.x;
			int32_t dcmy = ty * TILE_SIZE + TILE_SIZE/2 - u->cmpos.y;
#endif

#if 0
			int32_t d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int32_t d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			int32_t ui = u - g_unit;

			std::list<Widget*>::iterator vit=v->uvis.begin();
			while(vit!=v->uvis.end())
			{
				if(*vit == ui)
				{
					vit = v->uvis.erase(vit);
					//Log("rem vis "<<tx<<","<<ty);
					continue;
				}

				vit++;
			}
		}
#endif
}

void AddVis(Building* b)
{
	//return;

	//Log("bt "<<b->type);
	//

	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int32_t vr = bt->visrange;
	int32_t cmminx = bcmpos.x - vr;
	int32_t cmminy = bcmpos.y - vr;
	int32_t cmmaxx = bcmpos.x + vr;
	int32_t cmmaxy = bcmpos.y + vr;
	int32_t tminx = imax(0, cmminx / TILE_SIZE);
	int32_t tminy = imax(0, cmminy / TILE_SIZE);
	int32_t tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int32_t tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);
	int32_t cmposx = (cmminx+cmmaxx)/2;
	int32_t cmposy = (cmminy+cmmaxy)/2;

	//Log("bav "<<bcmpos.x<<","<<bcmpos.y);

	for(int32_t tx=tminx; tx<=tmaxx; tx++)
		for(int32_t ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int32_t dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int32_t dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int32_t dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int32_t dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int32_t dcmx = imin(dcmx1, dcmx2);
			int32_t dcmy = imin(dcmy1, dcmy2);
#else
			int32_t dcmx = tx * TILE_SIZE + TILE_SIZE/2 - cmposx;
			int32_t dcmy = ty * TILE_SIZE + TILE_SIZE/2 - cmposy;
#endif

#if 0
			int32_t d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int32_t d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible

			/*
			Using int32_t bi :

==7857== Invalid write of size 8
==7857==    at 0x5F73B3F: std::__detail::_List_node_base::_M_hook(std::__detail::_List_node_base*) (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.18)
==7857==    by 0x47DE74: void std::list<int16_t, std::allocator<int16_t> >::_M_insert<int16_t>(std::_List_iterator<int16_t>, int16_t&&) (stl_list.h:1562)
==7857==    by 0x47DCDE: std::list<int16_t, std::allocator<int16_t> >::push_back(int16_t&&) (stl_list.h:1021)
==7857==    by 0x47D086: AddVis(Building*) (fogofwar.cpp:160)
==7857==    by 0x46C514: FillColliderGrid() (collidertile.cpp:281)
==7857==    by 0x4A54B9: LoadMap(char const*) (savemap.cpp:1310)
==7857==    by 0x4368F6: Click_LV_Load() (loadview.cpp:78)
==7857==    by 0x421279: Button::inev(InEv*) (button.cpp:119)
==7857==    by 0x4371F7: LoadView::subinev(InEv*) (loadview.cpp:134)
==7857==    by 0x44C16A: Win::inev(InEv*) (windoww.cpp:347)
==7857==    by 0x41B127: GUI::inev(InEv*) (gui.cpp:100)
==7857==    by 0x550256: EventLoop() (appmain.cpp:1319)
==7857==  Address 0x10000003e2d3148 is not stack'd, malloc'd or (recently) free'd
			*/

			int32_t bi = b - g_building;

			//Log("v "<<(tx + ty * g_mapsz.x)<<"/"<<(g_mapsz.x*g_mapsz.y)<<" ");
			//

			//Log("(g_vistile == NULL) ?= " << (g_vistile == NULL) << std::endl;
			//

			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];

			//Log("((&g_vistile[ tx + ty * g_mapsz.x ]) == (&(g_vistile[ tx + ty * g_mapsz.x ]))) ?= " <<
			//((&g_vistile[ tx + ty * g_mapsz.x ]) == (&(g_vistile[ tx + ty * g_mapsz.x ]))) << std::endl;
			//

			//Log("(v == NULL) ?= " << (v == NULL) << std::endl;
			//

			//Log("v s="<<v->bvis.size());
			//

			//v->bvis.push_back( bi );
			v->vis[b->owner] = 255;

			//Log("bav "<<tx<<","<<ty);

			//Log("v s="<<v->bvis.size());
			//
		}
}

void RemVis(Building* b)
{
#if 0
	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int32_t vr = bt->visrange;
	int32_t cmminx = bcmpos.x - vr;
	int32_t cmminy = bcmpos.y - vr;
	int32_t cmmaxx = bcmpos.x + vr;
	int32_t cmmaxy = bcmpos.y + vr;
	int32_t tminx = imax(0, cmminx / TILE_SIZE);
	int32_t tminy = imax(0, cmminy / TILE_SIZE);
	int32_t tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int32_t tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);
	int32_t cmposx = (cmminx+cmmaxx)/2;
	int32_t cmposy = (cmminy+cmmaxy)/2;

	for(int32_t tx=tminx; tx<=tmaxx; tx++)
		for(int32_t ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 1
			int32_t dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int32_t dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int32_t dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int32_t dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int32_t dcmx = imin(dcmx1, dcmx2);
			int32_t dcmy = imin(dcmy1, dcmy2);
#else
			int32_t dcmx = tx * TILE_SIZE + TILE_SIZE/2 - cmposx;
			int32_t dcmy = ty * TILE_SIZE + TILE_SIZE/2 - cmposy;
#endif

#if 1
			int32_t d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int32_t d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			int32_t bi = b - g_building;

			std::list<Widget*>::iterator vit=v->bvis.begin();
			while(vit!=v->bvis.end())
			{
				if(*vit == bi)
				{
					vit = v->bvis.erase(vit);
					continue;
				}

				vit++;
			}
		}
#endif
}

void Explore(Unit* u)
{
	//return;

	//if(u->owner < 0)
	//	return;
#if 0
	if(u->owner >= PLAYERS)	//TODO figure why this case happens and fix it
	{
#if 0
		char m[126];
		sprintf(m, "uown=%d", u->owner);
		RichText rm = m;
		Mess(&rm);
#endif
		return;
	}
#endif

	UType* ut = &g_utype[u->type];
	int32_t vr = ut->visrange;
	int32_t cmminx = u->cmpos.x - vr;
	int32_t cmminy = u->cmpos.y - vr;
	int32_t cmmaxx = u->cmpos.x + vr;
	int32_t cmmaxy = u->cmpos.y + vr;
	int32_t tminx = imax(0, cmminx / TILE_SIZE);
	int32_t tminy = imax(0, cmminy / TILE_SIZE);
	int32_t tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int32_t tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	///Log("cymmp "<<u->cmpos.y<<","<<vr);
	//Log("cymm "<<cmminy<<","<<cmmaxy);
	//Log("tymm "<<tminy<<","<<tmaxy);

	for(int32_t tx=tminx; tx<=tmaxx; tx++)
		for(int32_t ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int32_t dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int32_t dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int32_t dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int32_t dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int32_t dcmx = imin(dcmx1, dcmx2);
			int32_t dcmy = imin(dcmy1, dcmy2);
#else
			int32_t dcmx = tx * TILE_SIZE + TILE_SIZE/2 - u->cmpos.x;
			int32_t dcmy = ty * TILE_SIZE + TILE_SIZE/2 - u->cmpos.y;
#endif

#if 0
			int32_t d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int32_t d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			int32_t ui = u - g_unit;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			v->explored[u->owner] = true;

			//Log("add vis "<<tx<<","<<ty);
		}
}

void Explore(Building* b)
{
	//return;

	if(b->owner < 0)
		return;

	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int32_t vr = bt->visrange;
	int32_t cmminx = bcmpos.x - vr;
	int32_t cmminy = bcmpos.y - vr;
	int32_t cmmaxx = bcmpos.x + vr;
	int32_t cmmaxy = bcmpos.y + vr;
	int32_t tminx = imax(0, cmminx / TILE_SIZE);
	int32_t tminy = imax(0, cmminy / TILE_SIZE);
	int32_t tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int32_t tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);
	int32_t cmposx = (cmminx+cmmaxx)/2;
	int32_t cmposy = (cmminy+cmmaxy)/2;

	//Log("bav "<<bcmpos.x<<","<<bcmpos.y);

	for(int32_t tx=tminx; tx<=tmaxx; tx++)
		for(int32_t ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int32_t dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int32_t dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int32_t dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int32_t dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int32_t dcmx = imin(dcmx1, dcmx2);
			int32_t dcmy = imin(dcmy1, dcmy2);
#else
			int32_t dcmx = tx * TILE_SIZE + TILE_SIZE/2 - cmposx;
			int32_t dcmy = ty * TILE_SIZE + TILE_SIZE/2 - cmposy;
#endif

#if 0
			int32_t d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int32_t d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			int32_t bi = b - g_building;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			v->explored[b->owner] = true;

			//Log("bav "<<tx<<","<<ty);
		}
}

bool IsTileVis(int16_t py, int16_t tx, int16_t ty)
{
	return true;	//this isn't an RTS... yet

	if(g_appmode == APPMODE_EDITOR)
		return true;

	//if(g_netmode != NETM_SINGLE)
	//	return true;	//for now

	VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];

#if 0
	for(std::list<Widget*>::iterator vit=v->bvis.begin(); vit!=v->bvis.end(); vit++)
	{
		Building* b = &g_building[ *vit ];

		if(b->owner == py)
			return true;
	}

	for(std::list<Widget*>::iterator vit=v->uvis.begin(); vit!=v->uvis.end(); vit++)
	{
		Unit* u = &g_unit[ *vit ];

		//InfoMess("v","v");
		if(u->owner == py)
			return true;
	}
#else
	return v->vis[py] > 0;
#endif

	return false;
}

bool Explored(int16_t py, int16_t tx, int16_t ty)
{
	//2015/10/27 now says every area is explored until it becomes a full RTS
	return true;	//this isn't an rts yet
	VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
	return v->explored[py];
}
