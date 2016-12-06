











#include "../render/heightmap.h"
#include "../texture.h"
#include "../utils.h"
#include "../math/vec3f.h"
#include "../window.h"
#include "../math/camera.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../math/hmapmath.h"
#include "../render/foliage.h"
#include "../sim/unit.h"
#include "../render/foliage.h"
#include "savemap.h"
#include "../sim/building.h"
#include "../sim/deposit.h"
#include "../render/water.h"
#include "../sim/player.h"
#include "../sim/selection.h"
#include "../path/collidertile.h"
#include "../path/pathjob.h"
#include "../path/pathnode.h"
#include "../debug.h"
#include "../sim/simflow.h"
#include "../sim/transport.h"
#include "../render/particle.h"
#include "../render/transaction.h"
#include "../path/fillbodies.h"
#include "../path/tilepath.h"
#include "../render/drawsort.h"
#include "../sim/map.h"
#include "../algo/checksum.h"
#include "../net/net.h"
#include "savetrigger.h"
#include "savehint.h"
#include "../trigger/hint.h"
#include "../trigger/trigger.h"
#include "../render/fogofwar.h"
#include "../sim/border.h"
#include "../render/graph.h"
#include "../gui/layouts/chattext.h"

float ConvertHeight(unsigned char brightness)
{
#if 0
	// Apply exponential scale to height data.
	float y = (float)brightness*IN_Y_SCALE/255.0f - IN_Y_SCALE/2.0f;
	y = y / fabs(y) * pow(fabs(y), IN_Y_POWER) * IN_Y_AFTERPOW;

	if(y <= WATER_LEVEL)
		y -= TILE_SIZE;
	else if(y > WATER_LEVEL && y < WATER_LEVEL + TILE_SIZE/100)
		y += TILE_SIZE/100;

	return y;
#else
	return brightness / 25 * TILE_SIZE / 4;
#endif
}

void PlaceUnits()
{
	UType* ut = &g_utype[UNIT_LABOURER];
	PathJob pj;
	pj.airborne = ut->airborne;
	pj.landborne = ut->landborne;
	pj.seaborne = ut->seaborne;
	pj.roaded = ut->roaded;
	pj.targb = 0;
	pj.targu = 0;
	pj.thisu = 0;
	pj.utype = UNIT_LABOURER;

	for(int32_t li=0; li<6*PLAYERS; li++)
	{
		int32_t ntries = 0;

		for(; ntries < 100; ntries++)
		{
			Vec2i cmpos(rand()%g_mapsz.x*TILE_SIZE, rand()%g_mapsz.y*TILE_SIZE);
			Vec2i tpos = cmpos / TILE_SIZE;
			Vec2i npos = cmpos / PATHNODE_SIZE;

			pj.cmstartx = cmpos.x;
			pj.cmstarty = cmpos.y;

			if(Standable2(&pj, cmpos.x, cmpos.y))
			{
				PlaceUnit(UNIT_LABOURER, cmpos, -1, NULL);
				//Log("placed "<<li<<" at"<<tpos.x<<","<<tpos.y);
				break;
			}
			//else
			//Log("not placed at"<<tpos.x<<","<<tpos.y);
		}
	}
}


void FreeMap()
{
	for(uint8_t gi=0; gi<GRAPHS; ++gi)
	{
		g_graph[gi].cycles = 0;
		g_graph[gi].points.clear();
	}
	
	for(uint8_t gi=0; gi<PLAYERS; ++gi)
	{
		g_protecg[gi].cycles = 0;
		g_protecg[gi].points.clear();
	}

	FreeScript();
	FreeBls();	//bl's must be freed before units, because of Building::destroy because of how it clears ->worker and ->occupier lists
	FreeUnits();
	FreeFol();
	FreeDeposits();
	g_hmap.destroy();
	FreeGrid();
	Player* py = &g_player[g_localP];
	g_sel.clear();
	FreeParts();
	FreeTransx();
	//FreeGraphs();
	g_drawlist.clear();
	for(int32_t i=0; i<PLAYERS; i++)
		g_protecg[i].points.clear();
	
	g_lasthint.message = "";
	g_lasthint.graphic = "gui\\transp.png";
}

void SaveHmap(FILE *fp)
{
	fwrite(&g_mapsz.x, sizeof(unsigned char), 1, fp);
	fwrite(&g_mapsz.y, sizeof(unsigned char), 1, fp);

	fwrite(g_hmap.hpt, sizeof(unsigned char), (g_mapsz.x+1)*(g_mapsz.y+1), fp);
	//fwrite(g_hmap.own, sizeof(int32_t), g_mapsz.x*g_mapsz.y, fp);
}

void ReadHmap(FILE *fp, bool live, bool downafter)
{
	unsigned char widthx=0, widthy=0;

	fread(&widthx, sizeof(unsigned char), 1, fp);
	fread(&widthy, sizeof(unsigned char), 1, fp);

	//alloc hmap TO DO
	if(!downafter)
		g_hmap.alloc(widthx, widthy);

	for(int32_t y=0; y<=widthy; y++)
		for(int32_t x=0; x<=widthx; x++)
		{
			unsigned char h;
			fread(&h, sizeof(unsigned char), 1, fp);
			g_hmap.setheight(x, y, h);
		}
    
    g_hmap.lowereven();
	g_hmap.remesh();

	if(!downafter)
	{
		AllocGrid(widthx, widthy);

		//AllocPathGrid((widthx+1)*TILE_SIZE-1, (widthy+1)*TILE_SIZE-1);
		AllocPathGrid((g_mapsz.x)*TILE_SIZE, (g_mapsz.y)*TILE_SIZE);
		//CalcMapView();
	}

	if(downafter || live)
	{
		for(uint8_t ctype=0; ctype<CD_TYPES; ++ctype)
		{
			for(uint8_t tx=0; tx<g_mapsz.x; ++tx)
			{
				for(uint8_t ty=0; ty<g_mapsz.y; ++ty)
				{
					CdTile* ctile = GetCd(ctype, tx, ty, false);

					if(ctile->on && ctile->depth)
						UpDraw(ctile, ctype, tx, ty);
				}
			}
		}
	}
}

void SaveFol(FILE *fp)
{
	for(int32_t i=0; i<FOLIAGES; i++)
	{
#if 0
		bool on;
		unsigned char type;
		Vec3f pos;
		float yaw;
#endif

		Foliage *f = &g_foliage[i];

		fwrite(&f->on, sizeof(bool), 1, fp);

		if(!f->on)
			continue;

		fwrite(&f->type, sizeof(unsigned char), 1, fp);
		fwrite(&f->cmpos, sizeof(Vec2i), 1, fp);
		fwrite(&f->drawpos, sizeof(Vec2f), 1, fp);
		fwrite(&f->yaw, sizeof(float), 1, fp);
	}
}

