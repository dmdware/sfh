











#include "unit.h"
#include "../render/shader.h"
#include "utype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../math/hmapmath.h"
#include "umove.h"
#include "simdef.h"
#include "simflow.h"
#include "labourer.h"
#include "../debug.h"
#include "../math/frustum.h"
#include "building.h"
#include "labourer.h"
#include "truck.h"
#include "../econ/demand.h"
#include "../math/vec4f.h"
#include "../path/pathjob.h"
#include "../render/anim.h"
#include "map.h"
#include "../render/drawqueue.h"
#include "../render/drawsort.h"
#include "../path/collidertile.h"
#include "../path/pathjob.h"
#include "../render/fogofwar.h"
#include "../gui/layouts/chattext.h"

#ifdef RANDOM8DEBUG
int32_t thatunit = -1;
#endif

Unit g_unit[UNITS];


int32_t CountU(int32_t utype)
{
	int32_t c = 0;

	for(int32_t i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;
		
		if(u->type != utype)
			continue;

		c++;
	}

	return c;
}

int32_t CountU(int32_t utype, int32_t owner)
{
	int32_t c = 0;

	for(int32_t i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;
		
		if(u->type != utype)
			continue;

		if(u->owner != owner)
			continue;

		c++;
	}

	return c;
}

Unit::Unit()
{
	on = false;
	threadwait = false;
	depth = NULL;
	//filled = false;
}

Unit::~Unit()
{
	destroy();
}

void Unit::destroy()
{
	//corpc fix
	if(on)
		ResetMode(this);	//make sure to disembark, reset truck driver, etc.

	//corpc fix TODO
	//if(g_collider.m_size && on)
	if(on && g_pathnode)
		freecollider();

	on = false;
	threadwait = false;

	if(home >= 0)
		Evict(this, true);

	if(mode == UMODE_GOSUP
		//|| mode == GOINGTOREFUEL
			//|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
				)
	{
		if(supplier >= 0)
		{
			//Building* b = &g_building[supplier];
			//b->transporter[cargotype] = -1;
		}
	}

	if(type == UNIT_TRUCK)
	{
		bool targbl = false;
		bool targcd = false;

		if( (mode == UMODE_GOSUP || mode == UMODE_ATSUP || mode == UMODE_GOREFUEL || mode == UMODE_REFUELING) && targtype == TARG_BL )
			targbl = true;

		if( (mode == UMODE_GOSUP || mode == UMODE_ATSUP || mode == UMODE_GOREFUEL || mode == UMODE_REFUELING) && targtype == TARG_CD )
			targbl = true;

		if(mode == UMODE_GODEMB)
			targbl = true;

		if(mode == UMODE_ATDEMB)
			targbl = true;

		if(mode == UMODE_GODEMCD)
			targcd = true;

		if(mode == UMODE_ATDEMCD)
			targcd = true;

		if(targtype == TARG_BL)
			targbl = true;

		if(targtype == TARG_CD)
			targcd = true;

		if( targbl )
		{
			if(target >= 0)
			{
				Building* b = &g_building[target];
				b->transporter[cargotype] = -1;
			}
		}
		else if( targcd )
		{
			if(target >= 0 && target2 >= 0 && cdtype >= 0)
			{
				CdTile* ctile = GetCd(cdtype, target, target2, false);
				ctile->transporter[cargotype] = -1;
			}
		}
	}

	cyclehist.clear();

	std::list<Depthable>::iterator qit=g_drawlist.begin();
	while(qit!=g_drawlist.end())
	{
		if(&*qit != depth)
		{
			qit++;
			continue;
		}

		qit = g_drawlist.erase(qit);
	}
	depth = NULL;
}

/*
How is this diff from ResetPath?
Find out and fix this.
*/
void Unit::resetpath()
{
	path.clear();
	tpath.clear();
	subgoal = cmpos;
	goal = cmpos;
	pathblocked = false;
}

