












#ifndef DRAWQUEUE_H
#define DRAWQUEUE_H

#include "../platform.h"

extern int32_t rendtrees;

void DrawQueue(uint32_t renderdepthtex, uint32_t renderfb);
void DrawQueueDepth(uint32_t renderdepthtex, uint32_t renderfb);

#endif
