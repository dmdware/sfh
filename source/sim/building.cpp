











#include "building.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "bltype.h"
#include "../render/foliage.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../render/shader.h"
#include "../math/barycentric.h"
#include "../math/physics.h"
#include "player.h"
#include "../render/transaction.h"
#include "selection.h"
#include "../gui/gui.h"
#include "../gui/widget.h"
#include "../gui/widgets/spez/cstrview.h"
#include "../app/appmain.h"
#include "../math/frustum.h"
#include "unit.h"
#include "simdef.h"
#include "labourer.h"
#include "../econ/demand.h"
#include "../math/fixmath.h"
#include "../ai/ai.h"
#include "simflow.h"
#include "../econ/utility.h"
#include "job.h"
#include "map.h"
#include "../render/drawqueue.h"
#include "../render/drawsort.h"
#include "../gui/layouts/chattext.h"
#include "../render/fogofwar.h"
#include "../net/client.h"
#include "../path/collidertile.h"
#include "border.h"
#include "labourer.h"

Building g_building[BUILDINGS];

std::list<uint16_t> g_onbl;

void Building::destroy()
{
    //corpc fix TODO
    //if(g_collider.m_size && on)
	if(on && g_pathnode)
        freecollider();

	if(on)
	{
		uint16_t bin = this - g_building;

		for(std::list<uint16_t>::iterator bit=g_onbl.begin(); bit!=g_onbl.end(); bit++)
		{
			uint16_t bin2 = *bit;

			if(bin2 != bin)
				continue;

			g_onbl.erase(bit);
			break;
		}
	}
    
	Evict(this);

	while(worker.size() > 0)
	{
		int32_t i = *worker.begin();
		//ResetMode(&g_unit[i]);
		Unit* u = &g_unit[i];

		//might already be freed in FreeMap.
		//don't want to UpDraw him because it crashes.
		if(!u->on)
		{
			worker.erase( worker.begin() );
			continue;
		}

		u->mode = UMODE_NONE;
		Vec2i cmplace;
		if(PlaceUAb(u->type, u->cmpos, &cmplace))
		{
			
	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f11");

			u->cmpos = cmplace;
			u->fillcollider();
			UpDraw(u);
#if 0
			//TODO
			u->drawpos.x = (float)u->cmpos.x;
			u->drawpos.y = (float)u->cmpos.y;
			u->drawpos.y = g_hmap.accheight(u->cmpos.x, u->cmpos.y);
#endif
		}
		worker.erase( worker.begin() );
	}

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(transporter[ri] < 0)
			continue;

		int32_t ui = transporter[ri];
		Unit* u = &g_unit[ui];
		ResetMode(u);
	}

	int32_t bi = this - g_building;

	for(std::list<CapSup>::iterator csit=capsup.begin(); csit!=capsup.end(); csit++)
	{
		if(csit->src == bi)
			continue;

		Building* b2 = &g_building[csit->dst];

		std::list<CapSup>::iterator csit2=b2->capsup.begin();

		while(csit2!=b2->capsup.end())
		{
			if(csit2->src == bi)
			{
				csit2 = b2->capsup.erase(csit2);
				continue;
			}

			csit2++;
		}
	}

	capsup.clear();

	roadnetw.clear();
	pownetw = -1;
	crpipenetw = -1;

	for(std::list<Depthable>::iterator qit=g_drawlist.begin(); qit!=g_drawlist.end(); qit++)
	{
		if(&*qit == depth)
		{
			g_drawlist.erase(qit);
			depth = NULL;
			break;
		}
	}

	cyclehist.clear();
	
	on = false;
}

Building::Building()
{
	on = false;
	depth = NULL;
	frame = 0;
}

Building::~Building()
{
	if(!on)
		return;

	destroy();
}

int32_t Building::netreq(int32_t rtype)
{
	//int32_t prodleft = prodlevel - cymet;
	
	BlType* bt = &g_bltype[type];

	//leave reserves for next cycle
	int32_t prodleft = (prodlevel * 3) - cymet;
	//int32_t prodleft = (prodlevel * 2) - cymet;
	//int32_t prodleft = (prodlevel * 4) - cymet;

	if(prodleft <= 0)
		return 0;

#if 0
	int32_t lowr = -1;
	int32_t lowamt = -1;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		if(lowr >= 0 && lowamt <= bt->input[ri])
			continue;

		lowr = ri;
		lowamt = bt->input[ri];
	}

	if(lowr < 0)
		return 0;
#endif

	int32_t net = iceil(bt->input[rtype], RATIO_DENOM) * prodleft;

	Resource* r = &g_resource[rtype];

	if(!r->capacity)
		net -= stocked[rtype];
	else //don't use this for adjcaps(); as cymet reaches prodlevel, this will become negative as
		//cap sources are no longer required, which makes them spike only at certain times,
		//not be continuous like electricity cap usage,
	{
		int32_t bi = this - g_building;

		for(std::list<CapSup>::iterator csit=capsup.begin(); csit!=capsup.end(); csit++)
		{
			if(csit->src == bi)
				continue;

			if(csit->rtype != rtype)
				continue;

			net -= csit->amt;
		}
	}

	Player* py = &g_player[owner];

	//important: resources must be obtained from outlet bl's,
	//even if they're globally available to the owner player.
	//if(r->physical)
	//	net -= py->global[rtype];

	return net;
}

bool Building::excin(int32_t rtype)
{
#if 0
	//CBuildingType* bt = &g_buildingType[type];
	Player* py = &g_player[owner];

	int32_t got = stocked[rtype];

	Resource* r = &g_resource[rtype];

	if(rtype != RES_LABOUR && r->physical)
		got += py->global[rtype];

	//if(got >= prodquota * bt->input[res])
	if(got >= netreq(rtype))
		return true;
#endif

	if(netreq(rtype) <= 0)
		return true;

	return false;
}

bool Building::metout()
{
	if(cymet >= prodlevel)
		return true;

	return false;
}

bool Building::hasworker(int32_t ui)
{
	for(std::list<int32_t>::iterator witer=worker.begin(); witer!=worker.end(); witer++)
		if(*witer == ui)
			return true;

	return false;
}


void Building::spawn(int32_t utype, int32_t uowner)
{
	Vec2i cmplace;

	if(!PlaceUAb(utype, tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2, &cmplace))
		return;

	PlaceUnit(utype, cmplace, uowner, NULL);
}

//try to manufacture a unit if one is in the production queue
bool Building::trymanuf()
{
	if(manufjob.size() <= 0)
		return false;

	ManufJob* mj = &*manufjob.begin();

	Player* bpy = &g_player[ owner ];
	UType* t = &g_utype[ mj->utype ];

	int32_t uprice = manufprc[ mj->utype ];
	Player* upy = &g_player[ mj->owner ];

	if( mj->owner != owner)
	{
		if( upy->global[RES_DOLLARS] < uprice )
		{
			if(mj->owner == g_localP)
			{
				Resource* r = &g_resource[RES_DOLLARS];
				char cmess[256];
				sprintf(cmess, "%s (%s %d, %s %d) %s %s.", 
					r->name.c_str(), 
					STRTABLE[STR_NEED].rawstr().c_str(), 
					uprice, 
					STRTABLE[STR_HAVE].rawstr().c_str(),
					upy->global[RES_DOLLARS], 
					STRTABLE[STR_TOORDERA].rawstr().c_str(),
					t->name);
				RichText mess = STRTABLE[STR_INSUFFICIENT] + RichText(" ") + RichText(RichPart(RICH_ICON, r->icon)) + RichText(cmess);
				AddNotif(&mess);
			}

			return false;
		}

		upy->global[RES_DOLLARS] -= uprice;
		bpy->global[RES_DOLLARS] += uprice;
		upy->gnp += uprice;
		bpy->gnp += uprice;

		if(cyclehist.size() > 0)
		{
			CycleHist* lastch = &*cyclehist.rbegin();
			lastch->prod[RES_DOLLARS] += uprice;
		}
	}

	int32_t netch[RESOURCES];
	Zero(netch);

	int32_t insufres = 0;
	if(!TrySub(t->cost, bpy->global, stocked, bpy->local, netch, &insufres))
	{
		if(mj->owner == g_localP)
		{
			Resource* r = &g_resource[insufres];
			char cmess[256];
			sprintf(cmess, "%s %s (%s %d, %s %d) %s %s.", 
				r->name.c_str(), 
				STRTABLE[STR_ATMANUF].rawstr().c_str(),
				STRTABLE[STR_NEED].rawstr().c_str(),
				t->cost[insufres], 
				STRTABLE[STR_HAVE].rawstr().c_str(),
				bpy->global[insufres] + stocked[insufres], 
				STRTABLE[STR_TOPRODUCE].rawstr().c_str(),
				t->name);
			RichText mess = STRTABLE[STR_INSUFFICIENT] + RichText(" ") + RichText(RichPart(RICH_ICON, r->icon)) + RichText(cmess);
			AddNotif(&mess);
		}

		return false;
	}

#if 0

	for(int32_t i=0; i<RESOURCES; i++)
	{
		recenth.consumed[i] += netch[i];
	}

#endif

#if 0
	////if(owner == g_localP)
	{
		//if(g_selection.size() > 0 && g_selectType == SELECT_BUILDING && g_selection[0] == BuildingID(this))
		//	UpdateQueueCount(mj.utype);

		if(mj->owner == g_localP)
		{
			char finmsg[128];
			sprintf(finmsg, "%s manufactured.", t->name);
			RichText rt(finmsg);
			AddNotif(&rt);
			//Chat(finmsg);
			//WaiFoO();
			// OnBuildU(mj.utype);
		}
	}
#else

	RichText uname = upy->name;
	RichText bname = bpy->name;
	if(upy->client >= 0)
	{
		Client* c = &g_client[upy->client];
		uname = c->name;
	}
	if(bpy->client >= 0)
	{
		Client* c = &g_client[bpy->client];
		bname = c->name;
	}
	if(mj->owner != owner)
	{
		char cmess[256];
		sprintf(cmess, " %s %s %s ",
			STRTABLE[STR_ORDEREDA].rawstr().c_str(),
			t->name,
			STRTABLE[STR_FROM].rawstr().c_str());
		char cmess2[128];
		RichText mess = uname +
			RichText(cmess) +
			bname +
			RichText(" ") +
			STRTABLE[STR_FOR] +
			RichText(" ") +
			RichText(RichPart(RICH_ICON, ICON_DOLLARS)) +
			RichText(iform(uprice).c_str()) +
			RichText(".");
		AddNotif(&mess);
	}
	else
	{
		char cmess[256];
		sprintf(cmess, " %s %s.", 
			STRTABLE[STR_MANUFDA].rawstr().c_str(),
			t->name);
		char cmess2[128];
		RichText mess = uname +
			RichText(cmess);
		AddNotif(&mess);
	}
#endif

	spawn(mj->utype, mj->owner);
	manufjob.erase( manufjob.begin() );

	return true;
}