#if 0
void DrawUnits()
{
	Shader* s = &g_shader[g_curS];

	for(int32_t i=0; i<UNITS; i++)
	{
		StartTimer(TIMER_DRAWUMAT);

		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hidden())
			continue;

		UType* t = &g_utype[u->type];
#if 0
		Vec3f vmin(u->drawpos.x - t->size.x/2, u->drawpos.y, u->drawpos.y - t->size.x/2);
		Vec3f vmax(u->drawpos.x + t->size.x/2, u->drawpos.y + t->size.y, u->drawpos.y + t->size.x/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		Player* py = &g_player[u->owner];
		float* color = py->color;
		glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

		Model* m = &g_model[t->model];
#endif
		StopTimer(TIMER_DRAWUMAT);

		//m->draw(u->frame[BODY_LOWER], u->drawpos, u->rotation.z);
		Sprite* sp = &g_sprite[t->sprite[0][0]];
		Texture* difftex = &g_texture[sp->difftexi];
		Texture* depthtex = &g_texture[sp->depthtexi];

		Vec3i cm3pos;
		cm3pos.x = u->cmpos.x;
		cm3pos.y = u->cmpos.y;
		cm3pos.z = (int32_t)( g_hmap.accheight(u->cmpos.x, u->cmpos.y) * TILE_RISE );
		Vec2i isopos = CartToIso(cm3pos);
		Vec2i screenpos = isopos - g_scroll;

#if 0
		DrawImage(difftex->texname,
			(float)screenpos.x + sp->offset[0], (float)screenpos.y + sp->offset[1],
			(float)screenpos.x + sp->offset[2], (float)screenpos.y + sp->offset[3], 
			0,0,1,1, g_gui.m_crop);
#else
		DrawDeep(difftex->texname, depthtex->texname, 0,
			(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
			(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
			sp->crop[0], sp->crop[1],
			sp->crop[2], sp->crop[3]);
#endif
	}
}
#endif


void CalcRot(Unit* u)
{
		Vec3f center = Vec3f(u->cmpos.x, u->cmpos.y, g_hmap.accheight(u->cmpos.x, u->cmpos.y) * TILE_RISE);
			
#if 0
		UType* ut = &g_utype[u->type];

		Vec3f sidevec = center + Vec3f(ut->size.x/2, 0, 0);
		Vec3f frontvec = center + Vec3f(0, ut->size.x/2, 0);
		
		//TODO get rid of deg-to-rad rad-to-deg conversions and use rad for all
		//sidevec = RotateAround(sidevec, center, ( u->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, ( u->rotation.z ), 0, 0, 1);
		
		//sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y);
		frontvec.z = g_hmap.accheight(frontvec.x, frontvec.y) * TILE_RISE;
		
		//sidevec = RotateAround(sidevec, center, -( u->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, -( u->rotation.z ), 0, 0, 1);

		u->rotation.x = atan2(frontvec.x, frontvec.z) - M_PI/2;	//pitch
		
		frontvec = RotateAround(frontvec, center, ( u->rotation.z ), 0, 0, 1);
		Vec3f viewvec = Normalize(frontvec - center);
		
		sidevec = RotateAround(sidevec, center, ( u->rotation.z ), 0, 0, 1);
		sidevec = RotateAround(sidevec, center, ( u->rotation.x ), viewvec.x, viewvec.y, viewvec.z);

		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y) * TILE_RISE;
		
		sidevec = RotateAround(sidevec, center, -( u->rotation.x ), viewvec.x, viewvec.y, viewvec.z);
		sidevec = RotateAround(sidevec, center, -( u->rotation.z ), 0, 0, 1);

		u->rotation.y = atan2(sidevec.y, sidevec.z) - M_PI/2; //roll
		//u->rotation.y = 0;
		//u->rotation.x = 0;
		//u->rotation.x = u->rotation.x + 40;

		//if(u->rotation.x > 360)
		//	u->rotation.x = 0;
#elif 1
		
		UType* ut = &g_utype[u->type];

		Vec3f sidevec = center + Vec3f(-ut->size.x/2, 0, 0);
		Vec3f frontvec = center + Vec3f(0, ut->size.x/2, 0);
		
		//TODO get rid of deg-to-rad rad-to-deg conversions and use rad for all
		sidevec = RotateAround(sidevec, center, ( u->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, ( u->rotation.z ), 0, 0, 1);
		
		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y) * TILE_RISE;
		frontvec.z = g_hmap.accheight(frontvec.x, frontvec.y) * TILE_RISE;
		
		sidevec = RotateAround(sidevec, center, -( u->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, -( u->rotation.z ), 0, 0, 1);

		u->rotation.x = atan2(frontvec.x, frontvec.z) - M_PI/2;	//pitch
#if 0
		frontvec = RotateAround(frontvec, center, ( u->rotation.z ), 0, 0, 1);
		Vec3f viewvec = Normalize(frontvec - center);
		
		sidevec = RotateAround(sidevec, center, ( u->rotation.z ), 0, 0, 1);
		sidevec = RotateAround(sidevec, center, ( u->rotation.x ), viewvec.x, viewvec.y, viewvec.z);

		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y);
		
		sidevec = RotateAround(sidevec, center, -( u->rotation.x ), viewvec.x, viewvec.y, viewvec.z);
		sidevec = RotateAround(sidevec, center, -( u->rotation.z ), 0, 0, 1);
#endif

		u->rotation.y = atan2(sidevec.y, sidevec.z) - M_PI/2; //roll
		//u->rotation.y = 0;
		//u->rotation.x = 0;
		//u->rotation.x = u->rotation.x + 40;

		//if(u->rotation.x > 360)
		//	u->rotation.x = 0;

#elif 0
		Vec3f sidevec = center + Vec3f(TILE_SIZE/2, 0, 0);
		Vec3f frontvec = center + Vec3f(0, TILE_SIZE/2, 0);
		
		//TODO get rid of deg-to-rad rad-to-deg conversions and use rad for all
		sidevec = RotateAround(sidevec, center, ( u->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, ( u->rotation.z ), 0, 0, 1);
		
		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y);
		frontvec.z = g_hmap.accheight(frontvec.x, frontvec.y);
		
		sidevec = sidevec - center;
		frontvec = frontvec - center;
		
		//sidevec.z *= 2;
		//frontvec.z *= 2;

#if 1
		
		sidevec = Normalize(sidevec);
		frontvec = Normalize(frontvec);

		//yaw
		Vec3f d = frontvec;
		//u->rotation.z = atan2(d.x, d.y);

		//pitch
		d = frontvec;
		float lateral = MAG_VEC3F(Vec3f(d.x, d.y, 0));
		u->rotation.x = ( atan2(d.z, lateral) ) * 500;

		//roll
		d = sidevec;
		lateral = MAG_VEC3F(Vec3f(d.x, d.y, 0));
		u->rotation.y = ( atan2(d.z, lateral) ) * 500;
#endif
#if 0
		if(sidevec.z < 0)
			u->rotation.y = 360 - 360/SDIRS;
		else if(sidevec.z > 0)
			u->rotation.y = 360/SDIRS;
		else
			u->rotation.y = 0;
		
		if(frontvec.z < 0)
			u->rotation.x = 360 - 360/SDIRS;
		else if(frontvec.z > 0)
			u->rotation.x = 360/SDIRS;
		else
			u->rotation.x = 0;
#endif
#elif 1

		UType* ut = &g_utype[u->type];

		// http://www.jldoty.com/code/DirectX/YPRfromUF/YPRfromUF.html
		
#if 0
		Vec3f sidevec = center + Vec3f(-ut->size.x/2, 0, 0);
		Vec3f side2vec = center + Vec3f(ut->size.x/2, 0, 0);
		Vec3f frontvec = center + Vec3f(0, ut->size.x/2, 0);
		Vec3f backvec = center + Vec3f(0, -ut->size.x/2, 0);
#endif
		Vec3f sidevec = center + Vec3f(-1, 0, 0);
		Vec3f side2vec = center + Vec3f(1, 0, 0);
		Vec3f frontvec = center + Vec3f(0, 1, 0);
		Vec3f backvec = center + Vec3f(0, -1, 0);
		
		sidevec = RotateAround(sidevec, center, ( u->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, ( u->rotation.z ), 0, 0, 1);
		side2vec = RotateAround(side2vec, center, ( u->rotation.z ), 0, 0, 1);
		backvec = RotateAround(backvec, center, ( u->rotation.z ), 0, 0, 1);
		
		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y) * TILE_RISE;
		frontvec.z = g_hmap.accheight(frontvec.x, frontvec.y) * TILE_RISE;
		side2vec.z = g_hmap.accheight(side2vec.x, side2vec.y) * TILE_RISE;
		backvec.z = g_hmap.accheight(backvec.x, backvec.y) * TILE_RISE;
		
		frontvec = ( frontvec - center );
		sidevec = ( sidevec - center );
		backvec = ( backvec - center );
		side2vec = ( side2vec - center );
		
		//frontvec = ( frontvec - backvec );
		//sidevec = ( sidevec - side2vec );
		
		frontvec.z = ( frontvec.z * 2 );
		sidevec.z = ( sidevec.z * 2 );
		
		frontvec = Normalize( frontvec );
		sidevec = Normalize( sidevec );

		u->rotation.x = asin( -frontvec.z );
		//u->rotation.z = atanf( frontvec.x / frontvec.y );
		//u->rotation.z = atanf( forward.x / forward.y );

		//up0 = with yaw and pitch but no roll
		Vec3f right0 = Cross(Vec3f(0,0,1), frontvec);
		//Vec3f right0 = Cross(frontvec, Vec3f(0,0,1));
		//Vec3f right0 = sidevec;
		Vec3f up0 = Cross(frontvec, right0);
		Vec3f upr = Cross(frontvec, sidevec);
		//u->rotation.y = acosf( Dot(up0, upr) );

		//The acos() function computes the principal value of the arc cosine of __x. The returned value is in the range [0, pi] radians. A domain error occurs for arguments not in the range [-1, +1].
		float iniroty = acosf( Dot(up0, upr) );

		float up0comp;
		float uprcomp;
		float right0comp;

		//x or z greatest
		if(sidevec.x > sidevec.y)
		{
			//z greatest
			if(sidevec.z > sidevec.x)
			{
				up0comp = up0.z;
				uprcomp = upr.z;
				right0comp = right0.z;

				//sin(Oz) = ( cos(Oz) y0z - yrz ) / x0z
				//sin(Oz) x0z = ( cos(Oz) y0z - yrz )
				//sin(Oz) x0z - cos(Oz) y0z = - yrz
				//
				//Oz = asin ( ( cos(Oz) y0z - yrz ) / x0z ) 
				//
				// op/hyp = ( adj/hyp y0z - yrz ) / x0z
				// hyp=1
				// op = ( adj y0z - yrz ) / x0z
				// 
			}
			//x greatest
			else
			{
				up0comp = up0.x;
				uprcomp = upr.x;
				right0comp = right0.x;
			}
		}
		//z greatest
		else if(sidevec.z > sidevec.y)
		{
			up0comp = up0.z;
			uprcomp = upr.z;
			right0comp = right0.z;
		}
		//y greatest
		else
		{
			up0comp = up0.y;
			uprcomp = upr.y;
			right0comp = right0.y;
		}

		//The asin() function computes the principal value of the arc sine of __x. The returned value is in the range [-pi/2, pi/2] radians. A domain error occurs for arguments not in the range [-1, +1].
		//float roty1 = asinf( ( cosf(iniroty) * up0comp - uprcomp ) / right0comp ) + M_PI*2.0f;
		//float roty2 = asinf( ( cosf(-iniroty) * up0comp - uprcomp ) / right0comp ) + M_PI*2.0f;
		float roty1 = asinf( ( cosf(iniroty) * up0comp - uprcomp ) / right0comp );
		float roty2 = asinf( ( cosf(-iniroty) * up0comp - uprcomp ) / right0comp );
		
		float d1 = fmin( fabs(roty1 - iniroty), fmin( fabs(2*M_PI + roty1 - iniroty), fabs(-2*M_PI + roty1 - iniroty)) );
		float d2 = fmin( fabs(roty2 - iniroty), fmin( fabs(2*M_PI + roty2 - iniroty), fabs(-2*M_PI + roty2 - iniroty)) );
		//float d1 = fabs(roty1 - iniroty);
		//float d2 = fabs(roty2 - iniroty);

		if(d1 < d2)
			u->rotation.y = iniroty;
		else
			u->rotation.y = -iniroty;
		
#if 0
		if(sidevec.z >= 0 && iniroty >= 0)
			u->rotation.y = iniroty;
		else if(sidevec.z < 0 && iniroty < 0)
			u->rotation.y = iniroty;
		else
			u->rotation.y = -iniroty;
#endif

#if 0
		if(frontvec.z < 0)
			u->rotation.x = -u->rotation.x;
#endif
#if 0
		if(sidevec.z < 0)
			u->rotation.y = -u->rotation.y;
#endif

#if 0
		if( fsign(sidevec.z) == fsign(u->rotation.y) )
			u->rotation.y = -u->rotation.y;	
#endif
#if 0
		if( fsign(frontvec.z) != fsign(u->rotation.x) )
			u->rotation.x = -u->rotation.x;
#endif

		/* 
		The atan2() function computes the principal value of the arc tangent of __y / __x, using the signs of 
		both arguments to determine the quadrant of the return value. The returned value is in the range [-pi, +pi] radians.
		*/

#if 0
		int32_t deg = (int32_t)( 720 + RADTODEG( u->rotation.z ) ) % 360;
		//if(u->rotation.z >= M_PI || u->rotation.z < 0)
		//if( ! ( (u->rotation.z >= M_PI/2 && u->rotation.z <= M_PI) ||
		//	(u->rotation.z <= -M_PI/2 && u->rotation.z >= -M_PI) ) )
		///if( u->rotation.z < 0 )
		//if( deg >= 45 && deg <= (180 + 45)%360 )
		if( deg >= 90+45 && deg <= (180 + 45+90)%360 &&
			!(deg >= 90+45+90 && deg <= (90+45+90+90)%360))
		{
			u->rotation.x = -u->rotation.x;
			u->rotation.y = -u->rotation.y;
		}
#endif
		
		//u->rotation.y = iniroty;
		//u->rotation.y *= 500;
		//u->rotation.x *= 500;
#endif
}


/*
TODO
Unit selection based on drawpos x,y.
Check if sprite clip rect + drawpos x,y
is in g_mouse + g_scroll.
Same for bl, cd.
*/

void DrawUnit(Unit* u, float rendz, uint32_t renderdepthtex, uint32_t renderfb)
{
	StartTimer(TIMER_DRAWUNITS);

	int16_t tx = u->cmpos.x / TILE_SIZE;
	int16_t ty = u->cmpos.y / TILE_SIZE;

	Shader* s = &g_shader[g_curS];
	
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

#if 0
	if(IsTileVis(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	else if(Explored(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, 1.0f);
	else
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}
#endif

	if(!IsTileVis(g_localP, tx, ty))
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}

	if(u->hidden())
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}

	if(USel(u - g_unit))
	{
		EndS();
#ifdef ISOTOP
		UseS(SHADER_COLOR2D);
#else
		UseS(SHADER_DEEPCOLOR);
#endif
		s = &g_shader[g_curS];
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

		UType* t = &g_utype[ u->type ];

		Depthable* d = u->depth;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z + TILE_SIZE/200);

		Vec2i top = CartToIso(top3) - g_scroll;
		Vec2i bot = CartToIso(bot3) - g_scroll;
		Vec2i lef = CartToIso(lef3) - g_scroll;
		Vec2i rig = CartToIso(rig3) - g_scroll;

		//glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 0.5f);
		glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 1.0f);

		int32_t topd, botd, lefd, rigd;
		
#if 1
		CartToDepth(top3, &topd);
		CartToDepth(bot3, &botd);
		CartToDepth(lef3, &lefd);
		CartToDepth(rig3, &rigd);

#if 0
		top = Vec2i(50, 50);
		bot = Vec2i(50, 100);
		lef = Vec2i(0, 75);
		rig = Vec2i(100, 75);
#endif

#if 0
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)topd,
			(float)rig.x, (float)rig.y, (float)rigd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)lef.x, (float)lef.y, (float)lefd,
			(float)top.x, (float)top.y, (float)topd
		};
