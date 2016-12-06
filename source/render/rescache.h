

#ifndef RESCACHE_H
#define RESCACHE_H

#include "../platform.h"

extern FILE* g_rescachefp;

#define RESCACHE		"rescache/sf"
#define RESCACHEVER		1

extern bool g_rescacheread;

bool LoadResCache(const char* relative);
bool MakeResCache(const char* relative);
void AddResCache(const char* identstr);

#endif