











#include "labourer.h"
#include "../platform.h"
#include "../econ/demand.h"
#include "utype.h"
#include "player.h"
#include "building.h"
#include "../econ/utility.h"
#include "../trigger/console.h"
#include "simdef.h"
#include "job.h"
#include "umove.h"
#include "../gui/layouts/chattext.h"
#include "../render/transaction.h"
#include "../sound/sound.h"
#include "truck.h"
#include "../math/fixmath.h"
#include "../path/pathjob.h"
#include "../path/fillbodies.h"
#include "simflow.h"
#include "../render/fogofwar.h"
#include "../gui/widgets/spez/pygraphs.h"
#include "../path/tilepath.h"
#include "../language.h"

//not engine
#include "../gui/layouts/playgui.h"

int16_t g_labsnd[LAB_SOUNDS] = {-1,-1,-1};

bool NeedFood(Unit* u)
{
	//if(u->belongings[RES_RETFOOD] < STARTING_RETFOOD/2)
	//	return true;

	Building* b;
	int32_t bestbi;
	int32_t bestutil;
	
	BestFood(u, &bestbi, &bestutil);

	if(bestbi < 0)
		return false;

	b = &g_building[bestbi];

	if(ConsumProp(u, b->price[RES_RETFOOD]) > 0)
		return true;

	return false;
}

void BestFood(Unit* u, int32_t* bestbi, int32_t* bestutil)
{
	Vec2i bcmpos;
	*bestbi = -1;
	*bestutil = -1;

	//for(int32_t bi=0; bi<BUILDINGS; bi++)
	for(std::list<uint16_t>::iterator bit=g_onbl.begin(); bit!=g_onbl.end(); bit++)
	{
		uint16_t bi = *bit;

		Building* b = &g_building[bi];

		//if(!b->on)
		//	continue;

		if(!b->finished)
			continue;

#if 0
		//only allow firm or state belonging to same state
		if(b->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
			continue;
#endif

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_RETFOOD] <= 0)
			continue;

		Player* py = &g_player[b->owner];

		if(b->stocked[RES_RETFOOD] + py->global[RES_RETFOOD] <= 0)
			continue;

		if(b->price[RES_RETFOOD] > u->belongings[RES_DOLLARS])
			continue;

		//leave enough money for next cycle's rent payment
		if(u->home >= 0)
		{
			Building* hm = &g_building[u->home];
			if(u->belongings[RES_DOLLARS] <= hm->price[RES_HOUSING]*2)
				continue;
		}

		bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = MAG_VEC3F(pos - camera.Position()) * p->price[CONSUMERGOODS];
		const Vec2i cmoff = bcmpos - u->cmpos;
		int32_t cmdist = FUELHEUR(cmoff);
		int32_t thisutil = PhUtil(b->price[RES_RETFOOD], cmdist);

		if(*bestutil > thisutil && *bestbi >= 0)
			continue;

		if(b->occupier.size() >= MAX_SHOP * bt->width.x * bt->width.y)
			continue;

		*bestbi = bi;
		*bestutil = thisutil;
	}
}

bool FindFood(Unit* u)
{
	int32_t bestbi;
	int32_t bestutil;

	BestFood(u, &bestbi, &bestutil);

	ResetGoal(u);
	u->target = bestbi;
	u->mode = UMODE_GOSHOP;
	Building* b = &g_building[u->target];
	u->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	u->exputil = bestutil;

	//Pathfind();
	//float pathlen = pathlength();
	//Log("found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)");
	//

	return true;
}

bool NeedRest(Unit* u)
{
	if(u->belongings[RES_LABOUR] <= 0)
		return true;

	return false;
}

void GoHome(Unit* u)
{
	ResetGoal(u);
	u->target = u->home;
	u->mode = UMODE_GOREST;
	Building* b = &g_building[u->target];
	u->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	const Vec2i cmoff = u->goal - u->cmpos;
	int32_t cmdist = JOBHEUR( cmoff );
	int32_t price = b->price[RES_HOUSING];
	u->exputil = PhUtil(price, cmdist);

	//Pathfind();
	//float pathlen = pathlength();
	//Log("go home path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)");
	//
}

