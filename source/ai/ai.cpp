









#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../sim/build.h"
#include "../sim/simdef.h"
#include "../econ/utility.h"
#include "../sim/unit.h"
#include "../sim/simflow.h"
#include "../net/lockstep.h"
#include "../gui/layouts/chattext.h"
#include "../path/tilepath.h"
#include "../sim/conduit.h"
#include "../net/packets.h"
#include "../gui/widgets/spez/gengraphs.h"
#include "../render/fogofwar.h"
#include "../gui/layouts/messbox.h"

void UpdAI()
{
#if 0
	//protectionism
	//for each state
	for(int32_t i=0; i<PLAYERS; i+=(FIRMSPERSTATE+1))
	{
		Player* st = &g_player[i];
		if(!st->protectionism)
			continue;
		int32_t mainfirm = i+1;
		Player* firm = &g_player[mainfirm];
		int32_t excessfunds = st->global[RES_DOLLARS] - INST_FUNDS;
		excessfunds = imax(0, excessfunds);
		//subsidies
		firm->global[RES_DOLLARS] += excessfunds;
		st->global[RES_DOLLARS] -= excessfunds;
	}
#endif

	//return;	//do nothing for now

	//only host updates AI and issues commands
	if(g_netmode != NETM_HOST &&
		g_netmode != NETM_SINGLE)
		return;

	bool caldem1 = false;

	for(int32_t i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		if(!p->on)
			continue;

		if(!p->ai)
			continue;

		if(g_simframe - p->lastthink < AI_FRAMES)
			continue;

		p->lastthink = g_simframe;

		UpdAI(p);
	}
}

/*
Building proportion number ai build rate

Houses 2 other 1 part

Trial and error wages and prices adjust

Rule if population decreasing then lower prices and increase wages
*/

void UpdAI(Player* p)
{
//#define BUILD_FRAMES		(SIM_FRAME_RATE*1*1/4)
#define BUILD_FRAMES		(NETTURN+13)
	uint64_t blphase = g_simframe / BUILD_FRAMES;
	int32_t pi = p - g_player;
	
	if((blphase+pi+g_simframe) % PLAYERS == 1)
	//if((blphase+g_simframe) % PLAYERS == 1)
		AIBuild(p);
	
	//if((blphase+pi+g_simframe) % PLAYERS == 2)
	//2016/04/27 ai manuf delay decreased
	if((blphase*pi+g_simframe) % (PLAYERS*100/60) == 2)
		AIManuf(p);
	
	//AdjProd(p);
	
	//if((blphase+pi+g_simframe) % PLAYERS == 3)
	if((blphase+g_simframe) % PLAYERS == 3)
		BuyProps(p);

	if((blphase+g_simframe/300) % PLAYERS == 3)
		AdjTrPrWg(p, NULL);

	if((blphase+g_simframe/3000) % PLAYERS == 1)
		AdjCs(p);

#if 0
	for(int32_t ui=0; ui<UNITS; ui++)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(u->owner != p-g_player)
			continue;

		if(u->forsale)
			continue;

		SetSalePropPacket sspp;
		sspp.header.type = PACKET_SETSALEPROP;
		sspp.propi = ui;
		sspp.proptype = PROP_U_BEG + u->type;
		sspp.selling = true;
		sspp.price = 0;
		LockCmd((PacketHeader*)&sspp);
	}
#endif

	//done in tryprod:
	//AdjPrWg(p);

	//RichText msg = RichText("UpdAI");
	//AddNotif(&msg);
}

void AIBuyProp(Player* py, int32_t propi, int32_t proptype, int8_t tx, int8_t ty)
{
	int32_t pyi = py - g_player;

	BuyPropPacket bpp;
	bpp.header.type = PACKET_BUYPROP;
	bpp.pi = pyi;
	bpp.proptype = proptype;
	bpp.propi = propi;
	bpp.tx = tx;
	bpp.ty = ty;
	LockCmd((PacketHeader*)&bpp);
}

void BuyProps(Player* p)
{
	int32_t pi = p - g_player;

	//if(p->global[RES_DOLLARS] < INST_FUNDS / 100000)
	//	return;

	for(int32_t bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		//cse fix
		if(!b->on)
			continue;
		
		if(!b->forsale)
			continue;

		if(!b->finished)
		{
			AIBuyProp(p, bi, PROP_BL_BEG + b->type, -1, -1);
			continue;
		}

		if(b->owner == pi)
			continue;

		if(b->propprice <= 0)
		{
			AIBuyProp(p, bi, PROP_BL_BEG + b->type, -1, -1);
			continue;
		}

		if(b->cyclehist.size() < 2)
			continue;
		
		std::list<CycleHist>::reverse_iterator chit = b->cyclehist.rbegin();
		CycleHist* lastch = &*chit;
		chit++;
		CycleHist* lastch2 = &*chit;

		int32_t profit = lastch->prod[RES_DOLLARS] + 
			lastch2->prod[RES_DOLLARS] - 
			lastch->cons[RES_DOLLARS] -
			lastch2->cons[RES_DOLLARS];

		if((10 + profit) * 2 < b->propprice)
			continue;
		
		AIBuyProp(p, bi, PROP_BL_BEG + b->type, -1, -1);
	}

	for(int32_t ui=0; ui<UNITS; ui++)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(!u->forsale)
			continue;

		if(u->owner == pi)
			continue;

		if(u->price <= 0 ||
			u->price <= (p->transpcost - p->truckwage) * 12)
		{
			AIBuyProp(p, ui, PROP_U_BEG + u->type, -1, -1);
		}
	}

	for(uint8_t ctype=0; ctype<CD_TYPES; ++ctype)
	{
		for(uint8_t tx=0; tx<g_mapsz.x; ++tx)
		{
			for(uint8_t ty=0; ty<g_mapsz.y; ++ty)
			{
				CdTile* ctile = GetCd(ctype, tx, ty, false);

				if(!ctile->on)
					continue;

				if(ctile->owner == pi)
					continue;

				if(!ctile->selling)
					continue;

				//p
				
				AIBuyProp(p, -1, PROP_CD_BEG + ctype, tx, ty);
			}
		}
	}
}

