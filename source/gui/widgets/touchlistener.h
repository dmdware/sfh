










#ifndef TOUCHLISTENER_H
#define TOUCHLISTENER_H

#include "../widget.h"

class TouchListener : public Widget
{
public:
	TouchListener();
	TouchListener(Widget* parent, const char* name, void (*reframef)(Widget* w), void (*click2)(int32_t p), void (*overf)(int32_t p), void (*out)(), int32_t parm);

	void inev(InEv* ie);
};

#endif