bool FindRest(Unit* u)
{
	Vec2i bcmpos;
	int32_t bestbi = -1;
	int32_t bestutil = -1;

	for(int32_t bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;
		
#if 0
		//only allow firm or state belonging to same state
		if(b->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
			continue;
#endif

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_HOUSING] <= 0)
			continue;

		//if(b->stocked[RES_HOUSING] <= 0)
		//	continue;

		//if(bt->output[RES_HOUSING] - b->inuse[RES_HOUSING] <= 0)
		//	continue;

		if(b->occupier.size() >= bt->output[RES_HOUSING])
			continue;

		if(b->price[RES_HOUSING] > u->belongings[RES_DOLLARS])
			continue;

		bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = MAG_VEC3F(pos - camera.Position()) * p->price[CONSUMERGOODS];
		const Vec2i cmoff = bcmpos - u->cmpos;
		int32_t cmdist = FUELHEUR(cmoff);
		int32_t thisutil = PhUtil(b->price[RES_HOUSING], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return false;

	ResetGoal(u);
	u->target = bestbi;
	u->mode = UMODE_GOREST;
	Building* b = &g_building[u->target];
	u->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	u->home = u->target;
	b->occupier.push_back(u-g_unit);
	//g_building[target].addoccupier(this);

	//Pathfind();
	//float pathlen = pathlength();
	//Log("found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)");
	//

	return true;
}

// check building construction job availability
bool CanCstBl(Unit* u)
{
	if(u->belongings[RES_LABOUR] <= 0)
		return false;

	Building* b = &g_building[u->target];
	BlType* bt = &g_bltype[b->type];
	Player* py = &g_player[b->owner];

	if(!b->on)
		return false;

	if(b->finished)
		return false;
	
#if 0
	//only allow firm or state belonging to same state
	if(b->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
		return false;
#endif

	if(b->conmat[RES_LABOUR] >= bt->conmat[RES_LABOUR])
		return false;
	
	if(b->worker.size() >= CS_WORKERS &&
		u->mode != UMODE_CSTJOB)
		return false;

	if(py->global[RES_DOLLARS] < b->conwage)
	{
		char reason[32];
		sprintf(reason, "%s construction", bt->name);
		Bankrupt(b->owner, reason);
		return false;
	}

	return true;
}

// go to construction job
void GoCstJob(Unit* u)
{
	if(!CanCstBl(u))
	{
		ResetMode(u);
		return;
	}
}

// do construction job
void DoCstJob(Unit* u)
{
	if(!CanCstBl(u))
	{
		ResetMode(u);
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	//if(u->framesleft > 0)
	if(u->cyframes > 0)
	{
		//u->framesleft --;
		return;
	}

	//last = GetTicks();
	//u->framesleft = WORK_DELAY;

	Building* b = &g_building[u->target];
	Player* py = &g_player[b->owner];
	CycleHist *lastch = &*b->cyclehist.rbegin();
	
	int32_t subt = imin(u->belongings[RES_LABOUR], WORK_RATE);

	b->conmat[RES_LABOUR] += subt;
	u->belongings[RES_LABOUR] -= subt;

	py->global[RES_DOLLARS] -= b->conwage * subt / WORK_RATE;
	u->belongings[RES_DOLLARS] += b->conwage * subt / WORK_RATE;
	u->incomerate += b->conwage * subt / WORK_RATE;
	//2015/11/16
	//lastch->cons[RES_DOLLARS] += b->conwage;
	py->gnp += b->conwage * subt / WORK_RATE;

#ifdef PYG
	PyGraph* pyg = &g_pygraphs[b->owner][PYGRAPH_CSEXP];
	*pyg->fig.rbegin() += b->conwage;
	pyg = &g_pygraphs[b->owner][PYGRAPH_TOTLABEXP];
	*pyg->fig.rbegin() += b->conwage;
#endif

#if 1
	//b->Emit(HAMMER);
#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_LABOUR];
			char numpart[128];
			sprintf(numpart, "%+d", subt);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -b->conwage * subt / WORK_RATE);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(Vec2i(b->tpos.x*TILE_SIZE, b->tpos.y*TILE_SIZE), &transx);

		if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
			g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
			PlayClip(g_labsnd[LABSND_WORK]);
	}
#endif

	if(b->checkconstruction())
		ResetMode(u);

	//char msg[128];
	//sprintf(msg, "construction %s", g_buildingType[b->type].name);
	//LogTransx(b->owner, -p->conwage, msg);
}

// check target building's job availability
bool CanBlJob(Unit* u)
{
	//CBuildingType* t = &g_buildingType[b->type];

	if(u->belongings[RES_LABOUR] <= 0)
		return false;

	Building* b = &g_building[u->target];

	if(!b->on)
		return false;

	if(!b->finished)
		return false;
	
#if 0
	//only allow firm or state belonging to same state
	if(b->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
		return false;
#endif

	//LastNum("checknorm1");

	if(b->metout())
		return false;

	if(b->excin(RES_LABOUR))
		return false;

	if(b->opwage <= 0)
		return false;

	BlType* bt = &g_bltype[b->type];

	if(b->worker.size() > iceil(bt->input[RES_LABOUR] * b->prodlevel, RATIO_DENOM * WORK_RATE) /* &&
		u->mode != UMODE_BLJOB */)
		return false;

	//LastNum("checknorm3");

	//if(b->occupier.size() > 0 && !b->hasworker(u - g_unit))
	//	return false;

	Player* py = &g_player[b->owner];

	if(py->global[RES_DOLLARS] < b->opwage)
	{
		char reason[64];
		sprintf(reason, "%s expenses", g_bltype[b->type].name);
		Bankrupt(b->owner, reason);
		return false;
	}

	return true;
}

// go to building job
void GoBlJob(Unit* u)
{
	//LastNum("gotonormjob1");
	if(!CanBlJob(u))
	{
		//LastNum("gotonormjob2!");
		ResetMode(u);
		//LastNum("gotonormjob3!");
		return;
	}
}

// do building job
void DoBlJob(Unit* u)
{
	//LastNum("gotonormjob1");
	if(!CanBlJob(u))
	{
		//LastNum("gotonormjob2!");
		ResetMode(u);
		//LastNum("gotonormjob3!");
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	if(u->cyframes > 0)
		return;

	//last = GetTicks();
	Building* b = &g_building[u->target];
	Player* py = &g_player[b->owner];
	CycleHist *lastch = &*b->cyclehist.rbegin();
	
	int32_t subt = imin(u->belongings[RES_LABOUR], WORK_RATE);

	b->stocked[RES_LABOUR] += subt;
	u->belongings[RES_LABOUR] -= subt;

	py->global[RES_DOLLARS] -= b->opwage * subt / WORK_RATE;
	u->belongings[RES_DOLLARS] += b->opwage * subt / WORK_RATE;
	u->incomerate += b->opwage * subt / WORK_RATE;
	lastch->cons[RES_DOLLARS] += b->opwage * subt / WORK_RATE;
	py->gnp += b->opwage * subt / WORK_RATE;
	
#ifdef PYG
	PyGraph* pyg = &g_pygraphs[b->owner][PYGRAPH_WORKLABEXP];
	*pyg->fig.rbegin() += b->opwage;
	pyg = &g_pygraphs[b->owner][PYGRAPH_TOTLABEXP];
	*pyg->fig.rbegin() += b->opwage;
#endif

	if(!b->tryprod())
	{
		//b->Emit(HAMMER);
#ifdef LOCAL_TRANSX
		if(b->owner == g_localP)
#endif
		{
			RichText tt;	//transaction text

			tt.m_part.push_back(RichPart(RICH_ICON, ICON_LABOUR));

			char ft[32];	//labour text
			sprintf(ft, "%+d ", subt);
			tt.m_part.push_back(RichPart(UStr(ft)));

			tt.m_part.push_back(RichPart(RICH_ICON, ICON_DOLLARS));

			char mt[32];	//money text
			sprintf(mt, "%+d ", b->opwage * subt / WORK_RATE);
			tt.m_part.push_back(RichPart(UStr(mt)));

			NewTransx(u->cmpos, &tt);
		}
	}
	
	if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
		g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
		PlayClip(g_labsnd[LABSND_WORK]);

	//char msg[128];
	//sprintf(msg, "job %s", g_buildingType[b->type].name);
	//LogTransx(b->owner, -p->wage[b->type], msg);
}

// check conduit construction job availability
bool CanCstCd(Unit* u)
{
	if(u->belongings[RES_LABOUR] <= 0)
		return false;

	CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);
	Player* py = &g_player[ctile->owner];

	if(!ctile->on)
		return false;

	if(ctile->finished)
		return false;
	
#if 0
	//only allow firm or state belonging to same state
	if(ctile->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
		return false;
#endif

	if(ctile->conwage <= 0)
		return false;

	CdType* ct = &g_cdtype[u->cdtype];

	if(ctile->conmat[RES_LABOUR] >= ct->conmat[RES_LABOUR])
		return false;
	
	if(CdWorkers(u->cdtype, u->target, u->target2) >= CS_WORKERS &&
		u->mode != UMODE_CDJOB)
		return false;

	if(py->global[RES_DOLLARS] < ctile->conwage)
	{
		Bankrupt(ctile->owner, "conduit construction");
		return false;
	}

	return true;
}

// go to conduit (construction) job
void GoCdJob(Unit* u)
{
	if(!CanCstCd(u))
	{
		ResetMode(u);
		return;
	}

	if(CheckIfArrived(u))
		OnArrived(u);
}

// do conduit (construction) job
void DoCdJob(Unit* u)
{
	if(!CanCstCd(u))
	{
		ResetMode(u);
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	if(u->cyframes > 0)
		return;

	//last = GetTicks();
	CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);
	Player* py = &g_player[ctile->owner];

#if 0
	if(ctile->owner < 0 || ctile->owner >= PLAYERS)
	{
		InfoMess("!","!");
	}
#endif

	int32_t subt = imin(u->belongings[RES_LABOUR], WORK_RATE);

	ctile->conmat[RES_LABOUR] += subt;
	u->belongings[RES_LABOUR] -= subt;

	py->global[RES_DOLLARS] -= ctile->conwage * subt / WORK_RATE;
	u->belongings[RES_DOLLARS] += ctile->conwage * subt / WORK_RATE;
	u->incomerate += ctile->conwage * subt / WORK_RATE;
	py->gnp += ctile->conwage * subt / WORK_RATE;
	
#ifdef PYG
	PyGraph* pyg = &g_pygraphs[ctile->owner][PYGRAPH_CSEXP];
	*pyg->fig.rbegin() += ctile->conwage;
	pyg = &g_pygraphs[ctile->owner][PYGRAPH_TOTLABEXP];
	*pyg->fig.rbegin() += ctile->conwage;
#endif

	//r->Emit(HAMMER);
#ifdef LOCAL_TRANSX
	if(r->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_LABOUR];
			char numpart[128];
			sprintf(numpart, "%+d", subt);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}
		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -ctile->conwage * subt / WORK_RATE);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(u->cmpos, &transx);

		//NewTransx(RoadPosition(target, target2), CURRENC, -p->conwage);
		//NewTransx(RoadPosition(target, target2), LABOUR, 1, CURRENC, -p->conwage);
		
		if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
			g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
			PlayClip(g_labsnd[LABSND_WORK]);
	}

	if(ctile->checkconstruction(u->cdtype))
		ResetMode(u);

	//LogTransx(r->owner, -p->conwage, "road job");
}

