













#include "manuf.h"
#include "building.h"
#include "bltype.h"
#include "unit.h"
#include "utype.h"
#include "player.h"
#include "../net/client.h"

void OrderMan(int32_t utype, int32_t bi, int32_t player)
{
	ManufJob mj;
	Building* b = &g_building[bi];
	mj.owner = player;
	mj.utype = utype;
	b->manufjob.push_back(mj);
	b->trymanuf();
	b->manufjob.clear();
}
