










#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../droplist.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "botpan.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../../sim/player.h"
#include "../../layouts/appviewport.h"

BotPan::BotPan(Widget* parent, const char* n, void (*reframef)(Widget* w))
{
	m_parent = parent;
	m_type = WIDGET_BOTTOMPANEL;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

	//left_outer_toprightcorner = Image(this, "", "gui/frames/outertopleft64x64.png", true, NULL, 1, 1, 1, 1,		1, 0, 0, 1);
	//left_outer_top = Image(this, "", "gui/frames/outertop2x64.png", true, NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	//left_minimap = Viewport(this, "minimap viewport", NULL, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_MINIMAP);

	//right_outer_topleftcorner = Image(this, "", "gui/frames/outertopleft64x64.png", true, NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	//right_outer_top = Image(this, "", "gui/frames/outertop2x64.png", true, NULL, 1, 1, 1, 1,		0, 0, 1, 1);

	//middle_outer_top = Image(this, "", "gui/frames/outertop2x12.png", true, NULL, 1, 1, 1, 1,		0, 0, 1, 1);

	//white_bg = Image(this, "", "gui/backg/white.jpg", true, NULL, 1, 1, 1, 1,		0, 0, 1, 1);

	for(int32_t i=0; i<9; i++)
	{
		//bottomright_button[i] = Button(this, "gui/transp.png", RichText(""), MAINFONT8, NULL, NULL, NULL, NULL, i);
		//bottomright_button[i] = Button(this, "gui/brbut/apartment1.png", RichText(""), MAINFONT8, NULL, NULL, NULL, NULL, i);
		bottomright_button_on[i] = false;
	}

	reframe();
}

void BotPan::reframe()	//resized or moved
{
	Widget::reframe();

#if 0
	graph_sel.m_pos[0] = m_pos[0] + MINIMAP_SIZE + 32;
	graph_sel.m_pos[1] = m_pos[1] + 64;
	graph_sel.m_pos[2] = graph_sel.m_pos[0] + 150;
	graph_sel.m_pos[3] = graph_sel.m_pos[1] + 10;
	graph_sel.reframe();
#endif
	
#if 0
	left_outer_toprightcorner.m_pos[0] = m_pos[0] + MINIMAP_SIZE - 32;
	left_outer_toprightcorner.m_pos[1] = m_pos[1];
	left_outer_toprightcorner.m_pos[2] = m_pos[0] + MINIMAP_SIZE + 32;
	left_outer_toprightcorner.m_pos[3] = m_pos[1] + 64;
	//left_outer_toprightcorner.m_pos[3] = m_pos[3];

	left_outer_toprightcorner.m_texc[3] = (left_outer_toprightcorner.m_pos[3]-left_outer_toprightcorner.m_pos[1])/64.0f;

	left_outer_top.m_pos[0] = m_pos[0];
	left_outer_top.m_pos[1] = m_pos[1];
	left_outer_top.m_pos[2] = m_pos[0] + MINIMAP_SIZE - 32;
	left_outer_top.m_pos[3] = m_pos[1] + 64;
	//left_outer_top.m_pos[3] = m_pos[3];

	left_outer_top.m_texc[2] = (left_outer_toprightcorner.m_pos[2]-left_outer_toprightcorner.m_pos[0])/2.0f;

	left_minimap.m_pos[0] = m_pos[0] + MINIMAP_OFF;
	left_minimap.m_pos[1] = m_pos[1] + 32 - MINIMAP_OFF;
	left_minimap.m_pos[2] = m_pos[0] + MINIMAP_SIZE + MINIMAP_OFF;
	left_minimap.m_pos[3] = m_pos[1] + 32 + MINIMAP_SIZE - MINIMAP_OFF;
#endif
	
#if 0
	Image right_outer_topleftcorner;
	Image right_outer_top;
#endif

#if 0
	right_outer_topleftcorner.m_pos[0] = m_pos[2] - MINIMAP_SIZE - 32;
	right_outer_topleftcorner.m_pos[1] = m_pos[1];
	right_outer_topleftcorner.m_pos[2] = m_pos[2] - MINIMAP_SIZE + 32;
	right_outer_topleftcorner.m_pos[3] = m_pos[1] + 64;

	right_outer_topleftcorner.m_texc[3] = (right_outer_topleftcorner.m_pos[3]-right_outer_topleftcorner.m_pos[1])/64.0f;
	right_outer_topleftcorner.reframe();

	right_outer_top.m_pos[0] = m_pos[2] - MINIMAP_SIZE + 32;
	right_outer_top.m_pos[1] = m_pos[1];
	right_outer_top.m_pos[2] = m_pos[2];
	right_outer_top.m_pos[3] = m_pos[1] + 64;
	right_outer_top.reframe();
#endif

#if 0
	middle_outer_top.m_pos[0] = m_pos[0] + MINIMAP_SIZE + 32 - 15;
	middle_outer_top.m_pos[1] = m_pos[1] + 64 - 12;
	middle_outer_top.m_pos[2] = m_pos[2] - MINIMAP_SIZE - 32 + 15;
	middle_outer_top.m_pos[3] = m_pos[1] + 64;

	white_bg.m_pos[0] = m_pos[0];
	white_bg.m_pos[1] = m_pos[1] + 64;
	white_bg.m_pos[2] = m_pos[2];
	white_bg.m_pos[3] = m_pos[3];
#endif

	float bottomright_left = m_pos[2] - MINIMAP_SIZE - MINIMAP_OFF;
	float bottomright_top = m_pos[1] + 40 - MINIMAP_OFF;

	for(int32_t y=0; y<3; y++)
	{
		for(int32_t x=0; x<3; x++)
		{
			int32_t i = y*3 + x;

			Button* b = &bottomright_button[i];

			b->m_pos[0] = bottomright_left + MINIMAP_SIZE/3 * x;
			b->m_pos[1] = bottomright_top + MINIMAP_SIZE/3 * y;
			b->m_pos[2] = bottomright_left + MINIMAP_SIZE/3 * (x+1);
			b->m_pos[3] = bottomright_top + MINIMAP_SIZE/3 * (y+1);
			b->reframe();

			CenterLabel(b);
		}
	}

#if 0
	leftinnerdiagblur.m_pos[0] = m_pos[0];
	leftinnerdiagblur.m_pos[1] = m_pos[1];
	leftinnerdiagblur.m_pos[2] = m_pos[0]+32;
	leftinnerdiagblur.m_pos[3] = m_pos[1]+24;

	rightinnerdiagblur.m_pos[0] = m_pos[2]-32;
	rightinnerdiagblur.m_pos[1] = m_pos[1];
	rightinnerdiagblur.m_pos[2] = m_pos[2];
	rightinnerdiagblur.m_pos[3] = m_pos[1]+24;

	restext.m_pos[0] = m_pos[0]+32;
	restext.m_pos[1] = m_pos[1];
	restext.m_pos[2] = m_pos[2]-32;
	restext.m_pos[3] = m_pos[1]+24-3;

#if 0
#if 1
	whitebg.m_pos[0] = m_pos[0]+32;
	whitebg.m_pos[1] = m_pos[1];
	whitebg.m_pos[2] = m_pos[2]-32;
	whitebg.m_pos[3] = m_pos[1]+24;
#else
	whitebg.m_pos[0] = m_pos[0];
	whitebg.m_pos[1] = m_pos[1];
	whitebg.m_pos[2] = m_pos[2];
	whitebg.m_pos[3] = m_pos[1]+24;
#endif
#endif

#if 0
	
	innerbottom.m_pos[0] = m_pos[0]+32;
	innerbottom.m_pos[1] = m_pos[1]+24-3;
	innerbottom.m_pos[2] = m_pos[2]-32;
	innerbottom.m_pos[3] = m_pos[1]+24;
#endif
	
	float centerw = (m_pos[0]+m_pos[2])/2;

#if 0
	lefthlineblur.m_pos[0] = m_pos[0]+32;
	lefthlineblur.m_pos[1] = m_pos[1]+24-3;
	lefthlineblur.m_pos[2] = centerw;
	lefthlineblur.m_pos[3] = m_pos[1]+24;
#endif
	
	righthlineblur.m_pos[0] = centerw;
	righthlineblur.m_pos[1] = m_pos[1]+24-3;
	righthlineblur.m_pos[2] = m_pos[2]-32;
	righthlineblur.m_pos[3] = m_pos[1]+24;
#endif
}