// check shop availability
bool CanShop(Unit* u)
{
	Building* b = &g_building[u->target];
	Player* p = &g_player[b->owner];

	if(!b->on)
		return false;

	if(!b->finished)
		return false;
	
#if 0
	//only allow firm or state belonging to same state
	if(b->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
		return false;
#endif

	if(b->stocked[RES_RETFOOD] + p->global[RES_RETFOOD] <= 0)
		return false;

	if(b->price[RES_RETFOOD] > u->belongings[RES_DOLLARS])
		return false;

	if(u->belongings[RES_RETFOOD] >= MUL_RETFOOD)
		return false;

	if(u->home >= 0)
	{
		Building* hm = &g_building[u->home];
		if(u->belongings[RES_DOLLARS] <= hm->price[RES_HOUSING]*2)
			return false;
	}

	int32_t shopq = ConsumProp(u, b->price[RES_RETFOOD]);

	if(shopq <= 0)
		return false;

	BlType* bt = &g_bltype[b->type];

	if(b->occupier.size() >= MAX_SHOP * bt->width.x * bt->width.y)
		return false;

	return true;
}

// go shop
void GoShop(Unit* u)
{
	if(!CanShop(u))
	{
		ResetMode(u);
		return;
	}
}

// check apartment availabillity
bool CanRest(Unit* u, bool* eviction)
{
	if(u->home < 0)
		return false;

	Building* b = &g_building[u->target];
	Player* p = &g_player[b->owner];
	*eviction = false;

	if(!b->on)
		return false;

	if(!b->finished)
		return false;
	
#if 0
	//only allow firm or state belonging to same state
	if(b->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
		return false;
#endif

	//if(b->stock[HOUSING] + p->global[HOUSING] <= 0.0f)
	//	return false;

	//if(bt->output[RES_HOUSING] - b->inuse[RES_HOUSING] <= 0)
	//	continue;

	BlType* bt = &g_bltype[b->type];

	if(b->occupier.size() > bt->output[RES_HOUSING])
		return false;

	if(b->price[RES_HOUSING] > u->belongings[RES_DOLLARS])
	{
		//char msg[128];
		//sprintf(msg, "eviction %f < %f", currency, p->price[HOUSING]);
		//LogTransx(b->owner, 0.0f, msg);
		*eviction = true;
		return false;
	}

	if(u->belongings[RES_LABOUR] >= STARTING_LABOUR)
		return false;

	return true;
}

//silent
void Evict(Building* b)
{
	while(b->occupier.size() > 0)
	{
		int32_t i = *b->occupier.begin();
		//Evict(&g_unit[i]);
		Unit* u = &g_unit[i];
		u->home = -1;
		b->occupier.erase( b->occupier.begin() );
		//ResetMode(u);
		if(u->mode == UMODE_RESTING ||
			u->mode == UMODE_GOREST)
			ResetMode(u);
	}
}

void Evict(Unit* u, bool silent)
{
	if(u->home < 0)
		return;

	if(!u->on)
		silent = true;	//bug?

	Building* b = &g_building[u->home];

	int32_t ui = u - g_unit;

	for(std::list<int32_t>::iterator uiter = b->occupier.begin(); uiter != b->occupier.end(); uiter++)
	{
		if(*uiter == ui)
		{
			b->occupier.erase( uiter );
			break;
		}
	}

	u->home = -1;

	if(!silent)
	{
		char msg[512];
		sprintf(msg, "%s ", Time().c_str());
		RichText em = RichText(msg) + STRTABLE[STR_EVICTION];
		//SubmitConsole(&em);
		AddNotif(&em);
	}
}

// go rest - perform checks that happen as the labourer is going to his home to rest
void GoRest(Unit* u)
{
	bool eviction;
	if(!CanRest(u, &eviction))
	{
		//Chat("!CanRest()");
		if(eviction)
			Evict(u, false);
		
		ResetMode(u);
		return;
	}
	/*
	if(goal - camera.Position() == Vec3f(0,0,0))
	{
	char msg[128];
	sprintf(msg, "faulty %d", UnitID(this));
	Chat(msg);
	ResetMode();
	}*/
}

// check if labourer has enough food to multiply
void CheckMul(Unit* u, int32_t foodpr)
{
	if(u->belongings[RES_RETFOOD] < MUL_RETFOOD)
		return;

	if(u->home < 0)
		return;

#if 0
	int32_t i = NewUnit();
	if(i < 0)
		return;

	CUnit* u = &g_unit[i];
	u->on = true;
	u->type = LABOURER;
	u->home = -1;

	CUnitType* t = &g_unitType[LABOURER];

	PlaceUAround(u, camera.Position().x-t->radius, camera.Position().z-t->radius, camera.Position().x+t->radius, camera.Position().z+t->radius, false);

	u->ResetLabourer();
	u->fuel = MULTIPLY_FUEL/2.0f;
	fuel -= MULTIPLY_FUEL/2.0f;
#endif

#if 1

	foodpr = imax(1, foodpr);

	int32_t saverent = 0;

	if(u->home >= 0)
	{
		Building* b = &g_building[u->home];
		saverent = b->price[RES_HOUSING] * 2;	//save
	}

	int32_t savefood = STARTING_RETFOOD * foodpr;
	int32_t divmoney = ( saverent + savefood ) / 2;	//min division money
	int32_t tospend = u->belongings[RES_DOLLARS] - saverent;
	int32_t maxbuy = tospend / foodpr;

	maxbuy = imax(1, maxbuy);

	int32_t portion = STARTING_RETFOOD * RATIO_DENOM / maxbuy;	//max division buys
	portion = imax(0, portion);
	portion = imin(RATIO_DENOM, portion);

	int32_t inherit = u->belongings[RES_DOLLARS] * portion / RATIO_DENOM;	//leave a portion equal among how many children possible
	inherit = imax(inherit, divmoney);	//don't have more children than can have divmoney
	inherit = imin(inherit, u->belongings[RES_DOLLARS] / 2);	//don't give more than half

	if(inherit < divmoney)
		return;

#endif

	Vec2i cmpos;

	if(!PlaceUAb(UNIT_LABOURER, u->cmpos, &cmpos))
		return;

	int32_t ui = -1;

	if(!PlaceUnit(UNIT_LABOURER, cmpos, u->owner, &ui))
		return;

	Unit* u2 = &g_unit[ui];
	StartBel(u2);

	//u->belongings[RES_RETFOOD] -= STARTING_RETFOOD;

#if 0
	u2->belongings[RES_RETFOOD] = u->belongings[RES_RETFOOD] / 2;
	u->belongings[RES_RETFOOD] -= u2->belongings[RES_RETFOOD];

	u2->belongings[RES_DOLLARS] = u->belongings[RES_DOLLARS] / 2;
	u->belongings[RES_DOLLARS] -= u2->belongings[RES_DOLLARS];
#else

	u2->belongings[RES_RETFOOD] = STARTING_RETFOOD;
	u->belongings[RES_RETFOOD] -= STARTING_RETFOOD;

	u2->belongings[RES_DOLLARS] = inherit;
	u->belongings[RES_DOLLARS] -= inherit;
#endif

	char msg[512];
	sprintf(msg, "%s %s (%s %d.)", 
		Time().c_str(), 
		STRTABLE[STR_GROWTH].rawstr().c_str(),
		STRTABLE[STR_POPULATION].rawstr().c_str(),
		CountU(UNIT_LABOURER));
	RichText gr;
	gr.m_part.push_back(UStr(msg));
	AddNotif(&gr);
	//SubmitConsole(&gr);
}

//react to changed saving req's on the spot
void CheckMul(Unit* u)
{
	if(u->belongings[RES_RETFOOD] < MUL_RETFOOD)
		return;

	Vec2i bcmpos;
	int32_t bestbi = -1;
	int32_t bestutil = -1;

	for(int32_t bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_RETFOOD] <= 0)
			continue;

		Player* py = &g_player[b->owner];

		if(b->stocked[RES_RETFOOD] + py->global[RES_RETFOOD] <= 0)
			continue;

		if(b->price[RES_RETFOOD] > u->belongings[RES_DOLLARS])
			continue;

		//leave enough money for next cycle's rent payment
		if(u->home >= 0)
		{
			Building* hm = &g_building[u->home];
			if(u->belongings[RES_DOLLARS] <= hm->price[RES_HOUSING]*2)
				continue;
		}

		bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = MAG_VEC3F(pos - camera.Position()) * p->price[CONSUMERGOODS];
		const Vec2i cmoff = bcmpos - u->cmpos;
		int32_t cmdist = FUELHEUR(cmoff);
		int32_t thisutil = PhUtil(b->price[RES_RETFOOD], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return;

#if 0
	ResetGoal(u);
	u->target = bestbi;
	u->mode = UMODE_GOSHOP;
	Building* b = &g_building[u->target];
	u->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
#endif

	Building* b = &g_building[bestbi];
	int32_t foodpr = b->price[RES_RETFOOD];
	CheckMul(u, foodpr);
}

int32_t ConsumProp(int32_t satiety, int32_t incomerate, int32_t bprice)
{
	bprice = imax(1, bprice);
	
	//TODO need limiting/cutting back on demand for elasticity?
	//int32_t extrashop = imax(0, incomerate / ( bprice * bprice ) );
	int32_t extrashop = 0;
	
	int32_t totalshop = extrashop - satiety;

	return totalshop;
}

//consumption propensity? TODO: better names
int32_t ConsumProp(Unit* u, int32_t bprice)
{
	int32_t satiety = u->belongings[RES_RETFOOD] - STARTING_RETFOOD;

	return ConsumProp(satiety,  u->incomerate, bprice);
}

// do shop
void DoShop(Unit* u)
{
	Building* b = &g_building[u->target];
	CheckMul(u, b->price[RES_RETFOOD]);

	if(!CanShop(u))
	{
		ResetMode(u);
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	// TO DO: make consume 50-15 per sec from py->global and then b->stocked

	if(u->cyframes > 0)
	//if(u->cyframes % 2 == 0)
		return;

	//last = GetTicks();

	Player* p = &g_player[b->owner];

#if 0
	if(p->global[RES_RETFOOD] > 0)
		p->global[RES_RETFOOD] -= 1;
	else
	{
		b->stocked[RES_RETFOOD] -= 1;
		p->local[RES_RETFOOD] -= 1;
	}

	u->belongings[RES_RETFOOD] += 1;
	p->global[RES_DOLLARS] += b->price[RES_RETFOOD];
	u->belongings[RES_DOLLARS] -= b->price[RES_RETFOOD];
	//b->recenth.consumed[CONSUMERGOODS] += 1.0f;
#endif
	
	CycleHist *lastch = &*b->cyclehist.rbegin();

	//add some so they don't come back for 1 more each next frame
	int32_t totalshop = ConsumProp(u, b->price[RES_RETFOOD]) + CYCLE_FRAMES / FOOD_CONSUM_DELAY_FRAMES * LABOURER_FOODCONSUM;

	//totalshop = imin(SHOP_RATE, totalshop);

	//divide trying shop rate by 2 each time until we can afford it and there's enough food available
	//for(int32_t trysub=SHOP_RATE; trysub>0; trysub>>=1)
	for(int32_t trysub=totalshop; trysub>0; trysub>>=1)
	{
		int32_t cost = b->price[RES_RETFOOD] * trysub;

		if(cost > u->belongings[RES_DOLLARS])
			continue;

		int32_t sub[RESOURCES];
		Zero(sub);
		sub[RES_RETFOOD] = trysub;

		if(!TrySub(sub, p->global, b->stocked, p->local, NULL, NULL))
			continue;

		u->belongings[RES_RETFOOD] += trysub;
		p->global[RES_DOLLARS] += b->price[RES_RETFOOD] * trysub;
		p->gnp += b->price[RES_RETFOOD] * trysub;
		u->belongings[RES_DOLLARS] -= b->price[RES_RETFOOD] * trysub;
		lastch->prod[RES_DOLLARS] += b->price[RES_RETFOOD] * trysub;	//"cons" is expenses and CONSUMPTION, edit: but "prod" is EARNINGS and production
		lastch->cons[RES_RETFOOD] += trysub;	//corpd fix xp
		
#if 1
	//b->Emit(SMILEY);
#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
	{
		RichText tt;	//transaction text

		tt.m_part.push_back(RichPart(RICH_ICON, ICON_DOLLARS));

		char mt[32];	//money text
		sprintf(mt, "%+d ", b->price[RES_RETFOOD]);
		tt.m_part.push_back(RichPart(UStr(mt)));

		tt.m_part.push_back(RichPart(RICH_ICON, ICON_RETFOOD));

		char ft[32];	//food text
		sprintf(ft, "%+d ", -trysub);
		tt.m_part.push_back(RichPart(UStr(ft)));

		NewTransx(u->cmpos, &tt);
		
		if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
			g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
			PlayClip(g_labsnd[LABSND_SHOP]);
	}
#endif

		break;
	}
}

// do rest
void DoRest(Unit* u)
{
	bool eviction;
	if(!CanRest(u, &eviction))
	{
		//Chat("!CheckApartmentAvailability()");
		if(eviction)
			Evict(u, false);
		
		ResetMode(u);
		return;
	}

	if(u->cyframes > 0)
		return;

	u->belongings[RES_LABOUR] += WORK_RATE;

	Building *b = &g_building[u->home];
	CycleHist *lastch = &*b->cyclehist.rbegin();
	//lastch->cons[RES_HOUSING] += 1;	//"cons" is expenses and CONSUMPTION

#if 1
	RichText rt(STRTABLE[STR_REST]);
	//SubmitConsole(&em);
	//AddNotif(&rt);
	NewTransx(u->cmpos, &rt);
	if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
		g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
		PlayClip(g_labsnd[LABSND_REST]);
#endif
}

// check transport vehicle availability
bool CanDrive(Unit* op)
{
	Unit* tr = &g_unit[op->target];

//#define HIERDEBUG

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive1");
	}
#endif

	if(!tr->on)
		return false;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive2");
	}
#endif

	if(tr->type != UNIT_TRUCK)
		return false;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive3");
	}
#endif

	if(tr->hp <= 0)
		return false;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive4");
	}
