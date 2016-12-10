











#include "unit.h"
#include "../render/shader.h"
#include "utype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../math/hmapmath.h"
#include "umove.h"
#include "../render/transaction.h"
#include "simdef.h"
#include "simflow.h"
#include "../phys/trace.h"
#include "../phys/collision.h"
#include "../path/collidertile.h"
#include "../path/pathdebug.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "../path/jpspath.h"
#include "../path/jpspartpath.h"
#include "../path/pathnode.h"
#include "../path/partialpath.h"
#include "../path/partialpath.h"
#include "labourer.h"
#include "../math/fixmath.h"
#include "build.h"
#include "../path/pathjob.h"
#include "../path/tilepath.h"
#include "../path/astarpath.h"
#include "../path/anypath.h"
#include "../path/fillbodies.h"
#include "../render/fogofwar.h"
#include "border.h"
#include "../gui/layouts/messbox.h"
#include "../math/vec3f.h"

//not engine
#include "../gui/layouts/chattext.h"

//TODO circular unit radius
bool UnitCollides(Unit* u, Vec2i cmpos, int32_t utype)
{
	UType* t = &g_utype[utype];
	int32_t cmminx = cmpos.x - t->size.x/2;
	int32_t cmminy = cmpos.y - t->size.x/2;
	int32_t cmmaxx = cmminx + t->size.x - 1;
	int32_t cmmaxy = cmminy + t->size.x - 1;
	
	if(cmminx < 0 || cmminy < 0 || cmmaxx >= g_mapsz.x * TILE_SIZE || cmmaxy >= g_mapsz.y * TILE_SIZE)
		return true;

	//int32_t ui = u-g_unit;
	//if(((g_simframe+ui)%1001!=1))
	//	return false;

	int32_t cx = cmpos.x / PATHNODE_SIZE;
	int32_t cy = cmpos.y / PATHNODE_SIZE;

#if 1
	//new

	//might still collide becuase in Standable there is
#if 0
	if(roaded)
	{
		CdTile* ctile = GetCd(CD_ROAD,
			nx / (TILE_SIZE/PATHNODE_SIZE),
			ny / (TILE_SIZE/PATHNODE_SIZE),
			false);

		return ctile->on && ctile->finished;
	}
#endif

	//so.... better check

	if(t->roaded)
	{
		int32_t pathoff = PathOff(t->size.x);

		CdTile* cdtile = GetCd(CD_ROAD, 
			(cmpos.x /* /PATHNODE_SIZE*PATHNODE_SIZE */ - pathoff)/PATHNODE_SIZE / (TILE_SIZE/PATHNODE_SIZE), 
			(cmpos.y /* /PATHNODE_SIZE*PATHNODE_SIZE */ - pathoff)/PATHNODE_SIZE / (TILE_SIZE/PATHNODE_SIZE), 
			false);

		if(!cdtile->on || !cdtile->finished)
			return true;
	}
#else	//old check...

	//ColliderTile* cell = ColliderAt(cx, cy);

	//if(!t->seaborne && !(cell->flags & FLAG_HASLAND))
	//	return true;

#if 0
	if(t->roaded && !(cell->flags & FLAG_HASROAD))
		return true;
#else
	if(t->roaded)
	{
		CdTile* cdtile = GetCd(CD_ROAD, cmpos.x / TILE_SIZE, cmpos.y / TILE_SIZE, false);

		if(!cdtile->on || !cdtile->finished)
			return true;
	}
#endif

	
#endif

	//if(cell->flags & FLAG_ABRUPT)
	//	return true;

	int32_t nminx = cmminx / PATHNODE_SIZE;
	int32_t nminy = cmminy / PATHNODE_SIZE;
	int32_t nmaxx = cmmaxx / PATHNODE_SIZE;
	int32_t nmaxy = cmmaxy / PATHNODE_SIZE;

	int32_t uin = -1;
	
	if(u)
		uin = u - g_unit;

	for(int32_t x=nminx; x<=nmaxx; x++)
		for(int32_t y=nminy; y<=nmaxy; y++)
		{
			int32_t ni = PATHNODEINDEX(x, y);
			//cell = ColliderAt(x, y);

#if 0
			if(cell->flags & FLAG_ABRUPT)
				return true;	//corpc fix

			if(cell->foliage != USHRT_MAX)
				return true;

			if(cell->building >= 0)
			{
				Building* b = &g_building[cell->building];
				BlType* t2 = &g_bltype[b->type];

				int32_t tminx = b->tpos.x - t2->width.x/2;
				int32_t tminy = b->tpos.y - t2->width.y/2;
				int32_t tmaxx = tminx + t2->width.x;
				int32_t tmaxz = tminy + t2->width.y;

				int32_t minx2 = tminx*TILE_SIZE;
				int32_t miny2 = tminy*TILE_SIZE;
				int32_t maxx2 = tmaxx*TILE_SIZE - 1;
				int32_t maxy2 = tmaxz*TILE_SIZE - 1;

				if(cmminx <= maxx2 && cmminy <= maxy2 && cmmaxx >= minx2 && cmmaxy >= miny2)
				{


					return true;
				}
			}
			
			if(cell->unit >= 0 && 
				cell->unit != uin && 
				!g_unit[cell->unit].hidden())
				return true;
#else
			//if (g_collider.on(ni))
			PathNode* n = g_pathnode + ni;
			if(n->flags & PATHNODE_BLOCKED)
				return true;
#endif

		}

	return false;
}

void CheckPath(Unit* u)
{
	UType* t = &g_utype[u->type];
	
#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("unitmove u=thatunit dgoal="<<(u->goal.x-u->cmpos.x)<<","<<(u->goal.y-u->cmpos.y)<<" ");
		Log("unitmove u=thatunit dsubgoal="<<(u->subgoal.x-u->cmpos.x)<<","<<(u->subgoal.y-u->cmpos.y)<<" ");
	}
