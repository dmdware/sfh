











#ifndef BUILD_H
#define BUILD_H

#include "../math/vec3f.h"
#include "../math/vec2i.h"

class Building;
class BlType;
class Matrix;

void DrawSBl(uint32_t renderdepthtex, uint32_t renderfb);
void UpdSBl();
bool CheckCanPlace(int32_t type, Vec2i tpos, int32_t targb);
void RecheckStand();
void DrawBReason(Matrix* mvp, float width, float height, bool persp);
bool PlaceBl(int32_t type, Vec2i pos, bool finished, int32_t owner, int32_t* bid);
bool PlaceBAb(int32_t btype, Vec2i tabout, Vec2i* tplace);
bool PlaceBAb4(int32_t btype, Vec2i tabout, Vec2i* tplace);
bool PlaceUAb(int32_t utype, Vec2i cmabout, Vec2i* cmplace);
bool BlCollides(int32_t type, Vec2i tpos, int32_t targb);
bool Collides(Vec2i cmmin, Vec2i cmmax, int32_t targb);

#endif
