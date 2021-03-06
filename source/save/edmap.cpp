

#include "edmap.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../utils.h"
#include "../save/modelholder.h"
#include "../debug.h"
#include "../render/shadow.h"
#include "../render/heightmap.h"
#include "../app/appmain.h"
#include "../tool/rendersprite.h"


#include "../sim/map.h"
#include "../sim/simtile.h"
#include "../save/saveedm.h"
#include "../save/edmap.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../utils.h"
#include "../debug.h"
#include "../render/shadow.h"

Map g_map;

Map::Map()
{
	m_brush = NULL;
	m_nbrush = 0;
}

Map::~Map()
{
	destroy();
}

void Map::destroy()
{
	for(int brushidx = 0; brushidx < m_nbrush; brushidx++)
	{
		Brush* pbrush = &m_brush[brushidx];

		for(int sideidx=0; sideidx < pbrush->m_nsides; sideidx++)
		{
			BrushSide* s = &pbrush->m_sides[sideidx];

			if(s->m_diffusem != 0)
				FreeTexture(s->m_diffusem);
			if(s->m_specularm != 0)
				FreeTexture(s->m_specularm);
			if(s->m_normalm != 0)
				FreeTexture(s->m_normalm);
			if(s->m_ownerm != 0)
				FreeTexture(s->m_ownerm);
		}
	}

	if(m_brush)
	{
		delete [] m_brush;
		m_brush = NULL;
	}

	m_nbrush = 0;
	m_transpbrush.clear();
	m_opaquebrush.clear();
	m_skybrush.clear();
}

/*
void LogBrush()
{
	for(std::list<Brush>::iterator b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];
		Vec3f* un = &s->m_tceq[0].m_normal;
		Vec3f* vn = &s->m_tceq[1].m_normal;
		float ud = s->m_tceq[0].m_d;
		float vd = s->m_tceq[1].m_d;

		for(int j=0; j<s->m_ntris; j++)
		{
			Triangle2* t = &s->m_tris[j];

			//for(int j=0; j<va->numverts; j++)
			for(int j=0; j<3; j++)
			{
		Vec2f* tc = &t->m_texcoord[j];
				//Log("u "<<va->texcoords[j].x<<"\t	v "<<va->texcoords[j].y<<std::endl;
				Log("u "<<tc->x<<"\t	v "<<tc->y<<std::endl;
				
			}
		}

		s->makeva();
	}
	}
}*/