//find a supplier building of a capacity resource type "rtype" and register the usage.
//if amt is negative, get rid of that much usage.
void Building::morecap(int32_t rtype, int32_t amt)
{
	int32_t dembi = this - g_building;

	Resource* r = &g_resource[rtype];
	CdType* ct = &g_cdtype[r->conduit];

	//TODO rename to avoid conflict and/or make use C90
	int16_t netw;
	std::list<int16_t>* netwlist;

	//can't do anything if we're not connected to the grid of whatever conduit type this capacity resource is supplied by
	if(ct->blconduct)
	{
		netw = *(int16_t*)(((char*)this)+ct->netwoff);
		//netw = this->netw[r->conduit];

		if(netw < 0)
			return;
	}
	else
	{
		netwlist = (std::list<int16_t>*)(((char*)this)+ct->netwoff);
		//netwlist = &this->netwlist[r->conduit];

		if(netwlist->size() <= 0)
			return;
	}

	//Vec2i demcmpos = tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	bool foundsup = false;

	//InfoMess(g_resource[rtype].name.c_str(), "supcap?");

	do
	{
		foundsup = false;
		//repeatedly find the best priced/distanced (based on utilty cost) supplier of this cap resource,
		//until we either cannot get anymore supplied or we don't need anymore.

		int32_t bestsrc = -1;
		int32_t bestutil = -1;
		int32_t bestamt = -1;

		for(int32_t bi=0; bi<BUILDINGS; bi++)
		{
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			if(!b->finished)
				continue;

			BlType* bt = &g_bltype[b->type];

			if(bt->output[rtype] <= 0)
				continue;

			int16_t netw2;
			std::list<int16_t>* netwlist2;

			//must be connected by conduit
			if(ct->blconduct)
			{
				netw2 = *(int16_t*)(((char*)b)+ct->netwoff);
				//netw2 = b->netw[r->conduit];

				if(netw != netw2)
					continue;
			}
			else
			{
				netwlist2 = (std::list<int16_t>*)(((char*)b)+ct->netwoff);
				//netwlist2 = &b->netwlist[r->conduit];

				bool foundnetw = false;

				//matching road networks? (might be connected to more than one seperate, isolated road grid.)
				for(std::list<int16_t>::iterator nit=netwlist->begin(); nit!=netwlist->end(); nit++)
					for(std::list<int16_t>::iterator nit2=netwlist2->begin(); nit2!=netwlist2->end(); nit2++)
					{
						if(*nit != *nit2)
							continue;

						foundnetw = true;
						break;
					}

				if(!foundnetw)
					continue;
			}

			int32_t minlevel = b->maxprod();
			//minlevel is not enough to determine ability to supply capacity type resources.
			//as soon as the inputs are used up, they into the "cymet" counter, and won't be resupplied
			//until cymet is reset to 0, next cycle. so use whichever is greater.
			int32_t suppliable = bt->output[rtype] * minlevel / RATIO_DENOM;
			suppliable = imax(suppliable, b->cymet * bt->output[rtype] / RATIO_DENOM);

			int32_t supplying = 0;

			//check how much this supplier is already supplying to other demanders
			for(std::list<CapSup>::iterator csit=b->capsup.begin(); csit!=b->capsup.end(); csit++)
			{
				if(csit->src != bi)
					continue;

				if(csit->rtype != rtype)
					continue;

				supplying += csit->amt;
			}

			suppliable -= supplying;

			if(suppliable <= 0)
				continue;

			//assuming here all capacity resource types are non-physical (thus global, as int32_t as we're connected to the right conduit)
			int32_t util = GlUtil(b->price[rtype]);

			if(bestutil >= 0 && bestutil > util)
				continue;

			bestutil = util;
			bestsrc = bi;
			bestamt = suppliable;
			foundsup = true;
		}

		if(!foundsup)
			break;

		int32_t supplied = imin(amt, bestamt);

		Building* supb = &g_building[bestsrc];
		CapSup newcs;
		newcs.amt = supplied;
		newcs.dst = dembi;
		newcs.src = bestsrc;
		newcs.rtype = rtype;
		supb->capsup.push_back(newcs);
		this->capsup.push_back(newcs);

		amt -= supplied;

#if 0
		char msg[128];
		sprintf(msg, "supcap to:%s amt:%d", g_bltype[this->type].name, supplied);
		InfoMess(g_resource[rtype].name.c_str(), msg);
#endif

	}while(foundsup && amt > 0);
}

void Building::lesscap(int32_t rtype, int32_t amt)
{
	int32_t dembi = this - g_building;

	//need to get rid of excess usage? (e.g., we've recently lowered the production level).

	//assuming price is the only factor to judge because of the assumption that all capacity resource types
	//are non-physical (thus global, as int32_t as we're connected to the right conduit).

	int32_t worstprc = -1;
	int32_t worstsrc = -1;

	//InfoMess(g_resource[rtype].name.c_str(), "less cap");

	do
	{
		worstsrc = -1;

		//repeatedly get rid of the more expensive sources until we've gotten rid of enough
		for(std::list<CapSup>::iterator csit=capsup.begin(); csit!=capsup.end(); csit++)
		{
			if(csit->src == dembi)
				continue;

			if(csit->rtype != rtype)
				continue;

			Building* supb = &g_building[csit->src];

			if(worstprc >= 0 && worstprc > supb->price[rtype])
				continue;

			worstprc = supb->price[rtype];
			worstsrc = csit->src;
		}

		if(worstsrc < 0)
			break;

		//there might be more than one CapSup associated with this source for this demander,
		//so cycle through until we've got them all or amt == 0.
		std::list<CapSup>::iterator csit=capsup.begin();
		while(csit!=capsup.end())
		{
			if(csit->src != worstsrc ||
				csit->rtype != rtype)
			{
				csit++;
				continue;
			}

			Building* supb = &g_building[csit->src];
			std::list<CapSup>::iterator csit2=supb->capsup.begin();
			while(csit2!=supb->capsup.end())
			{
				if(csit2->src != csit->src ||
					csit2->rtype != csit->rtype ||
					csit2->amt != csit->amt ||
					csit2->dst != csit->dst)
				{
					csit2++;
					continue;
				}

				if(csit2->amt > -amt)
					csit2->amt += amt;
				else
					csit2 = supb->capsup.erase(csit2);
				break;
			}

			if(csit->amt > -amt)
			{
				csit->amt += amt;
				amt = 0;
				break;
			}
			else
			{
				amt += csit->amt;
				csit = capsup.erase(csit);
			}
		}
	}while(amt < 0);

	//InfoMess(g_resource[rtype].name.c_str(), "/less cap");
}


