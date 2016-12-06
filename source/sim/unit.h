











#ifndef UNIT_H
#define UNIT_H

#include "../math/camera.h"
#include "../platform.h"
#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../math/vec2s.h"
#include "resources.h"
#include "../render/depthable.h"

#define BODY_LOWER	0
#define BODY_UPPER	1

class Demand;

class Unit
{
public:

	Depthable* depth;

	bool on;
	int32_t type;
	int32_t owner;

	/*
	The draw (floating-point) position vector is used for drawing.
	*/
	Vec2f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec2i cmpos;
	Vec3f facing;	//used for tank turret
	Vec3f rotation;
	float frame[2];	//BODY_LOWER and BODY_UPPER

	std::list<Vec2i> path;
	Vec2i goal;

	int32_t target;
	int32_t target2;
	bool targetu;
	bool underorder;
	int32_t fuelstation;
	int32_t belongings[RESOURCES];
	int32_t hp;
	bool passive;
	Vec2i prevpos;
	int32_t taskframe;
	bool pathblocked;
	int32_t jobframes;
	int32_t supplier;
	int32_t reqamt;
	int32_t targtype;
	int32_t home;
	int32_t car;
	//std::vector<TransportJob> bids;
	int32_t cargoamt;
	int32_t cargotype;
	int32_t cargoreq;	//req amt

	Vec2i subgoal;

	unsigned char mode;
	int32_t pathdelay;
	uint64_t lastpath;

	bool threadwait;

	// used for debugging - don't save to file
	bool collided;
	bool mazeavoid;
	int8_t winding;	//to avoid falling into an infinite loop/circle in path, and to know when the unit has made it around an obstacle

	signed char cdtype;	//conduit type for mode (going to construction)
	int32_t driver;
	//int16_t framesleft;
	int32_t cyframes;	//cycle frames (unit cycle of consumption and work)
	int32_t opwage;	//transport driver wage	//edit: NOT USED, set globally in Player class
	//std::list<TransportJob> bids;	//for trucks

	std::list<Vec2s> tpath;	//tile path

	int32_t exputil;	//expected utility on arrival

	bool forsale;
	int32_t price;

	int32_t incomerate;
	std::list<TrCycleHist> cyclehist;

	//bool filled;
	char turn;	/* used for MazePath */

	Unit();
	virtual ~Unit();
	void destroy();
	void fillcollider();
	void freecollider();
	void resetpath();
	bool hidden() const;
};

#define UNITS	(4096)
//#define UNITS	256

extern Unit g_unit[UNITS];

#define UMODE_NONE					0
#define UMODE_GOBLJOB				1
#define UMODE_BLJOB					2
#define UMODE_GOCSTJOB				3	//going to construction job
#define UMODE_CSTJOB				4
#define UMODE_GOCDJOB				5	//going to conduit (construction) job
#define UMODE_CDJOB					6	//conduit (construction) job
#define UMODE_GOSHOP				7
#define UMODE_SHOPPING				8
#define UMODE_GOREST				9
#define UMODE_RESTING				10
#define	UMODE_GODRIVE				11	//going to transport job
#define UMODE_DRIVE					12	//driving transport
#define UMODE_GOSUP					13	//transporter going to supplier
#define UMODE_GODEMB				14	//transporter going to demander bl
#define UMODE_GOREFUEL				15
#define UMODE_REFUELING				16
#define UMODE_ATDEMB				17	//at demanber bl, unloading load
#define UMODE_ATSUP					18	//at supplier loading resources
#define UMODE_GODEMCD				19	//going to demander conduit
#define UMODE_ATDEMCD				20	//at demander conduit, unloading

#define TARG_NONE		-1
#define TARG_BL			1	//building
#define TARG_U			2	//unit
#define TARG_CD			3	//conduit

#ifdef RANDOM8DEBUG
extern int32_t thatunit;
#endif

void DrawUnits();
void DrawUnit(Unit* u, float rendz, uint32_t renderdepthtex=0, uint32_t renderfb=0);
bool PlaceUnit(int32_t type, Vec2i cmpos, int32_t owner, int32_t *reti);
void FreeUnits();
void UpdUnits();
void ResetPath(Unit* u);
void ResetGoal(Unit* u);
void ResetMode(Unit* u, bool chcollider = true);
void ResetTarget(Unit* u);
void StartBel(Unit* u);
int32_t CountU(int32_t utype);
int32_t CountU(int32_t utype, int32_t owner);
void CalcRot(Unit* u);

#endif
