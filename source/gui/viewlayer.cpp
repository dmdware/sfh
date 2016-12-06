










#include "gui.h"
#include "../texture.h"
#include "../sim/player.h"

void ViewLayer::reframe()
{
	Player* py = &g_player[g_localP];

	m_pos[0] = 0;
	m_pos[1] = 0;
	m_pos[2] = g_width-(float)1;
	m_pos[3] = g_height-(float)1;

	Widget::reframe();
}

void ViewLayer::show()
{
	Widget::show();
	
	//necessary for window widgets:
	tofront();	//can't break list iterator, might shift
}