void ReadFol(FILE *fp, bool live)
{
	Foliage df;

	for(int32_t i=0; i<FOLIAGES; i++)
	{
		Foliage *f = &g_foliage[i];

		if(live)
			f = &df;

		fread(&f->on, sizeof(bool), 1, fp);

		if(!f->on)
			continue;

		fread(&f->type, sizeof(unsigned char), 1, fp);
		fread(&f->cmpos, sizeof(Vec2i), 1, fp);
		fread(&f->drawpos, sizeof(Vec2f), 1, fp);
		fread(&f->yaw, sizeof(float), 1, fp);

#if 1
		g_drawlist.push_back(Depthable());
		Depthable* d = &*g_drawlist.rbegin();
		d->dtype = DEPTH_FOL;
		d->index = i;
		f->depth = d;
		UpDraw(f);
#endif
	}
}

void SaveDeps(FILE *fp)
{
	for(int32_t i=0; i<DEPOSITS; i++)
	{
#if 0
		bool on;
		bool occupied;
		int32_t restype;
		int32_t amount;
		Vec2i tpos;
		Vec3f drawpos;
#endif

		Deposit *d = &g_deposit[i];

		fwrite(&d->on, sizeof(bool), 1, fp);

		if(!d->on)
			continue;

		fwrite(&d->occupied, sizeof(bool), 1, fp);
		fwrite(&d->restype, sizeof(int32_t), 1, fp);
		fwrite(&d->amount, sizeof(int32_t), 1, fp);
		fwrite(&d->tpos, sizeof(Vec2i), 1, fp);
		fwrite(&d->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void ReadDeps(FILE *fp)
{
	for(int32_t i=0; i<DEPOSITS; i++)
	{
		Deposit *d = &g_deposit[i];

		fread(&d->on, sizeof(bool), 1, fp);

		if(!d->on)
			continue;

		fread(&d->occupied, sizeof(bool), 1, fp);
		fread(&d->restype, sizeof(int32_t), 1, fp);
		fread(&d->amount, sizeof(int32_t), 1, fp);
		fread(&d->tpos, sizeof(Vec2i), 1, fp);
		fread(&d->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void SaveUnits(FILE *fp)
{
	for(int32_t i=0; i<UNITS; i++)
	{
#if 0
		bool on;
		int32_t type;
		int32_t stateowner;
		int32_t corpowner;
		int32_t unitowner;

		/*
		The f (floating-point) position vectory is used for drawing.
		*/
		Vec3f fpos;

		/*
		The real position is stored in integers.
		*/
		Vec2i cmpos;
		Vec3f facing;
		Vec2f rotation;

		std::list<Vec2i> path;
		Vec2i goal;
#endif

		Unit *u = &g_unit[i];

		fwrite(&u->on, sizeof(bool), 1, fp);

		if(!u->on)
			continue;

		fwrite(&u->type, sizeof(int32_t), 1, fp);
#if 0
		fwrite(&u->stateowner, sizeof(int32_t), 1, fp);
		fwrite(&u->corpowner, sizeof(int32_t), 1, fp);
		fwrite(&u->unitowner, sizeof(int32_t), 1, fp);
#else
		fwrite(&u->owner, sizeof(int32_t), 1, fp);
#endif

		fwrite(&u->drawpos, sizeof(Vec3f), 1, fp);
		fwrite(&u->cmpos, sizeof(Vec2i), 1, fp);
		fwrite(&u->rotation, sizeof(Vec3f), 1, fp);

		int32_t pathsz = u->path.size();

		fwrite(&pathsz, sizeof(int32_t), 1, fp);

		for(std::list<Vec2i>::iterator pathiter = u->path.begin(); pathiter != u->path.end(); pathiter++)
			fwrite(&*pathiter, sizeof(Vec2i), 1, fp);

		fwrite(&u->goal, sizeof(Vec2i), 1, fp);

#if 0
		int32_t target;
		int32_t target2;
		bool targetu;
		bool underorder;
		int32_t fuelstation;
		int32_t belongings[RESOURCES];
		int32_t hp;
		bool passive;
		Vec2i prevpos;
		int32_t taskframe;
		bool pathblocked;
		int32_t jobframes;
		int32_t supplier;
		int32_t reqamt;
		int32_t targtype;
		int32_t home;
		int32_t car;
		//std::vector<TransportJob> bids;

		float frame[2];

		Vec2i subgoal;
#endif

		fwrite(&u->target, sizeof(int32_t), 1, fp);
		fwrite(&u->target2, sizeof(int32_t), 1, fp);
		fwrite(&u->targetu, sizeof(bool), 1, fp);
		fwrite(&u->underorder, sizeof(bool), 1, fp);
		fwrite(&u->fuelstation, sizeof(int32_t), 1, fp);
		fwrite(u->belongings, sizeof(int32_t), RESOURCES, fp);
		fwrite(&u->hp, sizeof(int32_t), 1, fp);
		fwrite(&u->passive, sizeof(bool), 1, fp);
		fwrite(&u->prevpos, sizeof(Vec2i), 1, fp);
		fwrite(&u->taskframe, sizeof(int32_t), 1, fp);
		fwrite(&u->pathblocked, sizeof(bool), 1, fp);
		fwrite(&u->jobframes, sizeof(int32_t), 1, fp);
		fwrite(&u->supplier, sizeof(int32_t), 1, fp);
		fwrite(&u->reqamt, sizeof(int32_t), 1, fp);
		fwrite(&u->targtype, sizeof(int32_t), 1, fp);
		fwrite(&u->home, sizeof(int32_t), 1, fp);
		fwrite(&u->car, sizeof(int32_t), 1, fp);
		fwrite(&u->frame, sizeof(float), 2, fp);
		fwrite(&u->subgoal, sizeof(Vec2i), 1, fp);

#if 0
	unsigned char mode;
	int32_t pathdelay;
	uint64_t lastpath;

	bool threadwait;

	// used for debugging - don't save to file
	bool collided;

	unsigned char cdtype;	//conduit type for mode (going to construction)
	int32_t driver;
	//int16_t framesleft;
	int32_t cyframes;	//cycle frames (unit cycle of consumption and work)
	int32_t opwage;	//transport driver wage
	//std::list<TransportJob> bids;	//for trucks
#endif

		fwrite(&u->mode, sizeof(unsigned char), 1, fp);
		fwrite(&u->pathdelay, sizeof(int32_t), 1, fp);
		fwrite(&u->lastpath, sizeof(uint64_t), 1, fp);
		fwrite(&u->threadwait, sizeof(bool), 1, fp);
		fwrite(&u->collided, sizeof(bool), 1, fp);
		fwrite(&u->cdtype, sizeof(signed char), 1, fp);
		fwrite(&u->driver, sizeof(int32_t), 1, fp);
		fwrite(&u->cyframes, sizeof(int32_t), 1, fp);
		fwrite(&u->opwage, sizeof(int32_t), 1, fp);

#if 0
	int32_t cargoamt;
	int32_t cargotype;
	int32_t cargoreq;	//req amt
#endif

		fwrite(&u->cargoamt, sizeof(int32_t), 1, fp);
		fwrite(&u->cargotype, sizeof(int32_t), 1, fp);
		fwrite(&u->cargoreq, sizeof(int32_t), 1, fp);

		int32_t ntpath = u->tpath.size();
		fwrite(&ntpath, sizeof(int32_t), 1, fp);
		//u->tpath.clear();
		for(std::list<Vec2s>::iterator ti=u->tpath.begin(); ti!=u->tpath.end(); ti++)
			fwrite(&*ti, sizeof(Vec2s), 1, fp);

		fwrite(&u->exputil, sizeof(int32_t), 1, fp);

		fwrite(&u->forsale, sizeof(bool), 1, fp);
		fwrite(&u->price, sizeof(int32_t), 1, fp);
		
		fwrite(&u->incomerate, sizeof(int32_t), 1, fp);

		int32_t nhists = u->cyclehist.size();
		
		fwrite(&nhists, sizeof(int32_t), 1, fp);
		
		for(std::list<TrCycleHist>::iterator ch=u->cyclehist.begin(); ch!=u->cyclehist.end(); ch++)
		{
			fwrite(&*ch, sizeof(TrCycleHist), 1, fp);
		}

		fwrite(&u->mazeavoid, sizeof(bool), 1, fp);
		fwrite(&u->turn, sizeof(char), 1, fp);
	}
}

void ReadUnits(FILE *fp, bool live)
{
	Unit du;

	for(int32_t i=0; i<UNITS; i++)
	{
		Unit *u = &g_unit[i];

		if(live)
			u = &du;

		fread(&u->on, sizeof(bool), 1, fp);

		if(!u->on)
			continue;

		fread(&u->type, sizeof(int32_t), 1, fp);

#ifdef TRANSPORT_DEBUG
		if(u->type == UNIT_LABOURER || i==10 || i == 11)
		//if(i!=0)
			u->on = false;
#endif

#if 0
		fread(&u->stateowner, sizeof(int32_t), 1, fp);
		fread(&u->corpowner, sizeof(int32_t), 1, fp);
		fread(&u->unitowner, sizeof(int32_t), 1, fp);
#else
		fread(&u->owner, sizeof(int32_t), 1, fp);
#endif

		fread(&u->drawpos, sizeof(Vec3f), 1, fp);
		fread(&u->cmpos, sizeof(Vec2i), 1, fp);
		fread(&u->rotation, sizeof(Vec3f), 1, fp);

#ifdef RANDOM8DEBUG
		//u->drawpos = Vec3f(u->cmpos.x, g_hmap.accheight(u->cmpos.x, u->cmpos.y), u->cmpos.y);
#endif

		int32_t pathsz = 0;

		fread(&pathsz, sizeof(int32_t), 1, fp);
		u->path.clear();

		for(int32_t pathindex=0; pathindex<pathsz; pathindex++)
		{
			Vec2i waypoint;
			fread(&waypoint, sizeof(Vec2i), 1, fp);
			u->path.push_back(waypoint);
		}

		fread(&u->goal, sizeof(Vec2i), 1, fp);

		fread(&u->target, sizeof(int32_t), 1, fp);
		fread(&u->target2, sizeof(int32_t), 1, fp);
		fread(&u->targetu, sizeof(bool), 1, fp);
		fread(&u->underorder, sizeof(bool), 1, fp);
		fread(&u->fuelstation, sizeof(int32_t), 1, fp);
		fread(u->belongings, sizeof(int32_t), RESOURCES, fp);
		fread(&u->hp, sizeof(int32_t), 1, fp);
		fread(&u->passive, sizeof(bool), 1, fp);
		fread(&u->prevpos, sizeof(Vec2i), 1, fp);
		fread(&u->taskframe, sizeof(int32_t), 1, fp);
		fread(&u->pathblocked, sizeof(bool), 1, fp);
		fread(&u->jobframes, sizeof(int32_t), 1, fp);
		fread(&u->supplier, sizeof(int32_t), 1, fp);
		fread(&u->reqamt, sizeof(int32_t), 1, fp);
		fread(&u->targtype, sizeof(int32_t), 1, fp);
		fread(&u->home, sizeof(int32_t), 1, fp);
		fread(&u->car, sizeof(int32_t), 1, fp);
		fread(&u->frame, sizeof(float), 2, fp);
		fread(&u->subgoal, sizeof(Vec2i), 1, fp);

		fread(&u->mode, sizeof(unsigned char), 1, fp);
		fread(&u->pathdelay, sizeof(int32_t), 1, fp);
		fread(&u->lastpath, sizeof(uint64_t), 1, fp);
		fread(&u->threadwait, sizeof(bool), 1, fp);
		fread(&u->collided, sizeof(bool), 1, fp);
		fread(&u->cdtype, sizeof(signed char), 1, fp);
		fread(&u->driver, sizeof(int32_t), 1, fp);
		fread(&u->cyframes, sizeof(int32_t), 1, fp);
		fread(&u->opwage, sizeof(int32_t), 1, fp);

		fread(&u->cargoamt, sizeof(int32_t), 1, fp);
		fread(&u->cargotype, sizeof(int32_t), 1, fp);
		fread(&u->cargoreq, sizeof(int32_t), 1, fp);

#if 1
		int32_t ntpath = 0;
		fread(&ntpath, sizeof(int32_t), 1, fp);
		u->tpath.clear();
		for(int32_t ti=0; ti<ntpath; ti++)
		{
			Vec2s tpos;
			fread(&tpos, sizeof(Vec2s), 1, fp);
			u->tpath.push_back(tpos);
		}
#endif
		
		fread(&u->exputil, sizeof(int32_t), 1, fp);

		fread(&u->forsale, sizeof(bool), 1, fp);
		fread(&u->price, sizeof(int32_t), 1, fp);

		fread(&u->incomerate, sizeof(int32_t), 1, fp);
		
		int32_t nhists = 0;
		
		fread(&nhists, sizeof(int32_t), 1, fp);
		
		for(int32_t hi=0; hi<nhists; ++hi)
		{
			TrCycleHist ch;
			fread(&ch, sizeof(TrCycleHist), 1, fp);
			u->cyclehist.push_back(ch);
		}
		
		fread(&u->mazeavoid, sizeof(bool), 1, fp);
		fread(&u->turn, sizeof(char), 1, fp);

		g_drawlist.push_back(Depthable());
		Depthable* d = &*g_drawlist.rbegin();
		u->depth = d;
		d->dtype = DEPTH_U;
		d->index = i;
		UpDraw(u);

		//u->fillcollider();
	}

	du.home = -1;
	du.mode = UMODE_NONE;
	du.driver = -1;
}

void SaveCapSup(CapSup* cs, FILE* fp)
{
#if 0
	unsigned char rtype;
	int32_t amt;
	int32_t src;
	int32_t dst;
#endif

	fwrite(&cs->rtype, sizeof(unsigned char), 1, fp);
	fwrite(&cs->amt, sizeof(int32_t), 1, fp);
	fwrite(&cs->src, sizeof(int32_t), 1, fp);
	fwrite(&cs->dst, sizeof(int32_t), 1, fp);
}

void ReadCapSup(CapSup* cs, FILE* fp)
{
	fread(&cs->rtype, sizeof(unsigned char), 1, fp);
	fread(&cs->amt, sizeof(int32_t), 1, fp);
	fread(&cs->src, sizeof(int32_t), 1, fp);
	fread(&cs->dst, sizeof(int32_t), 1, fp);
}

void SaveManufJob(ManufJob* mj, FILE* fp)
{
#if 0
	int32_t utype;
	int32_t owner;
#endif

	fwrite(&mj->utype, sizeof(int32_t), 1, fp);
	fwrite(&mj->owner, sizeof(int32_t), 1, fp);
}

void ReadManufJob(ManufJob* mj, FILE* fp)
{
	fread(&mj->utype, sizeof(int32_t), 1, fp);
	fread(&mj->owner, sizeof(int32_t), 1, fp);
}

void SaveBls(FILE *fp)
{
	for(int32_t i=0; i<BUILDINGS; i++)
	{
#if 0
	bool on;
	int32_t type;
	int32_t owner;

	Vec2i tpos;	//position in tiles
	Vec3f drawpos;	//drawing position in centimeters

	bool finished;

	int16_t pownetw;
	int16_t crpipenetw;
	std::list<int16_t> roadnetw;
#endif

		Building *b = &g_building[i];

		fwrite(&b->on, sizeof(bool), 1, fp);

		if(!b->on)
			continue;

		fwrite(&b->type, sizeof(int32_t), 1, fp);
#if 0
		fwrite(&b->stateowner, sizeof(int32_t), 1, fp);
		fwrite(&b->corpowner, sizeof(int32_t), 1, fp);
		fwrite(&b->unitowner, sizeof(int32_t), 1, fp);
#else
		fwrite(&b->owner, sizeof(int32_t), 1, fp);
#endif

		fwrite(&b->tpos, sizeof(Vec2i), 1, fp);
		fwrite(&b->drawpos, sizeof(Vec3f), 1, fp);

		fwrite(&b->finished, sizeof(bool), 1, fp);

#if 1
		fwrite(&b->pownetw, sizeof(int16_t), 1, fp);
		fwrite(&b->crpipenetw, sizeof(int16_t), 1, fp);
		int32_t nroadnetw = b->roadnetw.size();
		fwrite(&nroadnetw, sizeof(int16_t), 1, fp);
		for(std::list<int16_t>::iterator rnetit = b->roadnetw.begin(); rnetit != b->roadnetw.end(); rnetit++)
			fwrite(&*rnetit, sizeof(int16_t), 1, fp);
#else
		fwrite(b->netw, sizeof(int16_t), CD_TYPES, fp);
		for(unsigned char ci=0; ci<CD_TYPES; ++ci)
		{
			int32_t nl = b->netwlist[ci].size();
			fwrite(&nl, sizeof(int32_t), 1, fp);
			for(std::list<Widget*>::iterator nit=b->netwlist[ci].begin(); nit!=b->netwlist[ci].end(); nit++)
				fwrite(&*nit, sizeof(int16_t), 1, fp);
		}
#endif

#if 0
	int32_t stocked[RESOURCES];
	int32_t inuse[RESOURCES];

	EmitterCounter emitterco[MAX_B_EMITTERS];

	VertexArray drawva;

	int32_t conmat[RESOURCES];
	bool inoperation;

	int32_t price[RESOURCES];	//price of produced goods
	int32_t propprice;	//price of this property

	std::list<int32_t> occupier;
	std::list<int32_t> worker;
	int32_t conwage;
	int32_t opwage;
	int32_t cydelay;	//the frame delay between production cycles, when production target is renewed
	int16_t prodlevel;	//production target level of max RATIO_DENOM
	int16_t cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	uint32_t lastcy;	//last simframe of last production cycle
	std::list<CapSup> capsup;	//capacity suppliers

	int32_t manufprc[UNIT_TYPES];
	std::list<ManufJob> manufjob;
	int16_t transporter[RESOURCES];
#endif

		fwrite(b->stocked, sizeof(int32_t), RESOURCES, fp);
		fwrite(b->inuse, sizeof(int32_t), RESOURCES, fp);

		fwrite(b->conmat, sizeof(int32_t), RESOURCES, fp);
		fwrite(&b->inoperation, sizeof(bool), 1, fp);
		fwrite(b->price, sizeof(int32_t), RESOURCES, fp);
		fwrite(&b->propprice, sizeof(int32_t), 1, fp);

		fwrite(&b->forsale, sizeof(bool), 1, fp);
		fwrite(&b->demolish, sizeof(bool), 1, fp);

		unsigned char noc = b->occupier.size();
		fwrite(&noc, sizeof(unsigned char), 1, fp);
		for(std::list<int32_t>::iterator ocit=b->occupier.begin(); ocit!=b->occupier.end(); ocit++)
			fwrite(&*ocit, sizeof(int32_t), 1, fp);

		unsigned char nwk = b->worker.size();
		fwrite(&nwk, sizeof(unsigned char), 1, fp);
		for(std::list<int32_t>::iterator wkit=b->worker.begin(); wkit!=b->worker.end(); wkit++)
			fwrite(&*wkit, sizeof(int32_t), 1, fp);

		fwrite(&b->conwage, sizeof(int32_t), 1, fp);
		fwrite(&b->opwage, sizeof(int32_t), 1, fp);
		fwrite(&b->cydelay, sizeof(int32_t), 1, fp);
		fwrite(&b->prodlevel, sizeof(int16_t), 1, fp);
		fwrite(&b->cymet, sizeof(int16_t), 1, fp);
		fwrite(&b->lastcy, sizeof(uint64_t), 1, fp);

		int32_t ncs = b->capsup.size();
		fwrite(&ncs, sizeof(int32_t), 1, fp);
		for(std::list<CapSup>::iterator ncit=b->capsup.begin(); ncit!=b->capsup.end(); ncit++)
			SaveCapSup(&*ncit, fp);

		fwrite(b->manufprc, sizeof(int32_t), UNIT_TYPES, fp);

		int32_t nmj = b->manufjob.size();
		fwrite(&nmj, sizeof(int32_t), 1, fp);
		for(std::list<ManufJob>::iterator mjit=b->manufjob.begin(); mjit!=b->manufjob.end(); mjit++)
			SaveManufJob(&*mjit, fp);

		fwrite(b->transporter, sizeof(int16_t), RESOURCES, fp);

		fwrite(b->varcost, sizeof(int32_t), RESOURCES, fp);
		fwrite(&b->fixcost, sizeof(int32_t), 1, fp);
		
		fwrite(&b->demolition, sizeof(bool), 1, fp);

#if 0
		int32_t hp;
#endif

		fwrite(&b->hp, sizeof(int32_t), 1, fp);
		
		int32_t nhists = b->cyclehist.size();
		
		fwrite(&nhists, sizeof(int32_t), 1, fp);
		
		for(std::list<CycleHist>::iterator ch=b->cyclehist.begin(); ch!=b->cyclehist.end(); ch++)
		{
			fwrite(&*ch, sizeof(CycleHist), 1, fp);
		}
	}
}

void ReadBls(FILE *fp, bool live)
{
	Building db;

	for(int32_t i=0; i<BUILDINGS; i++)
	{

		//Log("\t read bl"<<i);
		//

		Building *b = &g_building[i];

		if(live)
			b = &db;

		fread(&b->on, sizeof(bool), 1, fp);

		if(!b->on)
			continue;

		//Log("\t\t on"<<i);
		//
		
		//no need to sort, read in order
		g_onbl.push_back((uint16_t)i);

		fread(&b->type, sizeof(int32_t), 1, fp);
#if 0
		fread(&b->stateowner, sizeof(int32_t), 1, fp);
		fread(&b->corpowner, sizeof(int32_t), 1, fp);
		fread(&b->unitowner, sizeof(int32_t), 1, fp);
#else
		fread(&b->owner, sizeof(int32_t), 1, fp);
#endif

		fread(&b->tpos, sizeof(Vec2i), 1, fp);
		fread(&b->drawpos, sizeof(Vec3f), 1, fp);

#if 0
		BlType* t = &g_bltype[b->type];
		Vec2i tmin;
		Vec2i tmax;
		tmin.x = b->tpos.x - t->width.x/2;
		tmin.y = b->tpos.y - t->width.y/2;
		tmax.x = tmin.x + t->width.x;
		tmax.y = tmin.y + t->width.y;
		b->drawpos = Vec3f(b->tpos.x*TILE_SIZE, Lowest(tmin.x, tmin.y, tmax.x, tmax.y), b->tpos.y*TILE_SIZE);
		if(t->foundation == FD_SEA)
			b->drawpos.y = WATER_LEVEL;
		if(t->width.x % 2 == 1)
			b->drawpos.x += TILE_SIZE/2;
		if(t->width.y % 2 == 1)
			b->drawpos.y += TILE_SIZE/2;
#endif

		fread(&b->finished, sizeof(bool), 1, fp);

		//Log("\t\t netws..."<<i);
		//

#if 1
		fread(&b->pownetw, sizeof(int16_t), 1, fp);
		fread(&b->crpipenetw, sizeof(int16_t), 1, fp);
		int16_t nroadnetw = -1;
		fread(&nroadnetw, sizeof(int16_t), 1, fp);
		for(int32_t rni=0; rni<nroadnetw; rni++)
		{
			int16_t roadnetw = -1;
			fread(&roadnetw, sizeof(int16_t), 1, fp);
			b->roadnetw.push_back(roadnetw);
		}
#else
		fread(b->netw, sizeof(int16_t), CD_TYPES, fp);
		for(unsigned char ci=0; ci<CD_TYPES; ++ci)
		{
			int32_t nl = 0;
			fread(&nl, sizeof(int32_t), 1, fp);
			for(int32_t nli=0; nli<nl; ++nli)
			{
				int16_t ni;
				fread(&ni, sizeof(int16_t), 1, fp);
				b->netwlist[ci].push_back(ni);
			}
		}
#endif

		fread(b->stocked, sizeof(int32_t), RESOURCES, fp);
		fread(b->inuse, sizeof(int32_t), RESOURCES, fp);

		fread(b->conmat, sizeof(int32_t), RESOURCES, fp);
		fread(&b->inoperation, sizeof(bool), 1, fp);
		fread(b->price, sizeof(int32_t), RESOURCES, fp);
		fread(&b->propprice, sizeof(int32_t), 1, fp);
		
		fread(&b->forsale, sizeof(bool), 1, fp);
		fread(&b->demolish, sizeof(bool), 1, fp);

		//Log("\t\t ocs..."<<i);
		//

		unsigned char noc = b->occupier.size();
		fread(&noc, sizeof(unsigned char), 1, fp);
		for(int32_t oci=0; oci<noc; oci++)
		{
			int32_t oc;
			fread(&oc, sizeof(int32_t), 1, fp);
			b->occupier.push_back(oc);
		}

		unsigned char nwk = b->worker.size();
		fread(&nwk, sizeof(unsigned char), 1, fp);
		for(int32_t wki=0; wki<nwk; wki++)
		{
			int32_t wk;
			fread(&wk, sizeof(int32_t), 1, fp);
			b->worker.push_back(wk);
		}

		fread(&b->conwage, sizeof(int32_t), 1, fp);
		fread(&b->opwage, sizeof(int32_t), 1, fp);
		fread(&b->cydelay, sizeof(int32_t), 1, fp);
		fread(&b->prodlevel, sizeof(int16_t), 1, fp);
		fread(&b->cymet, sizeof(int16_t), 1, fp);
		fread(&b->lastcy, sizeof(uint64_t), 1, fp);

		int32_t ncs = 0;
		fread(&ncs, sizeof(int32_t), 1, fp);
		for(int32_t ci=0; ci<ncs; ci++)
		{
			CapSup cs;
			ReadCapSup(&cs, fp);
			b->capsup.push_back(cs);
		}

		fread(b->manufprc, sizeof(int32_t), UNIT_TYPES, fp);

		int32_t nmj = 0;
		fread(&nmj, sizeof(int32_t), 1, fp);
		for(int32_t mji=0; mji<nmj; mji++)
		{
			ManufJob mj;
			ReadManufJob(&mj, fp);
			b->manufjob.push_back(mj);
		}

		fread(b->transporter, sizeof(int16_t), RESOURCES, fp);
		
		fread(b->varcost, sizeof(int32_t), RESOURCES, fp);
		fread(&b->fixcost, sizeof(int32_t), 1, fp);
		
		fread(&b->demolition, sizeof(bool), 1, fp);

		fread(&b->hp, sizeof(int32_t), 1, fp);

		int32_t nhists = 0;
		
		fread(&nhists, sizeof(int32_t), 1, fp);
		
		for(int32_t hi=0; hi<nhists; ++hi)
		{
			CycleHist ch;
			fread(&ch, sizeof(CycleHist), 1, fp);
			b->cyclehist.push_back(ch);
		}
		
		g_drawlist.push_back(Depthable());
		Depthable* d = &*g_drawlist.rbegin();
		b->depth = d;
		d->dtype = DEPTH_BL;
		d->index = i;
		UpDraw(b);

		//b->fillcollider();
	}

	db.capsup.clear();
	db.worker.clear();
	db.occupier.clear();
}

void SaveView(FILE *fp)
{
	Player* py = &g_player[g_localP];
	//fwrite(&g_cam, sizeof(Camera), 1, fp);

	Vec2i scroll = g_scroll + Vec2i(g_width,g_height)/2;

	fwrite(&scroll, sizeof(Vec2i), 1, fp);
	fwrite(&g_zoom, sizeof(float), 1, fp);
}

void ReadView(FILE *fp)
{
	Player* py = &g_player[g_localP];
	//fread(&g_cam, sizeof(Camera), 1, fp);
	fread(&g_scroll, sizeof(Vec2i), 1, fp);
	fread(&g_zoom, sizeof(float), 1, fp);

	g_scroll = g_scroll - Vec2i(g_width,g_height)/2;
}

void ReadGraph(FILE* fp, Graph* g)
{
	fread(&g->cycles, sizeof(uint32_t), 1, fp);
	fread(&g->startframe, sizeof(uint64_t), 1, fp);

	uint32_t c;
	fread(&c, sizeof(uint32_t), 1, fp);

	for(uint32_t ci=0; ci<c; ++ci)
	{
		float cf;
		fread(&cf, sizeof(float), 1, fp);
		g->points.push_back(cf);
	}
}

void SaveGraph(FILE* fp, Graph* g)
{
	fwrite(&g->cycles, sizeof(uint32_t), 1, fp);
	fwrite(&g->startframe, sizeof(uint64_t), 1, fp);

	uint32_t c = g->points.size();
	fwrite(&c, sizeof(uint32_t), 1, fp);

	for(std::list<float>::iterator cf=g->points.begin(); cf!=g->points.end(); ++cf)
		fwrite(&*cf, sizeof(float), 1, fp);
}

void ReadGraphs(FILE* fp)
{
	for(uint8_t gi=0; gi<GRAPHS; ++gi)
		ReadGraph(fp, &g_graph[gi]);
	for(uint8_t gi=0; gi<PLAYERS; ++gi)
		ReadGraph(fp, &g_protecg[gi]);
}

void SaveGraphs(FILE* fp)
{
	for(uint8_t gi=0; gi<GRAPHS; ++gi)
		SaveGraph(fp, &g_graph[gi]);
	for(uint8_t gi=0; gi<PLAYERS; ++gi)
		SaveGraph(fp, &g_protecg[gi]);
}

void SaveCd(FILE* fp)
{
	for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];

		for(int32_t i=0; i<g_mapsz.x*g_mapsz.y; i++)
		{
#if 0
	bool on;
	unsigned char conntype;
	bool finished;
	unsigned char owner;
	int32_t conmat[RESOURCES];
	int16_t netw;	//network
	VertexArray drawva;
	//bool inaccessible;
	int16_t transporter[RESOURCES];
	Vec3f drawpos;
	//int32_t maxcost[RESOURCES];
	int32_t conwage;
#endif

			CdTile* ctile = &ct->cdtiles[(int32_t)false][i];

			fwrite(&ctile->on, sizeof(bool), 1, fp);

			if(!ctile->on)
				continue;

			fwrite(&ctile->conntype, sizeof(char), 1, fp);
			fwrite(&ctile->finished, sizeof(bool), 1, fp);
			fwrite(&ctile->owner, sizeof(unsigned char), 1, fp);
			fwrite(ctile->conmat, sizeof(int32_t), RESOURCES, fp);
			fwrite(&ctile->netw, sizeof(int16_t), 1, fp);
			fwrite(ctile->transporter, sizeof(int16_t), RESOURCES, fp);
			fwrite(&ctile->drawpos, sizeof(Vec3f), 1, fp);
			fwrite(&ctile->conwage, sizeof(int32_t), 1, fp);
			fwrite(&ctile->selling, sizeof(bool), 1, fp);
		}
	}
}

void ReadCd(FILE* fp, bool live)
{
	CdTile dctile;

	for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];

		for(int32_t i=0; i<g_mapsz.x*g_mapsz.y; i++)
		{
			CdTile* ctile = &ct->cdtiles[(int32_t)false][i];

			if(live)
				ctile = &dctile;

			fread(&ctile->on, sizeof(bool), 1, fp);

			if(!ctile->on)
				continue;

			fread(&ctile->conntype, sizeof(char), 1, fp);
			fread(&ctile->finished, sizeof(bool), 1, fp);
			fread(&ctile->owner, sizeof(unsigned char), 1, fp);
			fread(ctile->conmat, sizeof(int32_t), RESOURCES, fp);
			fread(&ctile->netw, sizeof(int16_t), 1, fp);
			fread(ctile->transporter, sizeof(int16_t), RESOURCES, fp);
			fread(&ctile->drawpos, sizeof(Vec3f), 1, fp);
			fread(&ctile->conwage, sizeof(int32_t), 1, fp);
			fread(&ctile->selling, sizeof(bool), 1, fp);

			g_drawlist.push_back(Depthable());
			Depthable* d = &*g_drawlist.rbegin();
			d->dtype = DEPTH_CD;
			d->cdtype = ctype;
			d->plan = false;
			d->index = i;
			ctile->depth = d;
			UpDraw(ctile, d->cdtype, i % g_mapsz.x, i / g_mapsz.x);	//cse fix
		}
	}
}

void ReadPys(FILE* fp)
{
	for(int32_t i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];

#if 0
	bool on;
	bool ai;

	int32_t local[RESOURCES];	// used just for counting; cannot be used
	int32_t global[RESOURCES];
	int32_t resch[RESOURCES];	//resource changes/deltas
	int32_t truckwage;	//truck driver wage per second
	int32_t transpcost;	//transport cost per second

#endif

		fread(&py->on, sizeof(bool), 1, fp);

		if(!py->on)
			continue;

		fread(&py->ai, sizeof(bool), 1, fp);
		fread(py->local, sizeof(int32_t), RESOURCES, fp);
		fread(py->global, sizeof(int32_t), RESOURCES, fp);
		fread(py->resch, sizeof(int32_t), RESOURCES, fp);
		fread(&py->truckwage, sizeof(int32_t), 1, fp);
		fread(&py->transpcost, sizeof(int32_t), 1, fp);
		//fread(&py->gnp, sizeof(uint32_t), 1, fp);
		fread(&py->gnp, sizeof(uint64_t), 1, fp);
		//fread(&py->util, sizeof(uint32_t), 1, fp);
		fread(&py->util, sizeof(uint64_t), 1, fp);

#if 0
	float color[4];
	RichText name;
#endif

		fread(py->color, sizeof(float), 4, fp);

		int32_t slen = 0;
		fread(&slen, sizeof(int32_t), 1, fp);
		char* name = new char[slen];
		fread(name, sizeof(char), slen, fp);
		py->name = RichText(name);
		delete [] name;
		
		fread(&py->parentst, sizeof(int32_t), 1, fp);
		fread(&py->insttype, sizeof(signed char), 1, fp);
		fread(&py->instin, sizeof(signed char), 1, fp);
		fread(&py->protectionism, sizeof(bool), 1, fp);
		fread(&py->extariffratio, sizeof(int32_t), 1, fp);
		fread(&py->imtariffratio, sizeof(int32_t), 1, fp);

		fread(&py->lastthink, sizeof(uint64_t), 1, fp);
	}
}

void SavePys(FILE* fp)
{
	for(int32_t i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];

		fwrite(&py->on, sizeof(bool), 1, fp);

		if(!py->on)
			continue;

		fwrite(&py->ai, sizeof(bool), 1, fp);
		fwrite(py->local, sizeof(int32_t), RESOURCES, fp);
		fwrite(py->global, sizeof(int32_t), RESOURCES, fp);
		fwrite(py->resch, sizeof(int32_t), RESOURCES, fp);
		fwrite(&py->truckwage, sizeof(int32_t), 1, fp);
		fwrite(&py->transpcost, sizeof(int32_t), 1, fp);
		fwrite(&py->gnp, sizeof(uint64_t), 1, fp);
		fwrite(&py->util, sizeof(uint64_t), 1, fp);
		fwrite(py->color, sizeof(float), 4, fp);

		std::string name = py->name.rawstr();
		int32_t slen = name.length() + 1;
		fwrite(&slen, sizeof(int32_t), 1, fp);
		fwrite(name.c_str(), sizeof(char), slen, fp);
		
		fwrite(&py->parentst, sizeof(int32_t), 1, fp);
		fwrite(&py->insttype, sizeof(signed char), 1, fp);
		fwrite(&py->instin, sizeof(signed char), 1, fp);
		fwrite(&py->protectionism, sizeof(bool), 1, fp);
		fwrite(&py->extariffratio, sizeof(int32_t), 1, fp);
		fwrite(&py->imtariffratio, sizeof(int32_t), 1, fp);

		fwrite(&py->lastthink, sizeof(uint64_t), 1, fp);
	}
}

