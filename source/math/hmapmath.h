










#ifndef HMAPMATH_H
#define HMAPMATH_H

#include "../math/vec3f.h"
#include "../math/vec2uc.h"

class Heightmap;

float Bilerp(Heightmap* hmap, float x, float y);
bool MapInter(Heightmap* hmap, Vec3f ray, Vec3f point, Vec3i* cmint);
bool FastMapIntersect(Heightmap* hmap, Vec2uc sz, Vec3f* line, Vec3f* intersect, bool flipyz=true);
bool MapBoundsIntersect(Vec2uc sz, Vec3f* line, Vec3f* intersect, bool flipyz=true);

#endif
