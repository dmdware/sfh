

#ifndef BORDER_H
#define BORDER_H

#include "../math/vec2i.h"

//territory per tile
extern signed char* g_border;

class Unit;
class Building;

void DrawBords(Vec2i tmin, Vec2i tmax);
void MarkTerr(Unit* u);
void MarkTerr(Building* b);

#endif