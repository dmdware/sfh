










#include "billboard.h"
#include "../platform.h"
#include "../math/3dmath.h"
#include "../texture.h"
#include "particle.h"
#include "shader.h"
#include "../utils.h"
#include "../math/vec3f.h"
#include "../math/camera.h"
#include "../sim/player.h"
#include "../math/isomath.h"
#include "../sim/map.h"

BillbType g_billbT[BILLBOARD_TYPES];
Billboard g_billb[BILLBOARDS];
uint32_t g_muzzle[4];

void Effects()
{
	QueueTex(&g_muzzle[0], "effects/muzzle0.png", true, true);
	QueueTex(&g_muzzle[1], "effects/muzzle1.png", true, true);
	QueueTex(&g_muzzle[2], "effects/muzzle2.png", true, true);
	QueueTex(&g_muzzle[3], "effects/muzzle3.png", true, true);
}

int32_t NewBillbT()
{
	for(int32_t i=0; i<BILLBOARD_TYPES; i++)
		if(!g_billbT[i].on)
			return i;

	return -1;
}

int32_t DefBillb(const char* tex)
{
	//BillbType t;
	int32_t i = NewBillbT();
	if(i < 0)
		return -1;

	BillbType* t = &g_billbT[i];
	t->on = true;

#if 0
	char rawtex[64];
	StripPathExt(tex, rawtex);
	strcpy(t->name, rawtex);
	char texpath[128];
	sprintf(texpath, "billboards/%s", rawtex);
	FindTextureExtension(texpath);
	//CreateTex(t.tex, texpath);
	QueueTex(&t->tex, texpath, true, true);
#else
	QueueTex(&t->tex, tex, true, true);
#endif
	//g_billbT.push_back(t);
	//return g_billbT.size() - 1;
	return i;
}

int32_t IdentifyBillboard(const char* name)
{
	//char rawname[64];
	//StripPathExt(name, rawname);

	//for(int32_t i=0; i<g_billbT.size(); i++)
	for(int32_t i=0; i<BILLBOARD_TYPES; i++)
	{
		//if(!_stricmp(g_billbT[i].name, rawname))
		if(g_billbT[i].on && !_stricmp(g_billbT[i].name, name))
			return i;
	}

	//return NewBillboard(rawname);
	return DefBillb(name);
}

int32_t NewBillboard()
{
	for(int32_t i=0; i<BILLBOARDS; i++)
		if(!g_billb[i].on)
			return i;

	return -1;
}

void PlaceBillboard(const char* n, Vec3f pos, float size, int32_t particle)
{
	int32_t type = IdentifyBillboard(n);
	if(type < 0)
		return;

	PlaceBillboard(type, pos, size, particle);
}

void PlaceBillboard(int32_t type, Vec3f pos, float size, int32_t particle)
{
	int32_t i = NewBillboard();
	if(i < 0)
		return;

	Billboard* b = &g_billb[i];
	b->on = true;
	b->type = type;
	b->pos = pos;
	b->size = size;
	b->particle = particle;
}

void SortBillboards()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f pos = c->m_pos;
	Vec3f dir = Normalize( c->m_view - c->m_pos );

	for(int32_t i=0; i<BILLBOARDS; i++)
	{
		if(!g_billb[i].on)
			continue;

		//g_billb[i].dist = Dot(pos - g_billb[i].pos);
		g_billb[i].dist = Dot(dir, g_billb[i].pos);
	}

	Billboard temp;
	int32_t leftoff = 0;
	bool backtracking = false;

	for(int32_t i=1; i<BILLBOARDS; i++)
	{
		//if(!g_billb[i].on)
		//	continue;

		if(i > 0)
		{
			if(g_billb[i].dist > g_billb[i-1].dist)
			{
				if(!backtracking)
				{
					leftoff = i;
					backtracking = true;
				}
				temp = g_billb[i];
				g_billb[i] = g_billb[i-1];
				g_billb[i-1] = temp;
				i-=2;
			}
			else
			{
				if(backtracking)
				{
					backtracking = false;
					i = leftoff;
				}
			}
		}
		else
			backtracking = false;
	}
}

