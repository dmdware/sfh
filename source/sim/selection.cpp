











#include "selection.h"
#include "../math/matrix.h"
#include "../window.h"
#include "../math/plane3f.h"
#include "../math/frustum.h"
#include "../bsp/brush.h"
#include "utype.h"
#include "unit.h"
#include "bltype.h"
#include "building.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../math/hmapmath.h"
#include "../utils.h"
#include "../app/appmain.h"
#include "build.h"
#include "player.h"
#include "../gui/widgets/spez/cstrview.h"
#include "../gui/widgets/spez/blview.h"
#include "../gui/widgets/spez/truckmgr.h"
#include "../gui/gui.h"
#include "map.h"
#include "../render/drawsort.h"
#include "../sound/sound.h"
#include "../path/pathjob.h"
#include "../render/foliage.h"
#include "../sim/umove.h"

// Selection circle texture index
uint32_t g_circle = 0;
Selection g_sel;

void Selection::clear()
{
	units.clear();
	buildings.clear();
	roads.clear();
	powls.clear();
	crpipes.clear();
}

// Selection frustum for drag/area-selection
static Vec3f normalLeft;
static Vec3f normalTop;
static Vec3f normalRight;
static Vec3f normalBottom;
static float distLeft;
static float distTop;
static float distRight;
static float distBottom;

static Frustum g_selfrust;	//selection frustum

//is unit selected?
bool USel(int16_t ui)
{
	for(std::list<int32_t>::iterator sit=g_sel.units.begin(); sit!=g_sel.units.end(); sit++)
		if(*sit == ui)
			return true;

	return false;
}

//is bl selected?
bool BSel(int16_t bi)
{
	for(std::list<int32_t>::iterator sit=g_sel.buildings.begin(); sit!=g_sel.buildings.end(); sit++)
		if(*sit == bi)
			return true;

	return false;
}

void DrawMarquee()
{
	Player* py = &g_player[g_localP];

	if(!g_mousekeys[0] || g_keyintercepted || g_appmode != APPMODE_PLAY || g_build != BL_NONE)
		return;

#ifdef PLATFORM_MOBILE
	return;
#endif

#if 0
	EndS();
	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[SHADER_COLOR2D].slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(g_shader[SHADER_COLOR2D].slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(g_shader[SHADER_COLOR2D].slot[SSLOT_COLOR], 0, 1, 0, 0.75f);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].slot[SSLOT_POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].slot[SSLOT_TEXCOORD0]);
#endif

	float vertices[] =
	{
		//posx, posy    texx, texy
		(float)g_mousestart.x,	(float)g_mousestart.y, 0,			0, 0,
		(float)g_mousestart.x,	(float)g_mouse.y,0,				1, 0,
		(float)g_mouse.x,			(float)g_mouse.y,0,				1, 1,

		(float)g_mouse.x,			(float)g_mousestart.y,0,			1, 1,
		(float)g_mousestart.x,	(float)g_mousestart.y,0,			0, 1
	};
	
	Shader *s = &g_shader[g_curS];

#ifdef PLATFORM_GL14
	//glVertexAttribPointer(g_shader[SHADER_COLOR2D].slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
	glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
#endif
	
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}

#if 0
void DrawSel(Matrix* projection, Matrix* modelmat, Matrix* viewmat)
{
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	UseS(SHADER_COLOR3D);
	Shader* s = &g_shader[g_curS];
	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, projection->m_matrix);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat->m_matrix);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, viewmat->m_matrix);

	Matrix mvp;
#if 0
	mvp.set(modelview.m_matrix);
	mvp.postmult(g_camproj);
#elif 0
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(modelview);
#else
	mvp.set(projection->m_matrix);
	mvp.postmult(*viewmat);
	mvp.postmult(*modelmat);
