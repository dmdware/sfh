













#include "truck.h"
#include "unit.h"
#include "transport.h"
#include "utype.h"
#include "building.h"
#include "bltype.h"
#include "conduit.h"
#include "simdef.h"
#include "../econ/utility.h"
#include "umove.h"
#include "../render/transaction.h"
#include "conduit.h"
#include "../sim/job.h"
#include "../econ/demand.h"
#include "../path/pathjob.h"
#include "../gui/layouts/chattext.h"
#include "../math/vec2i.h"
#include "../gui/widgets/spez/pygraphs.h"
#include "simflow.h"
#include "../ai/ai.h"

int16_t g_trsnd[TR_SOUNDS] = {-1,-1,-1};

bool FindFuel(Unit* u)
{
	Vec2i bcmpos;
	int32_t bestbi = -1;
	int32_t bestutil = -1;

	CdTile* rtile = GetCd(CD_ROAD, u->cmpos.x/TILE_SIZE, u->cmpos.y/TILE_SIZE, false);
	int16_t roadnet = rtile->netw;

	//InfoMess("ff", "ff?");

	for(int32_t bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_RETFUEL] <= 0)
			continue;

		Player* py = &g_player[b->owner];

		if(b->stocked[RES_RETFUEL] + py->global[RES_RETFUEL] <= 0)
			continue;

		bool samenet = false;

		for(std::list<int16_t>::iterator rnet=b->roadnetw.begin(); rnet!=b->roadnetw.end(); rnet++)
		{
			if(*rnet == roadnet)
			{
				samenet = true;
				break;
			}
		}

		if(!samenet)
			continue;

		//if(b->price[RES_RETFUEL] > u->belongings[RES_DOLLARS])
		//	continue;

		bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = MAG_VEC3F(pos - camera.Position()) * p->price[CONSUMERGOODS];
		const Vec2i cmoff = bcmpos - u->cmpos;
		int32_t cmdist = FUELHEUR(cmoff);
		int32_t thisutil = PhUtil(b->price[RES_RETFUEL], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return false;

	ResetGoal(u);
	u->fuelstation = bestbi;
	u->mode = UMODE_GOREFUEL;
	Building* b = &g_building[u->fuelstation];
	u->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;


#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset goal track u 1", "reset goal track u 1");
	}
#endif

	//Pathfind();
	//float pathlen = pathlength();
	//Log("found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)");
	//

	return true;
}

bool NeedFuel(Unit* u)
{
	if(u->belongings[RES_RETFUEL] < STARTING_FUEL*3/4)
		return true;

	return false;
}

void GoSup(Unit* u)
{
	//nothing here
}

