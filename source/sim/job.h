













#ifndef JOB_H
#define JOB_H

#include "labourer.h"

class Unit;
class Building;

//job opportunity
class JobOpp
{
public:
	int32_t jobutil;
	int32_t jobtype;
	int32_t target;
	int32_t target2;
	//float bestDistWage = -1;
	//float distWage;
	//bool fullquota;
	int32_t ctype;	//conduit type
	Vec2i goal;
	int32_t targtype;
	Unit* targu;
	Building* targb;
	int32_t owner;
};

bool FindJob(Unit* u);
void NewJob(int32_t jobtype, int32_t target, int32_t target2, int32_t cdtype);


#endif