










#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "../platform.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"

class BillbType
{
public:
	bool on;
	char name[32];
	uint32_t tex;

	BillbType()
	{
		on = false;
	}
};

#define BILLBOARD_TYPES			64
extern BillbType g_billbT[BILLBOARD_TYPES];

class Billboard
{
public:
	bool on;
	int32_t type;
	float size;
	Vec3f pos;
	float dist;
	int32_t particle;

	Billboard()
	{
		on = false;
		particle = -1;
	}
};

#define BILLBOARDS  256
extern Billboard g_billb[BILLBOARDS];

extern uint32_t g_muzzle[4];

void Effects();
int32_t NewBillboard();
int32_t NewBillboard(char* tex);
int32_t IdentifyBillboard(const char* name);
void SortBillboards();
void DrawBillboards();
void PlaceBillboard(const char* n, Vec3f pos, float size, int32_t particle=-1);
void PlaceBillboard(int32_t type, Vec3f pos, float size, int32_t particle=-1);

#endif