int32_t CountB()
{
	int32_t c = 0;

	for(int32_t i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		c ++;
	}

	return c;
}

//TODO Player* p -> Player* py
void AdjTrPrWg(Player* py, Unit* u)
{
	int32_t pi = py - g_player;
	int32_t newtrwg = py->truckwage;
	int32_t c = 0;
	int32_t jobsgiven = 0;
	int32_t jobsfini = 0;
	int32_t earn = 0;
	int32_t earn2 = 0;
	int32_t trprice = 0;
	int32_t trprice2 = 0;
	int32_t spend = 0;
	int32_t spend2 = 0;

	for(int32_t ui=0; ui<UNITS; ++ui)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(u->owner != pi)
			continue;

		if(u->cyclehist.size() < 3)
		{
			continue;
		}

		std::list<TrCycleHist>::reverse_iterator chit = u->cyclehist.rbegin();

		TrCycleHist* lastch = &*chit;
		chit++;
		TrCycleHist* lastch2 = &*chit;
		
		int32_t lastjobs = lastch->jobsgiven;

#if 0
		//2016/05/02 trying to reduce driver expenses
		if( !lastjobs && 
			u->mode != UMODE_NONE)
			++lastjobs;
#endif

		jobsgiven = iceil( jobsgiven * c + lastjobs, c+1);
		jobsfini = iceil( jobsfini * c + lastch->jobsfini, c+1);
		
		earn = ( earn * c + lastch->earn ) / (c+1);
		earn2 = ( earn2 * c + lastch2->earn ) / (c+1);
		
		spend = ( spend * c + lastch->pay ) / (c+1);
		spend2 = ( spend2 * c + lastch2->pay ) / (c+1);
		
		trprice = ( trprice * c + lastch->trprice ) / (c+1);
		trprice2 = ( trprice2 * c + lastch2->trprice ) / (c+1);

		++c;
	}
	
	if(!c)
		return;
	
	//2016/05/02 trying to reduce driver expenses
	//if(jobsgiven > jobsfini)
	//if(jobsgiven*RATIO_DENOM/imax(1,jobsfini) > RATIO_DENOM*2)
	
	//2016/05/22 now maximizes continued throughput
#if 0
	if(py->global[RES_DOLLARS] > INST_FUNDS*99/100)
	{
		if(g_simframe*321 % 11 == 1)
		{
			newtrwg = newtrwg + newtrwg / 5 + 2;
			newtrwg = imin(INST_FUNDS/100, newtrwg);
		}
	}
	else
#endif
	{
		if(jobsgiven > jobsfini &&
			g_simframe*321 % 11 == 1 &&
			earn + earn2 >= spend + spend2)
		{
			newtrwg = newtrwg + newtrwg / 5 + 2;
			newtrwg = imin(INST_FUNDS/100, newtrwg);
		}
		else if(!jobsgiven)
			;
		else
		{
			newtrwg = newtrwg - newtrwg / 5;
			newtrwg = imax(10, newtrwg);
		}
	}

	int32_t newtrpr = py->transpcost;
	int32_t pricepositive = false;
	
	//if(earn > earn2)
	//	pricepositive = true;
	
	spend = imax(spend, 1);
	spend2 = imax(spend2, 1);
	
	int32_t ratio = earn*RATIO_DENOM/spend;
	int32_t ratio2 = earn2*RATIO_DENOM/spend2;

	if(ratio > ratio2)
		pricepositive = true;
	//2016/05/02 trying to reduce driver expenses
	//else if(!ratio || !ratio2 || (earn+earn2) < (spend+spend2))
	//	pricepositive = false;
	else if(ratio == ratio2 && jobsgiven)
		pricepositive = (bool)((g_simframe*pi/(CYCLE_FRAMES+1)) % 2);
	else
		pricepositive = false;

	//2016/05/22 now maximizes continued throughput
#if 0
	if(py->global[RES_DOLLARS] > INST_FUNDS*99/100)
	{
		pricepositive = (trprice < trprice2); 
	}
#endif

	int32_t pricesign = 1;

	if(!pricepositive)
		pricesign = -1;

	int32_t prch = trprice - trprice;

	int32_t mult = 1;

	if(pricepositive)
		mult = 3;

	if(iabs(prch) < 2)
		prch += 2 + prch * 2;

	//2016/05/02 trying to reduce driver expenses
	if(!ratio || !ratio2 || 
		(earn+earn2) < (spend+spend2) ||
		newtrpr <= newtrwg)
	{
		pricesign = 1;
		prch = iabs(prch);
	}

	newtrpr = newtrpr + pricesign * prch * mult / 2;
	
#if 0
	int32_t nlab = CountU(UNIT_LABOURER);
	int32_t nbl = CountB();
	int32_t nlabreq = nbl * 30 / BL_TYPES;
	bool subreqlab = nlab < nlabreq;

	//a rule so the workers don't die off
	//TODO does this need dissappear after a int32_t enough realistic timescale?
	if(subreqlab)
	{
		int32_t chamt = iabs( py->truckwage - newtrwg );
		newtrwg = py->truckwage + chamt * 3;
	}
#endif

	newtrwg = imax(MINWAGE, newtrwg);
	newtrwg = imin(INST_FUNDS/100, newtrwg);

	if(newtrwg != py->truckwage)
	{
		//TODO don't do this for every owned player truck
		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_TRWAGE;
		cvp.player = pi;
		cvp.bi = -1;
		cvp.value = newtrwg;
		LockCmd((PacketHeader*)&cvp);
	}

	newtrpr = imax(10, newtrpr);
	newtrpr = imin(INST_FUNDS/100, newtrpr);

	if(newtrpr != py->transpcost)
	{
		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_TRPRICE;
		cvp.player = pi;
		cvp.bi = -1;
		cvp.value = newtrpr;
		LockCmd((PacketHeader*)&cvp);
	}
}

