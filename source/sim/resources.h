











#ifndef RESOURCES_H
#define RESOURCES_H

#include "../platform.h"

//TODO MAX_ICONS 256 mod
class Resource
{
public:
	char icon;
	bool physical;
	bool capacity;
	bool global;
	std::string name;	//TODO change to char[]?
	float rgba[4];
	std::string depositn;	//TODO change to char[]?
	int32_t conduit;
	std::string unit;	//measuring unit
	uint32_t halflife;
};

//conduit
#define CD_NONE		-1
#define CD_ROAD		0
#define CD_POWL		1
#define	CD_CRPIPE		2

#define RES_NONE			-1
#define RES_DOLLARS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_CHEMICALS		5
#define RES_IRONORE			6
#define RES_METAL			7
#define RES_STONE			8
#define RES_CEMENT			9
#define RES_COAL			10
#define RES_URANIUM			11
#define RES_PRODUCTION		12
#define RES_CRUDEOIL		13
#define RES_WSFUEL			14
#define RES_RETFUEL			15
#define RES_ENERGY			16
#define RESOURCES			17
//#define RES_ELECTRONICS		6
extern Resource g_resource[RESOURCES];

//TODO remove electronics plant to avoid crash

//TODO make Basket allocate *r = x RESOURCES for modding
//MAX_RESOURCES 256
class Basket
{
public:
	int32_t r[RESOURCES];

	int32_t& operator[](const int32_t i)
	{
		return r[i];
	}
};

//TODO implement Bundle
class Bundle
{
public:
	unsigned char res;
	int32_t amt;
};

//capacity supply (e.g. electricity, water pressure)
class CapSup
{
public:
	unsigned char rtype;
	int32_t amt;
	int32_t src;
	int32_t dst;
};

void DefR(int32_t resi, 
		  const char* n, 
		  const char* depn, 
		  int32_t iconindex, 
		  bool phys, bool cap, bool glob, 
		  float r, float g, float b, float a, 
		  int32_t conduit, const char* unit, uint32_t halflife);
void Zero(int32_t *b);
void UpdDecay();
bool ResB(int32_t building, int32_t res);
bool TrySub(const int32_t* cost, int32_t* universal, int32_t* stock, int32_t* local, int32_t* netch, int32_t* insufres);

class CycleHist
{
public:
	int32_t prod[RESOURCES];	//earnings and production
	int32_t cons[RESOURCES];	//expenses and consumption
	int32_t price[RESOURCES];
	int32_t wage;
	
	CycleHist()
	{
		reset();
	}
	
	void reset()
	{
		Zero(prod);
		Zero(cons);
		Zero(price);
		wage = -1;
	}
};

class TrCycleHist
{
public:
	int32_t earn;	//fees collected
	int32_t pay;	//wage paid
	int32_t trprice;	//transport fee
	int32_t opwage;	//driver wage
	int32_t jobsgiven;
	int32_t jobsfini;
	
	TrCycleHist()
	{
		reset();
	}
	
	void reset()
	{
		earn = 0;
		pay = 0;
		trprice = 0;
		opwage = 0;
		jobsgiven = 0;
		jobsfini = 0;
	}
};

#endif
