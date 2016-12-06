











#include "../texture.h"
#include "tile.h"
#include "../utils.h"
#include "../sim/map.h"
#include "../render/heightmap.h"

const char* INCLINENAME[] =
{
	"0000",
	"0001",
	"0010",
	"0011",
	"0100",
	"0101",
	"0110",
	"0111",
	"1000",
	"1001",
	"1010",
	"1011",
	"1100",
	"1101",
	"1110"
};

//InType g_intype[INCLINES];
uint32_t g_ground;
uint32_t g_100sph;
uint32_t g_fog0;
uint32_t g_fog1;

void DefTl(const char* sprel)
{
#if 0
	for(int32_t tti=0; tti<INCLINES; tti++)
	{
		InType* t = &g_intype[tti];
		//QueueTex(&t->sprite.texindex, texpath, true);
		//CreateTex(t->sprite.texindex, texpath, true, false);
		char specific[WF_MAX_PATH+1];
		sprintf(specific, "%s_inc%s", sprel, INCLINENAME[tti]);
		QueueSprite(specific, &t->sprite, false, true);
	}
#if 0
	t->sprite.offset[0] = spriteoffset.x;
	t->sprite.offset[1] = spriteoffset.y;
	t->sprite.offset[2] = t->sprite.offset[0] + spritesz.x;
	t->sprite.offset[3] = t->sprite.offset[1] + spritesz.y;
#endif
#endif
	if(!LoadSpriteList(sprel, &g_ground, false, true, true))
	{
		char m[128];
		sprintf(m, "Failed to load sprite list %s", sprel);
		ErrMess("Error", m);
	}
}

Tile &SurfTile(int32_t tx, int32_t ty, Heightmap* hm)
{
	tx = imin(tx, g_mapsz.x);
	ty = imin(ty, g_mapsz.y);
	return hm->m_surftile[ ty * g_mapsz.x + tx ];
}