void AdjPrWg(Player* p, Building* b)
{
	int32_t bi = b - g_building;
	int32_t pi = p - g_player;

	if(b->cyclehist.size() < 3)
	{
		Zero(b->varcost);
		b->fixcost = 0;
		return;
	}
		//goto end;
	
	//the completed cycle is [end-1], because tryprod() that calls this
	//func pushes a new one for the beginning of the new cycle.
	std::list<CycleHist>::reverse_iterator chit = b->cyclehist.rbegin();
	chit++;
	CycleHist* lastch = &*chit;
	chit++;
	CycleHist* lastch2 = &*chit;
	//CycleHist* lastch = &*(b->cyclehist.rbegin()++);
	//CycleHist* lastch2 = &*(b->cyclehist.rbegin()++++);
	
	//total profits (might be negative)
	int32_t profit = lastch->prod[RES_DOLLARS] - lastch->cons[RES_DOLLARS];
	int32_t profit2 = lastch2->prod[RES_DOLLARS] - lastch2->cons[RES_DOLLARS];
	int32_t earnings = lastch->prod[RES_DOLLARS];
	int32_t earnings2 = lastch2->prod[RES_DOLLARS];
	int32_t totprof = profit + profit2;
	int32_t spend = lastch->cons[RES_DOLLARS];
	int32_t spend2 = lastch2->cons[RES_DOLLARS];
	//earnings by resource
	//int32_t rearn[RESOURCES];
	//int32_t rearn2[RESOURCES];
	//Zero(rearn);
	//Zero(rearn2);
	//price changes by resource
	int32_t rprch[RESOURCES];
	Zero(rprch);
	int32_t haverch = -1;
	//wage change
	int32_t wch;
	//list of resources this bltype produces
	std::list<int32_t> prr;
	
	int32_t ratio[RESOURCES];
	int32_t ratio2[RESOURCES];

	spend = imax(spend, 1);
	spend2 = imax(spend2, 1);

	wch = lastch->wage - lastch2->wage;

#if 0
	GUI* gui = &g_gui;
	GenGraphs* gg = (GenGraphs*)gui->get("gen graphs");
	Graph* g = &gg->
#endif

	int32_t nlab = CountU(UNIT_LABOURER);
	int32_t nbl = CountB();
	int32_t nlabreq = nbl * 30 / BL_TYPES;
	//int32_t nlabreq = nbl * g_nlabperbl;
	bool subreqlab = nlab < nlabreq;

	BlType* bt = &g_bltype[b->type];

	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] > 0)
			prr.push_back(ri);

		//rearn[ri] = lastch->cons[ri] * lastch->price[ri];
		//rearn2[ri] = lastch2->cons[ri] * lastch2->price[ri];
		rprch[ri] = lastch->price[ri] - lastch2->price[ri];

		if(rprch[ri] != 0)
			haverch = ri;
	}

#if 0
	/*
	The general idea is to try adjusting price up,
	then the next cycle down, for the same resource,
	and then move on to the next resource if that fails.
	Then move on to the wage.
	*/

	//did we change wage?
	if(wch != 0)
	{
		//need more money?
		if(totprof < 0)
		{

		}
		//can spend more money?
		else if(totprof > 0)
		{
		}
	}


	//did we change resource price?
	if(haverch >= 0)
	{
	}
#endif

	/*
	Too complicated
	Plan 2
	Just decrease wage and increase all the prices if need more money
	And opposite if can spend
	*/
	
	//TODO calc earnings from each res using lastch->cons and price, and adj pricepositive for each out res

	//positive reinforcement?
	bool wagepositive = false;
	bool pricepositive[RESOURCES] = {false};
	
	//if(earnings > earnings2)
	//	pricepositive = true;
	for(int32_t ri=0; ri<RESOURCES; ri++)
	{
		//rearn[ri] = lastch->cons[ri] * lastch->price[ri];
		//rearn2[ri] = lastch2->cons[ri] * lastch2->price[ri];

		int32_t rearn = lastch->cons[ri] * lastch->price[ri];
		int32_t rearn2 = lastch2->cons[ri] * lastch2->price[ri];
		
		ratio[ri] = rearn*RATIO_DENOM/spend;
		ratio2[ri] = rearn2*RATIO_DENOM/spend2;
		
		if(ratio[ri] > ratio2[ri])
			pricepositive[ri] = true;
		else if(ratio[ri] == ratio2[ri] && rearn)
			pricepositive[ri] = (bool)((g_simframe*ri/(CYCLE_FRAMES+1)) % 2);
		else
			pricepositive[ri] = false;

		//2016/05/22 now maximizes continued throughput
#if 0
		if(p->global[RES_DOLLARS] > INST_FUNDS*99/100)
		{
			pricepositive[ri] = (lastch->price[ri] < 0); 
		}
#endif
	}

	//if(totprof > 0)
	//if(profit > 0)
	//did we make more profit this cycle than the cycle before?
	//if(profit > profit2)
	//did we get less labour than required last cycle?
	//if(lastch->cons[RES_LABOUR] < b->prodlevel * bt->input[RES_LABOUR] / RATIO_DENOM)
	if(lastch->cons[RES_LABOUR] /* + lastch2->cons[RES_LABOUR] */ + b->stocked[RES_LABOUR] < b->prodlevel * bt->input[RES_LABOUR] / RATIO_DENOM)
	{
		//TODO this is not enough as the labour might not have been consumed simply because some other requisite res was missing
#if 0
		if(p->global[RES_DOLLARS] > INST_FUNDS*3/4)
		//was the net profit of the last two cycle positive?
		//if(totprof > 0)
			positive = false;
		else if(p->global[RES_DOLLARS] < INST_FUNDS*3/4)
		//was the net profit of the last two cycle negative?
		//if(totprof < 0)
			positive = true;
#else
		wagepositive = true;

		//if(b->type == BL_SHMINE)
		//	g_applog<<bt->name<<" con "<<lastch->cons[RES_LABOUR]<<" + "<<lastch2->cons[RES_LABOUR]<<" + "<<b->stocked[RES_LABOUR]<<" < "<<(b->prodlevel * bt->input[RES_LABOUR] / RATIO_DENOM)<<std::endl;
#endif
	}