#else
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)topd,
			(float)rig.x, (float)rig.y, (float)rigd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)lef.x, (float)lef.y, (float)lefd,
			(float)top.x, (float)top.y, (float)topd
		};
#endif
#else
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)d->rendz,
			(float)rig.x, (float)rig.y, (float)d->rendz,
			(float)bot.x, (float)bot.y, (float)d->rendz,
			(float)lef.x, (float)lef.y, (float)d->rendz,
			(float)top.x, (float)top.y, (float)d->rendz
		};
#endif

#if 0
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
		//glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);

		glVertexPointer(3, GL_FLOAT, sizeof(float)*0, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glDrawArrays(GL_LINE_LOOP, 0, 4);
#else
		//DrawDeepColor(0, 1, 0, 1, vertices, 5, GL_LINE_STRIP);
		DrawDeepColor(0, 1, 0, 1, vertices, 6, GL_TRIANGLES);
#endif

		EndS();

#ifdef ISOTOP
		UseS(SHADER_ORTHO);
#else
		UseS(DEEPSHADER);
#endif
		s = &g_shader[g_curS];
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
		glUniform1f(s->slot[SSLOT_MIND], (float)MIN_DISTANCE);
		glUniform1f(s->slot[SSLOT_MAXD], (float)MAX_DISTANCE);
		glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		glUniform1f(s->slot[SSLOT_BASEELEV], 0);
	}

	UType* t = &g_utype[u->type];