#endif

	//Signs that we need a new path
	if( /*u->cmpos != u->goal && */	//corpd fix
	   ( u->path.size() <= 0 ||
		(*u->path.rbegin() != u->goal
#ifdef HIERPATH
		&&
			(u->tpath.size() == 0 //||
			//*u->tpath.rbegin() != Vec2s(u->goal.x/TILE_SIZE,u->goal.y/TILE_SIZE)	//doesn't work for corner-placed conduits
			//let's hope tpath will always be reset when new path is needed.
			)
#endif
			) ) )
	{
		
#if 0
			if(u - g_unit == 205)
			{
				InfoMess("t","t");
			}
#endif

#if 1
		if(t->military)
		//if(1)
		{
			if(g_simframe - u->lastpath < u->pathdelay)
			{
				return;
			}

			//u->pathdelay += 1;
			//u->pathdelay *= 2;
			//u->pathdelay += 50;
			u->pathdelay = (u->pathdelay + 50) % PATH_DELAY;
			u->lastpath = g_simframe;

#if 1
			const Vec2i noff = (u->goal - u->cmpos) / PATHNODE_SIZE;
			int32_t nodesdist = PATHHEUR( noff );

			PartialPath(u->type, u->mode,
			            u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype, u->supplier,
						&u->path, &u->subgoal,
			            u, NULL, NULL,
			            u->goal.x, u->goal.y,
			            u->goal.x, u->goal.y, u->goal.x, u->goal.y,
			            //nodesdist*nodesdist*1);
			TILE_SIZE*4/PATHNODE_SIZE, false, true);	//enough to move 3 tiles around a corner, filled with obstacles

#if 0
			if(u->path.size() <= 0)
			{
				u->path.push_back( u->goal );
				u->subgoal = u->goal;
			}
#endif
#elif 1
			u->path.clear();
			u->path.push_back( u->goal );
			u->subgoal = u->goal;
#elif 0
			JPSPath(u->type, u->mode,
			            u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, &u->path, &u->subgoal,
			            u, NULL, NULL,
			            u->goal.x, u->goal.y,
			            u->goal.x, u->goal.y, u->goal.x, u->goal.y);
#else
			JPSPartPath(u->type, u->mode,
			            u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, &u->path, &u->subgoal,
			            u, NULL, NULL,
			            u->goal.x, u->goal.y,
			            u->goal.x, u->goal.y, u->goal.x, u->goal.y,
			            nodesdist*4);
#endif

#if 0
			RichText rtext("ppathf");
			NewTransx(u->cmpos + Vec3f(0,t->size.y,0), &rtext);
#endif
		}
		//else if not military
		else //if(!u->pathblocked)
#endif
		{

			if(g_simframe - u->lastpath < u->pathdelay)
			{
				return;
			}

			
#if 0
			if(u - g_unit == 205)
			{
				InfoMess("t","t");
			}
#endif

			//u->pathdelay += 1;
			//u->pathdelay *= 2;
			//u->pathdelay += 10;
			u->pathdelay = (u->pathdelay + PATH_DELAY/3) % PATH_DELAY;
			u->lastpath = g_simframe;
			
			//if(Trapped()

			///UType* ut = &g_utype[u->type];
			//int32_t nodesdist = PATHHEUR( (u->goal - u->cmpos) / PATHNODE_SIZE );
			///int32_t nodesdist = MAXPATHN * ut->cmspeed;

#ifdef HIERPATH
			if(u->tpath.size() == 0 // ||
				//*u->tpath.rbegin() != Vec2s(u->goal.x/TILE_SIZE,u->goal.y/TILE_SIZE)	//not true for conduits on tile corners
					)
			{
				u->path.clear();
				u->tpath.clear();
				TilePath(u->type, u->mode,
					u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype, u->supplier,
					&u->tpath,
					u, NULL, NULL,
					u->goal.x, u->goal.y,
					u->goal.x, u->goal.y, u->goal.x, u->goal.y,	//corpd fix
					//10000
					MAX_U_TPATH);

#if 0
				if(u->tpath.size() == 0 && t->size.x > 60)
					TilePath(u->type, u->mode,
					u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
					&u->tpath,
					u, NULL, NULL,
					u->goal.x, u->goal.y,
					u->goal.x, u->goal.y, u->goal.x, u->goal.y,	//corpd fix
					10000,
					true);
#endif

#if 0
				if(u - g_unit == 36)
				{
					char msg[128];
					sprintf(msg, "u36 trepath goal=%d,%d tpath.sz=%d", u->goal.x, u->goal.y, (int32_t)u->tpath.size());
					RichText rm = RichText(msg);
					AddNotif(&rm);
				}
#endif

				//RichText rm = RichText("CONSTANTLY REPATHING");
				//AddNotif(&rm);
				//if(u->tpath.size() == 0)
				//	InfoMess("tpf", "tpf");
				//	AddNotif(&RichText("STILL NO PATH"));
			}
#endif

#if 0
			if(!FullPath(0,
				u->type, u->mode,
				u->cmpos.x, u->cmpos.y, u->target, u->target, u->target2, u->path, u->subgoal,
				u, NULL, NULL,
				u->goal.x, u->goal.y,
				u->goal.x, u->goal.y, u->goal.x, u->goal.y))
#elif !defined(HIERPATH)
#if 0
			if(AnyPath(u->type, u->mode,
				u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
				u, NULL, NULL,
				u->goal.x, u->goal.y,
				u->goal.x, u->goal.y, u->goal.x, u->goal.y,
				0, 0, g_pathdim.x-1, g_pathdim.y-1))
#endif
#if 0
				JPSPath(u->type, u->mode,
				u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
				&u->path, &u->subgoal,
				u, NULL, NULL,
				u->goal.x, u->goal.y,
				u->goal.x, u->goal.y, u->goal.x, u->goal.y,
				0, 0, g_pathdim.x-1, g_pathdim.y-1, false, true);
#elif 0
				AStarPath(u->type, u->mode,
				u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
				&u->path, &u->subgoal,
				u, NULL, NULL,
				u->goal.x, u->goal.y,
				u->goal.x, u->goal.y, u->goal.x, u->goal.y,
				100000,
				0, 0, g_pathdim.x-1, g_pathdim.y-1, false, true);
#else
				UType* ut = &g_utype[u->type];
			//int32_t nodesdist = PATHHEUR( (u->goal - u->cmpos) / PATHNODE_SIZE ) * 1 * TILE_SIZE / PATHNODE_SIZE;
			//int32_t nodesdist = MAXPATHN + MAXPATHN * (g_simframe % 30 == 0) + MAXPATHN * (g_simframe % 20 == 0) /* * ut->cmspeed */;
			int32_t nodesdist = MAXPATHN;

			//if ((u - g_unit) == 457)
			//	InfoMess("p", "o");

			//To avoid slow-downs.
			//MazePath(u->type, u->mode,
			PartialPath(u->type, u->mode,
				u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype, u->supplier,
				&u->path, &u->subgoal,
				u, NULL, NULL,
				u->goal.x, u->goal.y,
				u->goal.x, u->goal.y, u->goal.x, u->goal.y,
				//nodesdist*nodesdist*1,
				//TILE_SIZE*TILE_SIZE*20/PATHNODE_SIZE/PATHNODE_SIZE, 
				nodesdist /*TILE_SIZE*1/PATHNODE_SIZE*/,
				true, /*true*/ true);
			//Enough to move 19 tiles around a corner, filled with obstacles.

			
			//2016/05/04 trucks without possible path now give up
			if(!u->path.size() &&
				!((g_simframe / 3) % 20) )
			{
				//AddChat(&RichText("ga"));
				//AddNotif(&RichText("ga"));
				ResetMode(u);
			}

#if 0
			if(u - g_unit == 205)
			{
				bool f = u->path.size() > 0;

				char m[123];
				sprintf(m, "f %d", (int)f);
				InfoMess(m,m);
			}

#endif
			//if ((u - g_unit) == 457)
			{
			//	if(u->path.size())
			//		InfoMess("p", "ss");
			//	else
			//		InfoMess("p", "ff");
			}
#endif
#else	//else def HIERPATH
			if(u->tpath.size() > 0)
			{
#ifdef HIERDEBUG
				g_applog<<"tp sz > 0"<<std::endl;
#endif

				Vec2s tpos = *u->tpath.begin();

#ifdef HIERDEBUG
				g_applog<<"tpos pop "<<tpos.x<<","<<tpos.y<<std::endl;
#endif

				uint32_t cmtminx = tpos.x * TILE_SIZE;
				uint32_t cmtminy = tpos.y * TILE_SIZE;
				uint32_t cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
				uint32_t cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

				UType* ut = &g_utype[ u->type ];

				uint32_t ucmminx = u->cmpos.x - ut->size.x/2;
				uint32_t ucmminy = u->cmpos.y - ut->size.x/2;
				uint32_t ucmmaxx = ucmminx + ut->size.x - 1;
				uint32_t ucmmaxy = ucmminy + ut->size.x - 1;

#ifdef HIERDEBUG
				g_applog<<"\tucmminx <= cmtmaxx = "<<(ucmminx <= cmtmaxx)<<std::endl;
				g_applog<<"\tucmminy <= cmtmaxy = "<<(ucmminy <= cmtmaxy)<<std::endl;
				g_applog<<"\tucmmaxx >= cmtminx = "<<(ucmmaxx >= cmtminx)<<std::endl;
				g_applog<<"\tucmmaxy >= cmtminy = "<<(ucmmaxy >= cmtminy)<<std::endl;
#endif
				

				if(/* u->cmpos/TILE_SIZE == Vec2i(tpos.x,tpos.y) */
					ucmminx <= cmtmaxx &&
					ucmminy <= cmtmaxy &&
					ucmmaxx >= cmtminx &&
					ucmmaxy >= cmtminy)
				{
					u->tpath.erase( u->tpath.begin() );

					if(u->tpath.size() > 0)
					{
#ifdef HIERDEBUG
						g_applog<<"start in tpos first"<<std::endl;
#endif

						u->tpath.erase( u->tpath.begin() );
						//tpos = *u->tpath.begin();

						//Next tile goal bounds
						tpos = *u->tpath.begin();

#ifdef HIERDEBUG
						g_applog<<"tpos pop next "<<tpos.x<<","<<tpos.y<<std::endl;
#endif

						cmtminx = tpos.x * TILE_SIZE;
						cmtminy = tpos.y * TILE_SIZE;
						cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
						cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

						Vec2i cmsubgoal = Vec2i(tpos.x,tpos.y) * TILE_SIZE + Vec2i(1,1)*TILE_SIZE/2;	//corpd fix xp

						//Bounds around current unit cmpos
						tpos = Vec2s( u->cmpos.x / TILE_SIZE, u->cmpos.y / TILE_SIZE );

#define IN_PATHNODES (TILE_SIZE/PATHNODE_SIZE)

#if 0
						uint16_t nminx = imax(0, (tpos.x-1) * IN_PATHNODES - 1);
						uint16_t nminy = imax(0, (tpos.y-1) * IN_PATHNODES - 1);
						uint16_t nmaxx = imin(g_pathdim.x, (tpos.x+1) * IN_PATHNODES + 1);
						uint16_t nmaxy = imin(g_pathdim.y, (tpos.y+1) * IN_PATHNODES + 1);
#elif 1
						uint16_t nminx = imax(0, (tpos.x) * IN_PATHNODES - 1);
						uint16_t nminy = imax(0, (tpos.y) * IN_PATHNODES - 1);
						uint16_t nmaxx = imin(g_pathdim.x, (tpos.x+1) * IN_PATHNODES);
						uint16_t nmaxy = imin(g_pathdim.y, (tpos.y+1) * IN_PATHNODES);
#else
						uint16_t nminx = imax(0, (tpos.x-1) * IN_PATHNODES - 1 - ut->size.x/PATHNODE_SIZE/2);
						uint16_t nminy = imax(0, (tpos.y-1) * IN_PATHNODES - 1 - ut->size.x/PATHNODE_SIZE/2);
						uint16_t nmaxx = imin(g_pathdim.x, (tpos.x+1) * IN_PATHNODES + 1 + ut->size.x/PATHNODE_SIZE/2);
						uint16_t nmaxy = imin(g_pathdim.y, (tpos.y+1) * IN_PATHNODES + 1 + ut->size.x/PATHNODE_SIZE/2);
#endif

#undef IN_PATHNODES

#if 0
						JPSPath(u->type, u->mode,
							u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
							&u->path, &u->subgoal,
							u, NULL, NULL,
							u->goal.x, u->goal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							nminx, nminy, nmaxx, nmaxy, true, false);
#elif 0
						AStarPath(u->type, u->mode,
							u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
							&u->path, &u->subgoal,
							u, NULL, NULL,
							u->goal.x, u->goal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							100000,
							nminx, nminy, nmaxx, nmaxy, true, false);
#else
						//Better to use MazePath, setting a node search limit,
						//instead of bounds, so it's able to get around corners.
						MazePath(u->type, u->mode,
							u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype, u->supplier,
							&u->path, &u->subgoal,
							u, NULL, NULL,
							cmsubgoal.x, cmsubgoal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							//nodesdist*nodesdist*1);
							TILE_SIZE*TILE_SIZE*3/PATHNODE_SIZE/PATHNODE_SIZE, false, false,
							false);
#endif
					}
				}
				//else not yet at next tile pos bounds/goal
				else
				{
#ifdef HIERDEBUG
					g_applog<<"not start in pop tpath, u->tp.sz()="<<u->tpath.size()<<std::endl;
#endif
					tpos = *u->tpath.begin();

					//Next tile goal bounds
					cmtminx = tpos.x * TILE_SIZE;
					cmtminy = tpos.y * TILE_SIZE;
					cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
					cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

					Vec2i cmsubgoal = Vec2i(tpos.x,tpos.y) * TILE_SIZE + Vec2i(1,1)*TILE_SIZE/2;	//corpd fix xp

#if 0
					char cmfail[128];
					sprintf(cmfail, "cm %d,%d->%d,%d", cmtminx, cmtminy, cmtmaxx, cmtmaxy);
					RichText cmfailr = RichText(cmfail);
					AddNotif(&cmfailr);
#endif

					//Bounds around current unit cmpos
					tpos = Vec2s( u->cmpos.x / TILE_SIZE, u->cmpos.y / TILE_SIZE );

#define IN_PATHNODES (TILE_SIZE/PATHNODE_SIZE)

					uint16_t nminx = imax(0, (tpos.x-1) * IN_PATHNODES - 1);
					uint16_t nminy = imax(0, (tpos.y-1) * IN_PATHNODES - 1);
					uint16_t nmaxx = imin(g_pathdim.x, (tpos.x+1) * IN_PATHNODES + 1);
					uint16_t nmaxy = imin(g_pathdim.y, (tpos.y+1) * IN_PATHNODES + 1);

#undef IN_PATHNODES

					//Need to append cmgoal to path, but also bound JPS search
					if(u->tpath.size() <= 1)
#if 0
						JPSPath(u->type, u->mode,
						u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
						&u->path, &u->subgoal,
						u, NULL, NULL,
						u->goal.x, u->goal.y,
						u->goal.x, u->goal.y, u->goal.x, u->goal.y,
						nminx, nminy, nmaxx, nmaxy, true, true);
#elif 0
						AStarPath(u->type, u->mode,
						u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
						&u->path, &u->subgoal,
						u, NULL, NULL,
						u->goal.x, u->goal.y,
						u->goal.x, u->goal.y, u->goal.x, u->goal.y,
						100000,
						nminx, nminy, nmaxx, nmaxy, true, true);
#else

						MazePath(u->type, u->mode,
						u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype, u->supplier,
						&u->path, &u->subgoal,
						u, NULL, NULL,
						u->goal.x, u->goal.y,
						u->goal.x, u->goal.y, u->goal.x, u->goal.y,
						//nodesdist*nodesdist*1);
						TILE_SIZE*TILE_SIZE*3/PATHNODE_SIZE/PATHNODE_SIZE, true, false, true);
#endif
					else
					{
#if 0
						JPSPath(u->type, u->mode,
							u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
							&u->path, &u->subgoal,
							u, NULL, NULL,
							u->goal.x, u->goal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							nminx, nminy, nmaxx, nmaxy, true, false);
						elif 0
							AStarPath(u->type, u->mode,
							u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
							&u->path, &u->subgoal,
							u, NULL, NULL,
							u->goal.x, u->goal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							100000,
							nminx, nminy, nmaxx, nmaxy, true, false);
#else
						/*
						it's okay now to return partial path and that will not increase existing clumps
						as as soon as the unit can now longer move closer, he will have path.size()=0 
						and move randomly, then if he collides, his path will reset, and there won't 
						be a tpath. right?
						oh, since we're capping the end with the cmsubgoal (tile center for next tile goal),
						that possibly causing some inability to get a path since we demand no partials.
						actually, the opposite is the problem; we don't cap the end when we're right at
						the tile edge, so we get size=0 path, right?
						the solution is not to cap the end, since that would lead to pathing to tile centers,
						but to set the cm tile bounds such that the unit is fully inside the tile.
						no, it shouldn't return size=0 path if there's a starting node.
						*/

#if 0
						cmtminx += ut->size.x;
						cmtminy += ut->size.x;
						cmtmaxx -= ut->size.x;
						cmtmaxy -= ut->size.x;
#elif 0
						cmtminx = cmtminx + ut->size.x/2;
						cmtminy = cmtminy + ut->size.x/2;
						cmtmaxx = cmtmaxx - ut->size.x/2;
						cmtmaxy = cmtmaxy - ut->size.x/2;
#endif

						//Better to use MazePath, setting a node search limit,
						//instead of bounds, so it's able to get around corners.
						MazePath(u->type, u->mode,
							u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype, u->supplier,
							&u->path, &u->subgoal,
							u, NULL, NULL,
							//u->goal.x, u->goal.y,	//corpd fix
							cmsubgoal.x, cmsubgoal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							//nodesdist*nodesdist*1);
							TILE_SIZE*TILE_SIZE*3/PATHNODE_SIZE/PATHNODE_SIZE, false, false /*true*/,
							false);
						//Enough to move 2 tiles around a corner, filled with obstacles.
#endif
					}

#if 0
					MazePath(u->type, u->mode,
						u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
						&u->path, &u->subgoal,
						u, NULL, NULL,
						u->goal.x, u->goal.y,
						u->goal.x, u->goal.y, u->goal.x, u->goal.y,
						//nodesdist*nodesdist*1,
						//TILE_SIZE*TILE_SIZE*20/PATHNODE_SIZE/PATHNODE_SIZE, 
						nodesdist,
						true, /*true*/ false);
#endif
#if 0
					if(u->path.size() <= 0)
					{
						RichText failed = RichText("failed 2");
						AddNotif(&failed);
					}
#endif

				}

#ifdef HIERDEBUG
				g_applog<<"s "<<u->cmpos.x<<","<<u->cmpos.y<<" (t "<<(u->cmpos.x/TILE_SIZE)<<","<<(u->cmpos.y/TILE_SIZE)<<")"<<std::endl;

				g_applog<<"ps "<<u->path.size()<<"tps = "<<u->tpath.size()<<std::endl;

				g_applog<<"g "<<u->goal.x<<","<<u->goal.y<<" (t "<<(u->goal.x/TILE_SIZE)<<","<<(u->goal.y/TILE_SIZE)<<")"<<std::endl;

				for(std::list<Widget*>::iterator pit=u->path.begin(); pit!=u->path.end(); pit++)
				{
					g_applog<<"p "<<pit->x<<","<<pit->y<<" (t "<<(pit->x/TILE_SIZE)<<","<<(pit->y/TILE_SIZE)<<")"<<std::endl;
				}

				for(std::list<Widget*>::iterator tit=u->tpath.begin(); tit!=u->tpath.end(); tit++)
				{
					g_applog<<"t "<<tit->x<<","<<tit->y<<std::endl;
				}

				if(u->path.size() == 0)
				{
					g_applog<<"not found subpath ----"<<std::endl;
				}
				else
					g_applog<<"did find subpath ----"<<std::endl;
#endif

#if 0
				//Try a full path
				if(u->path.size() <= 0)
				{
					JPSPath(u->type, u->mode,
						u->cmpos.x, u->cmpos.y,
						u->target, u->target2, u->targtype, u->cdtype,
						&u->path, &u->subgoal,
						u, NULL, NULL,
						u->goal.x, u->goal.y,
						u->goal.x, u->goal.y, u->goal.x, u->goal.y,
						0, 0, g_pathdim.x-1, g_pathdim.y-1,
						false, true);

					if(u->path.size() > 0)
					{
						//Necessary for it to not reset path upon
						//arriving at next tile node?
						u->tpath.clear();
						//u->tpath.push_back(Vec2s(u->goal.x/TILE_SIZE, u->goal.y/TILE_SIZE));
					}
				}
#endif

#if 1	/* this is not a jam, this happens normally during the course of following a tpath
				but having turned it off, I noticed that lag increased as workers weren't as spaced out as before.
				so let's set jams=4 for this unit, and check for that condition specifically to avoid.
				edit: oh well, a small price to pay (only 90fps) for not having the workers die off from inefficient movement.
				edit: oh, it was just because I wasn't checking for tpath.sz>0.
				*/
				//check for jams
				if(u->tpath.size() > 0 && u->path.size() <= 0)
				{
					Jamify(u);
					//ResetMode(u);	//TODO works?
					//u->pathblocked = true;
				}
#endif
			}
			//else after TilePath() tpath.size() == 0
			else
			{
				//Jamify(u);
				//ResetMode(u);	//TODO works?
				//u->pathblocked = true;
			}
#endif	//end if def HIERPATH

#if 0
			//causes clumps, as units gather around a bl that might have all entrances blocked off, 
			//but unit will move closer to goal in the meanwhile while there might be a blockage
			if(u->path.size() <= 0)
			{
				int32_t nodesdist = PATHHEUR( (u->goal - u->cmpos) / PATHNODE_SIZE );

				MazePath(u->type, u->mode,
					u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
					&u->path, &u->subgoal,
					u, NULL, NULL,
					u->goal.x, u->goal.y,
					u->goal.x, u->goal.y, u->goal.x, u->goal.y,
					nodesdist*nodesdist*10, true, true);

				u->pathblocked = true;
			}
			else
				u->pathblocked = false;
#endif

//#ifdef HIERPATH
#if 1	//trucks don't work for some reason
			//TODO need to check for out of map bounds?
			//induce random walking to unclump from the clumped area
			//and just to make it look like the unit doesn't know where to go
			if(u->path.size() <= 0  && 
				g_utype[u->type].walker  )
			{
				//u->path.push_back( u->goal );
				//PlaceUAb(u->type, u->cmpos, &u->subgoal);
				//u->path.push_back( u->subgoal );
				//u->subgoal = u->goal;

				//set it in a new direction (out of 8 directions) every 3 simframes
				//uint32_t timeoff = (g_simframe % (8 * 3)) / 3;
				uint32_t timeoff = ( (g_simframe + (u-g_unit)) % (8 * 3)) / 3;
				//don't set goal because that would throw off the real target goal
				u->subgoal /*= u->goal*/ = u->cmpos + Vec2i(OFFSETS[timeoff].x,OFFSETS[timeoff].y) * TILE_SIZE / 2;
				//follow the goal after subgoal so that we bump into something and repath automatically
				u->path.push_back( u->goal );

				u->pathdelay += PATH_DELAY * 2;

#ifdef RANDOM8DEBUG
				if(u - g_unit == thatunit)
				{
					g_applog<<"unitmove u=thatunit random8dir"<<std::endl;
				}
#endif
			}
#endif
//#endif
		}

		//goto endcheck;

		return;
	}
}

