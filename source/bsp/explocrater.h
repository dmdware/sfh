
#ifndef EXPLOCRATER_H
#define EXPLOCRATER_H

#include "../phys/physics.h"
#include "../math/vec3f.h"

class EdMap;
class Brush;
class Plane3f;

void ExplodeCrater(EdMap* map, Vec3f line[], Vec3f vmin, Vec3f vmax);

#endif