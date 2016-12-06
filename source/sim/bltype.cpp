












#include "bltype.h"
#include "../sound/sound.h"
#include "../tool/rendersprite.h"

BlType g_bltype[BL_TYPES];

BlType::BlType()
{
	//sprite = NULL;
	//csprite = NULL;
	nframes = 0;
	cnframes = 0;
	//on = false;
}

BlType::~BlType()
{
	free();
}

void BlType::free()
{
	//on = false;

#if 0
	if(sprite)
	{
		delete [] sprite;
		sprite = NULL;
	}

	if(csprite)
	{
		delete [] csprite;
		csprite = NULL;
	}
#endif
}

void DefB(int32_t type,
		  const char* name,
		  const char* iconrel,
		  Vec2i size,
		  bool hugterr,
		  const char* sprel,
		  int32_t nframes,
		  const char* csprel,
		  int32_t cnframes,
		  int32_t foundation,
		  int32_t reqdeposit,
		  int32_t maxhp,
		  int32_t visrange,
		  int32_t opwage,
		  unsigned char flags,
		  int32_t prop,
		  int32_t wprop,
		  int32_t srate)
{
	BlType* t = &g_bltype[type];

	t->free();

	t->width.x = size.x;
	t->width.y = size.y;
	sprintf(t->name, name);

	QueueTex(&t->icontex, iconrel, true, false);

	//QueueModel(&t->model, sprel, scale, translate);
	//QueueModel(&t->cmodel, csprel, cscale, ctranslate);

#if 0
	/*
	TODO
	Neater way to do this by adding up a string together
	from pieces.
	*/
	if(hugterr)
	{
		t->sprite = new uint32_t [INCLINES];
		t->csprite = new uint32_t [INCLINES];

		for(int32_t i=0; i<INCLINES; i++)
		{
			char specific[WF_MAX_PATH+1];
			sprintf(specific, "%s_inc%s", sprel, INCLINENAME[i]);
			QueueSprite(specific, &t->sprite[i], true, true);
		}
		
		for(int32_t i=0; i<INCLINES; i++)
		{
			char specific[WF_MAX_PATH+1];
			sprintf(specific, "%s_inc%s", csprel, INCLINENAME[i]);
			QueueSprite(specific, &t->csprite[i], true, true);
		}
	}
	else
	{
		t->sprite = new uint32_t [nframes];
		t->csprite = new uint32_t [cnframes];

		for(int32_t f=0; f<nframes; f++)
		{
			char specific[WF_MAX_PATH+1];
			sprintf(specific, "%s_fr%03d", sprel, f);
			QueueSprite(specific, &t->sprite[f], true, true);
		}

		for(int32_t f=0; f<cnframes; f++)
		{
			char specific[WF_MAX_PATH+1];
			sprintf(specific, "%s_fr%03d", csprel, f);
			QueueSprite(specific, &t->csprite[f], true, true);
		}
	}
#elif 1
	if(!LoadSpriteList(sprel, &t->splist, true, true, true))
	{
		char m[128];
		sprintf(m, "Failed to load sprite list %s", sprel);
		ErrMess("Error", m);
	}

	if(!LoadSpriteList(csprel, &t->csplist, true, true, true))
	{
		char m[128];
		sprintf(m, "Failed to load sprite list %s", sprel);
		ErrMess("Error", m);
	}
#else
	//hugterr=false;
	QueueRend(sprel, RENDER_UNSPEC,
		&t->splist,
		true, hugterr, false, false,
		nframes, 1, 
		true, true, true, true,
		1);
	QueueRend(csprel, RENDER_UNSPEC,
		&t->csplist,
		true, hugterr, false, false,
		cnframes, 1, 
		true, true, true, true,
		1);
#endif

	t->foundation = foundation;
	t->hugterr = hugterr;

	Zero(t->input);
	Zero(t->output);
	Zero(t->conmat);

	t->reqdeposit = reqdeposit;

	for(int32_t i=0; i<BL_SOUNDS; i++)
		t->sound[i] = -1;

	t->maxhp = maxhp;
	t->manuf.clear();
	t->visrange = visrange;

	Zero(t->price);

	t->opwage = opwage;
	t->flags = flags;
	t->prop = prop;
	t->wprop = wprop;
	t->stockingrate = srate;

	t->nframes = nframes;
}


void DefB(int32_t type,
		  const char* name,
		  const char* iconrel,
		  Vec2i size,
		  Vec2i cmalign,
		  Vec2i cmalignoff,
		  bool hugterr,
		  const char* sprel,
		  int32_t nframes,
		  const char* csprel,
		  int32_t cnframes,
		  int32_t foundation,
		  int32_t reqdeposit,
		  int32_t maxhp,
		  int32_t visrange,
		  int32_t opwage,
		  unsigned char flags,
		  int32_t prop,
		  int32_t wprop,
		  int32_t srate)
{
	BlType* t = &g_bltype[type];

	t->free();

	t->width.x = size.x;
	t->width.y = size.y;
	sprintf(t->name, name);
	t->cmalign = cmalign;
	t->cmalignoff = cmalignoff;

	QueueTex(&t->icontex, iconrel, true, false);

//	hugterr=false;
	QueueRend(sprel, RENDER_UNSPEC,
		&t->splist,
		true, hugterr, false, false,
		nframes, 1, 
		true, true, true, true,
		1);
	QueueRend(csprel, RENDER_UNSPEC,
		&t->csplist,
		true, hugterr, false, false,
		cnframes, 1, 
		true, true, true, true,
		1);

	t->foundation = foundation;
	t->hugterr = hugterr;

	Zero(t->input);
	Zero(t->output);
	Zero(t->conmat);

	t->reqdeposit = reqdeposit;

	for(int32_t i=0; i<BL_SOUNDS; i++)
		t->sound[i] = -1;

	t->maxhp = maxhp;
	t->manuf.clear();
	t->visrange = visrange;

	Zero(t->price);

	t->opwage = opwage;
	t->flags = flags;
	t->prop = prop;
	t->wprop = wprop;
	t->stockingrate = srate;
}

void BMan(int32_t type, unsigned char utype)
{
	BlType* t = &g_bltype[type];
	t->manuf.push_back(utype);
}

void BSon(int32_t type, int32_t stype, const char* relative)
{
	BlType* t = &g_bltype[type];
	LoadSound(relative, &t->sound[stype]);
}

void BDes(int32_t type, const char* desc)
{
	BlType* t = &g_bltype[type];
	t->desc = desc;
}

void BMat(int32_t type, int32_t res, int32_t amt)
{
	BlType* t = &g_bltype[type];
	t->conmat[res] = amt;
}

void BIn(int32_t type, int32_t res, int32_t amt)
{
	BlType* t = &g_bltype[type];
	t->input[res] = amt;
}

//defl = default price
void BOut(int32_t type, int32_t res, int32_t amt, int32_t defl)
{
	BlType* t = &g_bltype[type];
	t->output[res] = amt;
	t->price[res] = defl;
}

void BEmit(int32_t type, int32_t emitterindex, int32_t ptype, Vec3i offset)
{
	BlType* t = &g_bltype[type];
	EmitterPlace* e = &t->emitterpl[emitterindex];
	*e = EmitterPlace(ptype, Vec3f(offset.x,offset.y,offset.z));	//TODO use Vec3i everywhere
}