#endif

	if(tr->driver >= 0 && &g_unit[tr->driver] != op)
		return false;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive5");
	}
#endif

	if(tr->mode != UMODE_GOSUP &&
		tr->mode != UMODE_GODEMB &&
		tr->mode != UMODE_GODEMCD &&
		tr->mode != UMODE_GOREFUEL)
		return false;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive6");
	}
#endif

	if(op->belongings[RES_LABOUR] <= 0)
		return false;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive7");
	}
#endif

	Player* p = &g_player[tr->owner];

	//if(p->global[RES_DOLLARS] < tr->opwage)
	if(p->global[RES_DOLLARS] < p->truckwage)
	{
		Bankrupt(tr->owner, "truck expenses");
		return false;
	}

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive8");
	}
#endif

	Player* demp = NULL;
	int32_t dempi = -1;

	if(tr->targtype == TARG_BL)
	{
		Building* b = &g_building[tr->target];
		dempi = b->owner;
		demp = &g_player[b->owner];
	}
	else if(tr->targtype == TARG_CD)
	{
		CdTile* ctile = GetCd(tr->cdtype, tr->target, tr->target2, false);
		dempi = ctile->owner;
		demp = &g_player[ctile->owner];
	}

	if(demp && demp->global[RES_DOLLARS] < p->transpcost)
	{
		Bankrupt(dempi, "transport fees");
		return false;
	}

	if(Trapped(tr, op))
	{
		Jamify(tr);
		return false;
	}

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_unit == 82)
	{
		Log("the 13th unit: candrive9");
	}
