










#include "../widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "droplist.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"
#include "../../sim/player.h"


void Link::draw()
{
	glDisable(GL_TEXTURE_2D);

	float color[] = {1, 1, 1, 1};

	if(m_over)
	{
		//glUniform4f(g_shader[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		//glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
		//glUniform4f(g_shader[SHADER_ORTHO].slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1.0f);
		color[0] = 0.8f;
		color[1] = 0.8f;
		color[2] = 0.8f;
	}

	DrawShadowedText(m_font, m_pos[0], m_pos[1], &m_text, color);

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);

	//glEnable(GL_TEXTURE_2D);
}

void Link::inev(InEv* ie)
{
	Player* py = &g_player[g_localP];

	if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//mousemove();
		
#ifdef PLATFORM_MOBILE
		if(m_over)
			Log("link up over");
		else
			Log("link up out");
#endif
		
		if(m_over && m_ldown)
		{
#ifdef PLATFORM_MOBILE
			Log("LINK CLICK");
#endif
			
			if(clickfunc != NULL)
				clickfunc();

			m_over = false;
			m_ldown = false;

			ie->intercepted = true;
			return;	// intercept mouse event
		}

		m_over = false;
		m_ldown = false;
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//mousemove();

		if(m_over)
		{
#ifdef PLATFORM_MOBILE
			Log("down over");
#endif
			
			m_ldown = true;
			ie->intercepted = true;
			return;	// intercept mouse event
		}
#ifdef PLATFORM_MOBILE
		else
			Log("down out");
#endif
	}
	else if(ie->type == INEV_MOUSEMOVE)
	{
		int32_t texlen = m_text.texlen();
		int32_t endx = EndX(&m_text, texlen, m_font, 0, 0);	//corpd fix
		if(g_mouse.x >= m_pos[0] && g_mouse.y >= m_pos[1] &&
		                g_mouse.x <= m_pos[0]+endx &&	//
		                g_mouse.y <= m_pos[1]+g_font[m_font].gheight)
		{
#ifdef PLATFORM_MOBILE
			Log("link over move");
#endif
		}
		else
		{
#ifdef PLATFORM_MOBILE
			Log("link out move");
#endif
			m_over = false;
		}

		if(!ie->intercepted)
		{
			if(g_mouse.x >= m_pos[0] && g_mouse.y >= m_pos[1] &&
			g_mouse.x <= m_pos[0]+endx &&	//
			                g_mouse.y <= m_pos[1]+g_font[m_font].gheight)
			{
				m_over = true;

				ie->intercepted = true;
			}
			else
			{
				m_over = false;
			}
		}
	}
}

