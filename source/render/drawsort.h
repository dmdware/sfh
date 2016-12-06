












#ifndef DRAWSORT_H
#define DRAWSORT_H

#include "../platform.h"
#include "depthable.h"

extern std::list<Depthable> g_drawlist;
extern std::list<Depthable*> g_subdrawq;
bool CompareDepth(const Depthable* a, const Depthable* b);
void DrawSort(std::list<Depthable*>& drawlist, std::list<Depthable*>& drawqueue);
void DrawSort2(std::list<Depthable*>& drawlist, std::list<Depthable*>& drawqueue);
void DrawSort3(std::list<Depthable*>& drawlist, std::list<Depthable*>& drawqueue);

#endif
