













#include "saveeffect.h"
#include "../platform.h"
#include "../trigger/effect.h"
#include "../trigger/trigger.h"

void SaveEffect(Effect* e, FILE* fp)
{
	fwrite(e->name, sizeof(char), CONDITION_LEN+1, fp);
	fwrite(&e->type, sizeof(int32_t), 1, fp);
	int32_t trigger = TriggerID(e->trigger);
	fwrite(&trigger, sizeof(int32_t), 1, fp);
	int32_t tvlen = strlen(e->textval.c_str())+1;
	fwrite(&tvlen, sizeof(int32_t), 1, fp);
	fwrite(e->textval.c_str(), sizeof(char), tvlen, fp);
	fwrite(&e->imgdw, sizeof(int32_t), 1, fp);
	fwrite(&e->imgdh, sizeof(int32_t), 1, fp);
}

void ReadEffect(Effect* e, std::vector<int32_t>* triggerrefs, FILE* fp)
{
	fread(e->name, sizeof(char), CONDITION_LEN+1, fp);
	fread(&e->type, sizeof(int32_t), 1, fp);
	int32_t ref;
	fread(&ref, sizeof(int32_t), 1, fp);
	triggerrefs->push_back(ref);
	int32_t tvlen = 0;
	fread(&tvlen, sizeof(int32_t), 1, fp);
	char* tval = new char[tvlen];
	fread(tval, sizeof(char), tvlen, fp);
	e->textval = tval;
	delete [] tval;
	fread(&e->imgdw, sizeof(int32_t), 1, fp);
	fread(&e->imgdh, sizeof(int32_t), 1, fp);
}