













#include "savetrigger.h"
#include "saveeffect.h"
#include "savecondition.h"
#include "../trigger/trigger.h"
#include "../trigger/effect.h"
#include "../trigger/condition.h"

void SaveTrigger(Trigger* t, FILE* fp)
{
	fwrite(t->name, sizeof(char), CONDITION_LEN+1, fp);
	fwrite(&t->enabled, sizeof(bool), 1, fp);
	int32_t numc = t->conditions.size();
	int32_t nume = t->effects.size();
	fwrite(&numc, sizeof(int32_t), 1, fp);
	fwrite(&nume, sizeof(int32_t), 1, fp);
    
	for(int32_t i=0; i<numc; i++)
		SaveCondition(&t->conditions[i], fp);
    
	for(int32_t i=0; i<nume; i++)
		SaveEffect(&t->effects[i], fp);
}

void ReadTrigger(Trigger* t, std::vector<int32_t>* triggerrefs, FILE* fp)
{
	fread(t->name, sizeof(char), CONDITION_LEN+1, fp);
	fread(&t->enabled, sizeof(bool), 1, fp);
	int32_t numc, nume;
	fread(&numc, sizeof(int32_t), 1, fp);
	fread(&nume, sizeof(int32_t), 1, fp);
    
	for(int32_t i=0; i<numc; i++)
	{
		Condition c;
		ReadCondition(&c, fp);
		t->conditions.push_back(c);
	}
    
	for(int32_t i=0; i<nume; i++)
	{
		Effect e;
		ReadEffect(&e, triggerrefs, fp);
		t->effects.push_back(e);
	}
}

void SaveTriggers(FILE* fp)
{
	int32_t numt = CountTriggers();
	fwrite(&numt, sizeof(int32_t), 1, fp);
    
	for(int32_t i=0; i<numt; i++)
		SaveTrigger(GetTrigger(i), fp);
    
	//Log("triggers save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;
}