#if 0
	//else if(totprof < 0)
	//else if(profit < 0)
	//did we make less profit this cycle than the cycle before?
	else if(profit < profit2)
	{
		if(p->global[RES_DOLLARS] > INST_FUNDS*3/4)
		//if(totprof > 0)
			positive = true;
		else if(p->global[RES_DOLLARS] < INST_FUNDS*3/4)
		//if(totprof < 0)
			positive = false;
	}
#endif

	//2016/05/22 now maximizes continued throughput
#if 0
	if(p->global[RES_DOLLARS] > INST_FUNDS*99/100)
	{
		wagepositive = true; 
	}
#endif

	//TODO decent options menu with labels and layout instead of using one resizing func
	//TODO adjust truck wages

	/*
	TODO adjust pr wg based on 4 cycles with 2 together, not just 2, 
	because it's erratic and sometimes there's no sales on 1 cycle
	TODO use realistic month timescale, so that it's guaranteed to
	have some transactions, or year for farms, depending on economic 
	cycle length for bl
	*/
	//TODO make sure conduits connect, because this faulty method failed to connect oil well, which I was running for a day

	//TODO ability to make manual deals micromanage instead of AI manager
	/*
	TODO
	load and unload only needed assets within view
	run on any platform spec even crappy android tablet
	*/

	/*
	If positive, do the same change but more.
	And if negative, try in the opposite direction, but half.
	*/
	int32_t wagesign = 1;
	//int32_t pricesign = 1;
	int32_t pricesign[RESOURCES] = {1};

	if(!wagepositive)
		wagesign = -1;
	//if(!pricepositive)
	///	pricesign = -1;
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		if(pricepositive[ri])
			pricesign[ri] = 1;
		else
			pricesign[ri] = -1;
	}

	uint32_t simrand = g_simframe * 1103515245 + 12345;
	simrand ^= bi;

	//wage adjustment...
	bool dowch = false;
	int32_t nextw;

	//new wage rule: increase if insufficient labour provided, wagepositive true = need increase wage

#if 0
	//if there was a wage change
	if(wch != 0)
	{
		//nextw = b->opwage + wagesign * wch * 10 / 9;
		nextw = b->opwage + wagesign * iabs(wch) * 10 / 9;
		dowch = true;
	}

	//extra rule for negative reinforcement: try changing one of the prices or 
	//the wage in the opposite direction, because there are many factors
	//if(!positive && (g_simframe+pi+bi) % (1+0) == 0)
	if(!wagepositive /* && simrand % (1+(int32_t)prr.size()) == 0 */ )
	{
		if(iabs(wch) > 4)
			nextw = b->opwage - wagesign * b->opwage / 2;
		else if(wch != 0)
			nextw = b->opwage - wagesign * b->opwage * 4;
		else
			nextw = b->opwage - wagesign * 4;

		dowch = true;
	}
#else
	if(b->prodlevel)
	{
		if(wagepositive)
		{
			nextw = b->opwage + b->opwage / 5;
			dowch = true;
		}
		//try lowering?
		else if(lastch->cons[RES_LABOUR])
		{
			nextw = b->opwage - b->opwage / 7;
			dowch = true;
		}
	}
#endif

	if(dowch)
	{
#if 0
		//a rule so the workers don't die off
		//TODO does this need dissappear after a int32_t enough realistic timescale?
		if(subreqlab)
		{
			int32_t chamt = iabs( b->opwage - nextw );
			nextw = b->opwage + chamt * 3;
		}
#endif

		if(nextw < MINWAGE)
			nextw = MINWAGE;
		else if(nextw > INST_FUNDS / 100)
			nextw = INST_FUNDS / 100;

		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_BLWAGE;
		cvp.player = pi;
		cvp.bi = bi;
		cvp.value = nextw;
		LockCmd((PacketHeader*)&cvp);
	}

#if 0
		char m[128];
		sprintf(m, "prr %d,%d", simrand % (1+(int32_t)prr.size()), (int32_t)prr.size());
		RichText r = RichText(m);
		AddNotif(&r);
