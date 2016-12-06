










#ifndef BARBUTTON_H
#define BARBUTTON_H

#include "../widget.h"
#include "button.h"

class BarButton : public Button
{
public:
	float m_healthbar;

	BarButton(Widget* parent, uint32_t sprite, float bar, void (*reframef)(Widget* w), void (*click)(), void (*overf)(), void (*out)());

	void draw();
};

#endif