void MoveUnit(Unit* u)
{
	UType* t = &g_utype[u->type];
	u->prevpos = u->cmpos;
	u->collided = false;

	if(u->threadwait)
		return;

	if(u->type == UNIT_TRUCK && u->driver < 0)
		return;

	//Needed so that with large MAXPATHN, we don't spend a lot of time
	//on unreachable paths in a crowd for labourers at the beginning of new game
	if( /* u->mode == UMODE_NONE && */
	   u->cmpos == u->goal &&
	   u->cmpos == u->subgoal )	//corpd fix
	{
		//RichText mess = RichText("goal-cmpos < speed");
		//AddNotif(&mess);
		return;
	}

	Vec2i goaloff = u->goal - u->cmpos;

	if(u->underorder && u->target < 0 && DOT_VEC2I(goaloff) <= PATHNODE_SIZE*PATHNODE_SIZE)
		return;

	CheckPath(u);

	//return;	//temp

	u->freecollider();
	RemVis(u);

	Vec2i dir = u->subgoal - u->cmpos;

	//Arrived at subgoal?
	//if(Dot(u->subgoal - u->cmpos) <= t->cmspeed * t->cmspeed)
	if(iabs(dir.x) <= t->cmspeed &&	//necesssary extra checks to make sure dir doesn't overflow when computing Dot (dot product).
		iabs(dir.y) <= t->cmspeed &&
		DOT_VEC2I(dir) <= t->cmspeed * t->cmspeed)
	{
		//TODO put this and below copy in UpDraw?
		//u->rotation.z = GetYaw(dir.x, dir.y);
		//u->rotation.z = atanf( (float)dir.x / (float)dir.y );
		u->rotation.z = atan2( (float)dir.x, (float)dir.y );
		CalcRot(u);

		//PlayAnim(u->frame[BODY_LOWER], 0, t->nframes, true, 1.0f);

		u->cmpos = u->subgoal;

		//dir = Vec2i(0, 0);	//fixed; no overreach collision now.

		if(u->path.size() >= 2)
		{
			u->path.erase( u->path.begin() );
			u->subgoal = *u->path.begin();
			dir = u->subgoal - u->cmpos;
		}
		else
		{
#ifdef HIERPATH
			//Did we finish the local path? Have tpath?
			if(u->tpath.size() > 0)
			{

				Vec2s tpos = *u->tpath.begin();

				uint32_t cmtminx = tpos.x * TILE_SIZE;
				uint32_t cmtminy = tpos.y * TILE_SIZE;
				uint32_t cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
				uint32_t cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

				UType* ut = &g_utype[ u->type ];

				uint32_t ucmminx = u->cmpos.x - ut->size.x/2;
				uint32_t ucmminy = u->cmpos.y - ut->size.x/2;
				uint32_t ucmmaxx = ucmminx + ut->size.x - 1;
				uint32_t ucmmaxy = ucmminy + ut->size.x - 1;

				if(/* u->cmpos/TILE_SIZE == Vec2i(tpos.x,tpos.y) */
					ucmminx <= cmtmaxx &&
					ucmminy <= cmtmaxy &&
					ucmmaxx >= cmtminx &&
					ucmmaxy >= cmtminy &&
					u->tpath.size() > 1)
				{
					u->path.clear();

					if(*u->tpath.begin() == Vec2s(u->cmpos.x/TILE_SIZE,u->cmpos.y/TILE_SIZE))
						u->tpath.erase( u->tpath.begin() );
					//Advance
				}
				else
				{
					ResetPath(u);
				}
			}
			//necessary so units can get a real path if
			//this was just a temp partial or random 8 move
			else
#endif
			{
				ResetPath(u);
			}
		}
	}
	//corpd fix 2016
	else if(dir.x != 0 || dir.y != 0)
	//if(mag > t->cmspeed)
	{
		//u->rotation.z = GetYaw(dir.x, dir.y);
		//u->rotation.z = atanf( (float)dir.x / (float)dir.y );
		u->rotation.z = atan2( (float)dir.x, (float)dir.y );
		CalcRot(u);
		//PlayAnim(u->frame[BODY_LOWER], 0, t->nframes, true, 1.0f);

		//actually, instead of all this bit shifting nonsense, why not do this?
		int32_t maxdelta = 1 << 12;
		//get absolute value and see if it exceeds max value
		//we're assuming dir.x can't equal INT_MIN, because that can't give a positive int32_t value.
		if(iabs(dir.x) >= maxdelta ||
			iabs(dir.y) >= maxdelta)
		{
			//pick the max element and divide by it (or something something a bit smaller than it.
			//some multiple smaller so we can still have something when we divide the coordinates by it.)
			int32_t dirmax = imax(iabs(dir.x), iabs(dir.y));
			//sign(dir.x);
			//int32_t hmax = dirmax / 2;	//half of max element
			//int32_t qmax = dirmax >> 2;	//quarter of max element
			int32_t dmax = dirmax >> 8;	//max divided by
			dir.x /= dmax;
			dir.y /= dmax;
			//dividing a vector's components by the same number gives a vector pointing in the same direction but smaller.
			//dividing by (max/256) gives us a maximum vector distance of 256 in the case that the line is straight/axial.
		}

		int32_t mag = MAG_VEC2I(dir);

		if(mag <= 0)
		{
			mag = 1;
		}
		
		Vec2i speeddir = dir * t->cmspeed / mag;

		//Can result in overshooting the target otherwise,
		//and being a few centimeters in collision with building,
		//leading to forever being unable to follow the path.
		if(t->cmspeed >= mag)
			speeddir = dir;

		u->cmpos = u->cmpos + speeddir;
	}

	//corpc fix, no more sublevel braces; jumping to subgoal and dir!=0 are both moves that can have collisions.
	if(UnitCollides(u, u->cmpos, u->type))
	{
		bool ar = CheckIfArrived(u);
		
//	if(u - g_unit == 182 && g_simframe > 118500)
	//	Log("f2");

		//if(ar //|| 
		//(g_simframe+(u-g_unit))%1001==1
		//)
		{
			u->collided = true;
			u->cmpos = u->prevpos;
			u->path.clear();
			u->tpath.clear();
			u->subgoal = u->cmpos;
			u->fillcollider();
			AddVis(u);
			Explore(u);
			UpDraw(u);
			ResetPath(u);
		}

		if(ar)
			OnArrived(u);
		
	//	if((g_simframe+(u-g_unit))%1001==1)
		return;
	}

	if(!u->hidden())
	{
		
	//if(u - g_unit == 182 && g_simframe > 118500)
	//	Log("f3");

		u->fillcollider();
		AddVis(u);
		Explore(u);
		UpDraw(u);

		//if(t->military)
		if(u->type != UNIT_TRUCK)
			MarkTerr(u);
	}

	u->drawpos.x = u->cmpos.x;
	u->drawpos.y = u->cmpos.y;
//	u->drawpos.z = g_hmap.accheight2(u->cmpos.x, u->cmpos.y);
}

