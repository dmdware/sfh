










#ifndef DRAW2D_H
#define DRAW2D_H

#include "../platform.h"

extern float g_basedepth;

//#define SPEDDEPTH	(28 + 256 * 200 + 256 * 256 * 7)
#define SPEDDEPTH	g_basedepth

#ifdef PLATFORM_MOBILE
#define ISOTOP	//topological depth sort? (instead of per-pixel using depth maps?)
#endif

//#define ISOTOP

//#define ISOTOP	//440 fps
//304 fps perpix

void DrawImage(uint32_t tex, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom, float *crop);
void DrawDeep(uint32_t difftex, uint32_t depthtex, uint32_t teamtex, int32_t basedepth, float baseelev,
			  uint32_t renderdepthtex, uint32_t renderfb,
			  float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);
void DrawDeep2(uint32_t difftex, uint32_t depthtex, uint32_t teamtex, uint32_t elevtex, 
			  uint32_t renderdepthtex, uint32_t renderfb,
			  int32_t basedepth,  float baseelev,
			  float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);
void DrawSquare(float r, float g, float b, float a, float left, float top, float right, float bottom, float *crop);
void DrawLine(float r, float g, float b, float a, float x1, float y1, float x2, float y2, float *crop);
void DrawDeepColor(float r, float g, float b, float a, float *v, int32_t nv, GLenum mode);
void DrawDepth(uint32_t difftex, uint32_t depthtex,  uint32_t renderdepthtex, uint32_t renderfb, int32_t basedepth, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);

void DrawSphericalBlend(uint32_t difftex, uint32_t depthtex, uint32_t renderdepthtex, uint32_t renderfb, float basedepth,
						float cx, float cy,
						float pixradius, float angle,
						float texleft, float textop, float texright, float texbottom);
#endif
