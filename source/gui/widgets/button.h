










#ifndef BUTTON_H
#define BUTTON_H

#include "../widget.h"

//styles
#define BUST_CORRODE			0
#define BUST_LINEBASED		1
#define BUST_LEFTIMAGE		2

class Button : public Widget
{
public:

	int32_t m_style;

	Button();
	Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip, int32_t f, int32_t style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int32_t p), void (*overf)(), void (*overf2)(int32_t p), void (*out)(), int32_t parm, void (*click3)(Widget* w));

	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);
};

#endif
