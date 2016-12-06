













#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include "../platform.h"
#include "../sim/player.h"

class VisTile
{
public:
	//std::list<int16_t> uvis;
	//std::list<int16_t> bvis;
	unsigned char vis[PLAYERS];
	bool explored[PLAYERS];

	VisTile()
	{
		for(int32_t i=0; i<PLAYERS; i++)
		{
			explored[i] = false;
			//vis[i] = 0;
		}
	}
};

extern VisTile* g_vistile;

class Unit;
class Building;

void RemVis(Unit* u);
void AddVis(Unit* u);
void RemVis(Building* b);
void AddVis(Building* b);
void Explore(Unit* u);
void Explore(Building* b);
bool IsTileVis(int16_t py, int16_t tx, int16_t ty);
bool Explored(int16_t py, int16_t tx, int16_t ty);
void UpdVis();

#endif