void BotPan::draw()
{
#if 0
	Text restext;
	Image leftinnerdiagblur;
	Image rightinnerdiagblur;
	Image innerbottom;
	Image lefthlineblur;
	Image righthlineblur;
	Image whitebg;
#endif

	Shader* s = &g_shader[g_curS];
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);

#if 0
	whitebg.draw();
#endif

	//left_outer_toprightcorner.draw();
	//left_outer_top.draw();

	//right_outer_topleftcorner.draw();
	//right_outer_top.draw();

	//right_inner.draw();
	//right_left.draw();

	//middle_outer_top.draw();

	//white_bg.draw();
	//left_minimap.draw();

	for(int32_t y=0; y<3; y++)
	{
		for(int32_t x=0; x<3; x++)
		{
			int32_t i = y*3 + x;

			if(!bottomright_button_on[i])
				continue;

			Button* b = &bottomright_button[i];

			b->draw();
		}
	}

	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->draw();

#if 0
	restext.draw();

	//RichText rt = RichText(")A)JJF)@J)(J)(cost$KJ(0jfjfjoi3jfwkjlekf");
	//DrawShadowedTextF(MAINFONT16, m_pos[0]+32, m_pos[1]+4, 0, 0, 50, 50, &rt);

	leftinnerdiagblur.draw();
	rightinnerdiagblur.draw();
	lefthlineblur.draw();
	righthlineblur.draw();
	//innerbottom.draw();
#endif

#if 0
	float grpos[4];
	grpos[0] = m_pos[0] + MINIMAP_SIZE + 32;
	grpos[1] = m_pos[1] + 64;
	grpos[2] = m_pos[2] - MINIMAP_SIZE - 32;
	grpos[3] = m_pos[3];
	//DrawGraph(&g_graph[GR_AVGFOD], grpos[0], grpos[1], grpos[2], grpos[3]);
	DrawGraph(&g_graph[graph_sel.m_selected], grpos[0], grpos[1], grpos[2], grpos[3]);

#endif
	//graph_sel.draw();
}

void BotPan::drawover()
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->drawover();

	//graph_sel.drawover();
}

void BotPan::inev(InEv* ie)
{
	for(int32_t i=0; i<9; i++)
		if(bottomright_button_on[i])
			bottomright_button[i].inev(ie);

	for(std::list<Widget*>::reverse_iterator i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->inev(ie);

	//graph_sel.inev(ie);
}

void BotPan::frameupd()
{
	for(std::list<Widget*>::reverse_iterator i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->frameupd();
}
