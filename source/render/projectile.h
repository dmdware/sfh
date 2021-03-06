











#include "../platform.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"

class ProjectileType
{
public:
	uint32_t tex;

	void Define(char* texpath);
};

enum PROJECTILE {GUNPROJ, PROJECIN_TYPES};
extern ProjectileType g_projectileType[PROJECIN_TYPES];

class Projectile
{
public:
	bool on;
	Vec3f start;
	Vec3f end;
	int32_t type;

	Projectile()
	{
		on = false;
	}

	Projectile(Vec3f s, Vec3f e, int32_t t)
	{
		on = true;
		start = s;
		end = e;
		type = t;
	}
};

#define PROJECTILES	128
extern Projectile g_projectile[PROJECTILES];

void LoadProjectiles();
void DrawProjectiles();
void NewProjectile(Vec3f start, Vec3f end, int32_t type);
