











#include "utype.h"
#include "../texture.h"
#include "resources.h"
#include "../path/pathnode.h"
#include "../utils.h"
#include "../tool/rendersprite.h"

UType g_utype[UNIT_TYPES];

void DefU(int32_t type, const char* sprel, int32_t nframes,
	Vec2s size, const char* name,
	int32_t starthp,
	bool landborne, bool walker, bool roaded, bool seaborne, bool airborne,
	int32_t cmspeed, bool military,
	int32_t visrange,
	int32_t prop)
{
	UType* t = &g_utype[type];

	//QueueTex(
	//QueueModel(&t->model, sprel);

	t->free();

	//t->on = true;

#if 0
	for(int32_t s=0; s<DIRS; s++)
	{
		t->sprite[s] = new uint32_t [ nframes ];

		for(int32_t f=0; f<nframes; f++)
		{
			char frrel[WF_MAX_PATH+1];
			sprintf(frrel, "%s_si%d_fr%03d", sprel, s, f);
			QueueSprite(frrel, &t->sprite[s][f], true, true);
			//Log("q "<<frrel);
		}
	}
#elif 1
	if(!LoadSpriteList(sprel, &t->splist, true, true, true))
	{
		char m[128];
		sprintf(m, "Failed to load sprite list %s", sprel);
		ErrMess("Error", m);
	}
#else
	QueueRend(sprel, RENDER_UNSPEC,
		&t->splist,
		true, false, true, false,
		nframes, DIRS, 
		true, true, true, true,
		1);
#endif

	t->nframes = nframes;

	t->size = size;
	strcpy(t->name, name);
	t->starthp = starthp;
	Zero(t->cost);
	t->landborne = landborne;
	t->walker = walker;
	t->roaded = roaded;
	t->seaborne = seaborne;
	t->airborne = airborne;
	t->cmspeed = cmspeed;
	t->military = military;
	t->visrange = visrange;
	t->prop = prop;

	for(int32_t i=0; i<U_SOUNDS; i++)
		t->sound[i] = -1;
}

void UCost(int32_t type, int32_t res, int32_t amt)
{
	UType* t = &g_utype[type];
	t->cost[res] = amt;
}