#endif

	//price adjustments...
	int32_t riti = 0;
	for(std::list<int32_t>::iterator rit=prr.begin(); rit!=prr.end(); rit++, riti++)
	{
		int32_t ri = *rit;
		
		//add on base cost to final price...

		int32_t nrpod = bt->output[ri] * b->prodlevel / RATIO_DENOM;
		int32_t rprodcost = 0;
		//int32_t lastnprod = lastch->prod[ri];

		//calculate variable cost of inputs for last cycle's production batch of this output ri
		for(int32_t ri2=0; ri2<RESOURCES; ++ri2)
		{
			Resource* r2 = &g_resource[ri2];

			//TODO multiple production chains

			if(bt->input[ri2] <= 0)
				continue;

			if(b->prodlevel <= 0)
				continue;

			/*
			the problem with this is we don't know which inputs go to which outputs,
			as they are equally necessary for all outputs.
			*/

			//int32_t rqty = bt->input[ri2] * b->prodlevel / RATIO_DENOM;
			//int32_t rqty = imax(1, bt->input[ri2] * b->prodlevel / RATIO_DENOM);
			//int32_t rqty = imax(1, lastch->prod[ri2]);
			//add cost per production batch at current level
			//rprodcost += b->varcost[ri2] / rqty;
			rprodcost += b->varcost[ri2];
		}
		
		//remaining fixed cost not covered by earnings from (other?) sold outputs
		int32_t remfixcost = 0;

		remfixcost = b->fixcost;
		int32_t outearn = 0;

		for(int32_t ri2=0; ri2<RESOURCES; ++ri2)
		{
			Resource* r2 = &g_resource[ri2];

			//TODO multiple production chains

			if(bt->output[ri2] > 0)
			{
				outearn += lastch->cons[ri2] * b->price[ri2];
				//prof = imax(0, prof);
				//remfixcost -= prof;
			}
		}

		outearn = imax(0, outearn - rprodcost);

		//remfixcost += rprodcost;	//?
		//remfixcost = imax(0, remfixcost - outearn);
		//remfixcost = imax(0, remfixcost);

		//final base
		int32_t basecost = rprodcost + remfixcost;
		//basically: basecost = variable input costs + imax(0, (fixed transport cost - imax(0, (output sales earnings - variable input costs)))

		//divide cost per sale
		basecost /= imax(1, lastch->cons[ri]);
#if 0
		char m[128];
		sprintf(m, "r %d,%d", simrand % (1+(int32_t)prr.size()), riti+1);
		RichText r = RichText(m);
		AddNotif(&r);
#endif

		bool doprch = false;
		int32_t nextrpr = 0;

		if(rprch[ri] != 0)
		{
			nextrpr = b->price[ri] + pricesign[ri] * rprch[ri] * 10 / 9;
			doprch = true;
		}

		//extra rule for negative reinforcement: try changing one of the prices or 
		//the wage in the opposite direction, because there are many factors
		//if(!positive && (g_simframe+pi+bi) % (1+(int32_t)prr.size()) == riti)
		//if(!pricepositive[ri] /* && simrand % (2+(int32_t)prr.size()) == riti+1 */ )
		{
			//2015/11/16/ - pricesign -> + pricesign, isign
			if(iabs(rprch[ri]) > 4)
				nextrpr = b->price[ri] + pricesign[ri] * isign(rprch[ri]) * iceil(b->price[ri], 20);
			else if(rprch[ri] != 0)
				nextrpr = b->price[ri] + pricesign[ri] * isign(rprch[ri]) * iceil(b->price[ri] * 40, 39);
			else
				nextrpr = b->price[ri] + pricesign[ri] * isign(1 - (g_simframe%4)) * 4;

			doprch = true;
		}

#if 0
		//must be at minimum base cost
		if(b->price[ri] < basecost)
		{
			doprch = true;
			nextrpr = imax(basecost, nextrpr);
		}
#endif

		if(doprch)
		{
#if 0
			//a rule so the workers don't die off
			if(subreqlab)
			{
				int32_t chamt = iabs( b->price[ri] - nextrpr );
				nextrpr = b->price[ri] - chamt * 3;
			}
#endif

			if(nextrpr < 1)
				nextrpr = 1;
			else if(nextrpr > INST_FUNDS / 100)
				nextrpr = INST_FUNDS / 100;

			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_BLPRICE;
			cvp.player = pi;
			cvp.bi = bi;
			cvp.value = nextrpr;
			cvp.res = ri;
			LockCmd((PacketHeader*)&cvp);
		}
	}

//end:

	Zero(b->varcost);
	b->fixcost = 0;

	//INST_FUNDS;
}

//2016/05/22 now maximizes continued throughput
void AdjCs(Player* p)
{
	return;

	int32_t pi = p - g_player;

	for(int32_t tx=0; tx<g_mapsz.x; ++tx)
	{
		for(int32_t ty=0; ty<g_mapsz.y; ++ty)
		{
			for(int32_t cti=0; cti<CD_TYPES; ++cti)
			{
				CdTile* ctile = GetCd(cti, tx, ty, false);

				if(ctile->owner != pi)
					continue;

				if(ctile->finished)
					continue;

				int32_t conwage = ctile->conwage;

				conwage = conwage * 3/2;
				conwage = imin(conwage, p->global[RES_DOLLARS]/20);

				if(conwage == ctile->conwage)
					continue;

				ChValPacket cvp;
				cvp.header.type = PACKET_CHVAL;
				cvp.chtype = CHVAL_CDWAGE;
				cvp.player = pi;
				cvp.cdtype = cti;
				cvp.x = tx;
				cvp.y = ty;
				cvp.bi = -1;
				cvp.value = conwage;
				LockCmd((PacketHeader*)&cvp);
			}
		}
	}

	for(int32_t bi=0; bi<BUILDINGS; ++bi)
	{
		Building* b = &g_building[bi];

		if(b->owner != pi)
			continue;

		if(b->finished)
			continue;

		int32_t conwage = b->conwage;

		conwage = conwage * 3/2;
		conwage = imin(conwage, p->global[RES_DOLLARS]/20);

		if(conwage == b->conwage)
			continue;

		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_CSTWAGE;
		cvp.player = pi;
		cvp.cdtype = -1;
		cvp.x = -1;
		cvp.y = -1;
		cvp.bi = bi;
		cvp.value = conwage;
		LockCmd((PacketHeader*)&cvp);
	}
}

