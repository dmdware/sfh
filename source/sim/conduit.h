











#ifndef CONDUIT_H
#define CONDUIT_H

#include "connectable.h"
#include "resources.h"
#include "../render/vertexarray.h"
#include "../math/vec3i.h"
#include "../render/heightmap.h"
#include "../render/depthable.h"

#if 1
#define CD_ROAD		0
#define CD_POWL		1
#define CD_CRPIPE	2
#define CD_TYPES	3
#endif
//extern int32_t g_ncdtypes;
//#define CD_TYPES	128

class CdTile
{
public:

	Depthable* depth;

	bool on;
	unsigned char conntype;
	bool finished;
	unsigned char owner;
	int32_t conmat[RESOURCES];
	int16_t netw;	//network
	//bool inaccessible;
	int16_t transporter[RESOURCES];
	Vec2f drawpos;
	//int32_t maxcost[RESOURCES];
	int32_t conwage;

	bool selling;

	CdTile();
	~CdTile();

	//virtual unsigned char cdtype();
	int32_t netreq(int32_t res, unsigned char cdtype);
	void destroy();
	void allocate(unsigned char cdtype);
	bool checkconstruction(unsigned char cdtype);
	virtual void fillcollider();
	virtual void freecollider();
};

class CdType
{
public:
	//bool on;
	uint32_t icontex;
	int32_t conmat[RESOURCES];
	uint16_t netwoff;	//offset to network list in Building class
	uint16_t seloff;	//offset to selection list in Selection class
	//TO DO elevation inclines
	//uint32_t sprite[CONNECTION_TYPES][2][INCLINES];	//0 = not finished, 1 = finished/constructed
	uint32_t splist[CONNECTION_TYPES][2];
	uint16_t maxforwincl;
	uint16_t maxsideincl;
	bool blconduct;	//do buildings conduct this resource (also act as conduit in a network?)
	Vec2i physoff;	//offset in cm. physoff from tile(tx,ty) corner, where workers go to build it (collider).
	Vec3i drawoff;	//offset in cm. drawoff from tile(tx,ty) corner, where the depthable sorted box begins.
	CdTile* cdtiles[2];	//0 = actual placed, 1 = plan proposed
	bool cornerpl;	//is the conduit centered on corners or tile centers?
	char name[256];
	std::string desc;
	uint32_t lacktex;
	//reqsource indicates if being connected to the grid 
	//requires connection to a source building, or just the network.
	bool reqsource;
	unsigned char flags;

	CdType()
	{
		//on = true;
		Zero(conmat);
		blconduct = false;
		cdtiles[0] = NULL;
		cdtiles[1] = NULL;
		cornerpl = false;
		name[0] = '\0';
		lacktex = 0;
		icontex = 0;
		reqsource = false;
	}

	~CdType()
	{
		for(int32_t i=0; i<2; i++)
			if(cdtiles[i])
			{
				delete [] cdtiles[i];
				cdtiles[i] = NULL;
			}
	}
};

//extern CdType g_cdtype[CD_TYPES];
extern CdType g_cdtype[CD_TYPES];

inline CdTile* GetCd(unsigned char ctype, int32_t tx, int32_t ty, bool plan)
{
	CdType* ct = &g_cdtype[ctype];
	CdTile* tilesarr = ct->cdtiles[(int32_t)plan];
	return &tilesarr[ tx + ty*(g_mapsz.x+0) ];
}

class Building;
struct PlaceCdPacket;

int32_t CdWorkers(unsigned char ctype, int32_t tx, int32_t ty);
void DefCd(unsigned char ctype,
			const char* name,
		   const char* iconrel,
          /* */ uint16_t netwoff,
           uint16_t seloff, /* */
           bool blconduct,
           bool cornerpl,
           Vec2i physoff,
           Vec3i drawoff,
		   const char* lacktex,
		   uint16_t maxsideincl,
		   uint16_t maxforwincl,
		   bool reqsource,
		  unsigned char flags);
void CdDes(unsigned char ctype, const char* desc);
void CdMat(unsigned char ctype, unsigned char rtype, int16_t ramt);
void UpdCdPlans(unsigned char ctype, char owner, Vec3i start, Vec3i end);
void ClearCdPlans(unsigned char ctype);
void ReNetw(unsigned char ctype);
void ResetNetw(unsigned char ctype);
bool ReNetwB(unsigned char ctype);
void MergeNetw(unsigned char ctype, int32_t A, int32_t B);
bool ReNetwTl(unsigned char ctype);
bool CompareCo(unsigned char ctype, CdTile* ctile, int32_t tx, int32_t ty);
bool BAdj(unsigned char ctype, int32_t i, int32_t tx, int32_t ty);
bool CompareB(unsigned char ctype, Building* b, CdTile* ctile);
bool CdLevel(unsigned char ctype, float iterx, float iterz, float testx, float testz, float dx, float dz, int32_t i, float d, bool plantoo);
void PlaceCd(unsigned char ctype, int32_t ownerpy);
void PlaceCd(PlaceCdPacket* pcp);
void PlaceCd(unsigned char ctype, int32_t tx, int32_t ty, int32_t owner, bool plan);
void Repossess(unsigned char ctype, int32_t tx, int32_t ty, int32_t owner);
void DrawCd(CdTile* ctile, unsigned char x, unsigned char y, unsigned char cdtype, bool plan, float rendz,
			uint32_t renderdepthtex=0, uint32_t renderfb=0);
void DrawCd(unsigned char ctype);
void CdXY(unsigned char ctype, CdTile* ctile, bool plan, int32_t& tx, int32_t& ty);
void DefConn(unsigned char conduittype,
	unsigned char connectiontype,
	bool finished,
	const char* sprel,
	int rendtype);
void PruneCd(unsigned char ctype);
void ConnectCdAround(unsigned char ctype, int32_t x, int32_t y, bool plan);

#endif
