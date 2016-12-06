



#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/polygon.h"
#include "../math/3dmath.h"
#include "hmapmath.h"
#include "../render/water.h"
#include "../utils.h"
#include "../phys/collision.h"
#include "../window.h"
#include "../math/camera.h"


float Bilerp(Heightmap* hmap, float x, float y)
{
	x /= (float)TILE_SIZE;
	y /= (float)TILE_SIZE;

	int32_t x1 = (int32_t)(x);
	int32_t x2 = x1 + 1;
	x2 = imin(x2, hmap->m_width.x);

	int32_t y1 = (int32_t)(y);
	int32_t y2 = y1 + 1;
	y2 = imin(y2, hmap->m_width.y);

	//TODO could eliminate xdenom, ydenom from this
	//implementation since we make them effectively equal to 1.

	float xdenom = x2-x1;
	float x2fac = (x2-x)/xdenom;
	float x1fac = (x-x1)/xdenom;

	float hR1 = hmap->getheight(x1,y1)*TILE_RISE*x2fac + hmap->getheight(x2,y1)*TILE_RISE*x1fac;
	float hR2 = hmap->getheight(x1,y2)*TILE_RISE*x2fac + hmap->getheight(x2,y2)*TILE_RISE*x1fac;

	float ydenom = y2-y1;

	return hR1*(y2-y)/ydenom + hR2*(y-y1)/ydenom;
}

