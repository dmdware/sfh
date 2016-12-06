









#include "simflow.h"
#include "evqueue.h"
#include "../net/client.h"
#include "unit.h"

std::list<SimEv> g_simev;

void PlanEvent(uint16_t type, uint64_t execframe, Unit* u)
{
	SimEv se;

	se.evtype = type;
	se.planframe = g_simframe;
	se.execframe = execframe;

	std::list<SimEv>::iterator sit=g_simev.begin();
	bool place = false;

	while(true)
	{
		if(sit == g_simev.end())
		{
			place = true;
		}
		else
		{
			SimEv* spotse = &*sit;

			if(spotse->execframe > execframe)
				place = true;
		}

		if(!place)
			sit++;

		g_simev.insert(sit, se);
		break;
	}
}

void ExecEvent(SimEv* se)
{
}

void ExecEvents()
{
}