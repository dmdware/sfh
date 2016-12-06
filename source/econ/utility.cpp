












#include "utility.h"
#include "../path/pathnode.h"


// food/housing/physical res utility evaluation
int32_t PhUtil(int32_t price, int32_t cmdist)
{
	cmdist /= INV_DIST_EFFECT;
	price /= INV_WAGE_EFFECT;

	if(price <= 0)
		return MAX_UTIL;

	if(cmdist <= 0)
		return MAX_UTIL;
	
	return MAX_UTIL / price / (cmdist);
	//return MAX_UTIL / price / (cmdist / PATHNODE_SIZE);
	//return MAX_UTIL / price / (cmdist * PATHNODE_SIZE);
	//return MAX_UTIL - (price * cmdist);
}

// electricity utility evaluation
int32_t GlUtil(int32_t price)
{
	price /= INV_WAGE_EFFECT;

	if(price <= 0)
		return MAX_UTIL;
	
	return MAX_UTIL / price;
	//return MAX_UTIL - price;
}

// inverse phys utility - solve for distance based on utility and price
int32_t InvPhUtilD(int32_t util, int32_t price)
{
	//util = 100000000 / price / (cmdist/PATHNODE_SIZE);
	//util / 100000000 = 1 / price / (cmdist//PATHNODE_SIZE);
	//util * price / 100000000 = 1 / (cmdist//PATHNODE_SIZE);
	//100000000 / (util * price) = (cmdist//PATHNODE_SIZE);
	
	price /= INV_WAGE_EFFECT;

	if(util <= 0)
		return MAX_UTIL;
	
	//if(price / PATHNODE_SIZE <= 0)
	if(price <= 0)
		return MAX_UTIL;
		//return -(util - MAX_UTIL);

	//util = MAX_UTIL - (price * cmdist)
	//util - MAX_UTIL = - (price * cmdist)
	//cmdist = - (util - MAX_UTIL)/price
	
	//return MAX_UTIL / (util * price);
	return MAX_UTIL / (util * price) * INV_DIST_EFFECT;
	//return MAX_UTIL / (util * price / PATHNODE_SIZE);
	//return MAX_UTIL / (util * price * PATHNODE_SIZE);
	//return -(util - MAX_UTIL)/price;
}

// inverse phys utility - solve for price based on utility and distance
int32_t InvPhUtilP(int32_t util, int32_t cmdist)
{
	cmdist /= INV_DIST_EFFECT;

	if(util <= 0)
		return MAX_UTIL;
	
	//if(cmdist/PATHNODE_SIZE <= 0)
	if(cmdist <= 0)
		return MAX_UTIL;
		//return -(util - MAX_UTIL);
	
	return MAX_UTIL / (util * (cmdist)) * INV_WAGE_EFFECT;
	//return MAX_UTIL / (util * (cmdist/PATHNODE_SIZE));
	//return MAX_UTIL / (util * (cmdist*PATHNODE_SIZE));
	//return -(util - MAX_UTIL)/cmdist;
}

int32_t InvGlUtilP(int32_t util)
{
	if(util <= 0)
		return MAX_UTIL;
	
	return MAX_UTIL / util;
	//return MAX_UTIL - util;
}


int32_t InvJobUtilP(int32_t util, int32_t cmdist, int32_t workdelay)
{
	cmdist /= INV_DIST_EFFECT;
	
	if(util * cmdist <= 0)
		return MAX_UTIL * INV_WAGE_EFFECT;

	return iceil(MAX_UTIL, util * cmdist) * INV_WAGE_EFFECT;
}

int32_t JobUtil(int32_t wage, int32_t cmdist, int32_t workdelay)
{
	wage /= INV_WAGE_EFFECT;

	cmdist /= INV_DIST_EFFECT;

	if(wage <= 0)
		return 0;
	
	//if(cmdist/PATHNODE_SIZE <= 0)
	if(cmdist <= 0)
		return MAX_UTIL;

	if(workdelay <= 0)
		return MAX_UTIL;
	
	return MAX_UTIL / (cmdist) /* / workdelay */ * wage;
	//return MAX_UTIL / (cmdist/PATHNODE_SIZE) / workdelay * wage;
	//return MAX_UTIL / (cmdist*PATHNODE_SIZE) / workdelay * wage;
	//return MAX_UTIL - (cmdist * workdelay) / wage;
}