//fast map intersect
bool MapInter(Heightmap* hmap, Vec3f ray, Vec3f point, Vec3i* cmint)
{
	//float maxx = (float)((g_mapsz.x + 1) * TILE_SIZE);
	//float maxy = (float)((g_mapsz.y + 1) * TILE_SIZE);

	float maxx = (float)( (g_mapsz.x+1) * TILE_SIZE - 1 );
	float maxy = (float)( (g_mapsz.y+1) * TILE_SIZE - 1 );

	//back up the ray to the farthest bottom-most corner of the screen
	//because we want to project from that direction upwards,
	//returning the first tile surface we collide with.
	//this only matters tiles can occlude each other, but,
	//either way we need to start from an edge.

	float xratio = ( maxx - point.x ) / ray.x;
	float yratio = ( maxy - point.y ) / ray.y;

	//move the least distance to an edge
	//we use fmax, not fmin, because ray is negative and we want the smallest negative (thus fmax).
	float minratio = fmax(xratio, yratio);
	point = point + ray * minratio;

	//go along the tiles, one by one,
	//checking if we're in between the low and high
	//boundaries of the tile.

	//the minimum distance by which we advance on to the next tile
	//we use fmax, not fmin, because ray is negative and we want the smallest negative (thus fmax).
	xratio = TILE_SIZE / ray.x;
	yratio = TILE_SIZE / ray.y;
	minratio = fmax(xratio, yratio);
	//minray will be positive
	//divide by two to make sure we don't miss a tile
	Vec3f minray = ray * minratio / 2.0f;

	//Log("start "<<point.x<<","<<point.y<<","<<point.z<<" ray*minratio="<<minray.x<<","<<minray.y<<","<<minray.z);

	//check far bound to prevent inaddressable access
	while(point.x >= maxx || point.y >= maxy)
	{
		point = point - minray;
	}

	for(Vec3f lastpoint = point;
		point.x >= 0.0f && point.y >= 0.0f && point.z >= 0.0f;
		lastpoint = point, point = point - minray)
	{
		unsigned char tx = (int32_t)(point.x)/TILE_SIZE;
		unsigned char ty = (int32_t)(point.y)/TILE_SIZE;

		if(tx >= g_mapsz.x)
			continue;

		if(ty >= g_mapsz.y)
			continue;

		if(tx < 0)
			continue;

		if(ty < 0)
			continue;

		Tile& tile = SurfTile( tx, ty, hmap );

		int32_t minz = tile.elev * TILE_RISE;
		int32_t maxz = minz + TILE_RISE;

		//better to have accidentally trip a tile that is higher on the screen,
		//as it will be a fallback if the front one isn't detected.
		minz -= TILE_RISE * 2;
		//maxy += TILE_RISE;

		//if(tile.incltype > IN_0000)
		//	maxy += TILE_RISE;

		//Log("point "<<point.x<<","<<point.y<<","<<point.z<<" tile "<<(int32_t)tx<<","<<(int32_t)ty<<", zrange="<<miny<<","<<maxy);

		//point.z is lower bound (because ray goes down)
		//lastpoint.z is upper bound

		//using 1.0f as epsilon

		if(point.z > (float)maxz + 1.0f)
			continue;

		if(lastpoint.z < (float)minz - 1.0f)
			continue;

		//using TILE_RISE as epsilon

		//if(point.z < (float)miny - TILE_RISE && lastpoint.z < (float)miny - TILE_RISE)
		//	continue;

		//if(point.z > (float)maxy + TILE_RISE && lastpoint.z > (float)maxy + TILE_RISE)
		//	continue;

		//once we get a tile, approximate the point
		//of intersect several times, going back and
		//forth, getting height using bilinear interp.

#define APPROXS		5	//how many times do we want to approach the intersect?

			Vec3f mintile( (float)(tx*TILE_SIZE), (float)(ty*TILE_SIZE), (float)minz );
			Vec3f maxtile( (float)((tx+1)*TILE_SIZE-1), (float)((ty+1)*TILE_SIZE-1), (float)maxz );
			Vec3f subpoint = point;

			float xratio2;
			float yratio2;

			//position mintile and maxtile along the ray, but with heights equal to tile elevations.
			//will epsilon effect this if point.x is greater than maxtile.x for example?

			xratio2 = (subpoint.x - mintile.x) / -ray.x;
			yratio2 = (subpoint.y - mintile.y) / -ray.y;
			mintile = subpoint + ray * fmin(xratio2,yratio2);

			xratio2 = (maxtile.x - subpoint.x) / -ray.x;
			yratio2 = (maxtile.y - subpoint.y) / -ray.y;
			maxtile = subpoint + ray * fmin(xratio2,yratio2);

			mintile.z = Bilerp(&g_hmap, mintile.x, mintile.y);
			maxtile.z = Bilerp(&g_hmap, maxtile.x, maxtile.y);

			//move the ray point along the ray to the far tile edge.
			xratio2 = (maxtile.x - point.x) / -ray.x;
			yratio2 = (maxtile.y - point.y) / -ray.y;
			Vec3f maxpoint = point + ray * fmin(xratio2,yratio2);

			//now move it to the center
			xratio2 = ( maxpoint.x - (maxtile.x+mintile.x)/2.0f ) / -ray.x;
			yratio2 = ( maxpoint.y - (maxtile.y+mintile.y)/2.0f ) / -ray.y;
			subpoint = maxpoint + ray * fmin(xratio2,yratio2);

			for(int32_t ap=0; ap<APPROXS; ap++)
			{
				//get mid point and see which side of it the intersect probably is

				Vec3f mid = ( maxtile + mintile ) / 2.0f;
				mid.z = Bilerp(&g_hmap, mid.x, mid.y);

				//are we seeking lower (negative) or higher (positive) ground?
				float zseek = subpoint.z - mid.z;

				//what is the change in elevation as we go from mid to near?
				//here near = closer to origin
				float nearzrise = mintile.z - mid.z;
				//what is the change in elevation as we go from mid to far?
				float farzrise = maxtile.z - mid.z;

				//fix for flat terrain
				if(nearzrise == farzrise)
				{
					nearzrise += ray.z;
					farzrise -= ray.z;
				}

				//simple trick to reduce work we have to do
				if(zseek < 0.0f)
				{
					zseek = -zseek;
					nearzrise = -nearzrise;
					farzrise = -farzrise;
				}

				//now just find the one with the highest rise
				if(nearzrise > farzrise)
				{
					//go closer to near / mintile
					//but first check if the raypoint will be out of z bounds
					//if(mid.z < subpoint.z)
					//	break;
					maxtile = mid;
				}
				else
				{
					//go closer to far / maxtile
					//but first check if the raypoint will be out of z bounds
					//if(mid.z > subpoint.z)
					//	break;
					mintile = mid;
				}

				//move the ray point closer along (x,y) to the new range.
				//first move it to the most positive point.
				subpoint = maxpoint;

				//now move it to the center
				xratio2 = ( subpoint.x - (maxtile.x+mintile.x)/2.0f ) / -ray.x;
				yratio2 = ( subpoint.y - (maxtile.y+mintile.y)/2.0f ) / -ray.y;
				subpoint = subpoint + ray * fmin(xratio2,yratio2);

				//is it out of the tile bounds?
				//if(mintile.z > raypoint.z || maxtile.z < raypoint.z)
				//	break;
				//edit: don't want to do this in here, because there might be a bump in the mid area of the tile.
			}

			//is it out of the tile bounds?
			//if(mintile.z > subpoint.z || maxtile.z < subpoint.z)
			//	continue;	//try next tile

			Vec3f fint = (maxtile + mintile) / 2.0f;
			fint.z = Bilerp(&g_hmap, fint.x, fint.y);

			*cmint = Vec3i((int32_t)fint.x, (int32_t)fint.y, (int32_t)fint.z);

			//Log("INT!");

			return true;
#undef APPROXS
	}

	//we might want an intersect off the map for view culling if we can't get a map intersect

	float zratio = point.z / ray.z;
	point = point - ray * zratio;

	*cmint = Vec3i((int32_t)point.x, (int32_t)point.y, (int32_t)point.z);

	return false;
}

