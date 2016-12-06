













#ifndef CONDITION_H
#define CONDITION_H

#include "../platform.h"

#define CONDITION_NONE			0
#define CONDITION_ONLOADMAP		1
#define CONDITION_ONPLACEB		2
#define CONDITION_ONBUILDU		3
#define CONDITION_ONPLACEOTHERB	4
#define CONDITION_ONBUILDOTHERU	5
#define CONDITION_PRICESET		6
#define CONDITION_UNITCOUNT		7
#define CONDITION_CAPTUREB		8
#define CONDITION_MESSAGECLOSED	9
#define CONDITION_ROADINACCESSABILITY	10
#define CONDITION_ALLROADACCESSIBLE		11
#define CONDITION_ONFINISHEDB	12
#define CONDITION_ALLCONSTRUCTIONFINI	13

#define CONDITION_LEN		31	//not counting null terminator

class Condition
{
public:
	char name[CONDITION_LEN+1];
	int32_t type;
	bool met;
	int32_t target;
	float fval;
	int32_t count;
	int32_t owner;
    
	Condition();
};


void OnPriceSet(int32_t targ, float val);
void OnLoadMap();
void OnRoadInacc();
void OnAllRoadAc();
void OnCaptureB(int32_t target);
void OnBuildU(int32_t target);
void OnPlaceB(int32_t target);
void OnCloseMessage();
void OnFinishedB(int32_t target);
bool ConditionMet(int32_t type, int32_t target, float fval, int32_t count, int32_t owner);

void GMessage(const char* text);
void GMessageG(const char* tex, int32_t w, int32_t h);
void Click_CloseMessage();
void ReGUICondition();
void ReGUITrigger();
void ReGUIScript();
void Click_CloseUnique();
bool UniqueCondition(const char* name);


#endif