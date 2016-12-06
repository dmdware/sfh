










#ifndef INEV_H
#define INEV_H

#include "../platform.h"

#define INEV_MOUSEMOVE		0
#define INEV_MOUSEDOWN		1
#define INEV_MOUSEUP		2
#define INEV_KEYDOWN		3
#define INEV_KEYUP			4
#define INEV_CHARIN			5
#define INEV_MOUSEWHEEL		6
#define INEV_TEXTED			7
#define INEV_TEXTIN			8
#define INEV_COPY			9
#define INEV_PASTE			10
#define INEV_SELALL			11	//select all

#define MOUSE_LEFT			0
#define MOUSE_MIDDLE		1
#define MOUSE_RIGHT			2

class Widget;

class InEv
{
public:
	int32_t type;
	int32_t x;
	int32_t y;
	int32_t dx;
	int32_t dy;
	uint32_t key;
	int32_t scancode;
	int32_t amount;
	bool intercepted;
	std::string text;	//UTF8
	int32_t cursor;	//cursor pos in composition
	int32_t sellen;	//selection length
	signed char curst;	//cursor state/type eg. drag, resize, normal
	Widget* interceptor;	//corpd fix

	InEv();
};


#endif
