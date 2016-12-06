













#ifndef SAVETRIGGER_H
#define SAVETRIGGER_H

#include "../platform.h"

class Trigger;

void SaveTrigger(Trigger* t, FILE* fp);
void ReadTrigger(Trigger* t, std::vector<int32_t>* triggerrefs, FILE* fp);
void SaveTriggers(FILE* fp);

#endif