bool CheckIfArrived(Unit* u)
{
	UType* ut = &g_utype[u->type];
	int32_t ui = (int32_t)(u-g_unit);

	int32_t ucmminx = u->cmpos.x - ut->size.x/2;
	int32_t ucmminy = u->cmpos.y - ut->size.x/2;
	int32_t ucmmaxx = ucmminx + ut->size.x - 1;
	int32_t ucmmaxy = ucmminy + ut->size.x - 1;

	//switched from fine-grained collisions to pathnode check
#if 1
	Building* b;
	BlType* bt;
	CdType* ct;
	CdTile* ctile;
	Unit* u2;
	UType* u2t;

	int32_t btminx;
	int32_t btminy;
	int32_t btmaxx;
	int32_t btmaxy;
	int32_t bcmminx;
	int32_t bcmminy;
	int32_t bcmmaxx;
	int32_t bcmmaxy;
	int32_t ccmposx;
	int32_t ccmposz;
	int32_t ccmminx;
	int32_t ccmminy;
	int32_t ccmmaxx;
	int32_t ccmmaxy;
	int32_t u2cmminx;
	int32_t u2cmminy;
	int32_t u2cmmaxx;
	int32_t u2cmmaxy;

	switch(u->mode)
	{
#if 1
	case UMODE_GOBLJOB:
	case UMODE_GOCSTJOB:
	case UMODE_GOSHOP:
	case UMODE_GOREST:
	case UMODE_GODEMB:
		b = &g_building[u->target];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminy = b->tpos.y - bt->width.y/2;
		//btmaxx = btminx + bt->width.x - 1;
		//btmaxy = btminy + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminy * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
		{
			//char msg[1280];
			//sprintf(msg, "arr ui=%d u(%d,%d) b(%d,%d) %d<=%d && %d<=%d && %d<=%d && %d<=%d", ui, u->cmpos.x/TILE_SIZE, u->cmpos.y/TILE_SIZE, b->tpos.x, b->tpos.y, ucmminx, bcmmaxx, ucmminy, bcmmaxy, bcmminx, ucmmaxx, bcmminy, ucmmaxy);
			//RichText at("arr");
			//RichText at(msg);
			//AddNotif(&at);
			return true;
		}
		break;
#endif
	case UMODE_GOCDJOB:
	case UMODE_GODEMCD:
		ct = &g_cdtype[u->cdtype];
		ctile = GetCd(u->cdtype, u->target, u->target2, false);

		ccmposx = u->target*TILE_SIZE + ct->physoff.x;
		ccmposz = u->target2*TILE_SIZE + ct->physoff.y;

		ccmminx = ccmposx - TILE_SIZE/2;
		ccmminy = ccmposz - TILE_SIZE/2;
		ccmmaxx = ccmminx + TILE_SIZE;
		ccmmaxy = ccmminy + TILE_SIZE;

		if(ucmminx <= ccmmaxx && ucmminy <= ccmmaxy && ccmminx <= ucmmaxx && ccmminy <= ucmmaxy)
			return true;
		break;
#if 1
	case UMODE_GOSUP:
		b = &g_building[u->supplier];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminy = b->tpos.y - bt->width.y/2;
		//btmaxx = btminx + bt->width.x - 1;
		//btmaxy = btminy + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminy * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
			return true;
		break;

	case UMODE_GOREFUEL:
		b = &g_building[u->fuelstation];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminy = b->tpos.y - bt->width.y/2;
		btmaxx = btminx + bt->width.x - 1;
		btmaxy = btminy + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminy * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
			return true;
		break;

	case UMODE_GODRIVE:
		u2 = &g_unit[u->target];
		u2t = &g_utype[u2->type];

		u2cmminx = u2->cmpos.x - u2t->size.x/2;
		u2cmminy = u2->cmpos.y - u2t->size.x/2;
		u2cmmaxx = u2cmminx + u2t->size.x - 1;
		u2cmmaxy = u2cmminy + u2t->size.x - 1;
		
		ucmminx = ucmminx / PATHNODE_SIZE * PATHNODE_SIZE;
		ucmminy = ucmminy / PATHNODE_SIZE * PATHNODE_SIZE;
		ucmmaxx = ucmmaxx / PATHNODE_SIZE * PATHNODE_SIZE;
		ucmmaxy = ucmmaxy / PATHNODE_SIZE * PATHNODE_SIZE;
		
		u2cmminx = u2cmminx / PATHNODE_SIZE * PATHNODE_SIZE;
		u2cmminy = u2cmminy / PATHNODE_SIZE * PATHNODE_SIZE;
		u2cmmaxx = u2cmmaxx / PATHNODE_SIZE * PATHNODE_SIZE;
		u2cmmaxy = u2cmmaxy / PATHNODE_SIZE * PATHNODE_SIZE;
		//TODO correct in all cases?
		//InfoMess("dr","d?");

		//AddNotif(&RichText("arrived tr----------"));

		if(ucmminx <= u2cmmaxx && ucmminy <= u2cmmaxy && u2cmminx <= ucmmaxx && u2cmminy <= ucmmaxy)
			return true;
		break;
#endif
	default:
		break;
	};

#else

	const int32_t unminx = ucmminx/PATHNODE_SIZE;
	const int32_t unminz = ucmminy/PATHNODE_SIZE;
	const int32_t unmaxx = ucmmaxx/PATHNODE_SIZE;
	const int32_t unmaxz = ucmmaxy/PATHNODE_SIZE;

	for(int32_t z=unminz; z<=unmaxz; z++)
		for(int32_t x=unminx; x<=unmaxx; x++)
		{
			ColliderTile* cell = ColliderAt(x, y);

			if(cell->building >= 0)
			{
				switch(u->mode)
				{
					case UMODE_GOBLJOB:
					case UMODE_GOCSTJOB:
					case UMODE_GOSHOP:
					case UMODE_GOREST:
					case UMODE_GODEMB:
						if(u->target == cell->building)
							return true;
						break;
					case UMODE_GOSUP:
						if(u->supplier == cell->building)
							return true;
						break;
					case UMODE_GOREFUEL:
						if(u->fuelstation == cell->building)
							return true;
						break;
					default:
						break;
				}
			}


			for(int16_t uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				int16_t uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u2 = &g_unit[uindex];

				if(!u2->hidden())
				{
					switch(u->mode)
					{
						case UMODE_GODRIVE:
							if(u->target == uindex)
								return true;
							break;
						default:
							break;
					}
				}
			}
		}

#endif

	return false;
}

