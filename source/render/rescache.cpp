

#include "rescache.h"
#include "../utils.h"
#include "../gui/gui.h"
#include "../app/appmain.h"
#include "../debug.h"

FILE* g_rescachefp = NULL;
bool g_rescacheread = false;

bool LoadResCache(const char* relative)
{
	char fulldir[WF_MAX_PATH+1];
	FullPath(RESCACHE, fulldir);
	g_rescachefp = fopen(fulldir, "rb");
	if(!g_rescachefp)
		return false;
	int ver = -1;
	fread(&ver, sizeof(int), 1, g_rescachefp);
	if(ver != RESCACHEVER)
	{
		fclose(g_rescachefp);
		return false;
	}
	g_appmode = APPMODE_LOADING;
	GUI* gui = &g_gui;
	gui->hideall();
	gui->show("loading");
	g_rescacheread = true;
	return true;
}

bool MakeResCache(const char* relative)
{
	char fulldir[WF_MAX_PATH+1];
	FullPath("rescache/", fulldir);
	MakeDir(fulldir);
	FullPath(RESCACHE, fulldir);
	g_rescachefp = fopen(fulldir, "wb");
	int ver = RESCACHEVER;
	fwrite(&ver, sizeof(int), 1, g_rescachefp);
	g_rescacheread = false;
	return true;
}

void AddResCache(const char* identstr)
{
	int len = strlen(identstr)+1;

	fwrite(&len, sizeof(int), 1, g_rescachefp);
	fwrite(identstr, sizeof(char), len, g_rescachefp);
}