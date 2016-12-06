










#ifndef DROPDOWNS_H
#define DROPDOWNS_H

#include "../widget.h"

class DropList : public Widget
{
public:

	int32_t m_mousedown[2];

	bool m_downdrop;

	DropList();
	DropList(Widget* parent, const char* n, int32_t f, void (*reframef)(Widget* w), void (*change)());

	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);

	int32_t rowsshown();
	int32_t square();
	void erase(int32_t which);

	float topratio()
	{
		return m_scroll[1] / (float)(m_options.size());
	}

	float bottomratio()
	{
		return (m_scroll[1]+rowsshown()) / (float)(m_options.size());
	}

	float scrollspace();
};

#endif
