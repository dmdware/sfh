



#ifndef APPMAIN_H
#define APPMAIN_H

#include "../window.h"
#include "../version.h"
#include "../language.h"

#if 0
#define VER_MAJ				0	//major version / sequal
#define VER_FS				1	//feature set
#define VER_ATT				0	//attempt
#define VER_STAT			VERSTAT_A		//status
#define VER_POST			0	//post-release number, or release candidate number, or hotfix?, local change only, should be compatible in multiplayer with other post-numbers
#define APPVERSION				MakeVer(VER_MAJ, VER_FS, VER_ATT, VER_STAT, VER_POST)
#else
#define APPVERSION				1
#endif

#define CONFIGFILE			"config.ini"
#define TITLE				"States, Firms, and Households"
//#define TITLE				(STRTABLE[STR_TITLE].rawstr().c_str())
#define SERVER_ADDR			"corpstates.com"

//#define TRUCK_DEBUG
//#define TRUCK_DEBUG2
//#define PATH_DEBUG

//#define LOCAL_TRANSX	//define this if you want only player-specific transaction hovering notifications to appear

class Heightmap;

#define APPMODE_LOGO		0
#define APPMODE_INTRO		1
#define APPMODE_LOADING		2
#define APPMODE_RELOADING	3
#define APPMODE_MENU		4
#define APPMODE_PLAY		5
#define APPMODE_PAUSE		6
#define APPMODE_EDITOR		7
#define APPMODE_JOINING		8
#define APPMODE_RENDERING	9
#define APPMODE_PRERENDADJFRAME	10

extern int32_t g_appmode;
extern int32_t g_reStage;

void LoadConfig();
void LoadName();
void WriteConfig();
void WriteName();
void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]);
void DrawSceneTeam(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]);
void DrawSceneDepth();
void DrawSceneElev();
void SkipLogo();
void Scroll(int32_t dx, int32_t dy);
int32_t HandleEvent(void *userdata, SDL_Event *e);
void Draw();

#endif
