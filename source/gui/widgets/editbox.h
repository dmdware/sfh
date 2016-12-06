










#ifndef EDITBOX_H
#define EDITBOX_H

#include "../widget.h"

class EditBox : public Widget
{
public:
	int32_t m_highl[2];	// highlighted (selected) text
	UStr m_compos;	//composition for unicode text
	void (*submitfunc)();
	void (*changefunc3)(uint32_t key, uint32_t scancode, bool down, int32_t parm);

	EditBox();
	EditBox(Widget* parent, const char* n, const RichText t, int32_t f, void (*reframef)(Widget* w), bool pw, int32_t maxl, void (*change3)(uint32_t key, uint32_t scancode, bool down, int32_t parm), void (*submitf)(), int32_t parm);

	void draw();
	RichText drawvalue();
	void frameupd();
	void inev(InEv* ie);
	void placestr(const RichText* str);
	void changevalue(const RichText* str);
	bool delnext();
	bool delprev();
	void copyval();
	void pasteval();
	void selectall();
	void hide();
	void gainfocus();
	void losefocus();
};

#endif