void FreeCaps(uint8_t ri, int32_t supbi, int32_t dembi)
{
	Building* supb = &g_building[supbi];
	Building* demb = &g_building[dembi];

	std::list<CapSup>::iterator cit = supb->capsup.begin();
	
	while(cit != supb->capsup.end())
	{
		if(cit->dst == dembi &&
			cit->rtype == ri)
			cit = supb->capsup.erase(cit);
		else
			++cit;
	}

	cit = demb->capsup.begin();
	
	while(cit != demb->capsup.end())
	{
		if(cit->src == supbi &&
			cit->rtype == ri)
			cit = demb->capsup.erase(cit);
		else
			++cit;
	}
}

void AdjCaps(uint8_t ri, int32_t supbi, int32_t dembi)
{
	Building* supb = &g_building[supbi];
	Building* demb = &g_building[dembi];

	FreeCaps(ri, supbi, dembi);

	demb->adjcaps();
}

void SupAdjCaps(uint8_t ri, int32_t supbi)
{
	Building* supb = &g_building[supbi];
	std::list<CapSup>::iterator cit = supb->capsup.begin();

	std::list<int32_t> dems;
	
	while(cit != supb->capsup.end())
	{
		if(cit->src == supbi &&
			cit->rtype == ri)
		{
			int32_t dembi = cit->dst;
			dems.push_back(dembi);
			cit = supb->capsup.erase(cit);
			continue;
		}
		else
			++cit;
	}

	dems.unique();

	for(std::list<int32_t>::iterator dembit=dems.begin(); dembit!=dems.end(); ++dembit)
	{
		Building* demb = &g_building[*dembit];
		std::list<CapSup>::iterator cit = demb->capsup.begin();

		while(cit != demb->capsup.end())
		{
			if(cit->src == supbi &&
				cit->rtype == ri)
			{
				cit = demb->capsup.erase(cit);
				continue;
			}

			++cit;
		}

		demb->adjcaps();
	}
}

void DemAdjCaps(uint8_t ri, int32_t dembi)
{
	Building* demb = &g_building[dembi];
	std::list<CapSup>::iterator cit = demb->capsup.begin();
	
	std::list<int32_t> sups;

	while(cit != demb->capsup.end())
	{
		if(cit->dst == dembi &&
			cit->rtype == ri)
		{
			int32_t supbi = cit->src;
			sups.push_back(supbi);
			cit = demb->capsup.erase(cit);
			continue;
		}

		++cit;
	}

	sups.unique();

	for(std::list<int32_t>::iterator supbit=sups.begin(); supbit!=sups.end(); ++supbit)
	{
		Building* supb = &g_building[*supbit];
		std::list<CapSup>::iterator cit = supb->capsup.begin();
	
		while(cit != supb->capsup.end())
		{
			if(cit->dst == dembi &&
				cit->rtype == ri)
			{
				cit = supb->capsup.erase(cit);
				continue;
			}
			else
				++cit;
		}
	}

	demb->adjcaps();
}

//update capacities - produce electricity, buy
void Building::adjcaps()
{
	int32_t bi = this - g_building;
	BlType* bt = &g_bltype[type];
	
	CycleHist* currch = &*cyclehist.rbegin();

	//cap the supply to other buildings of this building's cap output resource types
	int32_t minlevel = maxprod();

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(!r->capacity)
			continue;

		int32_t produced = minlevel * bt->output[ri] / RATIO_DENOM;
		//see note in function "adjcap" about "suppliable", also applies to "produced".
		produced = imax(produced, cymet * bt->output[ri] / RATIO_DENOM);
		int32_t used = 0;

		//because this is recalculated each frame but only recorded in intervals of CYCLE_FRAMES, get the greatest for this cycle
		if(produced > currch->prod[ri])
			currch->prod[ri] = produced;

		int32_t consumed = 0;

		if(ri == RES_HOUSING)
			consumed += occupier.size();
		
		std::list<CapSup>::iterator csit=capsup.begin();
		while(csit!=capsup.end())
		{
			CapSup* cs = &*csit;

			//get users of this bl of this res

			if(cs->rtype != ri)
			{
				csit++;
				continue;
			}

			//measure consumption both by this building (from others) and of this building (by others)
			consumed += cs->amt;

#if 0
			//is this resource building consumed by this building?
			if(cs->dst == bi)
			{
				//then record it
				consumed += cs->amt;
			}
#endif

			//is this resource being supplied by this building?
			if(cs->src != bi)
			{
				csit++;
				continue;	//this must NOT be an indicator of a user of this bl
			}

			//remove this user
			if(used >= produced)
			{
				Building* b2 = &g_building[cs->dst];
				std::list<CapSup>::iterator csit2=b2->capsup.begin();

				while(csit2!=b2->capsup.end())
				{
					//adjust the corresponding CapSup entry in the demander bl
					if(csit2->rtype == ri &&
						csit2->src == bi &&
						csit2->amt == csit->amt)
					{
						//InfoMess(g_bltype[b2->type].name, "cap erase");
						csit2 = b2->capsup.erase(csit2);
						break;
					}

					csit2++;
				}

				csit = capsup.erase(csit);
				continue;
			}
			//cap this user?
			else if(used + cs->amt > produced)
			{
				int32_t suphere = produced - used;

				Building* b2 = &g_building[cs->dst];
				std::list<CapSup>::iterator csit2=b2->capsup.begin();

				//adjust the corresponding CapSup entry in the demander bl
				while(csit2!=b2->capsup.end())
				{
					if(csit2->rtype == ri &&
						csit2->src == bi &&
						csit2->amt == csit->amt)
					{
						//InfoMess(g_bltype[b2->type].name, "cap erase");
						csit2->amt = suphere;
						break;
					}

					csit2++;
				}

				cs->amt = suphere;
				used = produced;
			}
			else
				used += cs->amt;

			csit++;
		}
		
		//record it
		if(consumed > currch->cons[ri])
			currch->cons[ri] = consumed;
	}

	//obtain capacity resource type sources...

#if 0
	int32_t total[RESOURCES];
	Zero(total);

	for(std::list<Widget*>::iterator csit=capsup.begin(); csit!=capsup.end(); csit++)
	{
		CapSup* cs = &*csit;

		if(cs->src == bi)
			continue;	//this must be an indicator of a user of this bl

		total[cs->rtype] += cs->amt;
	}

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{

	}
#else

	//repeat: obtain capacity resource type sources

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(!r->capacity)
			continue;

		int32_t req = iceil(bt->input[ri] * prodlevel, RATIO_DENOM);
		int32_t consumed = 0;	//corpd fix 2016

		if(req <= 0)
			continue;

		//if(type == BL_CHEMPL)
		//	InfoMess(g_resource[ri].name.c_str(), "req cap >?");

		//see how much requisite amt is already supplied
		for(std::list<CapSup>::iterator csit=capsup.begin(); csit!=capsup.end(); csit++)
		{
			if(csit->src == bi)
				continue;

			if(csit->rtype != ri)
				continue;
			
			req -= csit->amt;
			consumed += csit->amt;
		}
		
		if(consumed > currch->cons[ri])
			consumed = currch->cons[ri];
		
		if(req == 0)
		{
			//if(type == BL_CHEMPL)
			//	InfoMess(g_resource[ri].name.c_str(), "supd cap");

			continue;
		}

		//if(type == BL_CHEMPL)
		//	InfoMess(g_resource[ri].name.c_str(), "not supd cap");

		if(req < 0)
		{
			//if(type == BL_CHEMPL)
			//	InfoMess(g_resource[ri].name.c_str(), "less cap");

			lesscap(ri, req);
		}
		else if(req > 0)
		{
			//if(type == BL_CHEMPL)
			//	InfoMess(g_resource[ri].name.c_str(), "more cap");

			morecap(ri, req);
		}
	}

#endif

}

//requisition non-physical (hence "ethereal") resource types that aren't capacity types (e.g. crude oil).
//non-physical resources are probably all obtained by conduits other than roads.
//all capacity resource types are non-physical (ethereal), but not all non-physical resources are capacity types.
//edit: nevermind, crude oil is physical. I don't want to abuse the term. so I will check if it is transported by conduits or not.
//TODO need better fun names
void Building::getethereal()
{
	int32_t bi = this - g_building;
	BlType* bt = &g_bltype[type];

	//repeat: obtain capacity resource type sources

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(r->capacity)
			continue;

		//if(r->physical)
		//	continue;

		if(bt->input[ri] <= 0)
			continue;

		if(r->conduit == CD_ROAD)
			continue;

		if(r->conduit == CD_NONE)
			continue;

		int32_t req = bt->input[ri] * prodlevel / RATIO_DENOM;

		req -= stocked[ri];

		if(req <= 0)
			continue;

		getethereal(ri, req);
	}
}