void DrawBillboards()
{
	Billboard* billb;
	BillbType* t;
	float size;

	//Vec3f a, b, c, d;
	Vec2f vert, horiz;
	//Vec3i a2, b2, c2, d2;

	Particle* part;
	PlType* pT;

	Shader* s = &g_shader[g_curS];

	//glDisable(GL_CULL_FACE);

	for(int32_t i=0; i<BILLBOARDS; i++)
	{
		billb = &g_billb[i];
		if(!billb->on)
			continue;

		t = &g_billbT[billb->type];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_texture[t->tex].texname);
		glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

		if(billb->particle >= 0)
		{
			part = &g_particle[billb->particle];
			pT = &g_particleT[part->type];
			size = pT->minsize + pT->sizevariation*(1.0f - part->life);
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, part->life);
		}
		else
		{
			size = billb->size;
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		}

		//vert = Vec3f(0.0f,0.0f,1.0f)*size;
		//horiz = Vec3f(1.0f,-1.0f,0.0f)*size;
		vert = Vec2f(0.0f,1.0f)*size;
		horiz = Vec2f(1.0f,0.0f)*size;

		Vec2i pixpos = CartToIso(Vec3i((int32_t)billb->pos.x, (int32_t)billb->pos.z, (int32_t)billb->pos.y));
		Vec2i screenpos = pixpos - g_scroll;

#if 0
		a = billb->pos - horiz + vert;
		b = billb->pos + horiz + vert;
		c = billb->pos + horiz - vert;
		d = billb->pos - horiz - vert;
		
		const float vertices[] =
		{
			//posx, posy posz   texx, texy
			a.x, a.y, a.z,          1, 0,
			b.x, b.y, b.z,          1, 1,
			c.x, c.y, c.z,          0, 1,

			c.x, c.y, c.z,          0, 1,
			d.x, d.y, d.z,          0, 0,
			a.x, a.y, a.z,          1, 0
		};
#else
		float pos[4];
		pos[0] = (float)screenpos.x - size;
		pos[1] = (float)screenpos.y - size;
		pos[2] = (float)screenpos.x + size;
		pos[3] = (float)screenpos.y + size;

		const float vertices[] =
		{
			//posx, posy posz   texx, texy
			pos[2], pos[1], 0,          1, 0,
			pos[2], pos[3], 0,          1, 1,
			pos[0], pos[3], 0,          0, 1,

			pos[0], pos[3], 0,          0, 1,
			pos[0], pos[1], 0,          0, 0,
			pos[2], pos[1], 0,          1, 0
		};
#endif

#ifdef PLATFORM_GL14
		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
#endif

		glDrawArrays(GL_TRIANGLES, 0, 6);

		//Log("draw");
	}

	//glEnable(GL_CULL_FACE);

	/*
	CEntity* e;
	CPlayer* p;
	CHold* h;
	CItemType* iT;
	size = 8.0f;
	vert = vertical*size;
	horiz = horizontal*size;
	Vec3f muzz;
	Camera* cam;
	Vec3f offset;

	for(int32_t i=0; i<ENTITIES; i++)
	{
	e = &g_entity[i];

	if(!e->on)
	continue;

	if(e->controller < 0)
	continue;

	p = &g_player[e->controller];

	if(p->equipped < 0)
	continue;

	h = &p->items[p->equipped];
	iT = &g_itemType[h->type];

	if((e->frame[BODY_UPPER] < ANIM_SHOTSHOULDER_S || e->frame[BODY_UPPER] > ANIM_SHOTSHOULDER_S+4))
	continue;

	glBindTexture(GL_TEXTURE_2D, g_muzzle[rand()%4]);

	cam = &e->camera;

	if(p == &g_player[g_localP] && g_viewmode == FIRSTPERSON)
	muzz = Rotate(iT->front, -cam->Pitch(), 1, 0, 0);
	else
	muzz = RotateAround(iT->front, Vec3f(0, MID_HEIGHT_OFFSET, 0), -cam->Pitch(), 1, 0, 0);

	muzz = cam->m_pos + Rotate(muzz, cam->Yaw(), 0, 1, 0);

	a = muzz - horiz + vert;
	b = muzz - horiz - vert;
	c = muzz + horiz - vert;
	d = muzz + horiz + vert;

	float vertices[] =
	{
	//posx, posy posz   texx, texy
	a.x, a.y, a.z,          1, 0,
	b.x, b.y, b.z,          1, 1,
	c.x, c.y, c.z,          0, 1,

	c.x, c.y, c.z,          0, 1,
	d.x, d.y, d.z,          0, 0,
	a.x, a.y, a.z,          1, 0
	};

	glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	}*/
}
