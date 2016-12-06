











#ifndef SELECTION_H
#define SELECTION_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "conduit.h"

class Selection
{
public:
	std::list<int32_t> units;
	std::list<int32_t> buildings;
	//std::list<Vec2i> cdtiles[CD_TYPES];
	std::list<Vec2i> roads;
	std::list<Vec2i> powls;
	std::list<Vec2i> crpipes;
	std::list<uint16_t> fol;

	void clear();
};

extern uint32_t g_circle;
extern Selection g_sel;

Selection DoSel();
void DrawSel(Matrix* projection, Matrix* modelmat, Matrix* viewmat);
void DrawMarquee();
void ClearSel(Selection* s);
void AfterSel(Selection* s);
bool USel(int16_t ui);
bool BSel(int16_t bi);

#endif
