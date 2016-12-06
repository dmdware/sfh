










#ifndef DEMAND_H
#define DEMAND_H

#include "../math/vec2i.h"
#include "../platform.h"
#include "../sim/resources.h"
#include "../sim/utype.h"
#include "../sim/bltype.h"
#include "../sim/build.h"
#include "../sim/player.h"
#include "../debug.h"
#include "../sim/simdef.h"

//TODO loading sprites on demand, not all on startup
//TODO stop making window widget shrink vertically when dragging to top of real window until it dissappears

class DemPt
{
public:
	int32_t dembi;	//demander bl
	int32_t demui;	//demander unit
	int32_t minutil;	//minimum utility
	int32_t maxbid;	//maximum bid, if util=-1
	int32_t ri;	//resource
};

extern std::list<DemPt> g_dem;

void CalcDem();

#endif