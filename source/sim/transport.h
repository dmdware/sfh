














#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "simdef.h"

class Unit;
class Building;

#if 0
class TransportJob
{
public:
	int32_t targtype;
	int32_t target;
	int32_t target2;
	std::list<Vec2i> path;
	int32_t pathlen;
	int32_t restype;
	int32_t resamt;
	int32_t fuelcost;
	int32_t driverwagepaid;
	int32_t netprofit;	// truck profit
	int32_t transporter;
	int32_t totalcosttoclient;	// transport fee + resource cost
	int32_t clientcharge;	// transport fee
	int32_t supplier;
    
	TransportJob()
	{
		pathlen = 0;
		totalcosttoclient = 0;
		clientcharge = 0;
		driverwagepaid = 0;
		netprofit = 0;
		fuelcost = 0;
		resamt = 0;
	}
};
#else

//job opportunity
class TransportJob
{
public:
	int32_t jobutil;
	int32_t jobtype;
	int32_t target;
	int32_t target2;
	//float bestDistWage = -1;
	//float distWage;
	//bool fullquota;
	int32_t ctype;	//conduit type
	Vec2i goal;
	int32_t targtype;
	Unit* thisu;
	Unit* targu;
	Building* targb;
	int32_t suputil;
	int32_t supbi;
	Vec2i supcmpos;
	int16_t truckui;
	Building* potsupb;
};

#endif

class ResSource
{
public:
	int32_t supplier;
	int32_t amt;
};

//extern vector<int32_t> g_freetrucks;

////#define TRANSPORT_DEBUG	//chat output debug messages

void ManageTrips();

#endif