#if 0
void ReadGr(FILE* fp)
{
	for(int32_t i=0; i<GRAPHS; i++)
	{
		Graph* g = &g_graph[i];

#if 0
		std::list<float> points;
		uint32_t startframe;
		uint32_t cycles;
#endif

		fread(&g->startframe, sizeof(uint32_t), 1, fp);
		fread(&g->cycles, sizeof(uint32_t), 1, fp);

		uint32_t np = 0;
		fread(&np, sizeof(uint32_t), 1, fp);

		for(int32_t pi=0; pi<np; pi++)
		{
			float p;
			fread(&p, sizeof(float), 1, fp);
			g->points.push_back(p);
		}
	}
}

void SaveGr(FILE* fp)
{
	for(int32_t i=0; i<GRAPHS; i++)
	{
		Graph* g = &g_graph[i];

		fwrite(&g->startframe, sizeof(uint32_t), 1, fp);
		fwrite(&g->cycles, sizeof(uint32_t), 1, fp);

		uint32_t np = g->points.size();
		fwrite(&np, sizeof(uint32_t), 1, fp);

		for(std::list<Widget*>::iterator pit=g->points.begin(); pit!=g->points.end(); pit++)
		{
			fwrite(&*pit, sizeof(float), 1, fp);
		}
	}
}
#endif