void GoDemB(Unit* u)
{
#if 0
	if(mode == GOINGTODEMANDERB)
	{
		Building* b = &g_building[target];

		if(!g_diplomacy[b->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMROAD)
	{
		CRoad* road = RoadAt(target, target2);

		if(!g_diplomacy[road->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMPOWL)
	{
		CPowerline* powl = PowlAt(target, target2);

		if(!g_diplomacy[powl->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMPIPE)
	{
		CPipeline* pipe = PipeAt(target, target2);

		if(!g_diplomacy[pipe->owner][owner])
			ResetMode();
	}
#endif

	//for building's we check for arrival upon a collision in MoveUnit()
	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
	//if(CheckIfArrived(u))
	//	OnArrived(u);

}

void GoRefuel(Unit* u)
{
#if 0
	Building* b = &g_building[fuelStation];

	if(!g_diplomacy[b->owner][owner])
		ResetMode();
#endif

	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
}

void DoneRefuel(Unit* u)
{
	//Log("after bid done refueling";
	//

	u->fuelstation = -1;
	/*
	if(target2 >= 0)
	{
	mode = GOINGTOSUPPLIER;
	goal = g_building[target].pos;
	}
	else if(target >= 0)
	{
	mode = GOINGTODEMANDERB;
	goal = g_building[target].pos;
	}*//*
	if(transportAmt > 0)
	{
	if(targtype == GOINGTODEMANDERB)
	{
	mode = GOINGTODEMANDERB;
	goal = g_building[target].pos;
	}
	else if(targtype == GOINGTODEMROAD)
	{
	mode = GOINGTODEMROAD;
	goal = RoadPosition(target, target2);
	}
	else if(targtype == GOINGTODEMPIPE)
	{
	mode = GOINGTODEMPIPE;
	goal = PipelinePhysPos(target, target2);
	}
	else if(targtype == GOINGTODEMPOWL)
	{
	mode = GOINGTODEMPOWL;
	goal = PowerlinePosition(target, target2);
	}
	else
	{
	mode = targtype;
	ResetGoal();
	Pathfind();
	}
	}
	else if(targtype != NONE)
	{
	mode = GOINGTOSUPPLIER;
	goal = g_building[supplier].pos;
	}
	else*/
	{
		ResetMode(u);
		if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
			g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
		PlayClip(g_trsnd[TRSND_DONEJOB]);


#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset mode track u 1", "reset mode track u 1");
	}
#endif
	}
	//else
	//	ResetMode();
}

void DoRefuel(Unit* u)
{
	//InfoMess("do ref", "refling");

	if(u->belongings[RES_RETFUEL] >= STARTING_FUEL)
	{
		DoneRefuel(u);
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	//if(u->cyframes != 0)
	//	return;

	//last = GetTicks();

	Building* b = &g_building[u->fuelstation];
	Player* up = &g_player[u->owner];
	Player* bp = &g_player[b->owner];

	CycleHist* lastch = &*b->cyclehist.rbegin();

	if(b->stocked[RES_RETFUEL] + bp->global[RES_RETFUEL] < 0)
	{
		DoneRefuel(u);
		return;
	}

	int32_t req = STARTING_FUEL - u->belongings[RES_RETFUEL];
	int32_t got = req;

	if(b->stocked[RES_RETFUEL] + bp->global[RES_RETFUEL] < req)
		got = b->stocked[RES_RETFUEL] + bp->global[RES_RETFUEL];

	lastch->cons[RES_RETFUEL] += got;	//corpd fix xp

	int32_t paid;

	if(u->owner != b->owner)
	{
		//LogTransx(owner, -bp->price[ZETROL], "buy fuel");
		//LogTransx(b->owner, bp->price[ZETROL], "buy fuel");

		if(up->global[RES_DOLLARS] < b->price[RES_RETFUEL])
		{
			Bankrupt(u->owner, "fuel costs");
			DoneRefuel(u);
			return;
		}

		int32_t canafford = got;

		if(b->price[u->cargotype] > 0)
			canafford = up->global[RES_DOLLARS] / b->price[RES_RETFUEL];

		if(canafford < got)
			got = canafford;

		paid = got * b->price[RES_RETFUEL];

		up->global[RES_DOLLARS] -= paid;
		bp->global[RES_DOLLARS] += paid;
		up->gnp += paid;
		bp->gnp += paid;

		lastch->prod[RES_DOLLARS] += paid;	//corpd fix xp

#if 0
		up->recentth.consumed[CURRENC] += paid;
		bp->recentph[ZETROL].earnings += paid;
#endif

		if(up->global[RES_DOLLARS] <= 0)
			Bankrupt(u->owner, "fuel costs");
	}

	if(bp->global[RES_RETFUEL] >= got)
		bp->global[RES_RETFUEL] -= got;
	else
	{
		int32_t subgot = got;
		subgot -= bp->global[RES_RETFUEL];
		bp->global[RES_RETFUEL] = 0;

		b->stocked[RES_RETFUEL] -= got;
		bp->local[RES_RETFUEL] -= got;
	}

#if 0
	b->recenth.consumed[RES_RETFUEL] += got;
#endif
	u->belongings[RES_RETFUEL] += got;

	//b->Emit(BARREL);
#ifdef LOCAL_TRANSX
	if(
		b->owner == g_localP &&
		b->owner != u->owner)
		NewTransx(Vec2i(b->tpos.x*TILE_SIZE, b->tpos.y*TILE_SIZE), CURRENC, paid, ZETROL, -got);
#endif
	if(
#ifdef LOCAL_TRANSX
		owner == g_localP &&
#endif
		b->owner != u->owner)
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_RETFUEL];
			char numpart[128];
			sprintf(numpart, "%+d", got);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}
		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -paid);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(u->cmpos, &transx);

		//NewTransx(Vec2i(b->tpos.x*TILE_SIZE, b->tpos.y*TILE_SIZE), ZETROL, got, CURRENC, -paid);
	}

	DoneRefuel(u);
}

void AtDemB(Unit* u)
{
	//LastNum("at demander b");

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	//if(u->cyframes != 0)
	//	return;

	Building* b = &g_building[u->target];

	if(b->finished)
		//b->stock[u->cargotype] += 1.0f;
			b->stocked[u->cargotype] += u->cargoamt;
	else
	{
		//b->conmat[u->cargotype] += 1.0f;
		b->conmat[u->cargotype] += u->cargoamt;
		b->checkconstruction();
	}

#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[u->cargotype];
			char numpart[128];
			sprintf(numpart, "%+d", u->cargoamt);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(u->cmpos, &transx);

		//NewTransx(b->pos, transportRes, 1.0f);
		//NewTransx(b->pos, transportRes, transportAmt);
	}

	//u->cargoamt -= 1;
	u->cargoamt = 0;

	//char msg[128];
	//sprintf(msg, "unloaded %0.2f/%0.2f %s", 1.0f, transportAmt, g_resource[transportRes].name);
	//Chat(msg);

	if(u->cargoamt <= 0)
	{
		b->transporter[u->cargotype] = -1;
		ResetMode(u);
		
		std::list<TrCycleHist>::reverse_iterator chit = u->cyclehist.rbegin();
		TrCycleHist* lastch = &*chit;
		chit->jobsfini++;

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset mode track u 2", "reset mode track u 2");
	}
#endif
	}
	
	if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
		g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
		PlayClip(g_trsnd[TRSND_DONEJOB]);
}

