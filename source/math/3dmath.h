










#ifndef MATH3D_H
#define MATH3D_H

#include "../platform.h"
#include "vec2f.h"
#include "vec3f.h"
#include "vec2i.h"
#include "vec3i.h"
#include "fixmath.h"
#include "../utils.h"

#define DEGTORAD(a)		(M_PI * a / 180)
#define RADTODEG(a)		(180 * a / M_PI)

#ifndef PI_ON_180
#	define PI_ON_180	(M_PI/180.0)
#endif

#define CAMERA_SPEED	121

class Plane3f;
class Vec3f;
class Camera;
class Matrix;
class Triangle;
class Vec4f;
class Vec2i;
class Vec3i;
class Vec2f;

Vec3f VMin(float minf, Vec3f v);

#define MAG_VEC3F(vec)		((float)sqrtf( ((vec).x * (vec).x) + ((vec).y * (vec).y) + ((vec).z * (vec).z) ))
#define MAG_VEC2F(vec)		((float)sqrtf( ((vec).x * (vec).x) + ((vec).y * (vec).y) ))
#define DOT_VEC3F(vec)		(((vec).x * (vec).x) + ((vec).y * (vec).y) + ((vec).z * (vec).z))
#define DOT_VEC2F(vec)		(((vec).x * (vec).x) + ((vec).y * (vec).y))
#define MAG_VEC2I(vec)		(isqrt( (vec).x*(vec).x + (vec).y*(vec).y ))	////MUST BE DETERMINISTIC!!!!
#define DOT_VEC2I(vec)		((vec).x*(vec).x + (vec).y*(vec).y)
#define MAN_VEC2I(vec)		(iabs((vec).x) + iabs((vec).y))	// Manhattan distance

inline int32_t Diag(int32_t x, int32_t y)
{
	int32_t ax = iabs(x);
	int32_t ay = iabs(y);
	return ((((ax+ay)<<1) + (3 - 4) * imin(ax, ay))>>1);
}

//Diagonal distance
//#define DIA_VEC2I(vec)		((((iabs(vec.x)+iabs(vec.y))<<1) + (3 - 4) * imin(iabs(vec.x), iabs(vec.y)))>>1)
#define DIA_VEC2I(vec)			Diag(vec.x,vec.y)
//#define DIA_VEC2I(vec)		((((abs(vec.x)+abs(vec.y))<<1) + (3 - 4) * std::min(abs(vec.x), abs(vec.y)))>>1)

/*
    dx = abs(node.x - goal.x)
    dy = abs(node.y - goal.y)
    return D * (dx + dy) + (D2 - 2 * D) * min(dx, dy)

	D2 = 3
	D = 2
*/

Vec3f Normalize(Vec3f vNormal);
Vec2f Normalize(Vec2f vNormal);
Vec3f Cross(Vec3f vVector1, Vec3f vVector2);
float Dot(Vec3f vVector1, Vec3f vVector2);
float Dot(Vec3f vNormal);
int32_t Dot(Vec3i vVector1, Vec3i vVector2);
int32_t Dot(Vec2i vVector1, Vec2i vVector2);
Vec3f Vector(Vec3f vPoint1, Vec3f vPoint2);
Vec3f Normal(Vec3f vTriangle[]);
Vec3f Normal2(Vec3f vTriangle[]);
bool InterPlane(Vec3f vPoly[], Vec3f vLine[], Vec3f *vNormal, float *originDistance);
bool WithinYaw(Camera* c, Vec3f p, float angle);
float DYaw(Camera* c, Vec3f p);
double AngleBetweenVectors(Vec3f Vector1, Vec3f Vector2);
Vec3f IntersectionPoint(Vec3f vNormal, Vec3f vLine[], double distance);
bool Intersection(Vec3f l0, Vec3f l, Plane3f p, Vec3f* inter);
Vec3f OnNear(int32_t x, int32_t y, int32_t width, int32_t height, Vec3f posvec, Vec3f sidevec, Vec3f upvec);
Vec3f OnNearPersp(int32_t x, int32_t y, int32_t width, int32_t height, Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, float fov, float mind);
Vec3f ScreenPerspRay(int32_t x, int32_t y, int32_t width, int32_t height, Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, float fov);
Vec3f Rotate(Vec3f v, float rad, float x, float y, float z);
Vec3f RotateAround(Vec3f v, Vec3f around, float rad, float x, float y, float z);
float GetYaw(float dx, float dz);
Matrix gluLookAt2(float eyex, float eyey, float eyez,
				  float centerx, float centery, float centerz,
				  float upx, float upy, float upz);
Matrix LookAt(float eyex, float eyey, float eyez,
				  float centerx, float centery, float centerz,
				  float upx, float upy, float upz);
Matrix PerspProj(float fov, float aspect, float znear, float zfar);
Matrix OrthoProj(float l, float r, float t, float b, float n, float f);
Vec4f ScreenPos(Matrix* mvp, Vec3f vec, float width, float height, bool persp);
float Snap(float base, float value);
float SnapNearest(float base, float value);
int Clipi(int n, int lower, int upper);

#endif