void ReadJams(FILE* fp, bool live)
{
	//return;

	TileNode dtn;

	for(int16_t x=0; x<g_mapsz.x; x++)
		for(int16_t y=0; y<g_mapsz.y; y++)
		{
			int32_t tin = x + y * g_mapsz.x;
			TileNode* tn = &g_tilenode[tin];

			if(live)
				tn = &dtn;

			fread(&tn->jams, sizeof(unsigned char), 1, fp);
		}
}

void SaveJams(FILE* fp)
{
	for(int16_t x=0; x<g_mapsz.x; x++)
		for(int16_t y=0; y<g_mapsz.y; y++)
		{
			int32_t tin = x + y * g_mapsz.x;
			TileNode* tn = &g_tilenode[tin];
			fwrite(&tn->jams, sizeof(unsigned char), 1, fp);
		}
}

void ReadVis(FILE* fp, bool live)
{
	VisTile dvt;

	for(int16_t x=0; x<g_mapsz.x; x++)
		for(int16_t y=0; y<g_mapsz.y; y++)
		{
			VisTile* v = &g_vistile[ x + y * g_mapsz.x ];

			if(live)
				v = &dvt;

			fread(v->explored, sizeof(v->explored), 1, fp);
			fread(v->vis, sizeof(unsigned char), PLAYERS, fp);
		}
}

