











#ifndef BUILDING_H
#define BUILDING_H

#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "../platform.h"
#include "resources.h"
#include "bltype.h"
#include "../render/vertexarray.h"
#include "utype.h"
#include "../render/depthable.h"

class VertexArray;

//#define PROD_DEBUG	//bl's production debug output

//unit manufacturing job
class ManufJob
{
public:
	int32_t utype;
	int32_t owner;
};

class Building
{
public:

	Depthable* depth;

	bool on;
	int32_t type;
	int32_t owner;

	Vec2i tpos;	//position in tiles
	Vec2f drawpos;	//drawing position in world pixels

	bool finished;
	float frame;

#if 1
	int16_t pownetw;
	int16_t crpipenetw;
	std::list<int16_t> roadnetw;
#else
	std::list<int16_t> netwlist[CD_TYPES];
	int16_t netw[CD_TYPES];
#endif

	int32_t stocked[RESOURCES];
	int32_t inuse[RESOURCES];

	EmitterCounter emitterco[MAX_B_EMITTERS];

	int32_t conmat[RESOURCES];
	bool inoperation;

	int32_t price[RESOURCES];	//price of produced goods
	int32_t propprice;	//price of this property
	bool forsale;	//is this property for sale?
	bool demolish;	//was a demolition ordered?
	//TODO only one stocked[] and no conmat[]. set stocked[] to 0 after construction finishes, and after demolition ordered.

	std::list<int32_t> occupier;
	std::list<int32_t> worker;
	int32_t conwage;
	int32_t opwage;
	int32_t cydelay;	//the frame delay between production cycles, when production target is renewed
	int16_t prodlevel;	//production target level of max RATIO_DENOM
	int16_t cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	uint64_t lastcy;	//last simframe of last production cycle
	std::list<CapSup> capsup;	//capacity suppliers

	int32_t manufprc[UNIT_TYPES];
	std::list<ManufJob> manufjob;
	int16_t transporter[RESOURCES];

	int32_t hp;
	
	int32_t varcost[RESOURCES];	//variable cost for input resource ri
	int32_t fixcost;	//fixed cost for transport

	std::list<CycleHist> cyclehist;

	bool demolition;

	bool excin(int32_t rtype);	//excess input resource right now?
	bool metout();	//met production target for now?
	int32_t netreq(int32_t rtype);	//how much of an input is still required to meet production target
	bool hasworker(int32_t ui);
	bool tryprod();
	bool trymanuf();
	int32_t maxprod();
	void adjcaps();
	void spawn(int32_t utype, int32_t uowner);
	void morecap(int32_t rtype, int32_t amt);
	void lesscap(int32_t rtype, int32_t amt);
	void getethereal();
	void getethereal(int32_t rtype, int32_t amt);

	void destroy();
	void fillcollider();
	void freecollider();
	void allocres();
	bool checkconstruction();
	Building();
	~Building();
};

//#define BUILDINGS	256
//#define BUILDINGS	64
//#define BUILDINGS (MAX_MAP*MAX_MAP)
#define BUILDINGS	1024

extern Building g_building[BUILDINGS];
class Unit;

extern std::list<uint16_t> g_onbl;

int32_t NewBl();
void FreeBls();
void DrawBl();
void DrawBl(Building* b, float rendz, uint32_t renderdepthtex=0, uint32_t renderfb=0);
void DrawBl2(Building* b, float rendz, uint32_t renderdepthtex=0, uint32_t renderfb=0);
void UpdBls();
void StageCopyVA(VertexArray* to, VertexArray* from, float completion);
void HeightCopyVA(VertexArray* to, VertexArray* from, float completion);
void HugTerrain(VertexArray* va, Vec3f pos);
void Explode(Building* b);
float CompletPct(int32_t* cost, int32_t* current);
void RemWorker(Unit* w);
void SupAdjCaps(uint8_t ri, int32_t supbi);
void DemAdjCaps(uint8_t ri, int32_t dembi);
void RemShopper(Unit* w);

#endif
