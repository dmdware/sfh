











#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/vec3i.h"
#include "../math/vec2i.h"

class OrderMarker
{
public:
	Vec2i pos;
	int32_t tick;
	float radius;

	OrderMarker(Vec2i p, int32_t t, float r)
	{
		pos = p;
		tick = t;
		radius = r;
	}
};

extern std::list<OrderMarker> g_order;

#define ORDER_EXPIRE		2000

struct MoveOrderPacket;

void DrawOrders(Matrix* projection, Matrix* modelmat, Matrix* viewmat);
void Order(int32_t mousex, int32_t mousey, int32_t viewwidth, int32_t viewheight);
void MoveOrder(MoveOrderPacket* mop);