void AdjProd(Player* p, Building* b)
{
	int32_t pi = p - g_player;

	//for(int32_t bi=0; bi<BUILDINGS; bi++)
	{
		//Building* b = &g_building[bi];

		int32_t bi = b - g_building;

		//if(!b->on)
		//	continue;

		//if(b->owner != pi)
		//	continue;

		BlType* bt = &g_bltype[b->type];
		bool stop = true;
		int32_t value = 0;

		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			if(bt->output[ri] <= 0)
				continue;

			Resource* r = &g_resource[ri];

			if(r->capacity)
			{
				CycleHist* lastch = &*b->cyclehist.rbegin();

				if(lastch->prod[ri] > lastch->cons[ri] && 
					b->prodlevel > 0 && 
					lastch->cons[ri] > 0)
				{
					stop = false;
					int32_t minout = iceil(RATIO_DENOM, bt->output[ri]);
					value = imax( value, RATIO_DENOM * lastch->cons[ri] / bt->output[ri] + minout*2 );
				}
				else if(lastch->prod[ri] <= lastch->cons[ri])
				{
					stop = false;
					//value = imax( value, RATIO_DENOM/10 + b->prodlevel );
					int32_t minout = iceil(RATIO_DENOM, bt->output[ri]);
					value = imax( value, RATIO_DENOM * lastch->cons[ri] / bt->output[ri] + minout*2 );
				}
			}
			else
			{
				//2016/04/26 changed stocking behaviour
				if(b->stocked[ri] < bt->output[ri] * bt->stockingrate / RATIO_DENOM ||
					!b->stocked[ri])
				{
					stop = false;
					//value = RATIO_DENOM;
					//value = bt->stockingrate;
					int32_t more = (bt->output[ri] * bt->stockingrate / RATIO_DENOM) - b->stocked[ri];
					int32_t minout = iceil(RATIO_DENOM, bt->output[ri]);
					minout = imax( minout, RATIO_DENOM * more / bt->output[ri] );
					value = imax( value, minout );
					break;
				}
			}
		}

		//2016/05/22 now maximizes continued throughput
#if 0
		if(p->global[RES_DOLLARS] > INST_FUNDS*99/100 &&
			((pi + bi + g_simframe) % (CYCLE_FRAMES+1))/(CYCLE_FRAMES/3) == 1)
		{
			stop = false;
			value = RATIO_DENOM;
		}
#endif
		
		value = imin( value, RATIO_DENOM );

		if( (stop && b->prodlevel == 0) ||
			(!stop && b->prodlevel == value) )
			return;

		//if(!toomuch)
		//	value = RATIO_DENOM;

		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_PRODLEV;
		cvp.bi = bi;
		cvp.value = value;
		cvp.player = pi;
		LockCmd((PacketHeader*)&cvp);
	}
}

void Manuf(int32_t pi, int32_t uti)
{
	std::list<int32_t> manufers;

	for(int32_t bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		//TODO enemey check
		//TODO visibility check

		BlType* bt = &g_bltype[b->type];

		for(std::list<unsigned char>::iterator mt=bt->manuf.begin(); mt!=bt->manuf.end(); mt++)
		{
			if(*mt == uti)
			{
				manufers.push_back(bi);
				break;
			}
		}
	}

	if(!manufers.size())
		return;

	//chosen manuf index
	int32_t cmi = g_simframe % manufers.size();
	int32_t mi = 0;
	std::list<int32_t>::iterator mit = manufers.begin();

	for(; mit!=manufers.end(), mi<cmi; mit++, mi++)
	{
	}

	OrderManPacket omp;
	omp.header.type = PACKET_ORDERMAN;
	omp.player = pi;
	omp.bi = *mit;
	omp.utype = uti;
	LockCmd((PacketHeader*)&omp);
}

void AIManuf(Player* p)
{
	int32_t bcount = 0;
	int32_t pi = p - g_player;

	for(int32_t i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		//if(b->owner != pi)
		//	continue;

		bcount++;
	}

	int32_t ucount[UNIT_TYPES];
	memset(ucount, 0, sizeof(int32_t) * UNIT_TYPES);

	for(int32_t i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		//if(u->owner != pi)
		//	continue;

		ucount[u->type]++;
	}

	for(int32_t uti=0; uti<UNIT_TYPES; uti++)
	{
		UType* ut = &g_utype[uti];

		int32_t needed = ut->prop * bcount / RATIO_DENOM;
		int32_t tomanuf = needed - ucount[uti];

		if(tomanuf <= 0)
			continue;

		//for(int32_t j=0; j<tomanuf; j++)
		{
			Manuf(pi, uti);
		}
	}
}