#endif

	
//#undef HIERDEBUG

	return true;
}

// go to transport for drive job
void GoDrive(Unit* u)
{
	if(!CanDrive(u))
	{
		ResetMode(u);
		return;
	}

	if(CheckIfArrived(u))
		OnArrived(u);
}

//driver labourer to disembark driven transport vehicle
void Disembark(Unit* op)
{
#if 0
	char m[123];
	sprintf(m, "disembark-------- u%d m%d t%d d%d", (int32_t)(op-g_unit), (int32_t)op->mode, op->target, g_unit[op->target].driver);
	RichText rm(m);
	AddNotif(&rm);
#endif

	if(op->mode == UMODE_GODRIVE)
	{
		ResetMode(op);
		return;
	}

	if(op->mode != UMODE_DRIVE)
		return;

	Unit* tr = &g_unit[op->target];
	//camera.MoveTo( u->camera.Position() );
	Vec2i trpos = tr->cmpos;
	UType* t = &g_utype[tr->type];
	ResetMode(op, false);
	//must be a better way to do this - fillcollider is called already
	RemVis(op);
	//op->freecollider();
	Vec2i oppos;
	PlaceUAb(op->type, trpos, &oppos);
	op->cmpos = oppos;

	//if(op - g_unit == 182 && g_simframe > 118500)
		//Log("f7");

	op->fillcollider();
	AddVis(op);
	//TODO
	//op->drawpos = Vec3f(oppos.x, g_hmap.accheight(oppos.x, oppos.y), oppos.y);

	//corpc fix
	//make sure truck doesn't already have a different driver.
	if(tr->driver == op - g_unit)
		tr->driver = -1;
	/*u->driver = -1;

	if(u->mode != NONE)
	u->mode = AWAITINGDRIVER;*/
}

