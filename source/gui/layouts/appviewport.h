










#ifndef WEVIEWPORT_H
#define WEVIEWPORT_H

#include "../../math/3dmath.h"
#include "../../math/camera.h"
#include "../../math/vec2i.h"

#define VIEWPORT_MINIMAP		0
#define VIEWPORT_ENTVIEW		1
#define VIEWPORT_TYPES			2

class VpType
{
public:
	Vec3f m_offset;
	Vec3f m_up;
	char m_label[32];
	bool m_axial;

	VpType() {}
	VpType(Vec3f offset, Vec3f up, const char* label, bool axial);
};

extern VpType g_vptype[VIEWPORT_TYPES];

#if 0
class Viewport
{
public:
	int32_t m_type;
	bool m_ldown;
	bool m_rdown;
	bool m_mdown;
	Vec2i m_lastmouse;
	Vec2i m_curmouse;
	bool m_drag;

	Viewport();
	Viewport(int32_t type);
	Vec3f up();
	Vec3f up2();
	Vec3f strafe();
	Vec3f focus();
	Vec3f viewdir();
	Vec3f pos();
};

extern Viewport g_viewport[4];
//extern Vec3f g_focus;
#endif

void DrawViewport(int32_t which, int32_t x, int32_t y, int32_t width, int32_t height);

#if 0
bool ViewportLDown(int32_t which, int32_t relx, int32_t rely, int32_t width, int32_t height);
bool ViewportLUp(int32_t which, int32_t relx, int32_t rely, int32_t width, int32_t height);
bool ViewportMousemove(int32_t which, int32_t relx, int32_t rely, int32_t width, int32_t height);
bool ViewportRDown(int32_t which, int32_t relx, int32_t rely, int32_t width, int32_t height);
bool ViewportRUp(int32_t which, int32_t relx, int32_t rely, int32_t width, int32_t height);
bool NULL(int32_t which, int32_t delta);
#endif

#endif
