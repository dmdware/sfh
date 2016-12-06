











#ifndef LABOURER_H
#define LABOURER_H

#include "unit.h"

#define LABSND_WORK		0
#define LABSND_SHOP		1
#define LABSND_REST		2
#define LAB_SOUNDS		3

extern int16_t g_labsnd[LAB_SOUNDS];

void UpdLab(Unit* u);
void UpdLab2(Unit* u);
void Evict(Building* b);
void Evict(Unit* u, bool silent);
void Disembark(Unit* op);
int32_t ConsumProp(int32_t satiety, int32_t incomerate, int32_t bprice);
int32_t ConsumProp(Unit* u, int32_t bprice);
void BestFood(Unit* u, int32_t* bestbi, int32_t* bestutil);

#endif