// do transport drive job
void DoDrive(Unit* op)
{
	/*
	int32_t uID = UnitID(this);

	if(uID == 2)
	{
	Log("u[2]dodrive");
	


	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), MAG_VEC3F(camera.Position() - camera.View()), MAG_VEC3F(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), vw.x, vw.y, vw.z, MAG_VEC3F(camera.Velocity()));
	Vec3f vel = g_unit[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", MAG_VEC3F(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velbeforedodrive=%f,%f,%f", vel.x, vel.y, vel.z);
	int32_t unID = 5;

	Log("u["<<unID<<"]: "<<msg);
	

	Chat(msg);
	}
	}*/

	if(!CanDrive(op))
	{
		/*
		if(uID == 2)
		{
		Log("u[2]DISEMBARK");
		
		}
		*/
		//Log("disembark";
		//
		//g_unit[op->target].driver = -1;
		Disembark(op);
		//2016/05/03 reset mode already called in disembark, might be causing the leftover blockages
		//ResetMode(op);
		return;
	}
	/*
	if(uID == 2)
	{
	Log("u[2]dodrive");
	


	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), MAG_VEC3F(camera.Position() - camera.View()), MAG_VEC3F(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), vw.x, vw.y, vw.z, MAG_VEC3F(camera.Velocity()));
	Vec3f vel = g_unit[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", MAG_VEC3F(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velaftercheckava=%f,%f,%f", vel.x, vel.y, vel.z);
	int32_t unID = 5;

	Log("u["<<unID<<"]: "<<msg);
	

	Chat(msg);
	}
	}*/

	if(g_simframe % DRIVE_WORK_DELAY != 0)
	//if(op->cyframes > 0)
		return;

	//if(op->cyframes > 0)
	//	return;

	//last = GetTicks();
	//op->framesleft = DRIVE_WORK_DELAY;

	Unit* tr = &g_unit[op->target];
	Player* py = &g_player[tr->owner];

	//op->belongings[RES_LABOUR] --;
	int32_t subt = imin(op->belongings[RES_LABOUR], WORK_RATE);
	op->belongings[RES_LABOUR] -= subt;

	//py->global[RES_DOLLARS] -= tr->opwage;
	//op->belongings[RES_DOLLARS] += tr->opwage;
	py->global[RES_DOLLARS] -= py->truckwage * subt / WORK_RATE;
	py->gnp += py->truckwage * subt / WORK_RATE;
	op->belongings[RES_DOLLARS] += py->truckwage * subt / WORK_RATE;
	op->incomerate += py->truckwage * subt / WORK_RATE;

	TrCycleHist* trch = &*tr->cyclehist.rbegin();
	trch->pay += py->truckwage * subt / WORK_RATE;
	
#ifdef PYG
	PyGraph* pyg = &g_pygraphs[tr->owner][PYGRAPH_DREXP];
	*pyg->fig.rbegin() += py->truckwage;
	pyg = &g_pygraphs[tr->owner][PYGRAPH_TOTLABEXP];
	*pyg->fig.rbegin() += py->truckwage;
#endif
	
	Player* demp = NULL;
	int32_t dempi = -1;

	if(tr->targtype == TARG_BL)
	{
		Building* b = &g_building[tr->target];
		if(b->cyclehist.size())
		{
			CycleHist* lastch = &*b->cyclehist.rbegin();
			lastch->cons[RES_DOLLARS] += py->truckwage;
		}
		
		b->fixcost += py->truckwage * subt / WORK_RATE;

		dempi = b->owner;
		demp = &g_player[b->owner];
	}
	else if(tr->targtype == TARG_CD)
	{
		CdTile* ctile = GetCd(tr->cdtype, tr->target, tr->target2, false);
		
		dempi = ctile->owner;
		demp = &g_player[ctile->owner];
	}

	//how could i forget 2015/11/16
	if(demp /* && 
		tr->owner != dempi */)
	{
		demp->global[RES_DOLLARS] -= py->transpcost;
		py->global[RES_DOLLARS] += py->transpcost;
		trch->earn += py->transpcost;
	}

	//TODO in RecStats and RecPyStats add a blank list item for starting cycle

	//LogTransx(truck->owner, -p->truckwage, "driver wage");

#ifdef LOCAL_TRANSX
	if(truck->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -py->truckwage * subt / WORK_RATE);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(tr->cmpos, &transx);
		
		if(g_mapview[0].x <= tr->cmpos.x && g_mapview[0].y <= tr->cmpos.y &&
			g_mapview[1].x >= tr->cmpos.x && g_mapview[1].y >= tr->cmpos.y)
			PlayClip(g_trsnd[TRSND_WORK]);
	}

	/*
	if(uID == 2)
	{
	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), MAG_VEC3F(camera.Position() - camera.View()), MAG_VEC3F(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), vw.x, vw.y, vw.z, MAG_VEC3F(camera.Velocity()));
	Vec3f vel = g_unit[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", MAG_VEC3F(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velafterdodrive=%f,%f,%f", vel.x, vel.y, vel.z);
	int32_t unID = 5;

	Log("u["<<unID<<"]: "<<msg);
	

	Chat(msg);
	}
	}*/
}