void SaveVis(FILE* fp)
{
	for(int16_t x=0; x<g_mapsz.x; x++)
		for(int16_t y=0; y<g_mapsz.y; y++)
		{
			VisTile* v = &g_vistile[ x + y * g_mapsz.x ];
			fwrite(v->explored, sizeof(v->explored), 1, fp);
			fwrite(v->vis, sizeof(unsigned char), PLAYERS, fp);
		}
}

void ReadBords(FILE* fp, bool live)
{
	if(live)
	{
		fseek(fp, sizeof(signed char) * g_mapsz.x * g_mapsz.y, SEEK_CUR);
		return;
	}

	fread(g_border, sizeof(signed char), g_mapsz.x * g_mapsz.y, fp);
}

void SaveBords(FILE* fp)
{
	fwrite(g_border, sizeof(signed char), g_mapsz.x * g_mapsz.y, fp);
}

//consistency check
void ConsistCh()
{
	for(int32_t bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			int32_t ui = b->transporter[ri];

			if(ui < 0)
				continue;

			Unit* u = &g_unit[ui];

			bool fail = false;
			uint32_t flag = 0;

			if(!u->on)
			{
				flag |= 1;
				fail = true;
			}

			if(u->type != UNIT_TRUCK)
			{
				flag |= 2;
				fail = true;
			}

			if(u->mode != UMODE_ATDEMB &&
				u->mode != UMODE_GODEMB &&
				u->mode != UMODE_GOSUP &&
				u->mode != UMODE_ATSUP)
			{
				flag |= 4;
				fail = true;
			}

			if(u->target != bi)
			{
				flag |= 8;
				fail = true;
			}

			if(!fail)
				continue;

			BlType* bt = &g_bltype[b->type];

			char msg[1280];
			sprintf(msg, "Consistency check failed #%u:\ntransporter to %s umode=%d ucargotype=%d ucargoamt=%d", flag, bt->name, (int32_t)u->mode, u->cargotype, u->cargoamt);
			InfoMess("csfail", msg);
		}
	}
}

