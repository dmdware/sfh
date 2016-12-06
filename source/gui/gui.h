










#ifndef GUI_H
#define GUI_H

#include "../platform.h"
#include "../math/3dmath.h"
#include "viewlayer.h"
#include "widgets/image.h"
#include "widgets/barbutton.h"
#include "widgets/button.h"
#include "widgets/checkbox.h"
#include "widgets/droplist.h"
#include "widgets/editbox.h"
#include "widgets/insdraw.h"
#include "widgets/link.h"
#include "widgets/listbox.h"
#include "widgets/text.h"
#include "widgets/textarea.h"
#include "widgets/textblock.h"
#include "widgets/touchlistener.h"
#include "widgets/frame.h"
#include "widgets/viewportw.h"

#ifdef USEZOOM
extern uint32_t g_zoomtex;
extern bool g_zoomdrawframe;
extern uint32_t g_zoomfbo;
#endif

class GUI : public Widget
{
public:
	std::list<ViewLayer> view;
	void (*keyupfunc[SDL_NUM_SCANCODES])();
	void (*keydownfunc[SDL_NUM_SCANCODES])();
	void (*anykeyupfunc)(int32_t k);
	void (*anykeydownfunc)(int32_t k);
	void (*mousemovefunc)(InEv* ie);
	void (*lbuttondownfunc)();
	void (*lbuttonupfunc)();
	void (*rbuttondownfunc)();
	void (*rbuttonupfunc)();
	void (*mbuttondownfunc)();
	void (*mbuttonupfunc)();
	void (*mousewheelfunc)(int32_t delta);
	Widget* activewidg;

	GUI() : Widget()
	{
		m_type = WIDGET_GUI;

		//corpc fix
		for(int32_t i=0; i<SDL_NUM_SCANCODES; i++)
		{
			keyupfunc[i] = NULL;
			keydownfunc[i] = NULL;
		}
		
		anykeyupfunc = NULL;
		anykeydownfunc = NULL;
		lbuttondownfunc = NULL;
		lbuttonupfunc = NULL;
		rbuttondownfunc = NULL;
		rbuttonupfunc = NULL;
		mbuttondownfunc = NULL;
		mbuttonupfunc = NULL;
		mousewheelfunc = NULL;
		mousemovefunc = NULL;

		m_hidden = false;
	}

	void assignmousewheel(void (*wheel)(int32_t delta))
	{
		mousewheelfunc = wheel;
	}

	void assignlbutton(void (*down)(), void (*up)())
	{
		lbuttondownfunc = down;
		lbuttonupfunc = up;
	}

	void assignrbutton(void (*down)(), void (*up)())
	{
		rbuttondownfunc = down;
		rbuttonupfunc = up;
	}

	void assignmbutton(void (*down)(), void (*up)())
	{
		mbuttondownfunc = down;
		mbuttonupfunc = up;
	}

	void assignmousemove(void (*mouse)(InEv* ie))
	{
		mousemovefunc = mouse;
	}

	void assignkey(int32_t i, void (*down)(), void (*up)())
	{
		keydownfunc[i] = down;
		keyupfunc[i] = up;
	}

	void assignanykey(void (*down)(int32_t k), void (*up)(int32_t k))
	{
		anykeydownfunc = down;
		anykeyupfunc = up;
	}

	void draw();
	void drawover(){}
	void inev(InEv* ie);
	void reframe();
};

extern GUI g_gui;

bool MousePosition();
void CenterMouse();
void SetStatus(const char* status, bool logthis=false);
void Ortho(int32_t width, int32_t height, float r, float g, float b, float a);

#endif
