










#ifndef VIEWPORTW_H
#define VIEWPORTW_H

#include "../widget.h"

class Viewport : public Widget
{
public:
	void (*drawfunc)(int32_t p, int32_t x, int32_t y, int32_t w, int32_t h);
	bool (*ldownfunc)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h);
	bool (*lupfunc)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h);
	bool (*mousemovefunc)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h);
	bool (*rdownfunc)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h);
	bool (*rupfunc)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h);
	bool (*mousewfunc)(int32_t p, int32_t d);
	bool (*mdownfunc)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h);
	bool (*mupfunc)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h);

	Viewport();

	Viewport(Widget* parent, const char* n, void (*reframef)(Widget* w),
	          void (*drawf)(int32_t p, int32_t x, int32_t y, int32_t w, int32_t h),
	          bool (*ldownf)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h),
			  bool (*lupf)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h),
			  bool (*mousemovef)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h),
			  bool (*rdownf)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h),
			  bool (*rupf)(int32_t p, int32_t relx, int32_t rely, int32_t w, int32_t h),
			  bool (*mousewf)(int32_t p, int32_t d),
			  bool (*mdownf)(int p, int relx, int rely, int w, int h),
			  bool (*mupf)(int p, int relx, int rely, int w, int h),
	          int32_t parm);
	void inev(InEv* ie);
	void draw();
};

#endif