/*
To calc what bl next and where

For each bl on map multiply by 15 and decrease by radius from spot of bl

Spot with greatest num of all bl types is densest
Will be next spot where no bl placed

Then multiply bl proportion rate num by 15 also and subtract dense tile's num for all bl's of that type, and choose the bltype with greatest
For eg house proportion is 3 and theres only 1 gives 2

15 is some distance num for max city clustering distance that should effect interaction
*/
void AIBuild(Player* p)
{
	//RichText msg = RichText("AIBuild");
	//AddNotif(&msg);

	int32_t pi = p - g_player;

	//check for any unfinished bl's
	for(int32_t i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		if(b->owner != pi)
			continue;

		if(!b->finished)
			return;
	}

#define DIST_MULT		30

	int32_t* bldensity = (int32_t*)malloc(sizeof(int32_t) * (g_mapsz.x+0) * (g_mapsz.y+0));
	memset(bldensity, 0, sizeof(int32_t) * (g_mapsz.x+0) * (g_mapsz.y+0));

	//TODO only get density for visible tiles, so that each player builds own base

	//calc density
	for(int32_t i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		if(b->owner != pi)
			continue;

		BlType* bt = &g_bltype[b->type];
		Vec2i tstart = b->tpos;
		Vec2i tmin, tmax;

		tmin.x = tstart.x - DIST_MULT;
		tmin.y = tstart.y - DIST_MULT;
		tmax.x = tstart.x + DIST_MULT;
		tmax.y = tstart.y + DIST_MULT;
		tmin.x = imax(0, tmin.x);
		tmin.y = imax(0, tmin.y);
		tmax.x = imin(g_mapsz.x, tmax.x);
		tmax.y = imin(g_mapsz.y, tmax.y);

		for(int32_t tx=tmin.x; tx<tmax.x; tx++)
			for(int32_t ty=tmin.y; ty<tmax.y; ty++)
			{
				Vec2i tspot;
				tspot.x = tx;
				tspot.y = ty;
				const const Vec2i toff = tspot - tstart;
				int32_t tdist = MAG_VEC2I(toff);
				int32_t dens = DIST_MULT - tdist;
				
				int32_t* spoti = &bldensity[ tx + ty * g_mapsz.x ];
				//
				//(*spoti) += ((g_simframe % tx) * 31 * pi) % ty;

				if(dens <= 0)
					continue;

				(*spoti) += dens;
			}
	}

	//add unit's density
	for(int32_t i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->owner != pi)
			continue;

		Vec2i tstart = u->cmpos / TILE_SIZE;

		int32_t* spoti = &bldensity[ tstart.x + tstart.y * g_mapsz.x ];
		(*spoti) += 1;
	}

	Vec2i highspot;
	int32_t high = -1;

	//get highest density spot
	for(int32_t tx=0; tx<g_mapsz.x; tx++)
	{
		for(int32_t ty=0; ty<g_mapsz.y; ty++)
		{	
			if(!Explored(pi, tx, ty))
				continue;

			int32_t* spoti = &bldensity[ tx + ty * g_mapsz.x ];

			if(*spoti < high)
				continue;

			high = *spoti;
			highspot.x = tx;
			highspot.y = ty;
		}
	}

	free(bldensity);

	int32_t highscore = 0;
	int32_t highbti = -1;

	//calc bltype to build
	for(int32_t bti=0; bti<BL_TYPES; bti++)
	{
		BlType* bt = &g_bltype[bti];

		//if(!bt->on)
		//	continue;

		int32_t score = 0;
		//int32_t maxcontrib = DIST_MULT * bt->prop;

		//go through each bl of that type
		for(int32_t bi=0; bi<BUILDINGS; bi++)
		{
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			if(b->type != bti)
				continue;

			Vec2i tstart = b->tpos;

			const Vec2i toff = highspot - tstart;
			int32_t tdist = MAG_VEC2I(toff);
			tdist = imin(tdist, DIST_MULT);
			int32_t dens = DIST_MULT - tdist;
			dens = imax(0, dens);

			//add contribution of that bl for this tile
			score -= dens;
		}

		if(bt->prop == 0)
			continue;

		score /= bt->prop;

		if(score < highscore &&
			highbti >= 0)
			continue;

		highscore = score;
		highbti = bti;
	}

	if(highbti < 0)
		return;

	//check if proportional to workers

	int32_t bcount = 0;

	for(int32_t i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		//if(b->owner != pi)
		//	continue;

		if(b->type != highbti)
			continue;

		bcount++;
	}

	int32_t wcnt = CountU(UNIT_LABOURER);
	BlType* bt = &g_bltype[highbti];

	//RATIO_DENOM * 1*1
	if(wcnt < bcount * bt->wprop / RATIO_DENOM)
		return;

	//find a place to build
	Vec2i tplace;
	if(!PlaceBAb4(highbti, highspot, &tplace))
	{
		p->lastthink += CYCLE_FRAMES*4;
		return;
	}

	PlaceBlPacket pbp;
	pbp.header.type = PACKET_PLACEBL;
	pbp.btype = highbti;
	pbp.player = pi;
	pbp.tpos = tplace;
	LockCmd((PacketHeader*)&pbp);

	//connect conduits

	for(int32_t ctype=0; ctype<CD_TYPES; ctype++)
	{
		bool needed = false;

		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			Resource* r = &g_resource[ri];

			if(r->conduit != ctype)
				continue;

			if(bt->input[ri] > 0)
			{
				needed = true;
				break;
			}

			if(bt->output[ri] > 0)
			{
				needed = true;
				break;
			}
		}

		//if(!needed)
		//	continue;

		CdType* ct = &g_cdtype[ctype];

		if(ct->cornerpl)
			ConnectCd(p, ctype, tplace);
#if 1
		//if(pi < 0 || pi >= PLAYERS)
		//	InfoMess("!","!");

#if 0
		//if(tplace.x + bt->width.x % 2 == 4)
		PlotCd(pi, ctype, tplace, tplace + Vec2i(bt->width.x,0));
		PlotCd(pi, ctype, tplace, tplace - Vec2i(bt->width.x,0));
		PlotCd(pi, ctype, tplace, tplace + Vec2i(0,bt->width.y));
		PlotCd(pi, ctype, tplace, tplace - Vec2i(0,bt->width.y));
		PlotCd(pi, ctype, tplace, tplace + Vec2i(bt->width.x,bt->width.y));
		PlotCd(pi, ctype, tplace, tplace - Vec2i(bt->width.x,bt->width.y));
		PlotCd(pi, ctype, tplace, tplace + Vec2i(bt->width.x,-bt->width.y));
		PlotCd(pi, ctype, tplace, tplace + Vec2i(-bt->width.x,bt->width.y));
#else
		//TODO math between vec2i and vec2uc
		//TODO VEC2SUB VEC3SUB
		Vec2i tmin = tplace - Vec2i(bt->width.x/2+1,bt->width.y/2+1);
		Vec2i tmax = tmin + Vec2i(bt->width.x+1,bt->width.y+1);
		
		bool do1 = ((tmin.y) % 4 == 0) || (tmin.y == 0);
		bool do2 = ((tmax.y) % 4 == 0) || (tmax.y == 0);
		bool do3 = ((tmin.x) % 3 == 0) || (tmin.x == 0);
		bool do4 = ((tmax.x) % 3 == 0) || (tmax.x == 0);

//plot1:

		if(do1 && do4)
		{
			Vec2i northline[2] = {Vec2i(tmin.x-4,tmin.y), Vec2i(tmax.x+4,tmin.y)};
			
			if(ct->blconduct)
			{
				//do2 = true;
				if(tmin.x % 3 == 0)
				{
					northline[0].x = tmin.x;
					northline[1].x = tmax.x;
				}
				else if(tmax.x % 3 == 0)
				{
					northline[0].x = tmin.x;
					northline[1].x = tmax.x;
				}
				else
				{
					//int32_t nearc = () * 3;
					goto plot2;
				}
			}

			PlotCd(pi, ctype, northline[0], northline[1]);
		}