//buy through conduits
void Building::getethereal(int32_t rtype, int32_t amt)
{
	Player* py = &g_player[owner];
	int32_t dempi = owner;
	Player* demp = &g_player[dempi];
	int32_t dembi = this - g_building;

	//import state
	int32_t imsti = (int32_t)(dempi / (FIRMSPERSTATE+1)) * (FIRMSPERSTATE+1);
	Player* imst = &g_player[imsti];

	Resource* r = &g_resource[rtype];
	CdType* ct = &g_cdtype[r->conduit];
	int16_t netw;
	std::list<int16_t>* netwlist;

	//Log("? get ether r"<<r->name<<" amt"<<amt);

	//can't do anything if we're not connected to the grid of whatever conduit type this capacity resource is supplied by
	if(ct->blconduct)
	{
		netw = *(int16_t*)(((char*)this)+ct->netwoff);
		//netw = this->netw[r->conduit];

		if(netw < 0)
			return;
	}
	else
	{
		netwlist = (std::list<int16_t>*)(((char*)this)+ct->netwoff);
		//netwlist = &this->netwlist[r->conduit];

		if(netwlist->size() <= 0)
			return;
	}

	//Vec2i demcmpos = tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	bool found = false;

	//Log("get ether r"<<r->name<<" amt"<<amt);

	do
	{
		//repeatedly find the best priced/distanced (based on utilty cost) supplier of this cap resource,
		//until we either cannot get anymore supplied or we don't need anymore.

		int32_t bestsrc = -1;
		int32_t bestutil = -1;
		int32_t bestamt = -1;
		
		int32_t initprice = 0;	//initial price
		int32_t extariffprice = 0;	//export tariff price component
		int32_t imtariffprice = 0;	//import tariff price component
		int32_t effectprice = 0;	//effective price
		
		//supplying firm or state
		int32_t suppi;
		Player* supp;

		//export state
		int32_t exsti;
		Player* exst;

		for(int32_t bi=0; bi<BUILDINGS; bi++)
		{
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			if(!b->finished)
				continue;

			BlType* bt = &g_bltype[b->type];

			if(bt->output[rtype] <= 0)
				continue;

			int16_t netw2;
			std::list<int16_t>* netwlist2;

			if(ct->blconduct)
			{
				netw2 = *(int16_t*)(((char*)b)+ct->netwoff);
				//netw2 = this->netw[r->conduit];

				if(netw != netw2)
					continue;
			}
			else
			{
				netwlist2 = (std::list<int16_t>*)(((char*)b)+ct->netwoff);
				//netwlist2 = &this->netwlist[r->conduit];

				//bool found = false;	//corpd fix
				found = false;

				//matching road networks? (might be connected to more than one seperate, isolated road grid.)
				for(std::list<int16_t>::iterator nit=netwlist->begin(); nit!=netwlist->end(); nit++)
					for(std::list<int16_t>::iterator nit2=netwlist2->begin(); nit2!=netwlist2->end(); nit2++)
					{
						if(*nit != *nit2)
							continue;

						found = true;
						break;
					}

					if(!found)
						continue;
			}

			int32_t suppliable = b->stocked[rtype];

			if(suppliable <= 0)
				continue;

			int32_t initprice2 = b->price[rtype];	//initial price
			int32_t extariffprice2 = 0;	//export tariff price component
			int32_t imtariffprice2 = 0;	//import tariff price component
			int32_t effectprice2 = initprice2;	//effective price
			//supplying firm or state
			int32_t suppi2 = b->owner;
			Player* supp2 = &g_player[suppi2];
			//export state
			int32_t exsti2 = (int32_t)(suppi2 / (FIRMSPERSTATE+1)) * (FIRMSPERSTATE+1);
			Player* exst2 = &g_player[exsti2];

			//trade between countries?
			if(exsti2 != imsti)
			{
				if(imst->protectionism)
				{
					imtariffprice2 = imst->imtariffratio * initprice2 / RATIO_DENOM;
					effectprice2 += imtariffprice2;
				}
				
				if(exst2->protectionism)
				{
					extariffprice2 = exst2->extariffratio * initprice2 / RATIO_DENOM;
					effectprice2 += extariffprice2;
				}
			}

			if(effectprice2 > py->global[RES_DOLLARS])
				continue;

			//assuming here all capacity resource types are non-physical (thus global, as int32_t as we're connected to the right conduit)
			int32_t util = GlUtil(effectprice2);

			if(bestutil >= 0 && bestutil > util)
				continue;

			bestutil = util;
			bestsrc = bi;
			bestamt = suppliable;
			found = true;

			initprice = initprice2;
			extariffprice = extariffprice2;
			imtariffprice = imtariffprice2;
			effectprice = effectprice2;

			suppi = suppi2;
			supp = supp2;
			exsti = exsti2;
			exst = exst2;

			//Log("found ether");
		}

		if(!found)
			break;

		Building* b = &g_building[bestsrc];

		int32_t supplied = imin(amt, bestamt);
		int32_t divprice = imax(1, effectprice);
		supplied = imin(supplied, py->global[RES_DOLLARS] / divprice);

		if(supplied <= 0)
		{
			//no more suppliable, out of money?
			return;
		}
		
		int32_t paidtotal = supplied * effectprice;

		if(paidtotal > demp->global[RES_DOLLARS])
		{
			return;
		}

		int32_t paidtosup = supplied * initprice;
		int32_t paidtoimst = supplied * imtariffprice;
		int32_t paidtoexst = supplied * extariffprice;
		demp->global[RES_DOLLARS] -= paidtotal;
		//Player* supp = &g_player[b->owner];
		supp->global[RES_DOLLARS] += paidtosup;
		imst->global[RES_DOLLARS] += paidtoimst;
		exst->global[RES_DOLLARS] += paidtoexst;
		stocked[rtype] += supplied;

		if(owner != b->owner)
		{
			//TODO GDP for states, throughput for firms
			py->gnp += paidtotal;
			supp->gnp += paidtotal;

			//TODO currencies
			//TODO to c90
			//TODO memory safety RUDP packets!!!!
			this->varcost[rtype] += paidtotal;
		}

		RichText transx;

		{
			r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", paidtotal);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		{
			r = &g_resource[rtype];
			char numpart[128];
			sprintf(numpart, "%+d", -supplied);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

	if(transx.m_part.size() > 0
#ifdef LOCAL_TRANSX
		&& b->owner == g_localP
#endif
		)
			NewTransx(Vec2i(b->tpos.x*TILE_SIZE, b->tpos.y*TILE_SIZE), &transx);

		amt -= supplied;

	}while(found && amt > 0);
}

//get the maximum production level we can attain given our target and input levels
int32_t Building::maxprod()
{
	BlType* bt = &g_bltype[type];
	int32_t bi = this - g_building;

	int32_t total[RESOURCES];
	Zero(total);
	Player* py = &g_player[owner];

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		total[ri] += stocked[ri];

		//must still be obtained from a local resource, even if available globally (that local source can obtain it globally as an output)
		//if(r->physical)
		//	total[ri] += py->global[ri];
	}

	for(std::list<CapSup>::iterator csit=capsup.begin(); csit!=capsup.end(); csit++)
	{
		CapSup* cs = &*csit;

		if(cs->src == bi)
			continue;	//this must be an indicator of a user of this bl

		total[cs->rtype] += cs->amt;
	}

	int32_t minbund[RESOURCES];
	Zero(minbund);

	int32_t minr = -1;
	int32_t minamt = -1;
	int32_t minlevel = prodlevel;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		//if(minr >= 0 && bt->input[ri] >= minamt)
		//	continue;

		//the minimum level of production afforded by the bottleneck input
		//resource (whichever one is restricting us from producing more).
		//this gives us a ratio out of max of RATIO_DENOM.
		//warning: there's potential for int32_t overflow if total[minr] is some big number,
		//if the user is measuring resources in large amounts.
		int32_t subminlevel = total[ri] * RATIO_DENOM / bt->input[ri];

		if(subminlevel > minlevel)
			continue;

		minlevel = subminlevel;
		minr = ri;
		minamt = bt->input[ri];
	}

	return minlevel;
}