#if 0
	Vec3f vmin(u->drawpos.x - t->size.x/2, u->drawpos.y, u->drawpos.y - t->size.x/2);
	Vec3f vmax(u->drawpos.x + t->size.x/2, u->drawpos.y + t->size.y, u->drawpos.y + t->size.x/2);

	if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
		continue;

	Player* py = &g_player[u->owner];
	float* color = py->color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

	Model* m = &g_model[t->model];
#endif
	
	Player* py = &g_player[u->owner];
	float* color = py->color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], 1);

	//m->draw(u->frame[BODY_LOWER], u->drawpos, u->rotation.z);

	//assert( dir < 8 );

	//int32_t spi = t->sprite[ dir % DIRS ][(int32_t)(u->frame[BODY_LOWER]) % sl->nframes ];
	
	int32_t sli = t->splist;
	SpList* sl = &g_splist[ sli ];
	unsigned char dir = (int32_t)( sl->nsides - ((int32_t)(  u->rotation.z / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) ) % sl->nsides;
	unsigned char pitch = (int32_t)( (sl->nsides - (int32_t)(sl->nsides -  u->rotation.x / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
	unsigned char roll = (int32_t)( (sl->nsides - (int32_t)(sl->nsides -  u->rotation.y / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
	int32_t ci = SpriteRef(sl, (int32_t)(u->frame[BODY_LOWER]) % sl->nframes, 0, pitch, dir, roll, 0, 0);
	Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

	//char m[123];
	//sprintf(m,"draw u ci %d", ci);
	//InfoMess(m,m);

	//assert( (int32_t)u->frame[BODY_LOWER] < t->nframes );
	
	Texture* difftex = &g_texture[sp->difftexi];
	Texture* depthtex = &g_texture[sp->depthtexi];
	Texture* teamtex = &g_texture[sp->teamtexi];
	Texture* elevtex = &g_texture[sp->elevtexi];

	Vec3i cm3pos;
	cm3pos.x = u->cmpos.x;
	cm3pos.y = u->cmpos.y;
	cm3pos.z = (int32_t)( g_hmap.accheight2(u->cmpos.x, u->cmpos.y) );
	Vec2i isopos = CartToIso(cm3pos);
	Vec2i screenpos = isopos - g_scroll;
	glUniform1f(s->slot[SSLOT_BASEELEV], cm3pos.z);

#if 0
	DrawImage(difftex->texname,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->cropoff[0]/(float)difftex->width, sp->cropoff[1]/(float)difftex->height, 
		sp->cropoff[2]/(float)difftex->width, sp->cropoff[3]/(float)difftex->height,
		g_gui.m_crop);
#elif 0
	DrawDeep(difftex->texname, depthtex->texname, rendz,
		screenpos.x + sp->offset[0], screenpos.y + sp->offset[1],
		screenpos.x + sp->offset[2], screenpos.y + sp->offset[3],
		0, 0, 1, 1);
#else
	DrawDeep2(difftex->texname, depthtex->texname, teamtex->texname, elevtex->texname,
		renderdepthtex, renderfb,
		rendz, cm3pos.z,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->crop[0], sp->crop[1],
		sp->crop[2], sp->crop[3]);
#endif

	StopTimer(TIMER_DRAWUNITS);
}

int32_t NewUnit()
{
	for(int32_t i=0; i<UNITS; i++)
		if(!g_unit[i].on)
			return i;

	return -1;
}

// starting belongings for labourer
void StartBel(Unit* u)
{
	Zero(u->belongings);
	u->car = -1;
	u->home = -1;

	if(u->type == UNIT_LABOURER)
	{
		//if(u->owner >= 0)
		{
			//u->belongings[ RES_DOLLARS ] = 100;
			//u->belongings[ RES_DOLLARS ] = CYCLE_FRAMES * LABOURER_FOODCONSUM * 30;
			u->belongings[ RES_DOLLARS ] = CYCLE_FRAMES/FOOD_CONSUM_DELAY_FRAMES * LABOURER_FOODCONSUM * 10;
		}

		u->belongings[ RES_RETFOOD ] = STARTING_RETFOOD;
		u->belongings[ RES_LABOUR ] = STARTING_LABOUR;
	}
	else if(u->type == UNIT_TRUCK)
	{
		u->belongings[ RES_RETFUEL ] = STARTING_FUEL;
	}
}

bool PlaceUnit(int32_t type, Vec2i cmpos, int32_t owner, int32_t *reti)
{
	int32_t i = NewUnit();

	if(i < 0)
		return false;

	if(reti)
		*reti = i;

#if 0
	bool on;
	int32_t type;
	int32_t stateowner;
	int32_t corpowner;
	int32_t unitowner;

	/*
	The draw (floating-point) position vectory is used for drawing.
	*/
	Vec3f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec3i cmpos;
	Vec3f facing;
	Vec2f rotation;

	deque<Vec2i> path;
	Vec2i goal;

	int32_t step;
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

	float frame[2];
#endif

	Unit* u = &g_unit[i];
	UType* t = &g_utype[type];

	cmpos.x = imax(0, cmpos.x);
	cmpos.y = imax(0, cmpos.y);
	cmpos.x = imin(g_mapsz.x*TILE_SIZE-1, cmpos.x);
	cmpos.y = imin(g_mapsz.y*TILE_SIZE-1, cmpos.y);

	u->on = true;
	u->type = type;
	u->cmpos = cmpos;
	//u->drawpos = Vec3f(cmpos.x, cmpos.y, Bilerp(&g_hmap, (float)cmpos.x, (float)cmpos.y)*TILE_RISE);
	u->owner = owner;
	u->path.clear();
	u->goal = cmpos;
	u->target = -1;
	u->target2 = -1;
	u->targetu = false;
	u->underorder = false;
	u->fuelstation = -1;
	u->targtype = TARG_NONE;
	//u->home = -1;
	StartBel(u);
	u->hp = t->starthp;
	u->passive = false;
	u->prevpos = u->cmpos;
	u->taskframe = 0;
	u->pathblocked = false;
	u->jobframes = 0;
	//u->jobframes = (g_simframe * i + 1) % LOOKJOB_DELAY_MAX;
	u->supplier = -1;
	u->exputil = 0;
	u->reqamt = 0;
	u->targtype = -1;
	u->frame[BODY_LOWER] = 0;
	u->frame[BODY_UPPER] = 0;
	u->subgoal = u->goal;

	u->mode = UMODE_NONE;
	u->pathdelay = 0;
	u->lastpath = g_simframe;

	u->cdtype = CD_NONE;
	u->driver = -1;
	//u->framesleft = 0;
	u->cyframes = WORK_DELAY-1;
	//u->cyframes = (g_simframe * i + 111) % WORK_DELAY;
	u->cargoamt = 0;
	u->cargotype = -1;

	u->rotation.z = 0;
	u->frame[BODY_LOWER] = 0;
	CalcRot(u);

	u->forsale = false;
	u->price = 1;

	u->incomerate = 0;
	u->cyclehist.clear();

	if(type == UNIT_TRUCK)
	{
		u->cyclehist.push_back(TrCycleHist());
		
		while(u->cyclehist.size() > 100)
			u->cyclehist.erase(u->cyclehist.begin());
		
		TrCycleHist* trch = &*u->cyclehist.rbegin();
		Player* py = &g_player[owner];

		trch->opwage = py->truckwage;
		trch->trprice = py->transpcost;
	}

	if(type == UNIT_LABOURER)
	{
		u->jobframes = (g_simframe * i + 1) % LOOKJOB_DELAY_MAX;
		u->cyframes = (g_simframe * i + 111) % WORK_DELAY;
	}

	//if (UnitCollides(u, u->cmpos, u->type))
	//	InfoMess("c", "c");
	
	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f5");

	u->fillcollider();
	g_drawlist.push_back(Depthable());
	Depthable* d = &*g_drawlist.rbegin();
	d->dtype = DEPTH_U;
	d->index = i;
	u->depth = d;
	UpDraw(u);
	AddVis(u);
	Explore(u);

	u->mazeavoid = false;
	u->winding = -1;

	return true;
}

void FreeUnits()
{
	for(int32_t i=0; i<UNITS; i++)
	{
		g_unit[i].destroy();
		g_unit[i].on = false;
	}
}

bool Unit::hidden() const
{
	switch(mode)
	{
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
	//case UMODE_REFUELING:
	//case UMODE_ATDEMB:
	//case UMODE_ATDEMCD:
		return true;
	default:break;
	}

	return false;
}

void AnimUnit(Unit* u)
{
	UType* t = &g_utype[u->type];

	if(/* u->type == UNIT_BATTLECOMP || */ u->type == UNIT_LABOURER)
	{
		//if(u->prevpos == u->cmpos)
		//if(u->mode == UMODE_NONE)
		//if(!u->path.size())
		if(u->goal == u->cmpos)
		{
			//u->frame[BODY_LOWER] = 0;
			return;
		}

		PlayAni(u->frame[BODY_LOWER], 0, 29, true, 1.3f);
	}
}

void UpdAI(Unit* u)
{
	//return;	//do nothing for now?

	if(u->type == UNIT_LABOURER)
		UpdLab(u);
	else if(u->type == UNIT_TRUCK)
		UpdTruck(u);
}

void UpdCheck(Unit* u)
{
	if(u->type == UNIT_LABOURER)
		UpdLab2(u);
}

void UpdUnits()
{
#ifdef TSDEBUG
	if(tracku)
	{
		Log("tracku t"<<tracku->type<<" mode"<<(int32_t)tracku->mode<<" tpathsz"<<tracku->tpath.size()<<" pathsz"<<tracku->path.size());
	}
#endif

	for(int32_t i = 0; i < UNITS; i++)
	{
		StartTimer(TIMER_UPDUONCHECK);

		Unit* u = &g_unit[i];

		if(!u->on)
		{
			StopTimer(TIMER_UPDUONCHECK);
			continue;
		}

		RemVis(u);
		AddVis(u);

		StopTimer(TIMER_UPDUONCHECK);

#if 1
		StartTimer(TIMER_UPDUNITAI);
		UpdAI(u);
		StopTimer(TIMER_UPDUNITAI);
#endif

#if 1
		/*
		Second check, because unit might be destroyed in last call,
		which might remove Depthable ->depth, which would cause a
		crash in MoveUnit.
		*/
		if(!u->on)
			continue;
#endif

		//TODO animate units movement
		StartTimer(TIMER_MOVEUNIT);
		MoveUnit(u);
		StopTimer(TIMER_MOVEUNIT);
		//must be called after Move... labs without paths are stuck
		UpdCheck(u);
		StartTimer(TIMER_ANIMUNIT);
		AnimUnit(u);
		StopTimer(TIMER_ANIMUNIT);

#if 0
		/*
		Last call, because unit might be destroyed in this call,
		which might remove Depthable ->depth, which would cause a
		crash in MoveUnit.
		*/
		StartTimer(TIMER_UPDUNITAI);
		UpdAI(u);
		StopTimer(TIMER_UPDUNITAI);
#endif
	}
}

void ResetPath(Unit* u)
{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("path reset");
		InfoMess("pr", "pr");
	}
#endif

	u->winding = -1;
	u->path.clear();
	u->tpath.clear();

#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("ResetPath u=thatunit");
	}
#endif
}

void ResetGoal(Unit* u)
{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("g reset");
		InfoMess("rg", "rg");
	}
#endif

	u->exputil = 0;
	u->goal = u->subgoal = u->cmpos;
	ResetPath(u);

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset path track u 3", "reset path track u 3");
	}
#endif
}

void ResetMode(Unit* u, bool chcollider)
{
#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("\tResetMode u=thatunit");
	}
#endif

#ifdef HIERDEBUG
	if(u - g_unit == 5 && u->mode == UMODE_GOBLJOB && u->target == 5)
	{
		InfoMess("rsu5", "rsu5");
	}
#endif

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("mode reset");
		char msg[128];
		sprintf(msg, "rm %s prevm=%d", g_utype[u->type].name, (int32_t)u->mode);
		InfoMess("rm", msg);
	}
#endif

#if 0
	switch(u->mode)
	{
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
	//case UMODE_REFUELING:
	//case UMODE_ATDEMB:
	//case UMODE_ATDEMCD:
		u->freecollider();
		PlaceUAb(u->type, u->cmpos, &u->cmpos);
		u->fillcollider();
		UpDraw(u);
	default:break;
	}
#else
	if(u->mode == UMODE_DRIVE &&
		u->type == UNIT_LABOURER)	//corpc fix
	{
		Unit* tr = &g_unit[u->target];
		tr->driver = -1;
		
		RemVis(u);
		//u->freecollider();	//d fix
		PlaceUAb(u->type, tr->cmpos, &u->cmpos);
#if 0
		//TODO
		u->drawpos.x = u->cmpos.x;
		u->drawpos.y = u->cmpos.y;
		u->drawpos.y = g_hmap.accheight(u->cmpos.x, u->cmpos.y);
#endif
		
	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f6");

		if(chcollider)
			u->fillcollider();
		AddVis(u);
		UpDraw(u);
	}
	else if(u->hidden())
	{
		RemVis(u);
		//u->freecollider();	//d fix
		PlaceUAb(u->type, u->cmpos, &u->cmpos);
#if 0
		//TODO
		u->drawpos.x = u->cmpos.x;
		u->drawpos.y = u->cmpos.y;
		u->drawpos.y = g_hmap.accheight(u->cmpos.x, u->cmpos.y);
#endif
		
	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f13 m=%d", (int)u->mode);

		if (chcollider)
			u->fillcollider();
		AddVis(u);
		UpDraw(u);
	}
#endif

#if 0
	//URAN_DEBUG
	if(u-g_unit == 19)
	{
		Building* b = &g_building[5];
		char msg[1280];
		sprintf(msg, "ResetMode u13truck culprit \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d",
			(int32_t)b->transporter[RES_URANIUM],
			(int32_t)g_unit[b->transporter[RES_URANIUM]].mode,
			(int32_t)g_unit[b->transporter[RES_URANIUM]].target,
			5,
			(int32_t)g_unit[b->transporter[RES_URANIUM]].targtype,
			(int32_t)g_unit[b->transporter[RES_URANIUM]].cargotype);
		InfoMess(msg, msg);
	}
#endif

	//LastNum("resetmode 1");
	if(u->type == UNIT_LABOURER)
	{
		//LastNum("resetmode 1a");
		if(u->mode == UMODE_BLJOB ||
			u->mode == UMODE_CSTJOB)
			RemWorker(u);

		else if(u->mode == UMODE_SHOPPING)
			RemShopper(u);

		u->jobframes = LOOKJOB_DELAY_MAX;

		//if(hidden())
		//	relocate();
	}
	else if(u->type == UNIT_TRUCK)
	{
#if 1
		if(u->mode == UMODE_GOSUP
		                //|| mode == GOINGTOREFUEL
		                //|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
		  )
		{
			if(u->supplier >= 0)
			{
				//necessary?
				Building* b = &g_building[u->supplier];
				b->transporter[u->cargotype] = -1;
			}
		}

		bool targbl = false;
		bool targcd = false;

		if( (u->mode == UMODE_GOSUP || u->mode == UMODE_ATSUP || u->mode == UMODE_GOREFUEL || u->mode == UMODE_REFUELING) && u->targtype == TARG_BL )
			targbl = true;

		if( (u->mode == UMODE_GOSUP || u->mode == UMODE_ATSUP || u->mode == UMODE_GOREFUEL || u->mode == UMODE_REFUELING) && u->targtype == TARG_CD )
			targbl = true;

		if(u->mode == UMODE_GODEMB)
			targbl = true;

		if(u->mode == UMODE_ATDEMB)
			targbl = true;

		if(u->mode == UMODE_GODEMCD)
			targcd = true;

		if(u->mode == UMODE_ATDEMCD)
			targcd = true;

		if(u->targtype == TARG_BL)
			targbl = true;

		if(u->targtype == TARG_CD)
			targcd = true;

		if( targbl )
		{
			if(u->target >= 0)
			{
				Building* b = &g_building[u->target];
				b->transporter[u->cargotype] = -1;
			}
		}
		else if( targcd )
		{
			if(u->target >= 0 && u->target2 >= 0 && u->cdtype >= 0)
			{
				CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);
				ctile->transporter[u->cargotype] = -1;
			}
		}
#endif
		u->targtype = TARG_NONE;

		if(u->driver >= 0)
		{
			Unit* op = &g_unit[u->driver];
			//LastNum("resetmode 1b");
			//g_unit[u->driver].Disembark();

			//corpc fix
			//important to reset driver first and then tell him to disembark so we don't get stuck in a resetting loop
			u->driver = -1;

			if(op->mode == UMODE_DRIVE)
				Disembark(op);
		}
	}

	//LastNum("resetmode 2");

	//transportAmt = 0;
	u->targtype = TARG_NONE;
	u->target = u->target2 = -1;
	u->supplier = -1;
	u->mode = UMODE_NONE;
	ResetGoal(u);

#if 0
	//URAN_DEBUG
	if(u-g_unit == 19)
	{
		Building* b = &g_building[5];
		char msg[1280];
		sprintf(msg, "/ResetMode u13truck culprit \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n cargty%d",
			(int32_t)b->transporter[RES_URANIUM],
			(int32_t)g_unit[b->transporter[RES_URANIUM]].mode,
			(int32_t)g_unit[b->transporter[RES_URANIUM]].target,
			5,
			(int32_t)g_unit[b->transporter[RES_URANIUM]].targtype,
			(int32_t)g_unit[b->transporter[RES_URANIUM]].cargotype);
		InfoMess(msg, msg);
	}
#endif

	//LastNum("resetmode 3");
}

void ResetTarget(Unit* u)
{

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset mode track u 3", "reset mode track u 3");
	}
#endif

	ResetMode(u);
	u->target = -1;
}