bool SaveMap(const char* relative)
{
	char fullpath[WF_MAX_PATH+1];
	FullWritePath(relative, fullpath);

	FILE *fp = NULL;

	fp = fopen(fullpath, "wb");

	if(!fp)
		return false;

	char tag[] = MAP_TAG;
	int32_t version = MAP_VERSION;

	fwrite(&tag, sizeof(tag), 1, fp);
	fwrite(&version, sizeof(version), 1, fp);

	fwrite(&g_simframe, sizeof(g_simframe), 1, fp);

	SaveView(fp);
	SaveGraphs(fp);
	SavePys(fp);
	SaveHmap(fp);
	SaveDeps(fp);
	SaveFol(fp);
	SaveBls(fp);
	SaveUnits(fp);
	SaveCd(fp);
	//SaveGr(fp);
	SaveJams(fp);
	SaveVis(fp);
	SaveBords(fp);
	SaveTriggers(fp);
	SaveHint(&g_lasthint, fp);

	fclose(fp);

	return true;
}

//downafter: was the map downloaded and loaded here after the live sim data was loaded?
//live: basically downafter, but also works if map switched during play
bool LoadMap(const char* relative, bool live, bool downafter)
{
	if(!live)
		FreeMap();

	char fullpath[WF_MAX_PATH+1];
	FullWritePath(relative, fullpath);

	FILE *fp = NULL;

	fp = fopen(fullpath, "rb");

	if(!fp)
		return false;

	char realtag[] = MAP_TAG;
	int32_t version = MAP_VERSION;
	char tag[ sizeof(realtag) ];

	fread(&tag, sizeof(tag), 1, fp);

	if(memcmp(tag, realtag, sizeof(tag)) != 0)
	{
		ErrMess("Error", "Incorrect header tag in map file.");
		fclose(fp);
		return false;
	}

	fread(&version, sizeof(version), 1, fp);

	if(version != MAP_VERSION)
	{
		fclose(fp);
		char msg[128];
		sprintf(msg, "Map file version (%i) doesn't match %i.", version, MAP_VERSION);
		ErrMess("Error", msg);
		return false;
	}

	if(!live)
		fread(&g_simframe, sizeof(g_simframe), 1, fp);
	else
		fseek(fp, sizeof(uint64_t), SEEK_CUR);

	Log("read view");
	
	ReadView(fp);
	Log("read pys");

	ReadGraphs(fp);
	Log("read g");
	
	ReadPys(fp);
	Log("read hmap");
	
	ReadHmap(fp, live, downafter);
	Log("read deps");
	
	ReadDeps(fp);
	Log("read fol");
	
	ReadFol(fp, live);
	Log("read bls");
	
	ReadBls(fp, live);
	Log("read units");
	
	ReadUnits(fp, live);
	Log("read ctile");
	
	ReadCd(fp, live);
	Log("fill col grd");

	//ReadGr(fp);
	ReadJams(fp, live);
	ReadVis(fp, live);
	
	ReadBords(fp, live);

	FillColliderGrid();
	Log("loaded m");
	

	//ReadTriggers(fp);
	//ReadLastHint(fp);

	int32_t numt;
	fread(&numt, sizeof(int32_t), 1, fp);
    
	Trigger* prev = NULL;
	Trigger* t;
	std::vector<int32_t> triggerrefs;
    
	for(int32_t i=0; i<numt; i++)
	{
		t = new Trigger();
        
		if(i==0)
			g_scripthead = t;
        
		if(prev != NULL)
			prev->next = t;
        
		t->prev = prev;
        
		ReadTrigger(t, &triggerrefs, fp);
        
		prev = t;
	}
    
	int32_t refnum = 0;
	for(t=g_scripthead; t; t=t->next)
	{
		for(int32_t i=0; i<t->effects.size(); i++)
		{
			Effect* e = &t->effects[i];
			e->trigger = GetTrigger(triggerrefs[refnum]);
			refnum++;
		}
	}
    
	ReadHint(&g_lasthint, fp);

	fseek(fp, 0, SEEK_END);
	g_mapfsz = ftell(fp);

	fclose(fp);

	ConsistCh();

	g_mapcheck = CheckSum(fullpath);

	return true;
}