//try to produce outputs if a minimum bundle of input resources is had
bool Building::tryprod()
{
	//TO DO: produce, update cymet
	if(g_simframe - lastcy >= CYCLE_FRAMES)
	{
		cymet = 0;
		lastcy = g_simframe;
		//return false;
		
		//CycleHist* ch = &*cyclehist.rbegin();
		cyclehist.push_back(CycleHist());

		while(cyclehist.size() > 100)
			cyclehist.erase(cyclehist.begin());

		int32_t labreq = netreq(RES_LABOUR);

		if(labreq)
			//NewJob(UMODE_BLJOB, (this-g_building), -1, CD_NONE);
			//2016/05/04 fixed "shedding" because of incorrect job mode
			NewJob(UMODE_GOBLJOB, (this-g_building), -1, CD_NONE);
	}

#ifdef PROD_DEBUG
	if(type != BL_NUCPOW)
		return false;
#endif

	BlType* bt = &g_bltype[type];
	int32_t bi = this - g_building;
	Player* py = &g_player[owner];

	//Collect "ethereal" resorce bills?
	//and set lastch wages,prices
	if(lastcy == g_simframe)
	{
		CycleHist* lastch = &*cyclehist.rbegin();
		
		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			lastch->price[ri] = price[ri];
		}
		lastch->wage = opwage;

		std::list<CapSup>::iterator csit=capsup.begin();
		while(csit!=capsup.end())
		{
			//not originating from this bl?
			if(csit->src != bi)
			{
				csit++;
				continue;
			}

			Building* destb = &g_building[csit->dst];

			int32_t srcpi = owner;
			int32_t destpi = destb->owner;

			if(srcpi == destpi)
			{
				csit++;
				continue;
			}

			Player* destp = &g_player[destpi];

			int32_t subtract = price[csit->rtype] * csit->amt;

			//char msg[128];
			//sprintf(msg, "sub e tot%d amt%d pr%d", subtract, csit->amt, price[csit->rtype]);
			//InfoMess(msg, msg);

			CycleHist* lastchdest = NULL;

			if(destb->cyclehist.size() > 1)
				lastchdest = &*(destb->cyclehist.rbegin());

			if(destp->global[RES_DOLLARS] < subtract)
			{
				subtract = destp->global[RES_DOLLARS];
				destp->global[RES_DOLLARS] = 0;
				py->global[RES_DOLLARS] += subtract;
				lastch->prod[RES_DOLLARS] += subtract;

				if(lastchdest)
					lastchdest->cons[RES_DOLLARS] += subtract;

				csit = capsup.erase(csit);
				continue;
			}
		
			destp->global[RES_DOLLARS] -= subtract;
			py->global[RES_DOLLARS] += subtract;
			lastch->prod[RES_DOLLARS] += subtract;

			lastch->cons[csit->rtype] += csit->amt;
			
			if(lastchdest)
				lastchdest->cons[RES_DOLLARS] += subtract;

			csit++;
		}
	}

	//Collect rent?
	if(bt->output[RES_HOUSING] > 0 &&
		lastcy == g_simframe)
	{
		std::list<int32_t> toevict;
		int32_t rent = price[RES_HOUSING];
		
		CycleHist* lastch = &*(cyclehist.rbegin());
		
		//tenants
		for(std::list<int32_t>::iterator tit=occupier.begin(); tit!=occupier.end(); tit++)
		{
			Unit* u = &g_unit[*tit];

			if(!u->on)
			{
				toevict.push_back(*tit);
				continue;
			}

			int32_t* has = &u->belongings[RES_DOLLARS];
			int32_t sub = 0;

			if(*has < rent)
			{
				toevict.push_back(*tit);
				py->gnp += *has;
				py->global[RES_DOLLARS] += *has;
				lastch->prod[RES_DOLLARS] += *has;
				sub = *has;
				*has = 0;
			}
			else
			{
				py->gnp += rent;
				py->global[RES_DOLLARS] += rent;
				lastch->prod[RES_DOLLARS] += rent;
				*has -= rent;
				sub = rent;
			}
			
			lastch->prod[RES_HOUSING] += 1;

			RichText transx;
			{
				Resource* r = &g_resource[RES_DOLLARS];
				char numpart[128];
				sprintf(numpart, "%+d", sub);
				transx.m_part.push_back( RichPart( numpart ) );
				transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
				//transx.m_part.push_back( RichPart( r->name.c_str() ) );
				transx.m_part.push_back( RichPart( " \n" ) );
			}
#ifdef LOCAL_TRANSX
			if( owner == g_localP )
#endif
				NewTransx(u->cmpos, &transx);
		}

		for(std::list<int32_t>::iterator tit=toevict.begin(); tit!=toevict.end(); tit++)
			Evict(&g_unit[*tit], false);
	}

	//if we just completed a cycle and need labour, announce the new job
	if(g_simframe-lastcy == 0 &&
		netreq(RES_LABOUR) > 0)
		NewJob(UMODE_GOBLJOB, bi, -1, CD_NONE);

	//already met target, don't try to produce again for now
	if(cymet >= prodlevel)
		return false;

	int32_t minlevel = maxprod();

#ifdef PROD_DEBUG
	Log("min=r "<<minr<<" "<<g_resource[minr].name);
#endif


#ifdef PROD_DEBUG
	Log("minlevel "<<minlevel<<" "<<g_resource[minr].name);
#endif


	int32_t minbund[RESOURCES];
	Zero(minbund);
	bool somecon = false;

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		minbund[ri] = bt->input[ri] * minlevel / RATIO_DENOM;

		Resource* r = &g_resource[ri];

		//prevent constant sound effect of production
		//for bl's that don't consume anything or have a
		//constant capacity consumption of a single input
		if(minbund[ri] > 0 &&
			!r->capacity)
			somecon = true;
	}

	if(!somecon)
		return false;

	//enough to produce, go ahead

	cymet += minlevel;
	cymet = imin(cymet, RATIO_DENOM);
	RichText transx;
	
	CycleHist* lastch = &*(cyclehist.rbegin());

	//create output resources
	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(r->capacity)
		{
			//TO DO: check
			continue;
		}

		//add an amount of output based on the minimum level of production afforded by the
		//bottleneck input resource (whichever one is restricting us from producing more).
		int32_t add = bt->output[ri] * minlevel / RATIO_DENOM;
		stocked[ri] += add;
		
		lastch->prod[ri] += add;

		//TO DO: capacity, what if not enough supplied

		if(add > 0)
		{
			char numpart[128];
			sprintf(numpart, "%+d", add);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}
	}

	//subtract used raw inputs
	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		int32_t take = minbund[ri];

#if 0	//only local subtractions. only outputs can be subtracted globally by demanders/transporters.
		//subtract from global stock as higher priority. only local when global runs out.
		if(r->physical)
		{
			int32_t takeg = imin(take, py->global[ri]);
			py->global[ri] -= takeg;
			take -= takeg;
			
			lastch->cons[ri] += takeg;
		}
#endif

		int32_t takel = 0;

		if(!r->capacity)
		{
			//remainder from local
			takel = imin(take, stocked[ri]);
			py->local[ri] -= takel;
			stocked[ri] -= takel;
			take -= takel;
			
			lastch->cons[ri] += takel;
		}
		else
		{
			for(std::list<CapSup>::iterator cit=capsup.begin(); cit!=capsup.end(); ++cit)
			{
				if(cit->rtype == ri &&
					cit->dst == bi)
					takel += cit->amt;
			}

			//if(ri == RES_HOUSING)
			//	takel += occupier.size();
			
			lastch->cons[ri] += takel;
		}

		if(takel > 0)
		{
			char numpart[128];
			sprintf(numpart, "%+d", -takel);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}
	}

	if(transx.m_part.size() > 0
#ifdef LOCAL_TRANSX
		&& owner == g_localP
#endif
		)
	{
		Vec2i cmpos = Vec2i(tpos.x*TILE_SIZE, tpos.y*TILE_SIZE);
		NewTransx(cmpos, &transx);

		if(g_mapview[0].x <= cmpos.x && g_mapview[0].y <= cmpos.y &&
			g_mapview[1].x >= cmpos.x && g_mapview[1].y >= cmpos.y)
			PlayClip(bt->sound[BLSND_PROD]);
	}

	//TO DO: capacity resources like electricity have to be handled completely differently
	//amount input depends on required output, depends on duration of cycle, must be upward limited by max gen capacity of that res for that bl type
	//so lower bound of cycle at 60 sec, and upper bound? for those bl's that output capacity? or just disallow change cycle delay for those types of bl's?

	//cymet = cymet % RATIO_DENOM;

	return true;
}

void RemWorker(Unit* w)
{
	Building* b = &g_building[w->target];

	int32_t ui = w - g_unit;

	for(std::list<int32_t>::iterator witer=b->worker.begin(); witer!=b->worker.end(); witer++)
		if(*witer == ui)
		{
			b->worker.erase(witer);
			return;
		}
}

void RemShopper(Unit* w)
{
	Building* b = &g_building[w->target];

	int32_t ui = w - g_unit;

	for(std::list<int32_t>::iterator witer=b->occupier.begin(); witer!=b->occupier.end(); witer++)
		if(*witer == ui)
		{
			b->occupier.erase(witer);
			return;
		}
}

void FreeBls()
{
	for(int32_t i=0; i<BUILDINGS; i++)
	{
		g_building[i].destroy();
		g_building[i].on = false;
	}
}

int32_t NewBl()
{
	for(int32_t i=0; i<BUILDINGS; i++)
		if(!g_building[i].on)
			return i;

	return -1;
}

float CompletPct(int32_t* cost, int32_t* current)
{
	int32_t totalreq = 0;

	for(int32_t i=0; i<RESOURCES; i++)
	{
		totalreq += cost[i];
	}

	int32_t totalhave = 0;

	for(int32_t i=0; i<RESOURCES; i++)
	{
		totalhave += imin(cost[i], current[i]);
	}

	return (float)totalhave/(float)totalreq;
}