#endif
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	float* color = g_player[g_localP].color;
	glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], 0.5f);

	Player* py = &g_player[g_localP];

	glLineWidth(3);

	for(std::list<Widget*>::iterator selit = g_sel.buildings.begin(); selit != g_sel.buildings.end(); selit++)
	{
		const int32_t bi = *selit;
		const Building* b = &g_building[bi];
		const BlType* t = &g_bltype[b->type];

		const int32_t tminx = b->tpos.x - t->width.x/2;
		const int32_t tminz = b->tpos.y - t->width.y/2;
		const int32_t tmaxx = tminx + t->width.x;
		const int32_t tmaxz = tminz + t->width.y;

		const int32_t cmminx = tminx*TILE_SIZE;
		const int32_t cmminy = tminz*TILE_SIZE;
		const int32_t cmmaxx = tmaxx*TILE_SIZE;
		const int32_t cmmaxy = tmaxz*TILE_SIZE;

		const int32_t off = TILE_SIZE/100;

		const float y1 = g_hmap.accheight(cmmaxx + off, cmminy - off) + TILE_SIZE/20;
		const float y2 = g_hmap.accheight(cmmaxx + off, cmmaxy + off) + TILE_SIZE/20;
		const float y3 = g_hmap.accheight(cmminx - off, cmmaxy + off) + TILE_SIZE/20;
		const float y4 = g_hmap.accheight(cmminx - off, cmminy - off) + TILE_SIZE/20;

		const float vertices[] =
		{
			//posx, posy posz
			(float)(cmmaxx + off), y1, (float)(cmminy - off),
			(float)(cmmaxx + off), y2, (float)(cmmaxy + off),
			(float)(cmminx - off), y3, (float)(cmmaxy + off),

			(float)(cmminx - off), y4, (float)(cmminy - off),
			(float)(cmmaxx + off), y1, (float)(cmminy - off)
		};

		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}


	glLineWidth(1);
	EndS();

	//if(g_projtype == PROJ_PERSPECTIVE)
	UseS(SHADER_BILLBOARD);
	//else
	//	UseS(SHADER_BILLBOAR);

	s = &g_shader[g_curS];

	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, projection->m_matrix);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat->m_matrix);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, viewmat->m_matrix);

	color = g_player[g_localP].color;
	glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], 1.0f);

	//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(s->slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(s->slot[SSLOT_NORMAL]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_circle ].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

	for(std::list<Widget*>::iterator selit = g_sel.units.begin(); selit != g_sel.units.end(); selit++)
	{
		Unit* u = &g_unit[ *selit ];
		//Entity* e = g_entity[ 0 ];
		Vec2f p = u->drawpos;
		UType* t = &g_utype[ u->type ];

		//Vec3f p = c->m_pos + Vec3f(0, t->vmin.y, 0) + Vec3f(0, 1.0f, 0);

		const float r = t->size.x * 1.0f;

#if 0
		float y1 = Bilerp(&g_hmap, p.x + r, p.z - r);
		float y2 = Bilerp(&g_hmap, p.x + r, p.z + r);
		float y3 = Bilerp(&g_hmap, p.x - r, p.z + r);
		float y4 = Bilerp(&g_hmap, p.x - r, p.z - r);
#elif 1
		const float y1 = g_hmap.accheight(p.x + r, p.y - r) + TILE_SIZE/20;
		const float y2 = g_hmap.accheight(p.x + r, p.y + r) + TILE_SIZE/20;
		const float y3 = g_hmap.accheight(p.x - r, p.y + r) + TILE_SIZE/20;
		const float y4 = g_hmap.accheight(p.x - r, p.y - r) + TILE_SIZE/20;
#else
		float y1 = p.y;
		float y2 = p.y;
		float y3 = p.y;
		float y4 = p.y;
#endif

		const float vertices[] =
		{
			//posx, posy posz   texx, texy
			p.x + r, y1, p.y - r,          1, 0,
			p.x + r, y2, p.y + r,          1, 1,
			p.x - r, y3, p.y + r,          0, 1,

			p.x - r, y3, p.y + r,          0, 1,
			p.x - r, y4, p.y - r,          0, 0,
			p.x + r, y1, p.y - r,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	EndS();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
#endif

void SelPtCd(Vec2i pt, Selection *sel)
{
	//for(int32_t i=0; i<BUILDINGS; i++)
	//for(std::list<Widget*>::iterator dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	for(std::list<Depthable*>::reverse_iterator dit=g_subdrawq.rbegin(); dit!=g_subdrawq.rend(); dit++)
	{
		Depthable* d = *dit;

		if(d->dtype != DEPTH_CD)
			continue;

		int16_t tx = d->index % g_mapsz.x;
		int16_t ty = d->index / g_mapsz.x;

		CdTile* ctile = GetCd(d->cdtype, tx, ty, d->plan);
		int32_t i = d->index;

		CdType* ct = &g_cdtype[ d->cdtype ];
		Tile& tile = SurfTile(tx, ty, &g_hmap);
		
		int32_t sli = ct->splist[ctile->conntype][(int32_t)ctile->finished];
		SpList* sl = &g_splist[ sli ];
		int32_t ci = SpriteRef(sl, 0, tile.incltype, 0, 0, 0,
			0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Vec3i cmpos = Vec3i( tx * TILE_SIZE + TILE_SIZE/2, ty * TILE_SIZE + TILE_SIZE/2, tile.elev * TILE_RISE );

		Vec2i screenpos = CartToIso(cmpos) - g_scroll;
		
		//Shouldn't happen unless user mod
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		Texture* tex = &g_texture[ sp->difftexi ];
		
		int32_t pixx = pt.x - (screenpos.x + (int32_t)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int32_t pixy = pt.y - (screenpos.y + (int32_t)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int32_t pixin = pixx + pixy * sp->pixels->sizex;
		
		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		std::list<Vec2i> *sellist = (std::list<Vec2i>*)(((char*)sel)+ct->seloff);
		sellist->push_back(Vec2i(tx,ty));
	}
}

/*
TODO
Go over all the code
remove commented unecessary code
remove if-0'd code
rewrite in C90
*/

int32_t SelPtBl(Vec2i pt)
{
	int32_t sel = -1;

	//for(int32_t i=0; i<BUILDINGS; i++)
	//for(std::list<Widget*>::iterator dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	for(std::list<Depthable*>::reverse_iterator dit=g_subdrawq.rbegin(); dit!=g_subdrawq.rend(); dit++)
	{
		Depthable* d = *dit;

		if(d->dtype != DEPTH_BL)
			continue;

		Building* b = &g_building[d->index];
		int32_t i = d->index;

		//if(!b->on)
		//	continue;

		BlType* t = &g_bltype[ b->type ];

#if 0
		const int32_t tminx = b->tpos.x - t->width.x/2;
		const int32_t tminz = b->tpos.y - t->width.y/2;
		const int32_t tmaxx = tminx + t->width.x;
		const int32_t tmaxz = tminz + t->width.y;

		const int32_t cmminx = tminx*TILE_SIZE;
		const int32_t cmminy = tminz*TILE_SIZE;
		const int32_t cmmaxx = tmaxx*TILE_SIZE;
		const int32_t cmmaxy = tmaxz*TILE_SIZE;

		const int32_t cmx = (cmminx+cmmaxx)/2;
		const int32_t cmz = (cmminy+cmmaxy)/2;
#endif

#if 0
		Depthable* d = b;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmax.z);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z);

		Vec2i top = CartToIso(top3);
		Vec2i bot = CartToIso(bot3);
		Vec2i lef = CartToIso(lef3);
		Vec2i rig = CartToIso(rig3);

		if(minx > rig.x)
			continue;

		if(miny > bot.y)
			continue;

		if(maxx < lef.x)
			continue;

		if(maxy < top.y)
			continue;
#endif
		
		Tile tile = SurfTile(b->tpos.x, b->tpos.y, &g_hmap);
		int32_t sli = t->splist;
		if(!b->finished)
			sli = t->csplist;
		SpList* sl = &g_splist[ sli ];
		int32_t ci = SpriteRef(sl, 0, tile.incltype, 0, 0, 0,
			0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Vec3i cmpos = Vec3i( b->tpos.x * TILE_SIZE + ((t->width.x % 2 == 1) ? TILE_SIZE/2 : 0),
			b->tpos.y * TILE_SIZE + ((t->width.y % 2 == 1) ? TILE_SIZE/2 : 0),
			SurfTile(b->tpos.x, b->tpos.y, &g_hmap).elev * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Texture* tex = &g_texture[ sp->difftexi ];

#if 0
		DrawImage(tex->texname,
			(float)screenpos.x + sp->offset[0],
			(float)screenpos.y + sp->offset[1],
			(float)screenpos.x + sp->offset[2],
			(float)screenpos.y + sp->offset[3]);
#endif

#if 0
		if(!sp->pixels)
			InfoMess("!","!1");
		if(!sp->pixels->data)
			InfoMess("!","!2");
#endif

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		int32_t pixx = pt.x - (screenpos.x + (int32_t)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int32_t pixy = pt.y - (screenpos.y + (int32_t)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int32_t pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

uint16_t SelPtFol(Vec2i pt)
{
	uint16_t sel = USHRT_MAX;

	//for(int32_t i=0; i<BUILDINGS; i++)
	for(std::list<Depthable*>::iterator dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	//for(std::list<Widget*>::iterator dit=g_subdrawq.rbegin(); dit!=g_subdrawq.rend(); dit++)
	{
		Depthable* d = *dit;

		if(d->dtype != DEPTH_FOL)
			continue;

		Foliage* f = &g_foliage[d->index];
		int32_t i = d->index;

		//if(!b->on)
		//	continue;

		FlType* t = &g_fltype[ f->type ];
		
		int32_t sli = t->splist;
		SpList* sl = &g_splist[ sli ];
		int32_t ci = SpriteRef(sl, 0, 0, 0, 0, 0, 0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Vec2i screenmin = d->pixmin - g_scroll;
		Vec2i screenmax = d->pixmax - g_scroll;
	
		if(screenmin.x >= g_width)
			continue;

		if(screenmin.y >= g_height)
			continue;

		if(screenmax.x < 0)
			continue;

		if(screenmax.y < 0)
			continue;

		Vec3i cmpos = Vec3i( f->cmpos.x, f->cmpos.y, Bilerp(&g_hmap, (float)f->cmpos.x, (float)f->cmpos.y) * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		int32_t pixx = pt.x - screenmin.x;

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int32_t pixy = pt.y - screenmin.y;

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int32_t pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

//select unit at point
int32_t SelPtU(Vec2i pt)
{
	int32_t sel = -1;
	
	for(std::list<Depthable*>::iterator dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	{
		Depthable* d = *dit;

		if(d->dtype != DEPTH_U)
			continue;

		Unit* u = &g_unit[d->index];
		int32_t i = d->index;

		//if(!b->on)
		//	continue;

		UType* t = &g_utype[ u->type ];

		//assert( dir < 8 );

		//int32_t spi = t->sprite[ dir % DIRS ][(int32_t)(u->frame[BODY_LOWER]) % sl->nframes ];
	
		int32_t sli = t->splist;
		SpList* sl = &g_splist[ sli ];
		unsigned char dir = (int32_t)( sl->nsides - ((int32_t)(  u->rotation.z / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) ) % sl->nsides;
		unsigned char pitch = (int32_t)( (sl->nsides - (int32_t)(sl->nsides -  u->rotation.x / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
		unsigned char roll = (int32_t)( (sl->nsides - (int32_t)(sl->nsides -  u->rotation.y / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
		int32_t ci = SpriteRef(sl, (int32_t)(u->frame[BODY_LOWER]) % sl->nframes, 0, pitch, dir, roll,
			0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Vec3i cmpos = Vec3i( u->cmpos.x, u->cmpos.y, Bilerp(&g_hmap, (float)u->cmpos.x, (float)u->cmpos.y) * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;
		
		int32_t pixx = pt.x - (screenpos.x + (int32_t)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int32_t pixy = pt.y - (screenpos.y + (int32_t)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int32_t pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

//select a point
Selection SelPt()
{
	int32_t selu = SelPtU(g_mouse);

	Selection sel;

#if 0
	InfoMess("sel one", "a");
#endif

	if(selu >= 0)
	{
#if 0
		InfoMess("sel one", "b");
#endif
		sel.units.push_back( selu );
	}

	int32_t selb = SelPtBl(g_mouse);

	if(selb >= 0)
	{
		sel.buildings.push_back( selb );
	}

	SelPtCd(g_mouse, &sel);
	
	uint16_t self = SelPtFol(g_mouse);

	if(self != USHRT_MAX)
	{
		sel.fol.push_back(self);
	}

	return sel;
}


Selection SelHover()
{
		int32_t selu = SelPtU(g_mouse);

	Selection sel;

#if 0
	InfoMess("sel one", "a");
#endif

	if(selu >= 0)
	{
#if 0
		InfoMess("sel one", "b");
#endif
		sel.units.push_back( selu );
	}

	int32_t selb = SelPtBl(g_mouse);

	if(selb >= 0)
	{
		sel.buildings.push_back( selb );
	}

	SelPtCd(g_mouse, &sel);
	
	uint16_t self = SelPtFol(g_mouse);

	if(self != USHRT_MAX)
	{
		sel.fol.push_back(self);
	}

	return sel;
}
//select area units
std::list<int32_t> SelArU(int32_t minx, int32_t miny, int32_t maxx, int32_t maxy)
{
	std::list<int32_t> unitsel;

	bool haveowned = false;
	bool haveowmili = false;

	for(int32_t i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UType* t = &g_utype[ u->type ];

		Depthable* d = u->depth;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmax.z);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z);

		Vec2i top = CartToIso(top3);
		Vec2i bot = CartToIso(bot3);
		Vec2i lef = CartToIso(lef3);
		Vec2i rig = CartToIso(rig3);

		if(minx > rig.x)
			continue;

		if(miny > bot.y)
			continue;

		if(maxx < lef.x)
			continue;

		if(maxy < top.y)
			continue;

		unitsel.push_back(i);

		if(u->owner == g_localP && u->type != UNIT_LABOURER)
		{
			haveowned = true;

			if(t->military)
				haveowmili = true;
		}
	}

	//filter units....

	if(haveowmili)
	{
		//only owned military

		std::list<int32_t>::iterator uit=unitsel.begin();
		while(uit!=unitsel.end())
		{
			Unit* u = &g_unit[*uit];

			if(u->type == UNIT_LABOURER)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			if(u->owner != g_localP)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			UType* t = &g_utype[u->type];

			if(!t->military)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			uit++;
		}
	}
	else if(haveowned)
	{
		//only owned (no labourers)

		std::list<int32_t>::iterator uit=unitsel.begin();
		while(uit!=unitsel.end())
		{
			Unit* u = &g_unit[*uit];

			if(u->type == UNIT_LABOURER)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			if(u->owner != g_localP)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			uit++;
		}
	}

	return unitsel;
}

//select area
Selection SelAr()
{
	Player* py = &g_player[g_localP];

	int32_t minx = imin(g_mousestart.x, g_mouse.x) + g_scroll.x;
	int32_t maxx = imax(g_mousestart.x, g_mouse.x) + g_scroll.x;
	int32_t miny = imin(g_mousestart.y, g_mouse.y) + g_scroll.y;
	int32_t maxy = imax(g_mousestart.y, g_mouse.y) + g_scroll.y;

	Selection selection;

	selection.units = SelArU(minx, miny, maxx, maxy);

	return selection;
}

Selection DoSel()
{
	Selection sel;

	if(g_mousestart.x == g_mouse.x && g_mousestart.y == g_mouse.y)
		sel = SelPt();
#ifndef PLATFORM_MOBILE
	//no annoying area select each time touch is dragged to scroll
	else
		sel = SelAr();
#endif

	return sel;
}

void ClearSel(Selection* s)
{
	s->buildings.clear();
#if 1
	s->crpipes.clear();
	s->powls.clear();
	s->roads.clear();
#else
	for(unsigned char ctype=0; ctype<CD_TYPES; ++ctype)
		s->cdtiles[ctype].clear();
#endif
	s->units.clear();
}

void AfterSel(Selection* s)
{
	bool haveconstr = false;
	bool havefini = false;
	bool havetruck = false;

	BlType* bt = NULL;
	CdType* ct = NULL;
	UType* ut = NULL;

	for(std::list<int32_t>::iterator selit = s->buildings.begin(); selit != s->buildings.end(); selit++)
	{
		int32_t bi = *selit;
		Building* b = &g_building[bi];
		bt = &g_bltype[b->type];

		if(!b->finished)
		{
			haveconstr = true;
			break;
		}
		else
		{
			havefini = true;
			break;
		}
	}

	if(!havefini && !haveconstr && s->units.size() > 0)
	{
		std::list<int32_t>::iterator selit = s->units.begin();
		int32_t ui = *selit;
		Unit* u = &g_unit[ui];
		if(u->type == UNIT_TRUCK)
			havetruck = true;
		ut = &g_utype[u->type];
	}

#if 1//TODO make abstract generic conduits use
	for(std::list<Vec2i>::iterator selit = s->roads.begin(); selit != s->roads.end(); selit++)
	{
		ct = &g_cdtype[CD_ROAD];
		CdTile* cdtile = GetCd(CD_ROAD, selit->x, selit->y, false);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->buildings.clear();
			haveconstr = true;
			break;
		}
	}

	for(std::list<Vec2i>::iterator selit = s->powls.begin(); selit != s->powls.end(); selit++)
	{
		ct = &g_cdtype[CD_POWL];
		CdTile* cdtile = GetCd(CD_POWL, selit->x, selit->y, false);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->buildings.clear();
			s->roads.clear();
			haveconstr = true;
			break;
		}
	}

	for(std::list<Vec2i>::iterator selit = s->crpipes.begin(); selit != s->crpipes.end(); selit++)
	{
		ct = &g_cdtype[CD_CRPIPE];
		CdTile* cdtile = GetCd(CD_CRPIPE, selit->x, selit->y, false);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->buildings.clear();
			s->roads.clear();
			s->powls.clear();
			haveconstr = true;
			break;
		}
	}
#else
	for(unsigned char ctype=0; ctype<CD_TYPES; ++ctype)
	{
		CdType* ct = &g_cdtype[ctype];

		//if(!ct->on)
		//	continue;

		if(!s->cdtiles[ctype].size())
			continue;

		for(std::list<Widget*>::iterator selit = s->cdtiles[ctype].begin(); selit != s->cdtiles[ctype].end(); selit++)
		{
			CdTile* cdtile = GetCd(ctype, selit->x, selit->y, false);

			if(!cdtile->finished)
			{
				bt = NULL;
				s->buildings.clear();

				for(unsigned char ctype2=0; ctype2<CD_TYPES; ++ctype2)
				{
					if(ctype2 == ctype)
						continue;

					s->cdtiles[ctype2].clear();
				}

				haveconstr = true;
				break;
			}
		}
	}
#endif

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->hide("bl graphs");

	if(haveconstr)
	{
		CstrView* cv = (CstrView*)gui->get("cs view");
		cv->regen(s);
		gui->show("cs view");

		if(bt)
			PlayClip(bt->sound[BLSND_CSEL]);
		//else if(ct)
		//	PlayClip(ct->sound[BLSND_CSEL]);
	}
	else if(havefini)
	{
		BlView* bv = (BlView*)gui->get("bl view");
		bv->regen(s);
		gui->show("bl view");
		PlayClip(bt->sound[BLSND_SEL]);
	}
	else if(havetruck)
	{
		TruckMgr* tm = (TruckMgr*)gui->get("truck mgr");
		tm->regen(s);
		gui->show("truck mgr");
		PlayClip(ut->sound[USND_SEL]);

#ifdef TSDEBUG
		tracku = &g_unit[ *s->units.begin() ];
#endif
	}

#if 0
	//35
	//2
	//15
	//39
	//12
	//457
	if(s->units.size() > 0)
	{
		int32_t ui = *s->units.begin();
		g_unit[ui].freecollider();
		bool cl = UnitCollides(&g_unit[ui], g_unit[ui].cmpos, g_unit[ui].type);
		g_unit[ui].fillcollider();
		char msg[128];
		g_unit[ui].freecollider();
		bool c = UnitCollides(&g_unit[ui], g_unit[ui].cmpos, g_unit[ui].type);
		
		g_unit[ui].fillcollider();

		bool t = Trapped(&g_unit[ui], NULL);

		UType* ut = &g_utype[g_unit[ui].type];

		int cmminx = g_unit[ui].cmpos.x - ut->size.x/2;
		int cmminy = g_unit[ui].cmpos.y - ut->size.x/2;
		int cmmaxx = cmminx + ut->size.x - 1;
		int cmmaxy = cmminy + ut->size.x - 1;

		sprintf(msg, "sel %d u cl=%d \n target=%d \n dgoal=%d,%d \n psz=%d \n supi=%d \n c=%d \n p=(%d,%d),(%d,%d) t=%d",
			*s->units.begin(), (int32_t)cl,
			g_unit[*s->units.begin()].target,
			g_unit[*s->units.begin()].goal.x - g_unit[*s->units.begin()].cmpos.x,
			g_unit[*s->units.begin()].goal.y - g_unit[*s->units.begin()].cmpos.y,
			(int)g_unit[*s->units.begin()].path.size(),
			(int)g_unit[*s->units.begin()].supplier,
			(int)c, cmminx, cmminy, cmmaxx, cmmaxy, (int)t);
		InfoMess(msg, msg);
	}
#endif
}
