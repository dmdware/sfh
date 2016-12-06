










#ifndef AI_H
#define AI_H

#include "../sim/simdef.h"

class Player;
class Building;

#define AI_FRAMES	(CYCLE_FRAMES/30+1)
//#define AI_FRAMES	(SIM_FRAME_RATE*60/30+1)
//#define AI_FRAMES	CYCLE_FRAMES
//#define AI_FRAMES	1

void UpdAI();
void UpdAI(Player* p);
void AdjProd(Player* p, Building* b);
void AdjTrPrWg(Player* py, Unit* u);
void AdjPrWg(Player* p, Building* b);
void BuyProps(Player* p);
void AIBuild(Player* p);
void PlotCd(int32_t pi, int32_t ctype, Vec2i from, Vec2i to);
void AIManuf(Player* p);
void AdjCs(Player* p);
void ConnectCd(Player* p, int32_t ctype, Vec2i tplace);

#endif