void Building::allocres()
{
	BlType* t = &g_bltype[type];
	Player* py = &g_player[owner];

	int32_t alloc;

	RichText transx;
	
	CycleHist* currch = &*cyclehist.rbegin();

	for(int32_t i=0; i<RESOURCES; i++)
	{
		if(t->conmat[i] <= 0)
			continue;

		if(i == RES_LABOUR)
			continue;

		alloc = t->conmat[i] - conmat[i];

		if(py->global[i] < alloc)
			alloc = py->global[i];

		conmat[i] += alloc;
		py->global[i] -= alloc;

		if(i == RES_DOLLARS)
			py->gnp += alloc;

		currch->cons[i] += alloc;
		
		Resource* r = &g_resource[i];

		if(alloc > 0)
		{
			char numpart[128];
			sprintf(numpart, "%+d", -alloc);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( "\n" ) );
		}
	}

	if(transx.m_part.size() > 0
#ifdef LOCAL_TRANSX
		&& owner == g_localP
#endif
		)
		NewTransx(Vec2i(tpos.x*TILE_SIZE, tpos.y*TILE_SIZE), &transx);

	checkconstruction();
}

bool Building::checkconstruction()
{
	BlType* t = &g_bltype[type];

	bool haveall = true;

	for(int32_t i=0; i<RESOURCES; i++)
		if(conmat[i] < t->conmat[i])
		{
			haveall = false;
			break;
		}

	if(haveall && !finished)
		for(char ctype=0; ctype<CD_TYPES; ctype++)
			ReNetw(ctype);

	//if(owner == g_localP)
	//	OnFinishedB(type);

	bool prevfini = finished;
	finished = haveall;

	if(finished && !prevfini)
	{
		if(owner == g_localP)
		{
			char inform[128];
			sprintf(inform, "%s ", t->name);
			RichText rt = RichText(inform) + STRTABLE[STR_CONCOMP];
			AddNotif(&rt);
			//ConCom();
		}

		NewJob(UMODE_GOBLJOB, (int32_t)(this-g_building), -1, CD_NONE);

		Vec2i cmpos = Vec2i(tpos.x*TILE_SIZE, tpos.y*TILE_SIZE);

		if(g_mapview[0].x <= cmpos.x && g_mapview[0].y <= cmpos.y &&
			g_mapview[1].x >= cmpos.x && g_mapview[1].y >= cmpos.y)
			PlayClip(t->sound[BLSND_FINI]);

		hp = t->maxhp;

		Vec2i tmin;
		Vec2i tmax;

		tmin.x = tpos.x - t->width.x/2;
		tmin.y = tpos.y - t->width.y/2;
		tmax.x = tmin.x + t->width.x;
		tmax.y = tmin.y + t->width.y;

		int32_t cmminx = tmin.x*TILE_SIZE;
		int32_t cmminy = tmin.y*TILE_SIZE;
		int32_t cmmaxx = cmminx + t->width.x*TILE_SIZE - 1;
		int32_t cmmaxy = cmminy + t->width.y*TILE_SIZE - 1;

		ClearFol(cmminx, cmminy, cmmaxx, cmmaxy);
		
		Zero(conmat);

		MarkTerr(this);
	}

	return finished;
}

void DrawBl(Building* b, float rendz, uint32_t renderdepthtex, uint32_t renderfb)
{
	StartTimer(TIMER_DRAWBL);
#if 0
	if(g_build == BL_TRFAC)
	{
		Ortho(g_width,g_height,1,1,1,1);
		DrawImage(g_texture[g_sprite[g_splist[g_bltype[BL_TRFAC].splist].sprites[0]].depthtexi].texname,
			0,50,100,150,0,0,1,1,g_gui.m_crop);
		DrawImage(g_texture[g_sprite[g_splist[g_bltype[BL_TRFAC].splist].sprites[0]].difftexi].texname,
			0,50,200,150,0,0,1,1,g_gui.m_crop);

		//glBindBuffer(GL_TEXTURE_2D,g_texture[g_sprite[g_splist[g_bltype[BL_TRFAC].splist].sprites[0]].difftexi].texname);
		//glReadBuffer(GL_TEXTURE_2D);
		//glCopyPixels(150,0,100,100,GL_COLOR);
	}

#if 1
	UseS(DEEPSHADER);
	Shader* s = &g_shader[g_curS];
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
#endif
#endif
	Shader* s = &g_shader[g_curS];

	const BlType* t = &g_bltype[b->type];
	//const BlType* t = &g_bltype[BL_HOUSE1];

	Depthable* d = b->depth;

	float alpha = 1.0f;

	if(g_mouse3d.x + TILE_SIZE > d->cmmin.x &&
		g_mouse3d.x - TILE_SIZE < d->cmmax.x &&
		g_mouse3d.y + TILE_SIZE > d->cmmin.y &&
		g_mouse3d.y - TILE_SIZE < d->cmmax.y)
	{
		//glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 0.5f);
		alpha = 0.5f;
	}

	int16_t tx = b->tpos.x;
	int16_t ty = b->tpos.y;

	if(IsTileVis(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, alpha);
	else if(Explored(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, alpha);
	else
	{
		StopTimer(TIMER_DRAWBL);
		return;
		//goto end;
	}

	if(BSel(b - g_building))
	{
#ifndef ISOTOP
		glDisable(GL_DEPTH_TEST);
#endif
		UseS(SHADER_COLOR2D);
		s = &g_shader[g_curS];
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

		/*
		TODO
		Cleaner way to do this, reusing parts,
		and not calculating unecessary things.
		*/
		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, g_hmap.getheight(d->cmmin.x/TILE_SIZE, d->cmmin.y/TILE_SIZE)*TILE_RISE);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, g_hmap.getheight(d->cmmax.x/TILE_SIZE, d->cmmax.y/TILE_SIZE)*TILE_RISE);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, g_hmap.getheight(d->cmmin.x/TILE_SIZE, d->cmmax.y/TILE_SIZE)*TILE_RISE);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, g_hmap.getheight(d->cmmax.x/TILE_SIZE, d->cmmin.y/TILE_SIZE)*TILE_RISE);

		//Make it 1 pixel higher so that adjacent buildings in front don't block the bottom lines
		Vec2i top = CartToIso(top3) - g_scroll + Vec2i(0,-1);
		Vec2i bot = CartToIso(bot3) - g_scroll + Vec2i(0,-1);
		Vec2i lef = CartToIso(lef3) - g_scroll + Vec2i(0,-1);
		Vec2i rig = CartToIso(rig3) - g_scroll + Vec2i(0,-1);

		//glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 0.5f);
		glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 1.0f);

		const float vertices[] =
		{
			//posx, posy
			(float)lef.x, (float)lef.y, 0,
			(float)top.x, (float)top.y, 0,
			(float)rig.x, (float)rig.y, 0
		};

#ifdef PLATFORM_GL14
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
#endif

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glDrawArrays(GL_LINE_STRIP, 0, 3);

		EndS();

#ifndef ISOTOP
		UseS(DEEPSHADER);
#else
		UseS(SHADER_ORTHO);
#endif
		s = &g_shader[g_curS];
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
#ifndef ISOTOP
		glEnable(GL_DEPTH_TEST);
#endif
	}

	//Sprite* sp = NULL;

	//Vec3f vmin(b->drawpos.x - t->width.x*TILE_SIZE/2, b->drawpos.y, b->drawpos.y - t->width.y*TILE_SIZE/2);
	//Vec3f vmax(b->drawpos.x + t->width.x*TILE_SIZE/2, b->drawpos.y + (t->width.x+t->width.y)*TILE_SIZE/2, b->drawpos.y + t->width.y*TILE_SIZE/2);

	//if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
	//	continue;

	Tile tile = SurfTile(b->tpos.x, b->tpos.y, &g_hmap);
	int32_t sli = t->splist;
	if(!b->finished)
		sli = t->csplist;
	SpList* sl = &g_splist[ sli ];
	int32_t ci = SpriteRef(sl, 0, tile.incltype, 0, 0, 0, 0, 0);
	Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

	Player* py = &g_player[b->owner];
	float* color = py->color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

	Vec3i cmpos = Vec3i( b->tpos.x * TILE_SIZE + ((t->width.x % 2 == 1) ? TILE_SIZE/2 : 0), b->tpos.y * TILE_SIZE + ((t->width.y % 2 == 1) ? TILE_SIZE/2 : 0), 
		SurfTile(b->tpos.x, b->tpos.y, &g_hmap).elev * TILE_RISE );
	Vec2i screenpos = CartToIso(cmpos) - g_scroll;
	
	glUniform1f(s->slot[SSLOT_BASEELEV], cmpos.z);
	
	Texture* difftex = &g_texture[ sp->difftexi ];
	Texture* depthtex = &g_texture[ sp->depthtexi ];
	Texture* teamtex = &g_texture[ sp->teamtexi ];