//Draw opaque brushes first
void DrawMap(Map* map)
{

	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	Shader* s = &g_shader[g_curS];

	Matrix modelmat;
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult2(g_camview);
	mvp.postmult2(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
    modelview.set(g_camview.m_matrix);
#endif
    modelview.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	for(std::list<int>::iterator brushiterator = map->m_opaquebrush.begin(); brushiterator != map->m_opaquebrush.end(); brushiterator++)
	{
		int brushindex = *brushiterator;
#if 0
		Log("draw brush "<<brushidx<<std::endl;
#endif

		Brush* b = &map->m_brush[brushindex];
		Texture* t = &g_texture[b->m_texture];

		//TO DO: Replace with index table look-ups
		if(t->sky)
			continue;

		for(int sideindex = 0; sideindex < b->m_nsides; sideindex++)
		{
#if 0
			Log("\tdraw side "<<sideindex<<std::endl;
#endif

			BrushSide* pside = &b->m_sides[sideindex];
			/*
			CreateTex(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			pside->usedifftex();
			pside->usespectex();
			pside->usenormtex();
			pside->useteamtex();
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", false);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1iARB(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/
			VertexArray* va = &pside->m_drawva;

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
            glNormalPointer(GL_FLOAT, 0, va->normals);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

#ifdef DEBUG
			CHECKGLERROR();
#endif
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);

#if 0
			for(int vertindex = 0; vertindex < va->numverts; vertindex++)
			{
				Log("\t\tvert: ("<<va->vertices[vertindex].x<<","<<va->vertices[vertindex].y<<","<<va->vertices[vertindex].z<<")");
			}

			Log("\tdrew triangles: "<<(va->numverts/3)<<std::endl;
#endif
		}
	}
}


// Draw transparent faces on top of all models and opaque brushes
void DrawMap2(Map* map)
{
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	//Reset the model matrix from the model rotations and translations

	Shader* pshader = &g_shader[g_curS];
	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(pshader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult2(g_camview);
	mvp.postmult2(modelmat);
	glUniformMatrix4fv(pshader->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
    modelview.set(g_camview.m_matrix);
#endif
    modelview.postmult(modelmat);
	glUniformMatrix4fv(pshader->slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(pshader->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	Shader* shader = &g_shader[g_curS];

	for(std::list<int>::iterator brushiterator = map->m_transpbrush.begin(); brushiterator != map->m_transpbrush.end(); brushiterator++)
	{
		int brushindex = *brushiterator;
#if 0
		Log("draw brush "<<brushidx<<std::endl;
#endif

		Brush* b = &map->m_brush[brushindex];
		Texture* t = &g_texture[b->m_texture];

		//TO DO: Replace with index table look-ups
		if(t->sky)
			continue;

		for(int sideindex = 0; sideindex < b->m_nsides; sideindex++)
		{
#if 0
			Log("\tdraw side "<<sideindex<<std::endl;
#endif

			BrushSide* pside = &b->m_sides[sideindex];
			/*
			CreateTex(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			pside->usedifftex();
			pside->usespectex();
			pside->usenormtex();
			pside->useteamtex();
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", false);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1iARB(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/
			VertexArray* va = &pside->m_drawva;

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
            glNormalPointer(GL_FLOAT, 0, va->normals);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

#ifdef DEBUG
			CHECKGLERROR();
#endif
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);

#if 0
			for(int vertindex = 0; vertindex < va->numverts; vertindex++)
			{
				Log("\t\tvert: ("<<va->vertices[vertindex].x<<","<<va->vertices[vertindex].y<<","<<va->vertices[vertindex].z<<")");
			}

			Log("\tdrew triangles: "<<(va->numverts/3)<<std::endl;
#endif
		}
	}
}

//Draw opaque brushes first
void DrawMapDepth(Map* map)
{

	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	Shader* shader = &g_shader[g_curS];

	for(std::list<int>::iterator brushiterator = map->m_opaquebrush.begin(); brushiterator != map->m_opaquebrush.end(); brushiterator++)
	{
		int brushindex = *brushiterator;
#if 0
		Log("draw brush "<<brushidx<<std::endl;
#endif

		Brush* b = &map->m_brush[brushindex];
		Texture* t = &g_texture[b->m_texture];

		//TO DO: Replace with index table look-ups
		if(t->sky)
			continue;

		for(int sideindex = 0; sideindex < b->m_nsides; sideindex++)
		{
#if 0
			Log("\tdraw side "<<sideindex<<std::endl;
#endif

			BrushSide* pside = &b->m_sides[sideindex];
			/*
			CreateTex(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			pside->usedifftex();
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", false);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1iARB(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/
			VertexArray* va = &pside->m_drawva;

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

#ifdef DEBUG
			CHECKGLERROR();
#endif
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);

#if 0
			for(int vertindex = 0; vertindex < va->numverts; vertindex++)
			{
				Log("\t\tvert: ("<<va->vertices[vertindex].x<<","<<va->vertices[vertindex].y<<","<<va->vertices[vertindex].z<<")");
			}

			Log("\tdrew triangles: "<<(va->numverts/3)<<std::endl;
#endif
		}
	}
}




EdMap g_edmap;
std::vector<Brush*> g_selB;
Brush* g_sel1b = NULL;	//drag selected brush (brush being dragged or manipulated currently)
int g_dragV = -1;	//drag vertex of selected brush?
int g_dragS = -1;	//drag side of selected brush?
bool g_dragW = false;	//drag whole brush or model?
int g_dragD = -1;	// dragging DRAG_DOOR_POINT or DRAG_DOOR_AXIS ?
int g_dragM = -1;	//dragging model holder?
std::vector<ModelHolder*> g_selM;
ModelHolder* g_sel1m = NULL;	//drag selected model (model being dragged or manipulated currently)

void DrawEdMap(EdMap* map, bool showsky)
{
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	Shader* s = &g_shader[g_curS];

	//glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, g_camproj.m_matrix);
	//glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, g_camview.m_matrix);

	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
    CHECKGLERROR();

	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult2(g_camview);
	mvp.postmult2(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
    modelview.set(g_camview.m_matrix);
    modelview.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

#if 1
	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
    Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);
#endif

	Heightmap hm;

	if((g_appmode == APPMODE_PRERENDADJFRAME ||
		g_appmode == APPMODE_RENDERING) &&
		g_doinclines)
	{
		float heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = g_cornerinc[g_currincline][0];
		heights[1] = g_cornerinc[g_currincline][1];
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = g_cornerinc[g_currincline][2];
		heights[3] = g_cornerinc[g_currincline][3];

		//Heightmap hm;
		hm.alloc(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();
	}

	for(std::list<Brush>::iterator b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		Texture* t = &g_texture[b->m_texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* side = &b->m_sides[i];
			/*
			CreateTex(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			side->usedifftex();
			side->usespectex();
			side->usenormtex();
			side->useteamtex();

			CHECKGLERROR();
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", false);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1iARB(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/
			VertexArray* va = &side->m_drawva;
			VertexArray tempva;

			if((g_appmode == APPMODE_PRERENDADJFRAME ||
				g_appmode == APPMODE_RENDERING) &&
				g_doinclines)
			{
				tempva.alloc(va->numverts);

				for(int i=0; i<va->numverts; i++)
				{
					tempva.normals[i] = va->normals[i];
					tempva.texcoords[i] = va->texcoords[i];
					tempva.vertices[i] = va->vertices[i];

					//TransformedPos[i].y += Bilerp(&hm,
					//	g_tilesize/2.0f + h->translation.x + TransformedPos[i].x,
					//	g_tilesize/2.0f + h->translation.z + TransformedPos[i].z);
					tempva.vertices[i].y += hm.accheight2(
						g_tilesize/2.0f + tempva.vertices[i].x,
						g_tilesize/2.0f + tempva.vertices[i].z);
				}

				va = &tempva;
			}

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CHECKGLERROR();
#endif
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#ifdef DEBUG
			CHECKGLERROR();
#endif
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
			CHECKGLERROR();
#endif
    		glActiveTexture(GL_TEXTURE0);
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
            glNormalPointer(GL_FLOAT, 0, va->normals);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			CHECKGLERROR();
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
#ifdef DEBUG
			CHECKGLERROR();
#endif
		}
	}
}

void DrawEdMapDepth(EdMap* map, bool showsky)
{
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	Shader* shader = &g_shader[g_curS];

	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(shader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	for(std::list<Brush>::iterator b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		Texture* t = &g_texture[b->m_texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* side = &b->m_sides[i];
			/*
			CreateTex(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			side->usedifftex();
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", false);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1iARB(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/

			glActiveTexture(GL_TEXTURE0);
			glUniform1i(shader->slot[SSLOT_TEXTURE0], 0);

			VertexArray* va = &side->m_drawva;

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CHECKGLERROR();
#endif
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#ifdef DEBUG
			CHECKGLERROR();
#endif
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
#ifdef DEBUG
			CHECKGLERROR();
#endif
		}
	}
}
