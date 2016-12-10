











#ifndef SAVEMAP_H
#define SAVEMAP_H

#define MAP_TAG			{'S','F','M'}
#define MAP_VERSION		28

float ConvertHeight(unsigned char brightness);
void FreeMap();
bool LoadMap(const char* relative, bool live=false, bool afterdown=false);
bool SaveMap(const char* relative);

#endif