void DoneAtSup(Unit* u)
{
	//Log("after bid done at sup";
	//

	//LastNum("dat sup 1");

	if(u->cargoamt <= 0)
	{
		ResetMode(u);
		return;
	}
	//LastNum("dat sup 2");

	Building* supb = &g_building[u->supplier];
	//supb->transporter[u->cargotype] = -1;
	//u->mode = u->targtype;

	//LastNum("dat sup 3");
	if(u->targtype == TARG_BL)
	{
		Building* demb = &g_building[u->target];
		u->goal = demb->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		u->mode = UMODE_GODEMB;
	}
	else if(u->targtype == TARG_CD)
	{
		CdType* ctype = &g_cdtype[u->cdtype];
		//CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);
		u->goal = Vec2i(u->target, u->target2) * TILE_SIZE + ctype->physoff;
		u->mode = UMODE_GODEMCD;
	}
	//LastNum("dat sup 4");

	NewJob(UMODE_GODRIVE, (int32_t)(u-g_unit), -1, CD_NONE);
	//LastNum("dat sup 5");
	
	if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
		g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
		PlayClip(g_trsnd[TRSND_DONEJOB]);
}

void AtSup(Unit* u)
{
	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	//LastNum("at sup 1");

	//if(u->cyframes != 0)
	//	return;

	Building* supb = &g_building[u->supplier];
	Building* demb;
	Player* demp = NULL;
	int32_t supplayer = supb->owner;
	Player* supp = &g_player[supb->owner];
	BlType* demt;
	CdTile* demcd;
	//LastNum("at sup 2");

	//char msg[128];
	//sprintf(msg, "at sup targtype=%d @%d,%d", targtype, target, target2);
	//LastNum(msg);

	int32_t demplayer = -1;

	if(u->targtype == TARG_BL)
	{
		demb = &g_building[u->target];
		demplayer = demb->owner;
		demp = &g_player[demb->owner];
		demt = &g_bltype[demb->type];
	}
	else if(u->targtype == TARG_CD)
	{
		demcd = GetCd(u->cdtype, u->target, u->target2, false);
		demplayer = demcd->owner;
		demp = &g_player[demcd->owner];
	}

	//LastNum("at sup 3");
	//if(supb->stock[transportRes] + supp->global[transportRes] <= 0.0f)
	//	DoneAtSupplier();

	//LastNum("at sup 1");

	//if(demb->stock[transportRes] + demp->global[transportRes] + transportAmt >= demt->input[transportRes])
	if(u->cargoamt >= u->cargoreq)
	{
		DoneAtSup(u);
		return;
	}

	//LastNum("at sup 2");
	//LastNum("at sup 4");

	//float required = demt->input[transportRes] - transportAmt - demb->stock[transportRes] - demp->global[transportRes];
	int32_t reqnow = u->cargoreq - u->cargoamt;
	int32_t got = reqnow;

	if(supb->stocked[u->cargotype] + supp->global[u->cargotype] < reqnow)
		got = supb->stocked[u->cargotype] + supp->global[u->cargotype];

	int32_t paidtosup = 0;	//paid to supplier
	int32_t paidtoimst = 0;	//paid to import state
	int32_t paidtoexst = 0;	//paid to export state
	int32_t paidtotal = 0;	//total paid
	int32_t initprice = supb->price[u->cargotype];	//initial price
	int32_t imtariffprice = 0;	//import tariff component of price
	int32_t extariffprice = 0;	//export tariff component of price
	int32_t effectprice = initprice;	//effective price

	//supplier and demander states (export and import)
	//firms and states are grouped in pairs of (FIRMSPERSTATE+1)
	int32_t exsti = (int32_t)( supplayer / (FIRMSPERSTATE+1) ) * (FIRMSPERSTATE+1);
	int32_t imsti = (int32_t)( demplayer / (FIRMSPERSTATE+1) ) * (FIRMSPERSTATE+1);
	Player* exst = &g_player[exsti];
	Player* imst = &g_player[imsti];

	//LastNum("at sup 5");
	//LastNum("at sup 3");
	//if(targtype == GOINGTODEMANDERB)
	{
		//if(demb->owner != supb->owner)
		if(demp != supp)
		{
			//if trade between different countries
			if(imsti != exsti)
			{
				if(imst->protectionism)
				{
					imtariffprice = initprice * imst->imtariffratio / RATIO_DENOM;
					effectprice += imtariffprice;
				}

				if(exst->protectionism)
				{
					extariffprice = initprice * exst->extariffratio / RATIO_DENOM;
					effectprice += extariffprice;
				}
			}

			if(demp->global[RES_DOLLARS] < effectprice)
			{
				char reason[64];
				Resource* r = &g_resource[u->cargotype];
				sprintf(reason, "buying %s", r->name.c_str());
				Bankrupt(demplayer, reason);
				DoneAtSup(u);
				return;
			}

			int32_t canafford = got;

			//LastNum("at sup 5.2");
			if(supb->price[u->cargotype] > 0)
				canafford = demp->global[RES_DOLLARS] / effectprice;

			if(canafford < got)
				got = canafford;
			//LastNum("at sup 4");

			paidtosup = got * initprice;
			paidtoexst = got * extariffprice;
			paidtoimst = got * imtariffprice;
			paidtotal = paidtosup + paidtoexst + paidtoimst;

			//LastNum("at sup 5.3");
			demp->global[RES_DOLLARS] -= paidtotal;
			supp->global[RES_DOLLARS] += paidtosup;
			exst->global[RES_DOLLARS] += paidtoexst;
			imst->global[RES_DOLLARS] += paidtoimst;

			if(u->targtype == TARG_BL)
			{
				demb->varcost[u->cargotype] += paidtotal;
			}

			//TODO GDP for countries, throughput for firms
			demp->gnp += paidtotal;
			supp->gnp += paidtotal;
			
#ifdef PYG
			PyGraph* pyg = &g_pygraphs[demplayer][PYGRAPH_DREXP];
			*pyg->fig.rbegin() += paid;
			pyg = &g_pygraphs[demplayer][PYGRAPH_TOTLABEXP];
			*pyg->fig.rbegin() += paid;
#endif

#if 0
			supb->recenth.produced[CURRENC] += paid;
			if(targtype == GOINGTODEMANDERB)
				demb->recenth.consumed[CURRENC] += paid;
			else
				demp->recentth.consumed[CURRENC] += paid;
			supp->recentph[transportRes].earnings += paid;
#endif

			//LastNum("at sup 5.4");
			//char msg[128];
			//sprintf(msg, "purchase %s", g_resource[transportRes].name);
			//LogTransx(supb->owner, paid, msg);
			//LogTransx(demplayer, -paid, msg);
		}
	}
	//LastNum("at sup 6");

	//LastNum("at sup 5");
	if(supp->global[u->cargotype] >= got)
		supp->global[u->cargotype] -= got;
	else
	{
		//LastNum("at sup 6");
		//float subgot = got;	//corpd fix !!!!!
		int32_t subgot = got;
		subgot -= supp->global[u->cargotype];
		supp->global[u->cargotype] = 0;

		supb->stocked[u->cargotype] -= subgot;
		supp->local[u->cargotype] -= subgot;
	}

	CycleHist *supch = &*supb->cyclehist.rbegin();
	supch->cons[u->cargotype] += got;
	
	if(demp != supp)
	{
		//is the demander a building?
		if(u->targtype == TARG_BL)
		{
			//2015/11/16
			if(demb->finished)
			{
				CycleHist *demch = &*demb->cyclehist.rbegin();
				demch->cons[RES_DOLLARS] += paidtotal;
			}
		}

		supch->prod[RES_DOLLARS] += paidtosup;
	}
	//TODO rename "cons" "prod"
	//TODO truck expenses

	//LastNum("at sup 7");
	//LastNum("at sup 7");
	u->cargoamt += got;
#if 0
	supb->recenth.consumed[u->cargotype] += got;
#endif

	Vec2f transxpos = supb->drawpos;

#ifdef LOCAL_TRANSX
	if(supb->owner == g_localP && demb->owner != supb->owner)
		NewTransx(transxpos, CURRENC, paid, transportRes, -got);
	if(demb->owner == g_localP && demb->owner != supb->owner)
		NewTransx(transxpos, transportRes, got, CURRENC, -paid);
#else
	{
		RichText transx;

		{
			Resource* r = &g_resource[u->cargotype];
			char numpart[128];
			sprintf(numpart, "%+d", -got);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		//NewTransx(transxpos, &transx); //TODO paid and show transx

		//NewTransx(transxpos, u->cargotype, -got);
	}
#endif

	//LastNum("at sup 8");
	DoneAtSup(u);
	//LastNum("at sup 9");
}

void GoDemCd(Unit* u)
{
	//transporters can only deliver a certain resource to a building/conduit one at a time
	//(two transporters can't deliver the same res type). so we don't need to check if somebody
	//already delivered enough. we just need to check if we've arrived at a conduit because
	//we don't collide with them.
	if(CheckIfArrived(u))
		OnArrived(u);
}

void AtDemCd(Unit* u)
{
	//LastNum("at demander b");

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	if(u->cyframes != 0)
		return;

	//last = GetTicks();

	CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);

	ctile->conmat[u->cargotype] += 1;
	u->cargoamt -= 1;

#ifdef LOCAL_TRANSX
	if(r->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[u->cargotype];
			char numpart[128];
			//sprintf(numpart, "%+d", u->cargoamt);
			sprintf(numpart, "%+d", 1);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		CdType* ct = &g_cdtype[u->cdtype];
		Vec2i cmpos = Vec2i(u->target*TILE_SIZE + ct->physoff.x, u->target2*TILE_SIZE + ct->physoff.y);

		NewTransx(cmpos, &transx);

		//NewTransx(b->pos, transportRes, 1.0f);
		//NewTransx(b->pos, transportRes, transportAmt);
		
		if(g_mapview[0].x <= u->cmpos.x && g_mapview[0].y <= u->cmpos.y &&
			g_mapview[1].x >= u->cmpos.x && g_mapview[1].y >= u->cmpos.y)
			PlayClip(g_trsnd[TRSND_DONEJOB]);
	}

	if(u->cargoamt <= 0)
	{
		ctile->transporter[u->cargotype] = -1;
		ResetMode(u);
		
		std::list<TrCycleHist>::reverse_iterator chit = u->cyclehist.rbegin();
		TrCycleHist* lastch = &*chit;
		chit->jobsfini++;
	}

	ctile->checkconstruction(u->cdtype);
}

