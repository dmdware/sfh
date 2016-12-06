

#include "tut.h"
#include "../gui/gui.h"
#include "../gui/layouts/playgui.h"
#include "../gui/widgets/spez/botpan.h"
#include "../language.h"
#include "../gui/layouts/messbox.h"

#define TUT_START		0

int32_t g_tutstep = 0;
Vec2i g_enable[2];

void Highlight(float* pos)
{
	GUI* gui = &g_gui;

	if(!pos)
	{
		gui->hide("high");
		return;
	}

	if(!gui->get("high"))
	{
		ViewLayer* hl = new ViewLayer(gui, "high");
		Image* i0 = new Image(hl, "i0", "gui/backg/white.png", false, NULL, 1,1,1,0.5f);
		Image* i1 = new Image(hl, "i1", "gui/backg/white.png", false, NULL, 1,1,1,0.5f);
		Image* i2 = new Image(hl, "i2", "gui/backg/white.png", false, NULL, 1,1,1,0.5f);
		Image* i3 = new Image(hl, "i3", "gui/backg/white.png", false, NULL, 1,1,1,0.5f);
		hl->add(i0);
		hl->add(i1);
		hl->add(i2);
		hl->add(i3);
		gui->add(hl);
	}

	gui->show("high");
	
	ViewLayer* hl = (ViewLayer*)gui->get("high");
	Image* i0 = (Image*)hl->get("i0");
	Image* i1 = (Image*)hl->get("i1");
	Image* i2 = (Image*)hl->get("i2");
	Image* i3 = (Image*)hl->get("i3");
	
	i0->show();
	i0->m_pos[0] = 0;
	i0->m_pos[1] = 0;
	i0->m_pos[2] = pos[0];
	i0->m_pos[3] = g_height;
	
	i1->show();
	i1->m_pos[0] = pos[0];
	i1->m_pos[1] = 0;
	i1->m_pos[2] = pos[2];
	i1->m_pos[3] = pos[1];
	
	i2->show();
	i2->m_pos[0] = pos[2];
	i2->m_pos[1] = 0;
	i2->m_pos[2] = g_width;
	i2->m_pos[3] = g_height;
	
	i3->show();
	i3->m_pos[0] = pos[0];
	i3->m_pos[1] = pos[3];
	i3->m_pos[2] = pos[2];
	i3->m_pos[3] = g_height;
}

void Remind(RichText* rt)
{
	GUI* gui = &g_gui;

	if(!rt)
	{
		gui->hide("rem");
		return;
	}

	if(!gui->get("rem"))
	{
		ViewLayer* hl = new ViewLayer(gui, "rem");
		TextBlock* tb = new TextBlock(hl, "tb", *rt, MAINFONT32, NULL, 0.5f, 0.5f, 0.1f, 0.5f);
		gui->add(hl);
		hl->add(tb);
	}
	
	gui->show("rem");

	ViewLayer* hl = (ViewLayer*)gui->get("rem");
	TextBlock* tb = (TextBlock*)hl->get("tb");
	
	tb->show();
	tb->m_text = *rt;
	tb->m_pos[0] = g_width/10;
	tb->m_pos[1] = g_height/10;
	tb->m_pos[2] = g_width*8/10;
	tb->m_pos[3] = g_height*8/10;
}

#if 0
void Tut_start()
{
	float h[4];
	float bottomright_left = g_width - MINIMAP_SIZE - MINIMAP_OFF;
	float bottomright_top = (g_height - MINIMAP_SIZE - 32) + 40 - MINIMAP_OFF;
	
#if 0
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
#endif
	h[0] = bottomright_left + MINIMAP_SIZE/3 * 0;
	h[1] = bottomright_top + MINIMAP_SIZE/3 * 0;
	h[2] = bottomright_left + MINIMAP_SIZE/3 * (0+1);
	h[3] = bottomright_top + MINIMAP_SIZE/3 * (0+1);

	Click_NextBuildButton(0);
	Highlight(h);
	Remind(&STRTABLE[STR_T]);

	g_tutstep++;
}
#else
void Tut_start()
{
	Mess(&STRTABLE[STR_T], AdvTut);
	g_tutstep++;
}
#endif

void Tut_next()
{
	if(g_tutstep > 11)
		return;

	Mess(&STRTABLE[STR_T+g_tutstep], AdvTut);
	g_tutstep++;
}

void AdvTut()
{
	Remind(NULL);
	Highlight(NULL);

	switch(g_tutstep)
	{
	case TUT_START:
		Tut_start();
		break;
	default:
		Tut_next();
		break;
	}
}