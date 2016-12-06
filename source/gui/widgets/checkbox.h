










#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "../widget.h"

class CheckBox : public Widget
{
public:
	CheckBox();
	CheckBox(Widget* parent, const char* n, const RichText t, int32_t f, void (*reframef)(Widget* w), int32_t sel=0, float r=1, float g=1, float b=1, float a=1, void (*change)()=NULL);

	void draw();
	void inev(InEv* ie);
	int32_t square();
};

#endif