#if 0
	DrawImage(difftex->texname,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->cropoff[0]/(float)difftex->width, sp->cropoff[1]/(float)difftex->height, 
		sp->cropoff[2]/(float)difftex->width, sp->cropoff[3]/(float)difftex->height,
		g_gui.m_crop);
#else
	DrawDeep(difftex->texname, depthtex->texname, teamtex->texname, rendz, cmpos.z,
		renderdepthtex, renderfb,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->crop[0], sp->crop[1],
		sp->crop[2], sp->crop[3]);
#endif

	//glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

	if(BSel(b - g_building))
	{
#ifndef ISOTOP
		glDisable(GL_DEPTH_TEST);
#endif
		UseS(SHADER_COLOR2D);
		s = &g_shader[g_curS];
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

		Depthable* d = b->depth;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, g_hmap.getheight(d->cmmin.x/TILE_SIZE, d->cmmin.y/TILE_SIZE)*TILE_RISE);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, g_hmap.getheight(d->cmmax.x/TILE_SIZE, d->cmmax.y/TILE_SIZE)*TILE_RISE);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, g_hmap.getheight(d->cmmin.x/TILE_SIZE, d->cmmax.y/TILE_SIZE)*TILE_RISE);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, g_hmap.getheight(d->cmmax.x/TILE_SIZE, d->cmmin.y/TILE_SIZE)*TILE_RISE);

		//Make it 1 pixel higher so that adjacent buildings in front don't block the bottom lines
		Vec2i top = CartToIso(top3) - g_scroll + Vec2i(0,-1);
		Vec2i bot = CartToIso(bot3) - g_scroll + Vec2i(0,-1);
		Vec2i lef = CartToIso(lef3) - g_scroll + Vec2i(0,-1);
		Vec2i rig = CartToIso(rig3) - g_scroll + Vec2i(0,-1);

		//glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 0.5f);
		glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 1.0f);

		const float vertices[] =
		{
			//posx, posy
			(float)lef.x, (float)lef.y, 0,
			(float)bot.x, (float)bot.y, 0,
			(float)rig.x, (float)rig.y, 0
		};
		
#ifdef PLATFORM_GL14
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
#endif

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glDrawArrays(GL_LINE_STRIP, 0, 3);

#ifndef ISOTOP
		UseS(DEEPSHADER);
#else
		UseS(SHADER_ORTHO);
#endif
		s = &g_shader[g_curS];
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
#ifndef ISOTOP
		glEnable(GL_DEPTH_TEST);
#endif
	}

end:
	StopTimer(TIMER_DRAWBL);
}

void DrawBl2(Building* b, float rendz, uint32_t renderdepthtex, uint32_t renderfb)
{
	StartTimer(TIMER_DRAWBL);
#if 0
	if(g_build == BL_TRFAC)
	{
		Ortho(g_width,g_height,1,1,1,1);
		DrawImage(g_texture[g_sprite[g_splist[g_bltype[BL_TRFAC].splist].sprites[0]].depthtexi].texname,
			0,50,100,150,0,0,1,1,g_gui.m_crop);
		DrawImage(g_texture[g_sprite[g_splist[g_bltype[BL_TRFAC].splist].sprites[0]].difftexi].texname,
			0,50,200,150,0,0,1,1,g_gui.m_crop);

		//glBindBuffer(GL_TEXTURE_2D,g_texture[g_sprite[g_splist[g_bltype[BL_TRFAC].splist].sprites[0]].difftexi].texname);
		//glReadBuffer(GL_TEXTURE_2D);
		//glCopyPixels(150,0,100,100,GL_COLOR);
	}

#if 1
	UseS(DEEPSHADER);
	Shader* s = &g_shader[g_curS];
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
#endif
#endif
	Shader* s = &g_shader[g_curS];

	const BlType* t = &g_bltype[b->type];
	//const BlType* t = &g_bltype[BL_HOUSE1];

	Depthable* d = b->depth;

	float alpha = 1.0f;

	if(g_mouse3d.x + TILE_SIZE > d->cmmin.x &&
		g_mouse3d.x - TILE_SIZE < d->cmmax.x &&
		g_mouse3d.y + TILE_SIZE > d->cmmin.y &&
		g_mouse3d.y - TILE_SIZE < d->cmmax.y)
	{
		//glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 0.5f);
		alpha = 0.5f;
	}

	int16_t tx = b->tpos.x;
	int16_t ty = b->tpos.y;

	if(IsTileVis(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, alpha);
	else if(Explored(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, alpha);
	else
	{
		StopTimer(TIMER_DRAWBL);
		return;
		//goto end;
	}

	if(BSel(b - g_building))
	{
#ifndef ISOTOP
		glDisable(GL_DEPTH_TEST);
#endif
		UseS(SHADER_COLOR2D);
		s = &g_shader[g_curS];
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

		/*
		TODO
		Cleaner way to do this, reusing parts,
		and not calculating unecessary things.
		*/
		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, g_hmap.getheight(d->cmmin.x/TILE_SIZE, d->cmmin.y/TILE_SIZE)*TILE_RISE);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, g_hmap.getheight(d->cmmax.x/TILE_SIZE, d->cmmax.y/TILE_SIZE)*TILE_RISE);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, g_hmap.getheight(d->cmmin.x/TILE_SIZE, d->cmmax.y/TILE_SIZE)*TILE_RISE);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, g_hmap.getheight(d->cmmax.x/TILE_SIZE, d->cmmin.y/TILE_SIZE)*TILE_RISE);

		//Make it 1 pixel higher so that adjacent buildings in front don't block the bottom lines
		Vec2i top = CartToIso(top3) - g_scroll + Vec2i(0,-1);
		Vec2i bot = CartToIso(bot3) - g_scroll + Vec2i(0,-1);
		Vec2i lef = CartToIso(lef3) - g_scroll + Vec2i(0,-1);
		Vec2i rig = CartToIso(rig3) - g_scroll + Vec2i(0,-1);

		//glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 0.5f);
		glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 1.0f);

		const float vertices[] =
		{
			//posx, posy
			(float)lef.x, (float)lef.y, 0,
			(float)top.x, (float)top.y, 0,
			(float)rig.x, (float)rig.y, 0
		};

#ifdef PLATFORM_GL14
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
#endif

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glDrawArrays(GL_LINE_STRIP, 0, 3);

		EndS();

#ifndef ISOTOP
		UseS(DEEPSHADER);
#else
		UseS(SHADER_ORTHO);
#endif
		s = &g_shader[g_curS];
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
#ifndef ISOTOP
		glEnable(GL_DEPTH_TEST);
#endif
	}

	//Sprite* sp = NULL;

	//Vec3f vmin(b->drawpos.x - t->width.x*TILE_SIZE/2, b->drawpos.y, b->drawpos.y - t->width.y*TILE_SIZE/2);
	//Vec3f vmax(b->drawpos.x + t->width.x*TILE_SIZE/2, b->drawpos.y + (t->width.x+t->width.y)*TILE_SIZE/2, b->drawpos.y + t->width.y*TILE_SIZE/2);

	//if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
	//	continue;

	Tile tile = SurfTile(b->tpos.x, b->tpos.y, &g_hmap);
	int32_t sli = t->splist;
	if(!b->finished)
		sli = t->csplist;

	PlayAnim(b->frame, 0, t->nframes, true, 23.0f/30.0f * g_drawfrinterval * 100.0f);

	SpList* sl = &g_splist[ sli ];
	int32_t ci = SpriteRef(sl, (b->finished)?(int)b->frame:0, tile.incltype, 0, 0, 0, 0, 0);
	Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

	Player* py = &g_player[b->owner];
	float* color = py->color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

	Vec3i cmpos = Vec3i( b->tpos.x * TILE_SIZE + ((t->width.x % 2 == 1) ? TILE_SIZE/2 : 0), b->tpos.y * TILE_SIZE + ((t->width.y % 2 == 1) ? TILE_SIZE/2 : 0), 
		SurfTile(b->tpos.x, b->tpos.y, &g_hmap).elev * TILE_RISE );
	Vec2i screenpos = CartToIso(cmpos) - g_scroll;
	
	glUniform1f(s->slot[SSLOT_BASEELEV], cmpos.z);
	
	Texture* difftex = &g_texture[ sp->difftexi ];
	Texture* depthtex = &g_texture[ sp->depthtexi ];
	Texture* teamtex = &g_texture[ sp->teamtexi ];
	Texture* elevtex = &g_texture[ sp->elevtexi ];

#if 0
	DrawImage(difftex->texname,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->cropoff[0]/(float)difftex->width, sp->cropoff[1]/(float)difftex->height, 
		sp->cropoff[2]/(float)difftex->width, sp->cropoff[3]/(float)difftex->height,
		g_gui.m_crop);