//arrived at transport vehicle
void ArAtTra(Unit* u)
{
	Unit* u2 = &g_unit[u->target];
	//u->freecollider();
	u->mode = UMODE_DRIVE;
	u2->driver = u - g_unit;
}

void OnArrived(Unit* u)
{
	u->pathdelay = 0;
	
	//if(u->mode != UMODE_NONE)
		//u->cyframes = 1;	//TODO check if it doesn't interfere with anything. needed to charge minimum of one man-hour.

	if(u->mode == UMODE_GODRIVE)
		u->cyframes = DRIVE_WORK_DELAY-1;
	else
		u->cyframes = WORK_DELAY-1;

	Player* utilpy = NULL;
	Building* b = NULL;
	CdTile* ctile = NULL;
	Unit* u2 = NULL;

	int32_t ui = u - g_unit;

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset goal track u 2", "reset goal track u 2");
	}
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
#endif

	switch(u->mode)
	{
	case UMODE_GOBLJOB:

	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f14 bljob");

		u->mode = UMODE_BLJOB;
		u->freecollider();
		b = &g_building[u->target];
		b->worker.push_back(u-g_unit);
		utilpy = &g_player[b->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOCSTJOB:
		u->mode = UMODE_CSTJOB;
		u->freecollider();
		b = &g_building[u->target];
		b->worker.push_back(u-g_unit);
		utilpy = &g_player[b->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOCDJOB:
		u->mode = UMODE_CDJOB;
		u->freecollider();
		ctile = GetCd(u->cdtype, u->target, u->target2, false);
		utilpy = &g_player[ctile->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOSHOP:
		u->mode = UMODE_SHOPPING;
		u->freecollider();
		b = &g_building[u->target];
		utilpy = &g_player[b->owner];
		utilpy->util += u->exputil / 10000;
		b->occupier.push_back(ui);
		ResetGoal(u);
		break;
	case UMODE_GOREST:
		u->mode = UMODE_RESTING;
		u->freecollider();
		b = &g_building[u->home];
		utilpy = &g_player[b->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GODRIVE:
		ArAtTra(u);
		u->freecollider();
		RemVis(u);
		u2 = &g_unit[u->target];
		utilpy = &g_player[u2->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GODEMB:
		if(u->driver >= 0)
			Disembark(&g_unit[u->driver]);
		u->driver = -1;
		u->mode = UMODE_ATDEMB;
		ResetGoal(u);
		break;
	case UMODE_GODEMCD:
		if(u->driver >= 0)
			Disembark(&g_unit[u->driver]);
		u->driver = -1;
		u->mode = UMODE_ATDEMCD;
		ResetGoal(u);
		break;
	case UMODE_GOSUP:
		if(u->driver >= 0)
			Disembark(&g_unit[u->driver]);
		u->driver = -1;
		u->mode = UMODE_ATSUP;
		ResetGoal(u);
		break;
	case UMODE_GOREFUEL:
		if(u->driver >= 0)
			Disembark(&g_unit[u->driver]);
		u->driver = -1;
		u->mode = UMODE_REFUELING;
		ResetGoal(u);
		break;
	default:
		break;
	};

	//if(type == TRUCK && UnitSelected(this))
	//	RedoLeftPanel();

	///RecheckSelection();
}