plot2:
		
		if(do2 && do3)
		{
			Vec2i southline[2] = {Vec2i(tmin.x-4,tmax.y), Vec2i(tmax.x+4,tmax.y)};

			if(ct->blconduct)
			{
				if(tmin.x % 3 == 0)
				{
					southline[0].x = tmin.x;
					southline[1].x = tmax.x;
				}
				else if(tmax.x % 3 == 0)
				{
					southline[0].x = tmin.x;
					southline[1].x = tmax.x;
				}
				else
				{
					//int32_t nearc = () * 3;
					goto plot3;
				}
			}

			PlotCd(pi, ctype, southline[0], southline[1]);
		}
		
plot3:

		if(do3 && do2)
		{
			Vec2i westline[2] = {Vec2i(tmin.x,tmin.y-4), Vec2i(tmin.x,tmax.y+4)};

			if(ct->blconduct)
			{
				//do4 = true;
				if(tmin.y % 4 == 0)
				{
					westline[0].y = tmin.y;
					westline[1].y = tmax.y;
				}
				else if(tmax.y % 4 == 0)
				{
					westline[0].y = tmin.y;
					westline[1].y = tmax.y;
				}
				else
				{
					//int32_t nearc = () * 3;
					goto plot4;
				}
			}

			PlotCd(pi, ctype, westline[0], westline[1]);
		}
		
plot4:

		if(do4 && do1)
		{
			Vec2i eastline[2] = {Vec2i(tmax.x,tmin.y-4), Vec2i(tmax.x,tmax.y+4)};
			
			if(ct->blconduct)
			{
				if(tmin.y % 4 == 0)
				{
					eastline[0].y = tmin.y;
					eastline[1].y = tmax.y;
				}
				else if(tmax.y % 4 == 0)
				{
					eastline[0].y = tmin.y;
					eastline[1].y = tmax.y;
				}
				else
				{
					//int32_t nearc = () * 3;
					continue;
				}
			}

			PlotCd(pi, ctype, eastline[0], eastline[1]);
		}
#endif
#endif
	}
}

void PlotCd(int32_t pi, int32_t ctype, Vec2i from, Vec2i to)
{
	//return;

	from.x = imin(from.x, g_mapsz.x-1);
	from.y = imin(from.y, g_mapsz.y-1);
	to.x = imin(to.x, g_mapsz.x-1);
	to.y = imin(to.y, g_mapsz.y-1);
	from.x = imax(from.x, 0);
	from.y = imax(from.y, 0);
	to.x = imax(to.x, 0);
	to.y = imax(to.y, 0);

	std::list<Vec2s> places;

#if 1
	TilePath(UNIT_LABOURER, UMODE_NONE, from.x*TILE_SIZE + TILE_SIZE/2, from.y*TILE_SIZE + TILE_SIZE/2,
		-1, -1, TARG_NONE, -1,
		ctype, &places, NULL, NULL, NULL,
		to.x*TILE_SIZE + TILE_SIZE/2, to.y*TILE_SIZE + TILE_SIZE/2, 
		to.x*TILE_SIZE, to.y*TILE_SIZE, to.x*TILE_SIZE + TILE_SIZE - 1, to.y*TILE_SIZE + TILE_SIZE - 1,
		g_mapsz.x * g_mapsz.y / 2, true, false);
#endif

#if 0
	int32_t d = MAG_VEC3F(from - to);

	for(int32_t di=0; di<d; di++)
	{
		Vec2s cur = Vec2s(from.x,from.y) + Vec2s(to.x,to.y) * 10 * di / d;
		places.push_back(cur);
	}

	places.push_back(Vec2s(to.x,to.y));
#endif

	if(!places.size())
		return;

	PlaceCdPacket* pcp = (PlaceCdPacket*)malloc(
		sizeof(PlaceCdPacket) + 
		sizeof(Vec2uc)*(int16_t)places.size() );
	pcp->header.type = PACKET_PLACECD;
	pcp->cdtype = ctype;
	pcp->player = pi;
	pcp->ntiles = (int16_t)places.size();

	std::list<Vec2s>::iterator pit = places.begin();
	for(int16_t pin = 0; pit != places.end(); pit++, pin++)
		pcp->place[pin] = Vec2uc((uint8_t)pit->x, (uint8_t)pit->y);

	LockCmd((PacketHeader*)pcp);

	free(pcp);
}

void ConnectCd(Player* p, int32_t ctype, Vec2i tplace)
{
	return;
	Vec2i nearest;
	int32_t neard = -1;

	for(int32_t tx=0; tx<g_mapsz.x; tx++)
	{
		for(int32_t ty=0; ty<g_mapsz.y; ty++)
		{
			CdTile* ctile = GetCd(ctype, tx, ty, false);

			if(!ctile->on)
				continue;

			Vec2i to = Vec2i(tx, ty);
			const const Vec2i toff = to - tplace;
			int32_t d = MAG_VEC2I(toff);

			if(d < neard ||
				neard < 0)
			{
				neard = d;
				nearest = to;
			}
		}
	}

	int32_t pi = p - g_player;
	Vec2i to = nearest;

	//not found a target?
	if(neard < 0)
	{
		to = tplace;

		uint32_t dx = g_simframe % 5;
		uint32_t dy = (g_simframe + 11) % 5;

		to.x += dx - 2;
		to.y += dy - 2;

		to.x = imax(0, to.x);
		to.y = imax(0, to.y);
		to.x = imin(g_mapsz.x-1, to.x);
		to.y = imin(g_mapsz.y-1, to.y);
	}

	PlotCd(pi, ctype, tplace, to);
}