bool TileIntersect(Heightmap* hmap, Vec2uc sz, Vec3f* line, int32_t tx, int32_t ty, Vec3f* intersect)
{
    Vec3f tri[3];
    const int32_t wx = sz.x;
    //const int32_t wy = sz.y;
    const Vec3f* v = hmap->origv;
    
    tri[0] = v[ (ty * wx + tx) * 3 * 2 + 0 ];
    tri[1] = v[ (ty * wx + tx) * 3 * 2 + 1 ];
    tri[2] = v[ (ty * wx + tx) * 3 * 2 + 2 ];
	
	tri[0].y *= TILE_RISE;
	tri[1].y *= TILE_RISE;
	tri[2].y *= TILE_RISE;
    
    if(InterPoly(tri, line, 3, intersect))
        return true;
    
    tri[0] = v[ (ty * wx + tx) * 3 * 2 + 3 ];
    tri[1] = v[ (ty * wx + tx) * 3 * 2 + 4 ];
    tri[2] = v[ (ty * wx + tx) * 3 * 2 + 5 ];
    
	tri[0].y *= TILE_RISE;
	tri[1].y *= TILE_RISE;
	tri[2].y *= TILE_RISE;

    if(InterPoly(tri, line, 3, intersect))
        return true;

#if 0
	
    tri[0] = v[ (ty * wx + tx) * 3 * 2 + 2 ];
    tri[1] = v[ (ty * wx + tx) * 3 * 2 + 1 ];
    tri[2] = v[ (ty * wx + tx) * 3 * 2 + 0 ];
	
	tri[0].z *= TILE_RISE;
	tri[1].z *= TILE_RISE;
	tri[2].z *= TILE_RISE;
    
    if(InterPoly(tri, line, 3, intersect))
        return true;
    
    tri[0] = v[ (ty * wx + tx) * 3 * 2 + 5 ];
    tri[1] = v[ (ty * wx + tx) * 3 * 2 + 4 ];
    tri[2] = v[ (ty * wx + tx) * 3 * 2 + 3 ];
    
	tri[0].z *= TILE_RISE;
	tri[1].z *= TILE_RISE;
	tri[2].z *= TILE_RISE;

    if(InterPoly(tri, line, 3, intersect))
        return true;
#endif
    
    return false;
}

bool FastMapIntersect(Heightmap* hmap, Vec2uc sz, Vec3f* line, Vec3f* intersect, bool flipyz)
{
    Vec3f ray = line[1] - line[0];
    Vec3f tilestart = line[0] / TILE_SIZE;
    Vec3f tileray = ray / TILE_SIZE;

	flipyz=false;
	if(flipyz)
	{
		float temp = line[0].y;
		line[0].y = line[0].z;
		line[0].z = temp;
		temp = line[1].y;
		line[1].y = line[1].z;
		line[1].z = temp;
	}
    
    int32_t len = (int32_t)MAG_VEC2I(tileray)+1;

	for(int8_t tx=0; tx<g_mapsz.x; ++tx)
	{
		for(int8_t ty=0; ty<g_mapsz.y; ++ty)
		{
			if(TileIntersect(hmap, sz, line, tx, ty, intersect))
				return true;
		}
	}
    
	return false;

    for(int32_t i=-1; i<len; i++)
    {
        Vec3f pos = tilestart + ray * (float)i / (float)len / (float)TILE_SIZE;
        int32_t ftx = imax( (int32_t)floorf(pos.x), 0 );
        int32_t fty = imax( (int32_t)floorf(pos.y), 0 );
        int32_t ctx = imin( (int32_t)ceilf(pos.x), g_mapsz.x-1 );
        int32_t cty = imin( (int32_t)ceilf(pos.y), g_mapsz.y-1 );
        
        if(ctx < 0)
            continue;
        
        if(cty < 0)
            continue;
        
        if(ftx >= sz.x)
            continue;
        
        if(fty >= sz.y)
            continue;
        
        if(TileIntersect(hmap, sz, line, ftx, fty, intersect))
            return true;
		
        if(TileIntersect(hmap, sz, line, ctx, fty, intersect))
            return true;
		
        if(TileIntersect(hmap, sz, line, ctx, cty, intersect))
            return true;
		
        if(TileIntersect(hmap, sz, line, ftx, cty, intersect))
            return true;
    }
    
    return false;
}

