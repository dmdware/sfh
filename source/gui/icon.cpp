










#include "icon.h"
#include "../texture.h"

Icon g_icon[ICONS];

void DefI(int32_t type, const char* relative, const char* tag)
{
	Icon* i = &g_icon[type];

	i->m_tag = UStr(tag);
	//QueueTex(&i->m_tex, relative, true);
	CreateTex(i->m_tex, relative, true, false);
	Texture* t = &g_texture[i->m_tex];
	i->m_width = t->width;
	i->m_height = t->height;
}