#else
	DrawDeep2(difftex->texname, depthtex->texname, teamtex->texname, elevtex->texname,
		renderdepthtex, renderfb,
		rendz, cmpos.z,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->crop[0], sp->crop[1],
		sp->crop[2], sp->crop[3]);
#endif

	//glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

	if(BSel(b - g_building))
	{
#ifndef ISOTOP
		glDisable(GL_DEPTH_TEST);
#endif
		UseS(SHADER_COLOR2D);
		s = &g_shader[g_curS];
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

		Depthable* d = b->depth;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, g_hmap.getheight(d->cmmin.x/TILE_SIZE, d->cmmin.y/TILE_SIZE)*TILE_RISE);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, g_hmap.getheight(d->cmmax.x/TILE_SIZE, d->cmmax.y/TILE_SIZE)*TILE_RISE);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, g_hmap.getheight(d->cmmin.x/TILE_SIZE, d->cmmax.y/TILE_SIZE)*TILE_RISE);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, g_hmap.getheight(d->cmmax.x/TILE_SIZE, d->cmmin.y/TILE_SIZE)*TILE_RISE);

		//Make it 1 pixel higher so that adjacent buildings in front don't block the bottom lines
		Vec2i top = CartToIso(top3) - g_scroll + Vec2i(0,-1);
		Vec2i bot = CartToIso(bot3) - g_scroll + Vec2i(0,-1);
		Vec2i lef = CartToIso(lef3) - g_scroll + Vec2i(0,-1);
		Vec2i rig = CartToIso(rig3) - g_scroll + Vec2i(0,-1);

		//glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 0.5f);
		glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 1.0f);

		const float vertices[] =
		{
			//posx, posy
			(float)lef.x, (float)lef.y, 0,
			(float)bot.x, (float)bot.y, 0,
			(float)rig.x, (float)rig.y, 0
		};
		
#ifdef PLATFORM_GL14
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
#endif

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glDrawArrays(GL_LINE_STRIP, 0, 3);

#ifndef ISOTOP
		UseS(DEEPSHADER);
#else
		UseS(SHADER_ORTHO);
#endif
		s = &g_shader[g_curS];
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
#ifndef ISOTOP
		glEnable(GL_DEPTH_TEST);
#endif
	}

end:
	StopTimer(TIMER_DRAWBL);
}

void UpdBls()
{
	//if(g_simframe % BL_AI_DELAY != 0)
	//	return;
	
	for(int32_t i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		RemVis(b);
		AddVis(b);

#if 0
		if(rand()%100 == 1)
		{
			Explode(b);
			b->on = false;
			continue;
		}
#endif

		BlType* t = &g_bltype[b->type];
		EmitterPlace* ep;
		PlType* pt;

		if(!b->finished)
			continue;
		
#ifdef FREEZE_DEBUG
		Log("upcaps");
		
#endif

		b->adjcaps();

#ifdef FREEZE_DEBUG
		Log("getethereal");
		
#endif

		b->getethereal();

#ifdef FREEZE_DEBUG
		Log("tryprod ");
		
#endif

		b->tryprod();

#ifdef FREEZE_DEBUG
		Log("trymanuf");
		
#endif

		b->trymanuf();

		//AI adjust prices,wages
		if(b->lastcy == g_simframe)
		{
			Player* py = &g_player[b->owner];
			if(py->on && py->ai)
			{
				AdjPrWg(py, b);
				AdjProd(py, b);

				for(uint8_t ri=0; ri<RESOURCES; ri++)
				{
					if(t->input[ri] <= 0)
						continue;

					Resource* r = &g_resource[ri];

					if(!r->capacity)
						continue;

					DemAdjCaps(ri, i);
				}
			}
		}

		//emit decorations
		if(b->cymet > 0)
			for(int32_t j=0; j<MAX_B_EMITTERS; j++)
			{
				//first = true;

				//if(completion < 1)
				//	continue;

				ep = &t->emitterpl[j];

				if(!ep->on)
					continue;

				pt = &g_particleT[ep->type];

				//TODO
				if(b->emitterco[j].emitnext(pt->delay))
				{
					Vec3f cmpos = Vec3f(
						b->tpos.x * TILE_SIZE + ((t->width.x % 2 == 1) ? TILE_SIZE/2 : 0), 
						SurfTile(b->tpos.x,b->tpos.y, &g_hmap).elev*TILE_RISE,
						b->tpos.y * TILE_SIZE + ((t->width.y % 2 == 1) ? TILE_SIZE/2 : 0) );
					EmitParticle(ep->type, cmpos + ep->offset);
				}
			}
	}
}

void StageCopyVA(VertexArray* to, VertexArray* from, float completion)
{
	CopyVA(to, from);

	float maxy = 0;
	for(int32_t i=0; i<to->numverts; i++)
		if(to->vertices[i].y > maxy)
			maxy = to->vertices[i].y;

	float limity = maxy*completion;

	for(int32_t tri=0; tri<to->numverts/3; tri++)
	{
		Vec3f* belowv = NULL;
		Vec3f* belowv2 = NULL;

		for(int32_t v=tri*3; v<tri*3+3; v++)
		{
			if(to->vertices[v].y <= limity)
			{
				if(!belowv)
					belowv = &to->vertices[v];
				else if(!belowv2)
					belowv2 = &to->vertices[v];

				//break;
			}
		}

		Vec3f prevt[3];
		prevt[0] = to->vertices[tri*3+0];
		prevt[1] = to->vertices[tri*3+1];
		prevt[2] = to->vertices[tri*3+2];

		Vec2f prevc[3];
		prevc[0] = to->texcoords[tri*3+0];
		prevc[1] = to->texcoords[tri*3+1];
		prevc[2] = to->texcoords[tri*3+2];

		for(int32_t v=tri*3; v<tri*3+3; v++)
		{
			if(to->vertices[v].y > limity)
			{
				float prevy = to->vertices[v].y;
				to->vertices[v].y = limity;
#if 0
#if 0
				void barycent(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2,
					double vx, double vy, double vz,
					double *u, double *v, double *w)
#endif

					double ratio0 = 0;
				double ratio1 = 0;
				double ratio2 = 0;

				barycent(prevt[0].x, prevt[0].y, prevt[0].z,
					prevt[1].x, prevt[1].y, prevt[1].z,
					prevt[2].x, prevt[2].y, prevt[2].z,
					to->vertices[v].x, to->vertices[v].y, to->vertices[v].z,
					&ratio0, &ratio1, &ratio2);

				to->texcoords[v].x = ratio0 * prevc[0].x + ratio1 * prevc[1].x + ratio2 * prevc[2].x;
				to->texcoords[v].y = ratio0 * prevc[0].y + ratio1 * prevc[1].y + ratio2 * prevc[2].y;
#elif 0
				Vec3f* closebelowv = NULL;

				if(belowv)
					closebelowv = belowv;

				if(belowv2 && (!closebelowv || Dot((*closebelowv) - to->vertices[v]) > Dot((*belowv2) - to->vertices[v])))
					closebelowv = belowv2;

				float yratio = (closebelowv->y - prevy);
#endif
			}
		}
	}
}


void HeightCopyVA(VertexArray* to, VertexArray* from, float completion)
{
	CopyVA(to, from);

	float maxy = 0;
	for(int32_t i=0; i<to->numverts; i++)
		if(to->vertices[i].y > maxy)
			maxy = to->vertices[i].y;

	float dy = maxy*(1.0f-completion);

	for(int32_t tri=0; tri<to->numverts/3; tri++)
	{
		Vec3f* belowv = NULL;
		Vec3f* belowv2 = NULL;

		for(int32_t v=tri*3; v<tri*3+3; v++)
		{
			to->vertices[v].y -= dy;
		}
	}
}

void HugTerrain(VertexArray* va, Vec3f pos)
{
	for(int32_t i=0; i<va->numverts; i++)
	{
		va->vertices[i].y += Bilerp(&g_hmap, pos.x + va->vertices[i].x, pos.z + va->vertices[i].z) - pos.y;
	}
}

void Explode(Building* b)
{
	BlType* t = &g_bltype[b->type];
	float hwx = t->width.x*TILE_SIZE/2.0f;
	float hwz = t->width.y*TILE_SIZE/2.0f;
	Vec3f p;

	for(int32_t i=0; i<5; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		//EmitParticle(PARTICLE_FIREBALL, p + b->drawpos);	//TODO
	}

	for(int32_t i=0; i<10; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		//EmitParticle(PARTICLE_FIREBALL2, p + b->drawpos);	//TODO
	}
	/*
	for(int32_t i=0; i<5; i++)
	{
	p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
	p.y = 8;
	p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
	EmitParticle(SMOKE, p + pos);
	}

	for(int32_t i=0; i<5; i++)
	{
	p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
	p.y = 8;
	p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
	EmitParticle(SMOKE2, p + pos);
	}
	*/
	for(int32_t i=0; i<20; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		//EmitParticle(PARTICLE_DEBRIS, p + b->drawpos); //TODO
	}
}
