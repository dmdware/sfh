













#ifndef SAVEEFFECT_H
#define SAVEEFFECT_H

#include "../platform.h"

class Effect;

void SaveEffect(Effect* e, FILE* fp);
void ReadEffect(Effect* e, std::vector<int32_t>* triggerrefs, FILE* fp);

#endif