void UpdTruck(Unit* u)
{
	StartTimer(TIMER_UPDTRUCK);
	//return;	//do nothing for now
	
	Player* py = &g_player[u->owner];

	if(py->ai)
	{
		//u->jobframes ++;
		int32_t ui = u - g_unit;
		if((g_simframe+ui) % CYCLE_FRAMES == 0)
		//if(u->cyframes == 0)
		{
			//AdjTrPrWg(py, u);
		
			u->cyclehist.push_back(TrCycleHist());

			while(u->cyclehist.size() > 100)
				u->cyclehist.erase(u->cyclehist.begin());
		
			TrCycleHist* trch = &*u->cyclehist.rbegin();

			trch->opwage = py->truckwage;
			trch->trprice = py->transpcost;
		}
	}

	if(u->mode != UMODE_NONE)
	{
		int32_t m = PATHHEUR((u->cmpos - u->prevpos));

		u->cyframes--;

		if(u->cyframes < 0)
			u->cyframes = WORK_DELAY-1;

		//u->cyframes += m;

		//if(u->cyframes >= TRUCK_DIST)
		if((g_simframe * TRUCK_SPEED) % TRUCK_DIST < m)
		{
			u->belongings[RES_RETFUEL] -= TRUCK_CONSUMPRATE;
			//u->cyframes = 0;
		}

		//prevent job from expiring from inactivity ("stuck")
		if(u->cmpos != u->prevpos)
			u->jobframes = TBID_DELAY;

		Unit* targu = NULL;

		if(u->driver >= 0)
			targu = &g_unit[u->driver];

#if 0
		//Doesn't work, in situations where
		//approaching driver blocks path.
		if(Trapped(u, targu))
		{
			ResetMode(u);
			return;
		}
#endif
	}

	if(u->belongings[RES_RETFUEL] <= 0)
	{
		char msg[128];
		sprintf(msg, "%s Truck out of fuel! (Remaining %d)", Time().c_str(), CountU(UNIT_TRUCK));
		Log(msg);
		Log("\tRemain: %d\r\n", CountU(UNIT_TRUCK));
		RichText sr;
		sr.m_part.push_back(UStr(msg));
		//SubmitConsole(&sr);
		AddNotif(&sr);
		//SubmitConsole(&sr);
		AddNotif(&sr);
		//ResetMode(u);
		u->destroy();
		return;
	}

#if 0
	if(u->path.size() > 0)
	{
		InfoMess("trp", "trp");
	}
#endif

	//if(NeedFuel(u))
	//	InfoMess("nf", "nf");

	switch(u->mode)
	{
	case UMODE_NONE:
		{
			if(NeedFuel(u) && FindFuel(u))
			{
				//InfoMess("ff", "ff");
				//Log("find fuel");
				//
			}
			else if(u->mode == UMODE_NONE /* && u->target < 0 */)
			{
				//	FindDemander();
				if(u->jobframes >= TBID_DELAY)
				{
					//g_freetrucks.push_back(UnitID(this));
					//Log("free truck");
					//
				}
				u->jobframes ++;
			}
		} break;

	case UMODE_GOSUP:
		GoSup(u);
		break;

	case UMODE_GODEMB:
		GoDemB(u);
		break;

	case UMODE_GOREFUEL:
		GoRefuel(u);
		break;

	case UMODE_REFUELING:
		DoRefuel(u);
		break;

	case UMODE_ATDEMB:
		AtDemB(u);
		break;

	case UMODE_ATSUP:
		AtSup(u);
		break;

	case UMODE_GODEMCD:
		GoDemCd(u);
		break;

	case UMODE_ATDEMCD:
		AtDemCd(u);
		break;

	default:
		break;
	}

	StopTimer(TIMER_UPDTRUCK);
}