//inherit the money of dying unit "u"
void Inherit(Unit* u)
{
	Unit* bestu = NULL;
	int32_t bestutil = -1;

	for(int32_t i=0; i<UNITS; i++)
	{
		Unit* u2 = &g_unit[i];

		if(!u2->on)
			continue;

		if(u2->type != UNIT_LABOURER)
			continue;

		if(u2 == u)
			continue;

		const Vec2i cmoff = u2->cmpos - u->cmpos;
		int32_t dist = FUELHEUR(cmoff);
		int32_t util = PhUtil(u2->belongings[RES_DOLLARS], dist);

		if(bestutil >= 0 && util < bestutil)
			continue;

		bestutil = util;
		bestu = u2;
	}

	if(!bestu)
		return;

	bestu->belongings[RES_DOLLARS] += u->belongings[RES_DOLLARS];
	u->belongings[RES_DOLLARS] = 0;
}

void UpdLab(Unit* u)
{
	StartTimer(TIMER_UPDLAB);

	//return;	//do nothing for now

	u->jobframes++;
	u->cyframes--;

	if(g_simframe % CYCLE_FRAMES == 0)
	{
		u->incomerate = 0;
	}

	if(u->cyframes < 0)
		u->cyframes = WORK_DELAY-1;

	//if(u->cyframes == 0)
	if(g_simframe % FOOD_CONSUM_DELAY_FRAMES == 0)
	{
		u->belongings[RES_RETFOOD] -= LABOURER_FOODCONSUM;
		CheckMul(u);
	}

	if(u->belongings[RES_RETFOOD] <= 0)
	{
		char msg[512];
		int32_t cnt = CountU(UNIT_LABOURER)-1;
		sprintf(msg, "%s %s (%s %d.)", 
			Time().c_str(), 
			STRTABLE[STR_STARVATION].rawstr().c_str(),
			STRTABLE[STR_POPULATION].rawstr().c_str(),
			cnt);
		RichText sr;
		sr.m_part.push_back(UStr(msg));
		//SubmitConsole(&sr);
		AddNotif(&sr);
		Inherit(u);
		u->destroy();

		if(cnt <= 0)
			Click_Pause();

		return;
	}

#if 0
	//if(g_simframe % U_AI_DELAY != 0)
	if(u->cyframes > 0)
		return;
#endif
	
	int32_t ui = u - g_unit;

	//if(u->cyframes == 0)
	//if((ui + g_simframe) % 32 == 0)
	switch(u->mode)
	{
	case UMODE_NONE:
	{
		
		//if(u->cyframes)
		//	return;

		if(!u->cyframes)
		{
			if(NeedFood(u) /* && rand()%5==1 */ && FindFood(u))
			{	/*
				if(uID == 2)
				{
				Log("u[2]findfood");
			
				}*/
			}
			else if(NeedRest(u))
			{	/*
				if(uID == 2)
				{
				Log("u[2]needrest");
			
				}*/
				/*
				if(UnitID(this) == 0)
				{
				Log("0 needrest");
			
				}
				*/
				if(u->home >= 0)
				{	/*
					if(UnitID(this) == 0)
					{
					Log("home >= 0");
				
					}
					*/
					//Chat("go home");
					GoHome(u);
				}
				else
				{
					/*
					if(UnitID(this) == 0)
					{
					Log("findrest");
				
					}*/

					//Chat("find rest");
					if(!FindRest(u))
					{
						u->belongings[RES_LABOUR] += WORK_RATE;

						goto checkjob;
	#if 0
						if(u->belongings[RES_LABOUR] > 0)
						{	/*
							if(uID == 2)
							{
							Log("u[2]findjob");
			
							}*/
							if(!FindJob(u)
							   && u->path.size() == 0 && u->tpath.size() == 0 //TODO is this necessary? it probably stops workers from finding jobs. edit: shouldn't be run if FindJob returns true.
							   )
							{
				#if 1
								uint32_t ui = u - g_unit;
								//if(rand()%(FRAME_RATE*2) == 1)
								//if( (g_simframe + u->cmpos.x + u->cmpos.y * g_mapsz.x*TILE_SIZE) % (SIM_FRAME_RATE * 30) == 1 )
								//if( (g_simframe + ui) % (SIM_FRAME_RATE * 30) == 1 )	//adjust now that it only execs when cyframe==0
				
								{
									//move randomly?
									//goal = camera.Position() + Vec3f(rand()%TILE_SIZE - TILE_SIZE/2, 0, rand()%TILE_SIZE - TILE_SIZE/2);
					
									//set it in a new direction (out of 8 directions) every 3 simframes
									//uint32_t timeoff = (g_simframe % (8 * 3)) / 3;
									uint32_t timeoff = ( (g_simframe + ui) % (8 * 3)) / 3;
									//follow goal after subgoal; don't change goal (should be original position, since mode is none)
									u->subgoal /*= u->goal*/ = u->cmpos + Vec2i(OFFSETS[timeoff].x,OFFSETS[timeoff].y) * TILE_SIZE / 2;
									u->path.push_back( u->goal );
								}
				#endif
							}
						}
	#endif
					}
					else
						return;
				}
			}
			else
				goto checkjob;
		}
		
checkjob:

		if(u->jobframes >= LOOKJOB_DELAY_MAX)
		{
			if(u->belongings[RES_LABOUR] > 0)
			{	/*
				if(uID == 2)
				{
				Log("u[2]findjob");
			
				}*/
				if(!FindJob(u)
				   && u->path.size() == 0 && u->tpath.size() == 0 //TODO is this necessary? it probably stops workers from finding jobs. edit: shouldn't be run if FindJob returns true.
				   )
				{
	#if 0
					uint32_t ui = u - g_unit;
					//if(rand()%(FRAME_RATE*2) == 1)
					//if( (g_simframe + u->cmpos.x + u->cmpos.y * g_mapsz.x*TILE_SIZE) % (SIM_FRAME_RATE * 30) == 1 )
					//if( (g_simframe + ui) % (SIM_FRAME_RATE * 30) == 1 )	//adjust now that it only execs when cyframe==0
				
					{
						//move randomly?
						//goal = camera.Position() + Vec3f(rand()%TILE_SIZE - TILE_SIZE/2, 0, rand()%TILE_SIZE - TILE_SIZE/2);
					
						//set it in a new direction (out of 8 directions) every 3 simframes
						//uint32_t timeoff = (g_simframe % (8 * 3)) / 3;
						uint32_t timeoff = ( (g_simframe + ui) % (8 * 3)) / 3;
						//follow goal after subgoal; don't change goal (should be original position, since mode is none)
						u->subgoal /*= u->goal*/ = u->cmpos + Vec2i(OFFSETS[timeoff].x,OFFSETS[timeoff].y) * TILE_SIZE / 4;
						u->path.clear();
						u->path.push_back( u->goal );
					}
	#endif
				}
			}
		}
	} break;

	case UMODE_GOCSTJOB:
		GoCstJob(u);
		break;
	case UMODE_CSTJOB:
		if(u->cyframes)
			return;
		DoCstJob(u);
		break;
	case UMODE_GOBLJOB:
		GoBlJob(u);
		break;
	case UMODE_BLJOB:
		if(u->cyframes)
			return;
		DoBlJob(u);
		break;
	case UMODE_GOCDJOB:
		GoCdJob(u);
		break;
	case UMODE_CDJOB:
		if(u->cyframes)
			return;
		DoCdJob(u);
		break;
	case UMODE_GOSHOP:
		GoShop(u);
		break;
	case UMODE_GOREST:
		GoRest(u);
		break;
	case UMODE_SHOPPING:
		if(u->cyframes)
			return;
		DoShop(u);
		break;
	case UMODE_RESTING:
		if(u->cyframes)
			return;
		DoRest(u);
		break;
	case UMODE_GODRIVE:
		GoDrive(u);
		break;
	case UMODE_DRIVE:
		if(u->cyframes)
			return;
		DoDrive(u);
		break;
	default:
		break;
	}


	StopTimer(TIMER_UPDLAB);
}

void UpdLab2(Unit* u)
{
	switch(u->mode)
	{
	case UMODE_GODRIVE:
	case UMODE_GOBLJOB:
	case UMODE_GOCDJOB:
	case UMODE_GOCSTJOB:
		if(u->pathblocked)	//anticlump
			ResetMode(u);
		break;
	default:
		break;
	}
}