bool MapBoundsIntersect(Vec2uc sz, Vec3f* line, Vec3f* intersect, bool flipyz)
{
	Vec3f q[4];

	//map bottom
	flipyz=false;
	if(flipyz)
	{
		float temp = line[0].y;
		line[0].y = line[0].z;
		line[0].z = temp;
		temp = line[1].y;
		line[1].y = line[1].z;
		line[1].z = temp;
	}
#if 1
	q[0] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[1] = Vec3f((float)10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[2] = Vec3f((float)10*sz.x*TILE_SIZE,(float)10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[3] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
#else
	q[0] = Vec3f(-10*sz.x*TILE_SIZE, WATER_LEVEL*2.0f, -10*sz.y*TILE_SIZE);
	q[1] = Vec3f(10*sz.x*TILE_SIZE, WATER_LEVEL*2.0f, -10*sz.y*TILE_SIZE);
	q[2] = Vec3f(10*sz.x*TILE_SIZE, WATER_LEVEL*2.0f, 10*sz.y*TILE_SIZE);
	q[3] = Vec3f(-10*sz.x*TILE_SIZE, WATER_LEVEL*2.0f, 10*sz.y*TILE_SIZE);
#endif
	
	if(InterPoly(q, line, 4, intersect))
		return true;

	return false;
	//map sides, necessary for the frustum outline on the minimap (player's view outline) to not have corners stretching to origin if they're out of map bounds

	q[0] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[1] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, (float)TILE_SIZE*500);
	q[2] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, (float)TILE_SIZE*500);
	q[3] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	
	if(InterPoly(q, line, 4, intersect))
		return true;

	q[0] = Vec3f((float)10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, (float)TILE_SIZE*500);
	q[1] = Vec3f((float)10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[2] = Vec3f((float)10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[3] = Vec3f((float)10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, (float)TILE_SIZE*500);

	if(InterPoly(q, line, 4, intersect))
		return true;

	q[0] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[1] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, (float)TILE_SIZE*500);
	q[2] = Vec3f((float)10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, (float)TILE_SIZE*500);
	q[3] = Vec3f((float)10*sz.x*TILE_SIZE, (float)-10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);

	if(InterPoly(q, line, 4, intersect))
		return true;

	q[0] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, (float)TILE_SIZE*500);
	q[1] = Vec3f((float)-10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[2] = Vec3f((float)10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, WATER_LEVEL*2.0f);
	q[3] = Vec3f((float)10*sz.x*TILE_SIZE, (float)10*sz.y*TILE_SIZE, (float)TILE_SIZE*500);

	if(InterPoly(q, line, 4, intersect))
		return true;

	return false;
}

#if 0
void MapFrust(Vec3f* interTopLeft,
              Vec3f* interTopRight,
              Vec3f* interBottomLeft,
              Vec3f* interBottomRight)
{
    Camera* c = &g_cam;
    
    Vec3f campos = c->zoompos();
    Vec3f camside = c->m_strafe;
    Vec3f camup2 = c->up2();
    Vec3f viewdir = Normalize(c->m_view - c->m_pos);
    
    int32_t minx = 0;
    int32_t maxx = g_width;
    int32_t miny = 0;
    int32_t maxy = g_height;
    
    //Vec3f campos = c->m_pos;
    //Vec3f camside = c->m_strafe;
    //Vec3f camup2 = c->up2();
    //Vec3f viewdir = Normalize( c->m_view - c->m_pos );
    
    Vec3f topLeftRay = ScreenPerspRay(minx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
    Vec3f lineTopLeft[2];
    lineTopLeft[0] = campos;
    lineTopLeft[1] = campos + (topLeftRay * 1000000.0f);
    
    Vec3f topRightRay = ScreenPerspRay(maxx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
    Vec3f lineTopRight[2];
    lineTopRight[0] = campos;
    lineTopRight[1] = campos + (topRightRay * 1000000.0f);
    
    Vec3f bottomLeftRay = ScreenPerspRay(minx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
    Vec3f lineBottomLeft[2];
    lineBottomLeft[0] = campos;
    lineBottomLeft[1] = campos + (bottomLeftRay * 1000000.0f);
    
    Vec3f bottomRightRay = ScreenPerspRay(maxx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
    Vec3f lineBottomRight[2];
    lineBottomRight[0] = campos;
    lineBottomRight[1] = campos + (bottomRightRay * 1000000.0f);
    
    if(!FastMapIntersect(&g_hmap, lineTopLeft, interTopLeft))
        GetMapIntersection2(&g_hmap, lineTopLeft, interTopLeft);
    if(!FastMapIntersect(&g_hmap, lineTopRight, interTopRight))
        GetMapIntersection2(&g_hmap, lineTopRight, interTopRight);
    if(!FastMapIntersect(&g_hmap, lineBottomLeft, interBottomLeft))
        GetMapIntersection2(&g_hmap, lineBottomLeft, interBottomLeft);
    if(!FastMapIntersect(&g_hmap, lineBottomRight, interBottomRight))
        GetMapIntersection2(&g_hmap, lineBottomRight, interBottomRight);
}
#endif