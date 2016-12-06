












#include "depthable.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "foliage.h"
#include "../sim/conduit.h"
#include "../math/hmapmath.h"

/*
Update drawabilities:
Update cm min max of depthable
and elevation y value and draw pos.
This happens every time the object
moves or is placed.
Determinacy is unimportant as this
only affects the subject experience
of the viewer, not the simulation,
so floating point can be used.
*/

//TODO pixel min/max updates

void UpDraw(Unit* u)
{
	Depthable* d = u->depth;
	UType* ut = &g_utype[u->type];
	d->cmmin.x = u->cmpos.x - ut->size.x / 2;
	d->cmmin.y = u->cmpos.y - ut->size.x / 2;
	//d->cmmin.z = (int32_t)( Bilerp(&g_hmap, (float)u->cmpos.x, (float)u->cmpos.y) * TILE_RISE );
	d->cmmin.z = (int32_t)( g_hmap.accheight2(u->cmpos.x, u->cmpos.y) );
	d->cmmax.x = d->cmmin.x + ut->size.x;
	d->cmmax.y = d->cmmin.y + ut->size.x;
	d->cmmax.z = d->cmmin.z + ut->size.y;

	UType* t = &g_utype[u->type];
	int32_t sli = t->splist;
	SpList* sl = &g_splist[ sli ];
	unsigned char dir = (int32_t)( sl->nsides - ((int32_t)(  u->rotation.z / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) ) % sl->nsides;
	unsigned char pitch = (int32_t)( (sl->nsides - (int32_t)(sl->nsides -  u->rotation.x / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
	unsigned char roll = (int32_t)( (sl->nsides - (int32_t)(sl->nsides -  u->rotation.y / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
	int32_t ci = SpriteRef(sl, (int32_t)(u->frame[BODY_LOWER]) % sl->nframes, 0, pitch, dir, roll, 0, 0);
	Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];
	Texture* difftex = &g_texture[sp->difftexi];

	//Log("ro %f", (float)(sl->nsides - ((int32_t)(  u->rotation.z / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) ));

	Vec3i cmpos;
	cmpos.x = u->cmpos.x;
	cmpos.y = u->cmpos.y;
	cmpos.z = (int32_t)( g_hmap.accheight2(u->cmpos.x, u->cmpos.y) );
	//cmpos.z = (int32_t)( g_hmap.accheight(u->cmpos.x, u->cmpos.y) );
	Vec2i isopos = CartToIso(cmpos);
	//Vec2i screenpos = isopos - g_scroll;
	Vec2i pixpos = isopos;

	//DrawImage(difftex->texname,
	//screenpos.x + sp->offset[0], screenpos.y + sp->offset[1],
	//screenpos.x + sp->offset[2], screenpos.y + sp->offset[3]);

	d->pixmin.x = pixpos.x + (int32_t)sp->offset[0];
	d->pixmin.y = pixpos.y + (int32_t)sp->offset[1];
	d->pixmax.x = pixpos.x + (int32_t)sp->offset[2];
	d->pixmax.y = pixpos.y + (int32_t)sp->offset[3];

	CartToDepth(cmpos, &d->rendz);

	//d->rendz -= 150;

	//u->drawpos = Vec3f(u->cmpos.x, u->cmpos.y, Bilerp(&g_hmap, (float)u->cmpos.x, (float)u->cmpos.y)*TILE_RISE);
}

void UpDraw(Building* b)
{
	Depthable* d = b->depth;
	BlType* bt = &g_bltype[b->type];
	d->cmmin.x = b->tpos.x * TILE_SIZE - bt->width.x / 2 * TILE_SIZE;
	d->cmmin.y = b->tpos.y * TILE_SIZE - bt->width.y / 2 * TILE_SIZE;
	d->cmmin.z = (int32_t)( SurfTile(b->tpos.x, b->tpos.y, &g_hmap).elev * TILE_RISE );
	d->cmmax.x = d->cmmin.x + bt->width.x * TILE_SIZE;
	d->cmmax.y = d->cmmin.y + bt->width.y * TILE_SIZE;
	//d->cmmax.z = d->cmmin.z + TILE_RISE;
	d->cmmax.z = d->cmmin.z + 150 * 2;
	
	Tile surf = SurfTile(b->tpos.x, b->tpos.y, &g_hmap);
	int32_t sli = bt->splist;
	if(!b->finished)
		sli = bt->csplist;
	SpList* sl = &g_splist[ sli ];
	int32_t ci = SpriteRef(sl, 0, surf.incltype, 0, 0, 0, 0, 0);
	Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

	Vec3i cmpos = Vec3i( b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0), b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0), surf.elev * TILE_RISE );
	Vec2i pixpos = CartToIso(cmpos);

	Texture* tex = &g_texture[ sp->difftexi ];

	//DrawImage(difftex->texname,
	//screenpos.x + sp->offset[0], screenpos.y + sp->offset[1],
	//screenpos.x + sp->offset[2], screenpos.y + sp->offset[3]);

	d->pixmin.x = pixpos.x + (int32_t)sp->offset[0];
	d->pixmin.y = pixpos.y + (int32_t)sp->offset[1];
	d->pixmax.x = pixpos.x + (int32_t)sp->offset[2];
	d->pixmax.y = pixpos.y + (int32_t)sp->offset[3];
	
	CartToDepth(cmpos, &d->rendz);
}

void UpDraw(Foliage* f)
{
	Depthable* d = f->depth;
	FlType* ft = &g_fltype[f->type];
	//d->cmmin.x = f->cmpos.x - ft->size.x / 2;
	//d->cmmin.y = f->cmpos.y - ft->size.x / 2;
	d->cmmin.x = f->cmpos.x;
	d->cmmin.y = f->cmpos.y;
	d->cmmin.z = (int32_t) g_hmap.accheight2(f->cmpos.x, f->cmpos.y);
	//d->cmmin.z = (int32_t)( Bilerp(&g_hmap, (float)f->cmpos.x, (float)f->cmpos.y) * TILE_RISE );
	//d->cmmax.x = d->cmmin.x + ft->size.x;
	//d->cmmax.y = d->cmmin.y + ft->size.x;
	d->cmmax.x = d->cmmin.x;
	d->cmmax.y = d->cmmin.y;
	d->cmmax.z = d->cmmin.z + ft->size.y;
	
	int32_t sli = ft->splist;
	SpList* sl = &g_splist[ sli ];
	int32_t ci = SpriteRef(sl, 0, 0, 0, 0, 0, 0, 0);
	Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

	Texture* difftex = &g_texture[sp->difftexi];

	Vec3i cmpos;
	cmpos.x = f->cmpos.x;
	cmpos.y = f->cmpos.y;
	//cmpos.z = (int32_t)( Bilerp(&g_hmap, f->cmpos.x, f->cmpos.y) * TILE_RISE );
	cmpos.z = (int32_t) g_hmap.accheight2(f->cmpos.x, f->cmpos.y);
	Vec2i pixpos = CartToIso(cmpos);
	//Vec2i screenpos = isopos - g_scroll;

	d->pixmin.x = pixpos.x + (int32_t)sp->offset[0];
	d->pixmin.y = pixpos.y + (int32_t)sp->offset[1];
	d->pixmax.x = pixpos.x + (int32_t)sp->offset[2];
	d->pixmax.y = pixpos.y + (int32_t)sp->offset[3];
	
	CartToDepth(cmpos, &d->rendz);
	//d->rendz -= ft->size.x;
}

void UpDraw(CdTile* c, unsigned char ctype, int32_t tx, int32_t ty)
{
	Depthable* d = c->depth;
	CdType* ct = &g_cdtype[ctype];
#if 0
	d->cmmin.x = tx * TILE_SIZE + ct->physoff.x - TILE_SIZE / 2;
	d->cmmin.x = ty * TILE_SIZE + ct->physoff.y - TILE_SIZE / 2;
	//d->cmmin.z = (int32_t)( Bilerp(&g_hmap, (float)(tx * TILE_SIZE + ct->physoff.x), (float)(ty * TILE_SIZE + ct->physoff.y)) * TILE_RISE );
	d->cmmin.z = (int32_t)( Bilerp(&g_hmap, (float)(tx * TILE_SIZE + ct->physoff.x), (float)(ty * TILE_SIZE + ct->physoff.y)) * TILE_RISE );
	d->cmmax.x = d->cmmin.x + TILE_SIZE;
	d->cmmax.y = d->cmmin.y + TILE_SIZE;
	d->cmmax.z = d->cmmin.z + TILE_RISE;
#else

	//offset by 1 so that it will always be in front of building on same tile
	d->cmmin.x = tx * TILE_SIZE + ct->drawoff.x + 1;
	d->cmmin.y = ty * TILE_SIZE + ct->drawoff.y + 1;
	Tile& tile = SurfTile(tx, ty, &g_hmap);
	//d->cmmin.z = (int32_t)( Bilerp(&g_hmap, (float)(tx * TILE_SIZE + ct->physoff.x), (float)(ty * TILE_SIZE + ct->physoff.y)) * TILE_RISE );
	d->cmmin.z = (int32_t)( tile.elev * TILE_RISE );
	d->cmmax.x = d->cmmin.x + TILE_SIZE;
	d->cmmax.y = d->cmmin.y + TILE_SIZE;
	//d->cmmax.z = d->cmmin.z + TILE_RISE * 2;
	d->cmmax.z = d->cmmin.z;
#endif
	
	int32_t sli = ct->splist[c->conntype][(int32_t)c->finished];
	SpList* sl = &g_splist[ sli ];
	int32_t ci = SpriteRef(sl, 0, tile.incltype, 0, 0, 0, 0, 0);
	Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];
	Vec3i cmpos = Vec3i( tx * TILE_SIZE + TILE_SIZE/2, ty * TILE_SIZE + TILE_SIZE/2, tile.elev * TILE_RISE );
	Vec2i pixpos = CartToIso(cmpos);
	Texture* tex = &g_texture[ sp->difftexi ];

	//DrawImage(tex->texname,
	//(float)screenpos.x + sp->offset[0],
	//(float)screenpos.y + sp->offset[1],
	//(float)screenpos.x + sp->offset[2],
	//(float)screenpos.y + sp->offset[3]);

	d->pixmin.x = pixpos.x + (int32_t)sp->offset[0];
	d->pixmin.y = pixpos.y + (int32_t)sp->offset[1];
	d->pixmax.x = pixpos.x + (int32_t)sp->offset[2];
	d->pixmax.y = pixpos.y + (int32_t)sp->offset[3];
	
	CartToDepth(cmpos, &d->rendz);

	//d->rendz -= 5;
}
