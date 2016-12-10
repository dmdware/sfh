









#include "appmain.h"
#include "../gui/gui.h"
#include "../gui/keymap.h"
#include "../render/shader.h"
#include "appres.h"
#include "../gui/font.h"
#include "../texture.h"
#include "../math/frustum.h"
#include "../gui/layouts/appgui.h"
#include "../gui/gui.h"
#include "../debug.h"
#include "../render/heightmap.h"
#include "../sim/map.h"
#include "../sim/simtile.h"
#include "../math/camera.h"
#include "../window.h"
#include "../utils.h"
#include "../sim/simdef.h"
#include "../math/hmapmath.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/build.h"
#include "../sim/bltype.h"
#include "../render/foliage.h"
#include "../render/water.h"
#include "../sim/deposit.h"
#include "../sim/selection.h"
#include "../sim/player.h"
#include "../sim/order.h"
#include "../render/transaction.h"
#include "../path/collidertile.h"
#include "../path/pathdebug.h"
#include "../gui/layouts/playgui.h"
#include "../gui/widgets/spez/botpan.h"
#include "../texture.h"
#include "../script/script.h"
#include "../ai/ai.h"
#include "../net/lockstep.h"
#include "../sim/simflow.h"
#include "../sim/transport.h"
#include "../render/infoov.h"
#include "../sound/soundch.h"
#include "../net/netconn.h"
#include "../econ/demand.h"
#include "../save/savemap.h"
#include "../render/graph.h"
#include "../path/tilepath.h"
#include "../render/drawqueue.h"
#include "../sys/namegen.h"
#include "../net/client.h"
#include "../sys/unicode.h"
#include "../ustring.h"
#include "../gui/layouts/edgui.h"
#include "../language.h"
#include "../gui/widgets/spez/pygraphs.h"
#include "../path/fillbodies.h"
#include "../render/fogofwar.h"
#include "../render/screenshot.h"
#include "../mod/mod.h"
#include "../net/download.h"
#include "../save/edmap.h"
#include "segui.h"
#include "../render/shadow.h"
#include "../tool/compilebl.h"
#include "../tool/rendersprite.h"
#include "../render/rescache.h"

//int32_t g_appmode = APPMODE_LOADING;
int32_t g_appmode = APPMODE_LOGO;

//static int64_t g_lasttime = GetTicks();

#ifdef DEMO
static int64_t g_demostart = GetTicks();
#endif

void SkipLogo()
{
	g_appmode = APPMODE_LOADING;
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->hideall();
	gui->show("loading");
}

void UpdLogo()
{
	static int32_t stage = 0;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	if(stage < 60)
	{
		float a = (float)stage / 60.0f;
		gui->get("logo")->get("logo")->m_rgba[3] = a;
	}
	else if(stage < 120)
	{
		float a = 1.0f - (float)(stage-60) / 60.0f;
		gui->get("logo")->get("logo")->m_rgba[3] = a;
	}
	else
		SkipLogo();

	stage++;
}

int32_t g_restage = 0;
void UpdLoad()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
//InfoMess("asd","asd");
	switch(g_restage)
	{
	case 0:
		g_restage++;
		break;
	case 1:
		//RenderQueue();
		if(!Rend1()) g_restage++;
		break;
	case 2:
		if(!Load1Sprite()) g_restage++;
		//InfoMess("asd","asd");
		break;
	case 3:
		if(!Load1Texture())
		{
			g_spriteload.clear();
			g_texload.clear();
			g_lastLSp = -1;
			g_lastLTex = -1;
			g_lastRSp = -1;

			if(g_rescachefp)
				fclose(g_rescachefp);
			g_rescachefp = NULL;

			g_appmode = APPMODE_MENU;
			gui->hideall();
			gui->show("main");
			//g_appmode = APPMODE_PLAY;
			//Click_NewGame();
			//Click_OpenEd();
		}
		break;
	}
}

void UpdReload()
{
#if 0
	switch(g_reStage)
	{
	case 0:
		if(!Load1Texture())
		{
			g_appmode = APPMODE_MENU;
		}
		break;
	}
#else
	g_restage = 0;
	g_lastLTex = -1;
	//g_lastmodelload = -1;
	g_lastLSp = -1;
	g_lastRSp = -1;
	g_gui.freech();
	//FreeModels();
	FreeSprites();
	FreeTextures();
	BreakWin(TITLE);
	//ReloadTextures();
	MakeWin(TITLE);
#ifdef PLATFORM_MOBILE
	int32_t width = g_width;
	int32_t height = g_height;
	//halve for mobile small screen so it appears twice as big
	//g_width >>= 1;
	//g_height >>= 1;
#endif
	//ReloadModels();	//Important - VBO only possible after window GL context made.
	g_appmode = APPMODE_LOADING;
	//LoadFonts();	//already called in MakeWin
	//ReloadTextures();
	LoadSysRes();
	Queue();
	FillGUI();
	char exepath[WF_MAX_PATH+1];
	ExePath(exepath);
	Log("ExePath %s\r\n", exepath);
	
	GUI* gui = &g_gui;
	gui->hideall();
	gui->show("loading");
	
	glViewport(0,0,g_width,g_height);
	gui->reframe();
	//Resize(g_selres.width, g_selres.height);

	/*
	TODO
	Overhaul ui system
	Make scrollable everywhere
	Croppable pipeline 
	*/
	
#ifdef PLATFORM_MOBILE
	g_width = width;
	g_height = height;
#endif
#endif
}

void UpdSim()
{
	if(!CanStep())
		return;

	UpdTurn();

#ifdef FREEZE_DEBUG
	Log("updturn");
	
#endif

#if 0
	//if(g_netframe%100==0)
	//	Log("numlab: "<<CountU(UNIT_LABOURER));
	if(CountU(UNIT_LABOURER) <= 0 && !g_gameover)
	{
		g_gameover = true;
		char msg[128];
		sprintf(msg, "lasted %u simframes / %fs / %fm / %fh",
			g_simframe,
			(float)g_simframe / (float)SIM_FRAME_RATE,
			(float)g_simframe / (float)SIM_FRAME_RATE / 60.0f,
			(float)g_simframe / (float)SIM_FRAME_RATE / 60.0f / 60.0f);
		InfoMess("Game Over", msg);
#if 0
		EndSess();
		FreeMap();
		Player* py = &g_player[g_localP];
		GUI* gui = &g_gui;
		gui->hideall();
		gui->show("main");
		g_appmode = APPMODE_MENU;
		//g_quit = true;
#endif
	}
#endif

	//Can we continue the simulation?
	bool cansim = true;

	/*
	We need a separate counter for net frames and sim frames,
	because net frames might continue while sim frames are paused.
	*/

	//If anybody's paused, we can't continue.
	for(int32_t ci=0; ci<CLIENTS; ci++)
	{
		Client* c = &g_client[ci];

		if(!c->on)
			continue;

		if(c->speed == SPEED_PAUSE)
		{
			cansim = false;
			break;
		}
	}

	//2016/04/27 pauses while map downloading
	if(g_netmode == NETM_CLIENT && 
		g_downmap)
		cansim = false;

	//If we can't update the simulation,
	//execute any waiting net commands, and update net frame (lockstep).
	if(!cansim)
	{
		//UpdTurn();
		return;
	}

	/*
	Otherwise, we can update the simulation.
	First we execute any waiting net commands,
	and update net frame at the end of sim update.
	(Why is it important to only update netframe at the end?
	Because g_netframe tells you the current frame
	for important calculations done during updating sim
	and must only be incremented when that frame is done.)
	Edit: no, that is simframe. Why is netframe important
	for sim update? Sending packets dependent on netframe?
	*/
	//UpdTurn();

#if 1

	UpdVis();
	UpdJams();
	UpdDecay();

#ifdef FREEZE_DEBUG
	Log("updai");
	
#endif

	//only add lockstep commands for AI players if host or single
	//there must be only one authority
	if(g_netmode == NETM_HOST ||
		g_netmode == NETM_SINGLE)
		UpdAI();

#ifdef FREEZE_DEBUG
	Log("managetrips");
	
#endif

	ManageTrips();
	StartTimer(TIMER_UPDATEUNITS);

#ifdef FREEZE_DEBUG
	Log("updunits");
	
#endif

	UpdUnits();
	StopTimer(TIMER_UPDATEUNITS);
	StartTimer(TIMER_UPDATEBUILDINGS);

#ifdef FREEZE_DEBUG
	Log("updbls");
	
#endif

	UpdBls();
	StopTimer(TIMER_UPDATEBUILDINGS);

	Tally();
#endif

#ifdef FREEZE_DEBUG
	Log("/updsim");
	
#endif

	static int64_t tick = GetTicks();

	//must do this after UpdTurn
	g_simframe ++;

#if 0
	if(g_simframe == 4344000)
	{
		int64_t off = GetTicks() - tick;
		char msg[128];
		sprintf(msg, "atf4,344,000 \n min %f \n s %f", (float)(off)/(float)1000/(float)60, (float)(off)/(float)1000);
		InfoMess(msg, msg);
		//1.7m
	}
#endif

#if 1
	//autosave every sim month
	if(g_simframe % (SIM_FRAME_RATE * 60 * 60 * 24 * 30) == 0 && g_simframe > 0)
	{
		char fn[WF_MAX_PATH+1];
		sprintf(fn, "saves/%s autosave.sav", FileDateTime().c_str());
		SaveMap(fn);
	}
#endif
}

void UpdEd()
{
#if 0
	UpdateFPS();
#endif
}

void Update()
{
	//if(g_netmode != NETM_SINGLE)
	if(g_sock)
		UpdNet();

	if(g_appmode == APPMODE_LOGO)
		UpdLogo();
	//else if(g_appmode == APPMODE_INTRO)
	//	UpdateIntro();
	else if(g_appmode == APPMODE_LOADING)
		UpdLoad();
	else if(g_appmode == APPMODE_RELOADING)
		UpdReload();
	else if(g_appmode == APPMODE_PLAY)
		UpdSim();
	else if(g_appmode == APPMODE_EDITOR)
		UpdEd();
    else if(g_appmode == APPMODE_RENDERING ||
		g_appmode == APPMODE_PRERENDADJFRAME)
        UpdRend();

#ifdef DEMO
	if(GetTicks() - g_demostart > DEMOTIME)
		g_quit = true;
#endif
}

void MakeFBO(uint32_t* rendertex, uint32_t* renderrb, uint32_t* renderfb, uint32_t* renderdepthtex, int32_t w, int32_t h)
{
#if 0  //OpenGL 3.0 way
#if 1
	
	glGenFramebuffersEXT(1, renderfb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *renderfb);

	glGenTextures(1, rendertex);
	glBindTexture(GL_TEXTURE_2D, *rendertex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	CHECKGLERROR();
#if 0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	CheckGLError(__FILE__, __LINE__);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffersEXT(1, renderrb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, *renderrb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
	//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	CheckGLError(__FILE__, __LINE__);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, *rendertex, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, *renderrb);
	//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	CheckGLError(__FILE__, __LINE__);
#else
	//RGBA8 2D texture, 24 bit depth texture, 256x256
	glGenTextures(1, &g_rendertex);
	glBindTexture(GL_TEXTURE_2D, g_rendertex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//NULL means reserve texture memory, but texels are undefined
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	//-------------------------
	glGenFramebuffersEXT(1, &g_renderfb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
	//Attach 2D texture to this FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
	//-------------------------
	glGenRenderbuffersEXT(1, &g_renderrb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
	//-------------------------
	//Attach depth buffer to FBO
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
	//-------------------------
#endif
	
#elif defined(PLATFORM_MOBILE)	//for mobile
	
	glGenFramebuffers(1, renderfb);
	glBindFramebuffer(GL_FRAMEBUFFER, *renderfb);
	
	//NSLog(@"1 glGetError() = %d", glGetError());
	
	w = Max2Pow(w);
	h = Max2Pow(h);
	
	w = 512;
	h = 512;
	
	glGenTextures(1, rendertex);
	glBindTexture(GL_TEXTURE_2D, *rendertex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	//NSLog(@"2 glGetError() = %d", glGetError());
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *rendertex, 0);
	
	//NSLog(@"3 glGetError() = %d", glGetError());
	/*
	glGenRenderbuffers(1, renderrb);
	glBindRenderbuffer(GL_RENDERBUFFER, *renderrb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *renderrb);
	
	NSLog(@"4 glGetError() = %d", glGetError());
	
	glGenRenderbuffers(1, renderdepthtex);
	glBindRenderbuffer(GL_RENDERBUFFER, *renderdepthtex);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *renderdepthtex);
	
	NSLog(@"5 glGetError() = %d", glGetError());
	*/
	uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		char mess[128];
		sprintf(mess, "Couldn't create framebuffer for render, error: %u.", status);
		ErrMess("Error", mess);
		return;
	}
	
	//InfoMess("suc","fbosuc");
	
#else   //OpenGL 1.4 way

	// http://stackoverflow.com/questions/7324208/opengl-framebuffer-can-clear-it-but-cant-draw-to-it

	glGenFramebuffers(1, renderfb);
	glBindFramebuffer(GL_FRAMEBUFFER, *renderfb);

	glGenTextures(1, rendertex);
	glBindTexture(GL_TEXTURE_2D, *rendertex);
#ifndef PLATFORM_MOBILE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_OES, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//g_applog<<"gge1 "<<glGetError());

	glGenTextures(1, renderdepthtex);
	glBindTexture(GL_TEXTURE_2D, *renderdepthtex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
#ifndef PLATFORM_MOBILE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	//g_applog<<"gge2 "<<glGetError());
	//glDrawBuffer(GL_NONE); // No color buffer is drawn
	//glReadBuffer(GL_NONE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *rendertex, 0);
#ifndef PLATFORM_MOBILE
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *renderdepthtex, 0);
#endif

	//g_applog<<"gge3 "<<glGetError());

#ifndef PLATFORM_MOBILE
	//glDrawBuffers requires OpenGL ES 3.0
	//GL_DEPTH_ATTACHMENT_EXT
	GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
	glDrawBuffers(1, DrawBuffers);
#else
	GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
#endif
	
	//g_applog<<"gge4 "<<glGetError());

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		ErrMess("Error", "Couldn't create framebuffer for render.");
		return;
	}
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

#ifdef APPDEBUG
	g_applog<<__FILE__<<":"<<__LINE__<<"create check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<" ok="<<(int32_t)(GL_FRAMEBUFFER_COMPLETE));
	g_applog<<__FILE__<<":"<<__LINE__<<"create check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<" ok="<<(int32_t)(GL_FRAMEBUFFER_COMPLETE));
#endif
}

void DelFBO(uint32_t* rendertex, uint32_t* renderrb, uint32_t* renderfb, uint32_t* renderdepthtex)
{
#if 0
	CHECKGLERROR();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CHECKGLERROR();
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	CHECKGLERROR();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECKGLERROR();
	glDeleteFramebuffers(1, &g_renderfb);
	CHECKGLERROR();
	glDeleteRenderbuffers(1, &g_renderrb);
	CHECKGLERROR();
	glDeleteTextures(1, &g_rendertex);
	CHECKGLERROR();
	
#elif defined(PLATFORM_MOBILE)
	
	glDeleteTextures(1, rendertex);
	//glDeleteRenderbuffers(1, renderrb);
	glDeleteFramebuffers(1, renderfb);
	//glDeleteRenderbuffers(1, renderdepthtex);
	CHECKGLERROR();
	
#else
	
	//Delete resources
	glDeleteTextures(1, rendertex);
#ifndef PLATFORM_MOBILE
	glDeleteTextures(1, renderdepthtex);
#endif
#ifdef PLATFORM_MOBILE
	//corpd fix sped fix
	glDeleteRenderbuffers(1, renderrb);
#endif
#ifndef PLATFORM_MOBILE
	//Bind 0, which means render to back buffer, as a result, fb is unbound
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
	glDeleteFramebuffers(1, renderfb);
	CHECKGLERROR();
#endif
}

#if 0
uint32_t g_rendertex[4];
uint32_t g_renderdepthtex[4];
uint32_t g_renderrb[4];
uint32_t g_renderfb[4];
bool g_renderbs = false;
#endif

#if 0
void MakeFBO(int32_t sample, int32_t rendstage, int32_t w, int32_t h)
{
	//if(g_renderbs)
	//	return;

	//if(g_appmode == APPMODE_RENDERING || g_appmode == APPMODE_PRERENDADJFRAME)
	{
		g_renderbs = true;
#if 0   //OpenGL 3.0 way
#if 1
		glGenTextures(1, &g_rendertex);
		glBindTexture(GL_TEXTURE_2D, g_rendertex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		CHECKGLERROR();
#if 0
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		CHECKGLERROR();
		//glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenRenderbuffersEXT(1, &g_renderrb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		CHECKGLERROR();

		glGenFramebuffersEXT(1, &g_renderfb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		CHECKGLERROR();
#else
		//RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &g_rendertex);
		glBindTexture(GL_TEXTURE_2D, g_rendertex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//NULL means reserve texture memory, but texels are undefined
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//-------------------------
		glGenFramebuffersEXT(1, &g_renderfb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
		//Attach 2D texture to this FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
		//-------------------------
		glGenRenderbuffersEXT(1, &g_renderrb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
		//-------------------------
#endif
#elif defined( PLATFORM_GL14 )   //OpenGL 1.4 way

        glGenTextures(1, &g_rendertex[sample]);
        glBindTexture(GL_TEXTURE_2D, g_rendertex[sample]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //g_applog<<"gge1 "<<glGetError()<<std::endl;

        glGenTextures(1, &g_renderdepthtex[sample]);
        glBindTexture(GL_TEXTURE_2D, g_renderdepthtex[sample]);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //g_applog<<"gge2 "<<glGetError()<<std::endl;
        //glDrawBuffer(GL_NONE); // No color buffer is drawn
        //glReadBuffer(GL_NONE);

        glGenFramebuffers(1, &g_renderfb[sample]);
        glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[sample]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_rendertex[sample], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_renderdepthtex[sample], 0);

        //g_applog<<"gge3 "<<glGetError()<<std::endl;

        GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT_EXT};
        glDrawBuffers(1, DrawBuffers);

        //g_applog<<"gge4 "<<glGetError()<<std::endl;

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            ErrMess("Error", "Couldn't create framebuffer for render.");
            return;
        }
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
#elif defined( PLATFORM_OPENGLES20 )
		
		glGenTextures(1, &g_rendertex[sample]);
		glBindTexture(GL_TEXTURE_2D, g_rendertex[sample]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		//g_applog<<"gge1 "<<glGetError()<<std::endl;
		
		glGenTextures(1, &g_renderdepthtex[sample]);
		glBindTexture(GL_TEXTURE_2D, g_renderdepthtex[sample]);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		//g_applog<<"gge2 "<<glGetError()<<std::endl;
		//glDrawBuffer(GL_NONE); // No color buffer is drawn
		//glReadBuffer(GL_NONE);
		
		glGenFramebuffers(1, &g_renderfb[sample]);
		glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[sample]);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_rendertex[sample], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_renderdepthtex[sample], 0);
		
		//g_applog<<"gge3 "<<glGetError()<<std::endl;
		
		GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
		glDrawBuffers(1, DrawBuffers);
		
		//g_applog<<"gge4 "<<glGetError()<<std::endl;
		
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			ErrMess("Error", "Couldn't create framebuffer for render.");
			return;
		}

#endif

#ifdef DEBUG
		g_applog<<__FILE__<<":"<<__LINE__<<"create check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<" ok="<<(int32_t)(GL_FRAMEBUFFER_COMPLETE)<<std::endl;
		g_applog<<__FILE__<<":"<<__LINE__<<"create check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<" ok="<<(int32_t)(GL_FRAMEBUFFER_COMPLETE)<<std::endl;
#endif
	}
}

void DelFBO(int32_t sample)
{
#if 0
	CHECKGLERROR();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	CHECKGLERROR();
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	CHECKGLERROR();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	CHECKGLERROR();
	glDeleteFramebuffers(1, &g_renderfb);
	CHECKGLERROR();
	glDeleteRenderbuffers(1, &g_renderrb);
	CHECKGLERROR();
	glDeleteTextures(1, &g_rendertex);
	CHECKGLERROR();
#else
	//Delete resources
	glDeleteTextures(1, &g_rendertex[sample]);
	glDeleteTextures(1, &g_renderdepthtex[sample]);
	//glDeleteRenderbuffers(1, &g_renderrb);
	//Bind 0, which means render to back buffer, as a result, fb is unbound
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &g_renderfb[sample]);
#ifdef DEBUG
	CHECKGLERROR();
#endif
#endif

	g_renderbs = false;
}
#endif

void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#if 0
    if(g_appmode == RENDERING)
    {
        //SwapBuffers(g_hDC);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
        Log(__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
        CHECKGLERROR();
        glViewport(0, 0, g_width, g_height);
        CHECKGLERROR();
        glClearColor(1.0, 1.0, 1.0, 0.0);
        CHECKGLERROR();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECKGLERROR();
        Log(__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
        Log(__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::endl;
    }
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#if 1
	//if(g_projtype == PROJ_ORTHO)
		UseShadow(SHADER_MODEL, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	//else
	//	UseShadow(SHADER_MODELPERSP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    glUniform1i(g_shader[g_curS].slot[SSLOT_SHADOWMAP], 4);
    /*
    glActiveTextureARB(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_transparency);
    glUniform1iARB(g_shader[SHADER_OWNED].slot[SSLOT_TEXTURE1], 1);
    glActiveTextureARB(GL_TEXTURE0);*/
    DrawModelHolders();
#ifdef DEBUG
    CHECKGLERROR();
#endif
	//if(g_model2[0].m_on)
	//	g_model2[0].Render();
    EndS();
#endif

#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

#if 1
	if(g_projtype == PROJ_ORTHO)
		UseShadow(SHADER_MAP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	else
		UseShadow(SHADER_MAPPERSP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
#else
	UseShadow(SHADER_LIGHTTEST, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glUniformMatrix4fv(g_shader[SHADER_LIGHTTEST].slot[SSLOT_MVP], 1, GL_FALSE, g_lightmat.m_matrix);
#endif
    CHECKGLERROR();
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    //glBindTexture(GL_TEXTURE_2D, g_texture[0].texname);
    glUniform1i(g_shader[g_curS].slot[SSLOT_SHADOWMAP], 4);
	DrawTile();
	///Matrix offmat;
	///Vec3f offvec(-g_tilesize, 0, -g_tilesize);
	//offmat.translation((float*)&offvec);
	//glUniformMatrix4fv(g_shader[g_curS].slot[SSLOT_MODELMAT], 1, GL_FALSE, offmat.m_matrix);
	//DrawTile();
	//offvec = Vec3f(-g_tilesize, 0, 0);
	//offmat.translation((float*)&offvec);
	//glUniformMatrix4fv(g_shader[g_curS].slot[SSLOT_MODELMAT], 1, GL_FALSE, offmat.m_matrix);
	//DrawTile();
    CHECKGLERROR();
    DrawEdMap(&g_edmap, g_showsky);
    CHECKGLERROR();
    EndS();

	SDL_Delay(2);

#if 0
	glDisable(GL_DEPTH_TEST);
    UseShadow(SHADER_COLOR3D, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	Matrix mvp;
	mvp.set(projection.m_matrix);
	mvp.postmult(viewmat);
	glUniformMatrix4fv(g_shader[g_curS].slot[SSLOT_MVP], 1, GL_FALSE, mvp.m_matrix);
	glUniform4f(g_shader[g_curS].slot[SSLOT_COLOR], 0, 0, 1, 0.2f);
	VertexArray* va = &g_tileva[g_currincline];
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	//glDrawArrays(GL_LINE_LOOP, 0, va->numverts);
	glUniform4f(g_shader[g_curS].slot[SSLOT_COLOR], 1, 0, 0, 0.2f);
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	glDrawArrays(GL_POINTS, 0, va->numverts);
    EndS();
	glEnable(GL_DEPTH_TEST);
#endif

#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

#if 0
    if(g_appmode == RENDERING)
    {
        CHECKGLERROR();
        glFlush();
        CHECKGLERROR();
        //glFinish();
        CHECKGLERROR();
        SaveRender();
        CHECKGLERROR();
    }
#endif
}

void DrawSceneTeam(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{

#if 1
    UseShadow(SHADER_TEAM, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    glUniform1i(g_shader[g_curS].slot[SSLOT_SHADOWMAP], 4);
    /*
    glActiveTextureARB(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_transparency);
    glUniform1iARB(g_shader[SHADER_OWNED].slot[SSLOT_TEXTURE1], 1);
    glActiveTextureARB(GL_TEXTURE0);*/
    DrawModelHolders();
#ifdef DEBUG
    CHECKGLERROR();
#endif
    EndS();
#endif

    UseShadow(SHADER_TEAM, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    CHECKGLERROR();
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    //glBindTexture(GL_TEXTURE_2D, g_texture[0].texname);
    glUniform1i(g_shader[g_curS].slot[SSLOT_SHADOWMAP], 4);
	DrawTile();
    CHECKGLERROR();
    DrawEdMap(&g_edmap, g_showsky);
    CHECKGLERROR();
    EndS();
}


void DrawSceneDepth()
{
    //g_model[themodel].draw(0, Vec3f(0,0,0), 0);
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

    //DrawModelHoldersDepth();
    DrawModelHolders();

#ifdef DEBUG
    CHECKGLERROR();
#endif
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
    //if(g_model[0].m_on)
    //	g_model[0].draw(0, Vec3f(0,0,0), 0);

    //DrawEdMapDepth(&g_edmap, false);
    DrawEdMap(&g_edmap, false);/*
	for(int i=0; i<10; i++)
		for(int j=0; j<5; j++)
			g_model[themodel].draw(0, Vec3f(-5*180 + 180*i,0,-2.5f*90 + j*90), 0);*/

	DrawTile();
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
#ifdef DEBUG
    CHECKGLERROR();
#endif
}

void DrawSceneElev()
{
    //g_model[themodel].draw(0, Vec3f(0,0,0), 0);
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

    //DrawModelHoldersDepth();
    DrawModelHolders();

#ifdef DEBUG
    CHECKGLERROR();
#endif
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
    //if(g_model[0].m_on)
    //	g_model[0].draw(0, Vec3f(0,0,0), 0);

    //DrawEdMapDepth(&g_edmap, false);
    DrawEdMap(&g_edmap, false);/*
	for(int i=0; i<10; i++)
		for(int j=0; j<5; j++)
			g_model[themodel].draw(0, Vec3f(-5*180 + 180*i,0,-2.5f*90 + j*90), 0);*/

	DrawTile();
#ifdef DEBUG
	Log("draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	
#endif
#ifdef DEBUG
    CHECKGLERROR();
#endif
}

void Draw3()
{
		uint32_t rendertex;
		uint32_t renderrb;
		uint32_t renderfb;
		uint32_t renderdepthtex;

		//glEnable(GL_DEPTH_TEST);

		GUI* gui = &g_gui;

#if 1
		glClear(GL_DEPTH_BUFFER_BIT);
		
		GLint oldfb;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb);

	//	MakeFBO(&rendertex, &renderrb, &renderfb, &renderdepthtex, g_width, g_height);
	//	glBindFramebuffer(GL_FRAMEBUFFER, renderfb);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	glBindFramebuffer(GL_FRAMEBUFFER, oldfb);
#endif

		//EndS();
		//UseS(SHADER_PARTICLEBLENDDEPTH);
		//Shader* s = &g_shader[g_curS];
		//glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glDisable(GL_DEPTH_TEST);
		//glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		//glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
		//glBindFramebuffer(GL_FRAMEBUFFER, renderfb);

		//DrawQueueDepth(renderdepthtex, renderfb);
		DrawQueue(renderdepthtex, renderfb);

#if 0
		Vec3i cmpos(5*TILE_SIZE, 5*TILE_SIZE, TILE_RISE);
		Vec2i pixpos = CartToIso(cmpos) - g_scroll;

		int rendz;
		CartToDepth(cmpos, &rendz);

		SpList* spl100 = &g_splist[g_100sph];
		Sprite* sp100 = &g_sprite[spl100->sprites[0]];
		Texture* tex100depth = &g_texture[sp100->depthtexi];

		Texture* texfog0 = &g_texture[g_fog0];
		Texture* texfog1 = &g_texture[g_fog1];
#if 0
		DrawDepth(
			//DrawDeep(
			texfog0->texname,
			tex100depth->texname,
			renderdepthtex, renderfb,
			0,
			pixpos.x-32,
			pixpos.y-32,
			pixpos.x+32,
			pixpos.y+32,
			0, 0, 1, 1);
#endif
		EndS();
#if 0	
		UseS(SHADER_PARTICLEBLEND);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);

		DrawSphericalBlend(
			texfog0->texname,
			tex100depth->texname,
			renderdepthtex, renderfb,
			rendz,
			pixpos.x,
			pixpos.y,
			64, 0,
			0, 0, 1, 1 );

		EndS();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		CHECKGLERROR();

		glDisable(GL_DEPTH_TEST);
		Ortho(g_width, g_height, 1, 1, 1, 1);
		//DrawImage(rendertex, 0, 0, g_width, g_height, 0, 1, 1, 0, gui->m_crop);
		//DrawImage(rendertex, 0, 0, 2048, 2048, 0, 0, 1, 1, g_gui.m_crop);
		DrawImage(renderdepthtex, 100, 100, 200, 200, 0, 0, 1, 1, g_gui.m_crop);
		EndS();

		glFlush();
#endif
		//DelFBO(&rendertex, &renderrb, &renderfb, &renderdepthtex);
#endif
		EndS();

	CHECKGLERROR();

	//StartTimer(TIMER_DRAWGUI);
	//gui->frameupd();

	CHECKGLERROR();
	//gui->draw();
	StopTimer(TIMER_DRAWGUI);
	Ortho(g_width, g_height, 1, 1, 1, 1);

	//DrawImage(
	//		texfog0->texname, 0,0,100,100, 0,0,1,1,gui->m_crop);
	
	//DrawImage(
	//		tex100depth->texname,0,100,100,200, 0,0,1,1,gui->m_crop);
}

void Draw()
{
	if(g_quit)
		return;
	
	StartTimer(TIMER_DRAWSETUP);

	//glViewport(0,0,g_origwidth,g_origheight);
	//Resize(g_origwidth,g_origheight);
	GUI* gui = &g_gui;
	//gui->reframe();

	CHECKGLERROR();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECKGLERROR();

	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	if(g_appmode == APPMODE_PLAY || g_appmode == APPMODE_EDITOR)
	{
		
		uint32_t rendertex;
		uint32_t renderrb;
		uint32_t renderfb;
		uint32_t renderdepthtex;

		//MakeFBO(&rendertex, &renderrb, &renderfb, &renderdepthtex, g_width, g_height);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		DrawQueue(renderdepthtex, renderfb);
		//glDisable(GL_DEPTH_TEST);

		//DelFBO(&rendertex, &renderrb, &renderfb, &renderdepthtex);

		GUI* gui = &g_gui;
		
		Ortho(g_width, g_height, 1, 1, 1, 1);

		DrawOv();
		Ortho(g_width, g_height, 1, 1, 1, 1);
		DrawTransx();
		UpdParts();
		SortBillboards();
		DrawBillboards();

		
#if 1
		{
			UseS(SHADER_COLOR2D);
			Shader* s = &g_shader[g_curS];
			glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
			glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
			float graphbounds[4];
			graphbounds[0] = (float)g_width/2;
			graphbounds[1] = (float)30;
			graphbounds[2] = (float)g_width - 30;
			graphbounds[3] = (float)30 + 130;

			const float color[GRAPHS][4] = 
			{ 
				{1.0f,0.0f,0.0f,1.0f},
				{1.0f,1.0f,0.0f,1.0f},
				{1.0f,0.0f,1.0f,1.0f},
				{0.0f,0.0f,1.0f,1.0f}
			};

			for(uint8_t gi=0; gi<GRAPHS; ++gi)
			{
				DrawGraph(&g_graph[gi], graphbounds[0], graphbounds[1], graphbounds[2], graphbounds[3], 0, color[gi]);
			}

			EndS();

			Ortho(g_width, g_height, 1, 1, 1, 1);
			for(uint8_t gi=0; gi<GRAPHS; ++gi)
			{
				DrawShadowedText(MAINFONT16, graphbounds[0], graphbounds[1] + 16*gi, GRAPHNAME[gi], color[gi]);
			}
			EndS();
		}
#endif

#if 1
		{
			//draw protectionism graphs
			UseS(SHADER_COLOR2D);
			Shader* s = &g_shader[g_curS];
			glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
			glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
			float graphbounds[4];
			graphbounds[0] = (float)g_width/2;
			graphbounds[1] = (float)g_height - 60 - 130;
			graphbounds[2] = (float)g_width - 30;
			graphbounds[3] = (float)g_height - 60;
			float highest=0;
			for(int32_t i=0; i<PLAYERS; i++)
			//for(int32_t i=1; i<(FIRMSPERSTATE+1)*4; i+=(FIRMSPERSTATE+1))
			{
				for(std::list<float>::iterator pit=g_protecg[i].points.begin(); pit!=g_protecg[i].points.end(); pit++)
				{
					highest = fmax(highest,  *pit);
					highest = fmax(highest,  -*pit);
				}
			}
			for(int32_t i=0; i<PLAYERS; i++)
			//for(int32_t i=1; i<(FIRMSPERSTATE+1)*4; i+=(FIRMSPERSTATE+1))
			{
				float* c = g_player[i].color;
				//glUniform4f(s->slot[SSLOT_COLOR], c[0], c[1], c[2], 1);
				DrawGraph(&g_protecg[i], graphbounds[0], graphbounds[1], graphbounds[2], graphbounds[3], highest, c);
			}
			EndS();

		
			Ortho(g_width, g_height, 1, 1, 1, 1);
				DrawShadowedText(MAINFONT16, graphbounds[0], graphbounds[1], &STRTABLE[STR_NETWORTH], NULL);
			EndS();
		}
#endif

	}

	Ortho(g_width, g_height, 1, 1, 1, 1);
	if(g_debuglines && g_appmode == APPMODE_PLAY)
	{
		char fpsstr[22345];
		sprintf(fpsstr, "drw:%lf (%lf s/frame), upd:%lf (%lf s/frame), simfr:%s, netfr:%s", 
			g_instantdrawfps, 1.0/g_instantdrawfps, 
			g_instantupdfps, 1.0/g_instantupdfps, 
			ullform(g_simframe).c_str(),
			ullform(g_netframe).c_str());
		RichText fpsrstr(fpsstr);
		//fpsrstr = ParseTags(fpsrstr, NULL);
		DrawShadowedText(MAINFONT8, 0, (float)(g_height-MINIMAP_SIZE), &fpsrstr);
		CHECKGLERROR();
#if 01
		int32_t line = 1;
		for (std::list<NetConn>::iterator cit = g_conn.begin(); cit != g_conn.end(); cit++)
		{
			uint64_t curnetfr = 0;

			if (cit->client >= 0)
			{
				Client* c = &g_client[cit->client];
				curnetfr = c->curnetfr;
			}

			sprintf(fpsstr, "conn: m%d,h%d,c%d,i%d hsk%d,d%d nsa%hu,lr%hu curnetfr%llu",
				(int32_t)cit->ismatch, (int32_t)cit->isourhost, (int32_t)cit->isclient, (int32_t)cit->ishostinfo,
				(int32_t)cit->handshook, (int32_t)cit->disconnecting,
				(uint16_t)cit->nextsendack, (uint16_t)cit->lastrecvack,
				(uint64_t)curnetfr);

			for (std::list<OldPacket>::iterator pit = g_recv.begin(); pit != g_recv.end(); pit++)
			{
				if(strlen(fpsstr) > 1000)
					break;

				char add[540];

				if (!Same(&pit->addr, ((PacketHeader*)pit->buffer)->senddock, &cit->addr, cit->dock) &&
					!Same(&pit->addr, 0, &cit->addr, cit->dock))
					continue;

				sprintf(add, " rv%hu", (uint16_t)(((PacketHeader*)pit->buffer)->ack));
				strcat(fpsstr, add);

				if(strlen(fpsstr) > 1000)
					break;
			}

			for (std::list<OldPacket>::iterator pit = g_outgo.begin(); pit != g_outgo.end(); pit++)
			{
				if(strlen(fpsstr) > 1000)
					break;

				char add[540];

				if (!Same(&pit->addr, ((PacketHeader*)pit->buffer)->recvdock, &cit->addr, cit->dock) &&
					!Same(&pit->addr, 0, &cit->addr, cit->dock))
					continue;

				sprintf(add, " ot%hu", (uint16_t)(((PacketHeader*)pit->buffer)->ack));
				strcat(fpsstr, add);

				if(strlen(fpsstr) > 1000)
					break;
			}

			fpsrstr = RichText(fpsstr);
			DrawShadowedText(MAINFONT8, 0, (float)(g_height - MINIMAP_SIZE - 8 * line), &fpsrstr);


			line++;
		}

		fpsstr[0] = 0;

		if (g_next.canturn || g_netmode == NETM_HOST || g_netmode == NETM_SINGLE)
		{
			char add[32];
			sprintf(add, "turn%llu,sz%d,ct=%d ", (uint64_t)g_next.startnetfr, (int32_t)g_next.cmds.size(), (int32_t)g_next.canturn);
			strcat(fpsstr, add);
		}

		if (g_next2.canturn)
		{
			char add[32];
			sprintf(add, "turn%llu,sz%d ", (uint64_t)g_next2.startnetfr, (int32_t)g_next2.cmds.size(), (int32_t)g_next2.canturn);
			strcat(fpsstr, add);
		}

		for (std::list<NetTurn>::iterator tit = g_next3.begin(); tit != g_next3.end(); tit++)
		{
			char add[32];
			sprintf(add, "turn%llu,sz%d ", (uint64_t)tit->startnetfr, (int32_t)tit->cmds.size(), (int32_t)tit->canturn);
			strcat(fpsstr, add);
		}

		fpsrstr = RichText(fpsstr);
		DrawShadowedText(MAINFONT8, 0, g_height - MINIMAP_SIZE + 8, &fpsrstr);
#endif
	}

	gui->frameupd();
	gui->draw();

#ifdef DEMO
	{
		uint32_t msleft = DEMOTIME - (GetTicks() - g_demostart);
		char msg[128];
		sprintf(msg, "Demo time %d:%02d", msleft / 1000 / 60, (msleft % (1000 * 60)) / 1000);
		RichText msgrt(msg);
		float color[] = {0.5f,1.0f,0.5f,1.0f};
		DrawShadowedText(MAINFONT16, g_width - 130, g_height-16, &msgrt, color);
	}
#endif

	EndS();
	CHECKGLERROR();


	//SDL_GL_SwapWindow(g_window);

	//CheckNum("post draw");
}

bool OverMinimap()
{
	return false;
}

void Scroll(int32_t dx, int32_t dy)
{
	Vec2i old = g_scroll;
	bool moved = false;
	
	if(dx != 0 || dy != 0)
	{
		moved = true;
		g_scroll.x += dx;
		g_scroll.y += dy;
	}
	
	if(!moved)
		return;
	
	Vec2i onlyx = Vec2i(g_scroll.x,old.y);
	Vec2i onlyy = Vec2i(old.x,g_scroll.y);
	Vec3i temp;
	Vec3f ray;
	Vec3f point;
	
	IsoToCart(Vec2i(g_width,g_height)/2+g_scroll+Vec2i(rand()%10-4,rand()%10-4), &ray, &point);
	
	Vec3f line[2];
	Vec3f fint;
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);

#if 0
	Vec3f ray;
	Vec3f point;
	IsoToCart(g_mouse+g_scroll, &ray, &point);
	Vec3i intersect;
    Vec3f fint;
    Vec3f line[2];
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
	//if(!MapInter(&g_hmap, ray, point, &intersect))
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		return;
    intersect = Vec3i((int32_t)fint.x, (int32_t)fint.y, (int32_t)fint.z);
#endif

	if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		//if(!MapInter(&g_hmap, ray, point, &temp))
	{
		g_scroll = old;
		return;
		
		IsoToCart(Vec2i(g_width,g_height)/2+onlyx, &ray, &point);
		line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
		line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
		
		if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
			//if(!MapInter(&g_hmap, ray, point, &temp))
		{
			IsoToCart(Vec2i(g_width,g_height)/2+onlyy, &ray, &point);
			line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
			line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
			
			if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
				//if(!MapInter(&g_hmap, ray, point, &temp))
			{
				g_scroll = old;
				moved = false;
			}
			else
				g_scroll = onlyy;
		}
		else
			g_scroll = onlyx;
	}
	
	if(!moved)
		return;
	
	UpdSBl();
	
	IsoToCart(g_mouse+g_scroll, &ray, &point);
	//MapInter(&g_hmap, ray, point, &g_mouse3d);
	line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
	line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
	if(FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		g_mouse3d = Vec3i((int32_t)fint.x, (int32_t)fint.y, (int32_t)fint.z);
}

void Scroll()
{
	static double accum = 0;

	accum += g_drawfrinterval;
	//return;	//disable for now

	//if(accum < 1.0f/(CAMERA_SPEED+1))
	if(accum < 0.03f)
		return;

	GUI* gui = &g_gui;
	Widget* ingame = gui->get("ingame");

	if(!ingame->m_hidden)
		return;

	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	//if(g_mouseout)
	//	return;
	
	Vec2i scroll(0,0);

	//const Uint8 *keys = SDL_GetKeyboardState(NULL);
	//SDL_BUTTON_LEFT;
	if((/* !g_keyintercepted && */ (g_keys[SDL_SCANCODE_UP] || g_keys[SDL_SCANCODE_W])) || (g_mouse.y <= SCROLL_BORDER))
	{
		//scroll.y -= fmax(0, CAMERA_SPEED * accum);
		scroll.y -= (int32_t)(accum * CAMERA_SPEED * 10.0f);
	}

	if((/* !g_keyintercepted && */ (g_keys[SDL_SCANCODE_DOWN] || g_keys[SDL_SCANCODE_S])) || (g_mouse.y >= g_height-SCROLL_BORDER))
	{
		//scroll.y += fmax(0, CAMERA_SPEED * accum);
		scroll.y += (int32_t)(accum * CAMERA_SPEED * 10.0f);
	}

	if((/* !g_keyintercepted && */ (g_keys[SDL_SCANCODE_LEFT] || g_keys[SDL_SCANCODE_A])) || (g_mouse.x <= SCROLL_BORDER))
	{
		//scroll.x -= fmax(0, CAMERA_SPEED * accum);
		scroll.x -= (int32_t)(accum * CAMERA_SPEED * 10.0f);
	}

	if((/* !g_keyintercepted && */ (g_keys[SDL_SCANCODE_RIGHT] || g_keys[SDL_SCANCODE_D])) || (g_mouse.x >= g_width-SCROLL_BORDER))
	{
		//scroll.x += fmax(0, CAMERA_SPEED * accum);
		scroll.x += (int32_t)(accum * CAMERA_SPEED * 10.0f);
	}
	
	accum = 0;

	if(scroll.x == 0 && scroll.y == 0)
		return;
	
	Scroll(scroll.x, scroll.y);
}

void LoadConfig()
{
	EnumDisplay();
	
	if(g_resolution.size())
		g_selres = g_resolution[ 0 ];
	else
	{
		g_selres.width = 1280;
		g_selres.height = 640;
		
		
		Vec2i winsz;
		//SDL_GetWindowSize(g_window, &winsz.x, &winsz.y);
		SDL_GL_GetDrawableSize(g_window, &winsz.x, &winsz.y);
		
		int32_t w = imax(winsz.x, winsz.y);
		int32_t h = imin(winsz.x, winsz.y);
		
		float scale = 640.0f / (float)h;
		
		g_selres.width = w * scale;
		g_selres.height = h * scale;
	}
	
	Log("selres = %d,%d", g_selres.width, g_selres.height);
	
	for(std::vector<Resolution>::iterator rit=g_resolution.begin(); rit!=g_resolution.end(); rit++)
	{
#if 1
		//below acceptable height?
		if(g_selres.height < 480)
		{
			if(rit->height > g_selres.height &&
			   rit->width > rit->height)
			{
				g_selres = *rit;
			}
		}
		//already of acceptable height?
		else
#endif
		{
			//get smallest acceptable resolution
			if(rit->height < g_selres.height &&
			   rit->width > rit->height)
			{
				g_selres = *rit;
			}
			
			break;
		}
	}
	
	//g_selres.width = 568;
	//g_selres.height = 320;
	
	//g_selres.height = 568;
	//g_selres.width = 320;
	
	g_lang = "english";
	SwitchLang(g_lang.c_str());
	
	char cfgfull[WF_MAX_PATH+1];
	FullWritePath(CONFIGFILE, cfgfull);

	std::ifstream f(cfgfull);

	if(!f)
		return;

	std::string line;
	char keystr[128];
	char actstr[128];

	Player* py = &g_player[g_localP];

	while(!f.eof())
	{
		strcpy(keystr, "");
		strcpy(actstr, "");

		getline(f, line);

		if(line.length() > 127)
			continue;

		actstr[0] = 0;

		sscanf(line.c_str(), "%s %s", keystr, actstr);

		float valuef = StrToFloat(actstr);
		int32_t valuei = StrToInt(actstr);
		bool valueb = valuei ? true : false;

		if(stricmp(keystr, "fullscreen") == 0)					g_fullscreen = valueb;
		else if(stricmp(keystr, "client_width") == 0)			g_width = g_selres.width = g_origwidth = valuei;
		else if(stricmp(keystr, "client_height") == 0)			g_height = g_selres.height = g_origheight = valuei;
		else if(stricmp(keystr, "screen_bpp") == 0)				g_bpp = valuei;
		else if(stricmp(keystr, "volume") == 0)					SetVol(valuei);
		else if(stricmp(keystr, "language") == 0)				SwitchLang(actstr);
	}

	f.close();
	
#if 0
	const char* l = SteamApps()->GetCurrentGameLanguage();

	InfoMess(l,l);

	if(SwitchLang(l))
	{
		g_lang = l;
	}
	else
	{
		SwitchLang("english");
		g_lang = "english";
	}
	
	WriteConfig();
#endif

	//g_lang = LANG_ENG;
}

void LoadName()
{
	char cfgfull[WF_MAX_PATH+1];
	FullWritePath("name.txt", cfgfull);

	std::ifstream f(cfgfull);

	if(!f)
	{
		char name[GENNAMELEN];
		GenName(name);
		g_name = RichText(name);
		return;
	}

	std::string line;

	getline(f, line);

	uint32_t* ustr = ToUTF32((unsigned char*)line.c_str());
	g_name = RichText(UStr(ustr));
	delete [] ustr;

	f.close();
}

void WriteConfig()
{
	char cfgfull[WF_MAX_PATH+1];
	FullWritePath(CONFIGFILE, cfgfull);
	FILE* fp = fopen(cfgfull, "w");
	if(!fp)
		return;
	fprintf(fp, "fullscreen %d \r\n\r\n", g_fullscreen ? 1 : 0);
	fprintf(fp, "client_width %d \r\n\r\n", g_selres.width);
	fprintf(fp, "client_height %d \r\n\r\n", g_selres.height);
	fprintf(fp, "screen_bpp %d \r\n\r\n", g_bpp);
	fprintf(fp, "volume %d \r\n\r\n", g_volume);
	fprintf(fp, "language %s\r\n\r\n", g_lang.c_str());
	fclose(fp);
}

void WriteName()
{
	char cfgfull[WF_MAX_PATH+1];
	FullWritePath("name.txt", cfgfull);
	FILE* fp = fopen(cfgfull, "w");
	if(!fp)
		return;
	std::string name = g_name.rawstr();
	fprintf(fp, "%s", name.c_str());
	fclose(fp);
}

int32_t testfunc(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	InfoMess("os", "test");
	return 1;
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void SignalCallback(int32_t signum)
{
	//printf("Caught signal %d\n",signum);
	// Cleanup and hide up stuff here

	// Terminate program
	//g_quit = true;
	exit(0);	//force quit NOW
}

#ifdef PLATFORM_MOBILE
void CopyFile(const char* from, const char* to)
{
	FILE* fromfp = fopen(from, "rb");
	FILE* tofp = fopen(to, "wb");
	
	unsigned char byte = 0;

	int32_t i = 0;
	int32_t len = 0;

	fseek(fromfp, 0L, SEEK_END);
	len = ftell(fromfp);
	fseek(fromfp, 0L, SEEK_SET);
	
	//check remark about feof in CheckSum(...)
	//while(!feof(fromfp))
	for(i=0; i<len; i++)
	{
		fread(&byte, 1, 1, fromfp);
		fwrite(&byte, 1, 1, tofp);
	}
	
	fclose(fromfp);
	fclose(tofp);
}

void FirstRun()
{
	char checkfull[WF_MAX_PATH+1];
	char checkfile[128];
	sprintf(checkfile, "first%llu.txt", APPVERSION);
	FullWritePath(checkfile, checkfull);
	
	FILE* fp = fopen(checkfull, "rb");
	
	if(fp)
	{
		fclose(fp);
		return;
	}
	
	//else, this is the first run for this version
	fp = fopen(checkfull, "wb");
	fwrite("1", sizeof(char), 2, fp);
	fclose(fp);
	
	std::list<std::string> maps;
	std::list<std::string> saves;
	
	char mapswritefull[WF_MAX_PATH+1];
	char saveswritefull[WF_MAX_PATH+1];
	char screenwritefull[WF_MAX_PATH+1];
	FullWritePath("maps", mapswritefull);
	FullWritePath("saves", saveswritefull);
	FullWritePath("screenshot", screenwritefull);
	mkdir(mapswritefull, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir(saveswritefull, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir(screenwritefull, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	
	char mapsreadfull[WF_MAX_PATH+1];
	char savesreadfull[WF_MAX_PATH+1];
	FullPath("maps", mapsreadfull);
	FullPath("saves", savesreadfull);
	
	ListFiles(mapsreadfull, maps);
	ListFiles(savesreadfull, saves);
	
	for(std::list<Widget*>::iterator fit=maps.begin(); fit!=maps.end(); fit++)
	{
		char fullfrom[WF_MAX_PATH+1];
		char fullto[WF_MAX_PATH+1];
		sprintf(fullfrom, "%s/%s", mapsreadfull, fit->c_str());
		sprintf(fullto, "%s/%s", mapswritefull, fit->c_str());
		CopyFile(fullfrom, fullto);
	}
	
	for(std::list<Widget*>::iterator fit=saves.begin(); fit!=saves.end(); fit++)
	{
		char fullfrom[WF_MAX_PATH+1];
		char fullto[WF_MAX_PATH+1];
		sprintf(fullfrom, "%s/%s", savesreadfull, fit->c_str());
		sprintf(fullto, "%s/%s", saveswritefull, fit->c_str());
		CopyFile(fullfrom, fullto);
	}
}
#endif

void log(void* userdata, int category, SDL_LogPriority priority, const char* message) 
{ 
	Log("Log : %s", message); 
} 

void Init()
{
#ifdef PLATFORM_LINUX
	signal(SIGINT, SignalCallback);
#endif
	
#ifdef PLATFORM_IOS
	//SDL_SetEventFilter(HandleEvent, NULL);
#endif
	
#ifdef PLATFORM_MOBILE
	FirstRun();
#endif

#if 0
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		[director enableRetinaDisplay:NO];
		
		
	} else {
		
		[director enableRetinaDisplay:YES];
		
	}
#endif
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetPriority(SDL_LOG_CATEGORY_SYSTEM, SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetPriority(SDL_LOG_CATEGORY_AUDIO, SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetPriority(SDL_LOG_CATEGORY_VIDEO, SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetPriority(SDL_LOG_CATEGORY_INPUT, SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetOutputFunction(log, NULL);

	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

#if 0
	if(!SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1"))
		ErrMess("failed to set dpi hint", "failed to set dpi hint");
	else
		ErrMess("set to set dpi hint", "set to set dpi hint");
#endif
	
	if(SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		ErrMess("Error", msg);
	}

	
	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
	// Create a logger instance for Console Output
	Assimp::DefaultLogger::create("",severity, aiDefaultLogStream_STDOUT);
	// Create a logger instance for File Output (found in project folder or near .exe)
	Assimp::DefaultLogger::create("assimp_log.txt", severity, aiDefaultLogStream_FILE);
	// Now I am ready for logging my stuff
	Assimp::DefaultLogger::get()->info("this is my info-call");

	if(SDLNet_Init() == -1)
	{
		char msg[1280];
		sprintf(msg, "SDLNet_Init: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
	}

	SDL_version compile_version;
	const SDL_version *link_version=Mix_Linked_Version();
	SDL_MIXER_VERSION(&compile_version);
	printf("compiled with SDL_mixer version: %d.%d.%d\n",
		compile_version.major,
		compile_version.minor,
		compile_version.patch);
	printf("running with SDL_mixer version: %d.%d.%d\n",
		link_version->major,
		link_version->minor,
		link_version->patch);

	// load support for the OGG and MOD sample/music formats
	//int32_t flags=MIX_INIT_OGG|MIX_INIT_MOD|MIX_INIT_MP3;
	int32_t flags=MIX_INIT_OGG|MIX_INIT_MP3;
	int32_t initted=Mix_Init(flags);
	if( (initted & flags) != flags)
	{
		char msg[1280];
		sprintf(msg, "Mix_Init: Failed to init required ogg and mod support!\nMix_Init: %s", Mix_GetError());
		//ErrMess("Error", msg);
		// handle error
	}

	Mix_ChannelFinished(ChannelDone);

	// start SDL with audio support
	if(SDL_Init(SDL_INIT_AUDIO)==-1) {
		char msg[1280];
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		ErrMess("Error", msg);
		// handle error
		//exit(1);
	}
	// show 44.1KHz, signed 16bit, system byte order,
	//      stereo audio, using 1024 byte chunks
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
		char msg[1280];
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		ErrMess("Error", msg);
		// handle error
		//exit(2);
	}

	Mix_AllocateChannels(SOUND_CHANNELS);

	if(!g_applog)
		OpenLog("log.txt", APPVERSION);

//	if(TTF_Init() == -1)
	{
		//char msg[1280];
		//printf("TTF_Init: %s\n", TTF_GetError());
		//ErrMess("Error", msg);
	}

	srand((uint32_t)GetTicks());

	for(int32_t i=0; i<BL_TYPES; i++)
	{
		BlType* bt = &g_bltype[i];

		//if(!bt->on)
		//	continue;

		PYGRAPHSTR[PYGRAPH_BLFIN_BEG + i] = bt->name;
	}

	//TODO c-style inits, not constructors
	FreePys();	//must be called before loading name? because cl's deinited here
	LoadConfig();
	LoadName();
	//LoadMod();

	//g_os = ObjectScript::OS::create();
	//g_os->pushCFunction(testfunc);
	//g_os->setGlobal("testfunc");
	//os->eval("testfunc();");
	//os->eval("function require(){ /* if(relative == \"called.os\") */ { testfunc(); } }");
	char autoexecpath[WF_MAX_PATH+1];
	FullPath("scripts/autoexec.os", autoexecpath);
	//g_os->require(autoexecpath);
	//g_os->release();

	//EnumerateMaps();
	//EnumerateDisplay();
	MapKeys();

	//InitProfiles();
}

void Deinit()
{
	EndSess();
	FreeMap();

	g_gui.freech();

	WriteProfiles(-1, 0);
	BreakWin(TITLE);

	for(std::list<NetConn>::iterator cit = g_conn.begin(); cit != g_conn.end(); cit++)
	{
		Disconnect(&*cit);
	}

	const uint64_t start = GetTicks();
	//After quit, wait to send out quit packet to make sure host/clients recieve it.
	while (GetTicks() - start < QUIT_DELAY)
	{
		if(NetQuit())
			break;
		if(g_sock)
			UpdNet();
	}

	// Clean up

	if(g_sock)
	{
		SDLNet_UDP_Close(g_sock);
		g_sock = NULL;
	}

	FreeSounds();

	for(BmpFont* f=g_font; f<g_font+FONTS; ++f)
		f->close();

//	TTF_Quit();

	Mix_CloseAudio();

	// force a quit
	//while(Mix_Init(0))
	//	Mix_Quit();
	Mix_Quit();

	SDLNet_Quit();
	SDL_Quit();

#ifdef USESTEAM
	SteamAPI_Shutdown();
#endif
}

void CaptureZoom()
{
#if 0
#ifdef PLATFORM_MOBILE
	//if(g_zoomdrawframe)
	{
		//glFlush();
		//glFinish();
		
		Vec2i zoombox[2];
		
		zoombox[0].x = g_mouse.x - ZOOMBOX;
		zoombox[0].y = g_mouse.y - ZOOMBOX;
		zoombox[1].x = g_mouse.x + ZOOMBOX;
		zoombox[1].y = g_mouse.y + ZOOMBOX;
		
		zoombox[0].x = imax(0, zoombox[0].x);
		zoombox[0].y = imax(0, zoombox[0].y);
		zoombox[1].x = imin(g_width, zoombox[1].x);
		zoombox[1].y = imin(g_height, zoombox[1].y);
		
		LoadedTex zoompix;
		zoompix.sizex = zoombox[1].x - zoombox[0].x;
		zoompix.sizey = zoombox[1].y - zoombox[0].y;
		zoompix.channels = 4;
		zoompix.data = (unsigned char*)malloc( sizeof(unsigned char) * 4 * zoompix.sizex * zoompix.sizey );
		
		//glReadBuffer( GL_FRONT );
		glReadPixels(zoombox[0].x, g_height - zoombox[1].y, zoompix.sizex, zoompix.sizey, GL_RGBA, GL_UNSIGNED_BYTE, zoompix.data);
		//CreateTex(&zoompix, &zoomtex, true, false);
		
		glBindTexture(GL_TEXTURE_2D, g_zoomtex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, zoombox[0].x - (g_mouse.x - ZOOMBOX), zoombox[0].y - (g_mouse.y - ZOOMBOX), zoompix.sizex, zoompix.sizey, GL_RGBA, GL_UNSIGNED_BYTE, zoompix.data);
	}
#endif
#endif
}

#ifdef USEZOOM
void DrawZoom()
{
	DrawImage(g_zoomtex, g_mouse.x - ZOOMBOX*2, g_mouse.y - ZOOMBOX*2, g_mouse.x + ZOOMBOX*2, g_mouse.y + ZOOMBOX*2, 0,1,1,0, g_gui.m_crop);
}
#endif

int32_t HandleEvent(void *userdata, SDL_Event *e)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	InEv ie;
	ie.intercepted = false;
	ie.curst = CU_DEFAULT;
	Vec2i old;
	
	SDL_TouchFingerEvent* tfe = &e->tfinger;

	float inzoombox[4];
	float zoombox[4];
	float zoomtexco[4];
	
	switch(e->type)
	{
#ifdef PLATFORM_IOS
		case SDL_APP_TERMINATING:
			/* Terminate the app.
			 Shut everything down before returning from this function.
			 */
			g_quit = true;
			g_background = true;
			exit(0);
			return 0;
		case SDL_APP_LOWMEMORY:
			/* You will get this when your app is paused and iOS wants more memory.
			 Release as much memory as possible.
			 */
			exit(0);
			return 0;
		case SDL_APP_WILLENTERBACKGROUND:
			/* Prepare your app to go into the background.  Stop loops, etc.
			 This gets called when the user hits the home button, or gets a call.
			 */
			g_quit = true;
			g_background = true;
			exit(0);
			return 0;
		case SDL_APP_DIDENTERBACKGROUND:
			/* This will get called if the user accepted whatever sent your app to the background.
			 If the user got a phone call and canceled it, you'll instead get an SDL_APP_DIDENTERFOREGROUND event and restart your loops.
			 When you get this, you have 5 seconds to save all your state or the app will be terminated.
			 Your app is NOT active at this point.
			 */
			g_quit = true;
			g_background = true;
			exit(0);
			return 0;
		case SDL_APP_WILLENTERFOREGROUND:
			/* This call happens when your app is coming back to the foreground.
			 Restore all your state here.
			 */
			g_background = false;
			return 0;
		case SDL_APP_DIDENTERFOREGROUND:
			/* Restart your loops here.
			 Your app is interactive and getting CPU again.
			 */
			g_background = false;
			return 0;
#endif
		case SDL_WINDOWEVENT:
			switch (e->window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:

				//InfoMess("info", "resz");

				if(g_appmode == APPMODE_PRERENDADJFRAME)
				{
#ifdef DEBUG
					Log("to r");
					Log("rf"<<g_renderframe<<" rsz "<<g_deswidth<<","<<g_desheight<<std::endl;
#endif
					Resize(g_deswidth, g_desheight);
					glViewport(0,0,g_width, g_height);
					gui->reframe();
					//g_appmode = APPMODE_RENDERING;
				}
				else
				{
#ifdef DEBUG
					Log("rf"<<g_renderframe<<" rsz "<<e.window.data1<<","<<e.window.data2<<std::endl;
#endif
					Resize(e->window.data1, e->window.data2);
					glViewport(0,0,g_width, g_height);
					gui->reframe();
				}
				break;
			default:
				break;
			}
			break;
		case SDL_QUIT:
			g_quit = true;
			break;
		case SDL_KEYDOWN:
			ie.type = INEV_KEYDOWN;
			ie.key = e->key.keysym.sym;
			ie.scancode = e->key.keysym.scancode;
			
			//Handle copy
			if( e->key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
			{
				//SDL_SetClipboardText( inputText.c_str() );
				ie.type = INEV_COPY;
			}
			//Handle paste
			if( e->key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
			{
				//inputText = SDL_GetClipboardText();
				//renderText = true;
				ie.type = INEV_PASTE;
			}
			//Select all
			if( e->key.keysym.sym == SDLK_a && SDL_GetModState() & KMOD_CTRL )
			{
				//inputText = SDL_GetClipboardText();
				//renderText = true;
				ie.type = INEV_SELALL;
			}
			
			gui->inev(&ie);
			
			if(!ie.intercepted)
				g_keys[e->key.keysym.scancode] = true;
			
			g_keyintercepted = ie.intercepted;
			break;
		case SDL_KEYUP:
			ie.type = INEV_KEYUP;
			ie.key = e->key.keysym.sym;
			ie.scancode = e->key.keysym.scancode;
			
			gui->inev(&ie);
			
			if(!ie.intercepted)
				g_keys[e->key.keysym.scancode] = false;
			
			g_keyintercepted = ie.intercepted;
			break;
		case SDL_TEXTINPUT:
			//g_GUI.charin(e->text.text);	//UTF8
			ie.type = INEV_TEXTIN;
			ie.text = e->text.text;
			
			gui->inev(&ie);
			break;
#if 0
		case SDL_TEXTEDITING:
			//g_GUI.charin(e->text.text);	//UTF8
			ie.type = INEV_TEXTED;
			//ie.text = e->text.text;
			ie.text = e->edit.text;
			//Log("texed: "<<e->text.text<<" [0]="<<(uint32_t)e->text.text[0]);
			//
			ie.cursor = e->edit.start;
			ie.sellen = e->edit.length;
			
			
			gui->inev(&ie);
#if 0
			ie.intercepted = false;
			ie.type = INEV_TEXTIN;
			ie.text = e->text.text;
			
			gui->inev(&ie);
#endif
			break;
#endif
#if 0
		case SDL_TEXTINPUT:
			/* Add new text onto the end of our text */
			strcat(text, event.text.text);
#if 0
			ie.type = INEV_CHARIN;
			ie.key = wParam;
			ie.scancode = 0;
			
			gui->inev(&ie);
#endif
			break;
		case SDL_TEXTEDITING:
			/*
			 Update the composition text.
			 Update the cursor position.
			 Update the selection length (if any).
			 */
			composition = event.edit.text;
			cursor = event.edit.start;
			selection_len = event.edit.length;
			break;
#endif
			
#ifndef PLATFORM_MOBILE
			//else if(e->type == SDL_BUTTONDOWN)
			//{
			//}
		case SDL_MOUSEWHEEL:
			ie.type = INEV_MOUSEWHEEL;
			ie.amount = e->wheel.y;
			
			gui->inev(&ie);
			break;
		case SDL_MOUSEBUTTONDOWN:

#ifdef USEZOOM
#if 0
			inzoombox[0] = fmax(g_mouse.x - ZOOMBOX/2, 0);
			inzoombox[1] = fmax(g_mouse.y - ZOOMBOX/2, 0);
			inzoombox[2] = fmin(g_width-1, g_mouse.x + ZOOMBOX/2);
			inzoombox[3] = fmin(g_height-1, g_mouse.y + ZOOMBOX/2);

			zoombox[0] = fmax(g_mouse.x - ZOOMBOX, 0);
			zoombox[1] = fmax(g_mouse.y - ZOOMBOX, 0);
			zoombox[2] = fmin(g_width-1, g_mouse.x + ZOOMBOX);
			zoombox[3] = fmin(g_height-1, g_mouse.y + ZOOMBOX);

			zoomtexco[0] = inzoombox[0] / (float)g_width;
			zoomtexco[1] = inzoombox[1] / (float)g_height;
			zoomtexco[2] = inzoombox[2] / (float)g_width;
			zoomtexco[3] = inzoombox[3] / (float)g_height;
#endif
#endif

			switch (e->button.button)
		{
			case SDL_BUTTON_LEFT:
				g_mousekeys[MOUSE_LEFT] = true;
				g_moved = false;
				
				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_LEFT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				
#ifdef USEZOOM	//attempt at zoombox coordinate translation
#if 0
				if(e->button.x > g_mouse.x - ZOOMBOX &&
				   e->button.x < g_mouse.x + ZOOMBOX &&
				   e->button.y > g_mouse.y - ZOOMBOX &&
				   e->button.y < g_mouse.y + ZOOMBOX)
				{
					//ie.x = (e->button.x - g_mouse.x) * 2 + g_mouse.x;
					//ie.y = (e->button.y - g_mouse.y) * 2 + g_mouse.y;
					ie.x = (e->button.x/2 - g_mouse.x) * 2 + g_mouse.x;
					ie.y = (e->button.y/2 - g_mouse.y) * 2 + g_mouse.y;
					
					g_mouse.x = ie.x;
					g_mouse.y = ie.y;
				}
#endif
#endif
				
				gui->inev(&ie);
				
				g_keyintercepted = ie.intercepted;
				break;
			case SDL_BUTTON_RIGHT:
				g_mousekeys[MOUSE_RIGHT] = true;
				
				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_RIGHT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				
				gui->inev(&ie);
				break;
			case SDL_BUTTON_MIDDLE:
				g_mousekeys[MOUSE_MIDDLE] = true;
				
				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_MIDDLE;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				
				gui->inev(&ie);
				break;
		}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (e->button.button)
		{
				case SDL_BUTTON_LEFT:
					g_mousekeys[MOUSE_LEFT] = false;
					
					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_LEFT;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;
					
					gui->inev(&ie);
					break;
				case SDL_BUTTON_RIGHT:
					g_mousekeys[MOUSE_RIGHT] = false;
					
					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_RIGHT;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;
					
					gui->inev(&ie);
					break;
				case SDL_BUTTON_MIDDLE:
					g_mousekeys[MOUSE_MIDDLE] = false;
					
					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_MIDDLE;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;
					
					gui->inev(&ie);
					break;
			}
			break;
		case SDL_MOUSEMOTION:
			//g_mouse.x = e->motion.x;
			//g_mouse.y = e->motion.y;
			
			if(g_mouseout)
			{
				//TrackMouse();
				g_mouseout = false;
			}
			
			old = g_mouse;
			
			if(MousePosition())
			{
				g_moved = true;
				
				ie.type = INEV_MOUSEMOVE;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				ie.dx = g_mouse.x - old.x;
				ie.dy = g_mouse.y - old.y;
				
#ifdef PLATFORM_IOS
#if 0
				ie.x *= 2;
				ie.y *= 2;
				ie.dx *= 2;
				ie.dy *= 2;
#endif
#endif
				
				gui->inev(&ie);
				
				g_curst = ie.curst;
			}
			break;
#else	//def PLATFORM_MOBILE
		case SDL_FINGERMOTION:
			
			if(g_mouseout)
			{
				//TrackMouse();
				g_mouseout = false;
			}
			
			old = g_mouse;
			
			//if(MousePosition())
			{
				g_moved = true;
				
				ie.type = INEV_MOUSEMOVE;
				ie.x = tfe->x * g_width;
				ie.y = tfe->y * g_height;
				ie.dx = tfe->dx * g_width;
				ie.dy = tfe->dy * g_height;
				g_mouse.x = tfe->x * g_width;
				g_mouse.y = tfe->y * g_height;
				
				gui->inev(&ie);
				
				g_curst = ie.curst;
			}
			break;
		case SDL_FINGERDOWN:
		{
			g_mousekeys[MOUSE_LEFT] = true;
			g_moved = false;
			
			ie.type = INEV_MOUSEDOWN;
			ie.key = MOUSE_LEFT;
			ie.amount = 1;
			ie.x = tfe->x * g_width;
			ie.y = tfe->y * g_height;
			ie.dx = ie.x - g_mouse.x;
			ie.dy = ie.y - g_mouse.y;
			g_mouse.x = tfe->x * g_width;
			g_mouse.y = tfe->y * g_height;
			
#if 0
			ie.type = INEV_MOUSEMOVE;
			gui->inev(&ie);
			ie.intercepted = false;
			ie.interceptor = NULL;
			ie.type = INEV_MOUSEDOWN;
#endif
			gui->inev(&ie);
			
			g_keyintercepted = ie.intercepted;
		}
			break;
		case SDL_FINGERUP:
		{
			
			g_mousekeys[MOUSE_LEFT] = false;
			
			ie.type = INEV_MOUSEUP;
			ie.key = MOUSE_LEFT;
			ie.amount = 1;
			ie.x = tfe->x * g_width;
			ie.y = tfe->y * g_height;
			ie.dx = ie.x - g_mouse.x;
			ie.dy = ie.y - g_mouse.y;
			g_mouse.x = tfe->x * g_width;
			g_mouse.y = tfe->y * g_height;
			
#if 0
			ie.type = INEV_MOUSEMOVE;
			gui->inev(&ie);
			ie.intercepted = false;
			ie.interceptor = NULL;
			ie.type = INEV_MOUSEUP;
#endif
			gui->inev(&ie);
		}
		break;
#endif
	}

	return 0;
}

void EventLoop()
{
	
#ifdef PLATFORM_MOBILE
	int32_t width = g_width;
	int32_t height = g_height;
	//g_width >>= 1;
	//g_height >>= 1;
#endif
	
#if 0
	key->keysym.scancode
		SDLMod  e->key.keysym.mod
		key->keysym.unicode

		if( mod & KMOD_NUM ) printf( "NUMLOCK " );
	if( mod & KMOD_CAPS ) printf( "CAPSLOCK " );
	if( mod & KMOD_LCTRL ) printf( "LCTRL " );
	if( mod & KMOD_RCTRL ) printf( "RCTRL " );
	if( mod & KMOD_RSHIFT ) printf( "RSHIFT " );
	if( mod & KMOD_LSHIFT ) printf( "LSHIFT " );
	if( mod & KMOD_RALT ) printf( "RALT " );
	if( mod & KMOD_LALT ) printf( "LALT " );
	if( mod & KMOD_CTRL ) printf( "CTRL " );
	if( mod & KMOD_SHIFT ) printf( "SHIFT " );
	if( mod & KMOD_ALT ) printf( "ALT " );
#endif

	//SDL_EnableUNICODE(SDL_ENABLE);

	while (!g_quit)
	{
		StartTimer(TIMER_FRAME);
		StartTimer(TIMER_EVENT);

//#ifndef PLATFORM_IOS
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			HandleEvent(NULL, &e);
		}
//#endif

		StopTimer(TIMER_EVENT);
		
		if(g_quit)
			break;
		
		CHECKERR();

#if 1
		//if ((g_appmode == APPMODE_LOADING || g_appmode == APPMODE_RELOADING) || true /* DrawNextFrame(DRAW_FRAME_RATE) */ )
		if ( !g_background &&
			( (g_appmode == APPMODE_LOADING || g_appmode == APPMODE_RELOADING) || DrawNextFrame() ) )
#endif
		{
			StartTimer(TIMER_DRAW);

#ifdef DEBUGLOG
			Log("main "<<__FILE__<<" "<<__LINE__);
			
#endif
			CalcDrawRate();

			CHECKGLERROR();

#ifdef DEBUGLOG
			Log("main "<<__FILE__<<" "<<__LINE__);
			
#endif
			
#ifndef USEZOOM
			Draw();
			SDL_GL_SwapWindow(g_window);

#else	//attempt not working

			GLint deffbo;
			//GL_FRAMEBUFFER_BINDING_OES
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &deffbo);
			
			uint32_t offtex;
			uint32_t offrb;
			uint32_t offfb;
			uint32_t offdepth;

			int32_t oldw = g_width;
			int32_t oldh = g_height;

			int32_t neww = Max2Pow(oldw);
			int32_t newh = Max2Pow(oldh);

			//g_width = neww;
			//g_height = newh;

			MakeFBO(&offtex, &offrb, &offfb, &offdepth, g_width, g_height);
			glBindFramebuffer(GL_FRAMEBUFFER, offfb);
			//MakeFBO(0, 0, neww, newh);
			//glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
			glViewport(0,0,g_width,g_height);

			Draw();
			glFlush();

			//SaveScreenshot();

			uint32_t screentex;

			{
				LoadedTex screenshot;
				screenshot.channels = 3;
				screenshot.sizex = g_width;
				screenshot.sizey = g_height;
				screenshot.data = (unsigned char*)malloc( sizeof(unsigned char) * g_width * g_height * 3 );

				if(!screenshot.data)
				{
					OUTOFMEM();
					return;
				}

				//memset(screenshot.data, 0, g_width * g_height * 3);
	
				glPixelStorei(GL_UNPACK_ALIGNMENT,1);
				glPixelStorei(GL_PACK_ALIGNMENT, 1);

				CHECKGLERROR();
				glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screenshot.data);
				CHECKGLERROR();

				FlipImage(&screenshot);

				CreateTex(&screenshot, &screentex, true, false);
			}
			
			//glBindFramebuffer(GL_FRAMEBUFFER, deffbo);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0,0,g_width,g_height);
			
			float inzoombox[4];
			inzoombox[0] = fmax(g_mouse.x - ZOOMBOX/2, 0);
			inzoombox[1] = fmax(g_mouse.y - ZOOMBOX/2, 0);
			inzoombox[2] = fmin(oldw-1, g_mouse.x + ZOOMBOX/2);
			inzoombox[3] = fmin(oldh-1, g_mouse.y + ZOOMBOX/2);

			float zoombox[4];
			zoombox[0] = fmax(g_mouse.x - ZOOMBOX, 0);
			zoombox[1] = fmax(g_mouse.y - ZOOMBOX, 0);
			zoombox[2] = fmin(oldw-1, g_mouse.x + ZOOMBOX);
			zoombox[3] = fmin(oldh-1, g_mouse.y + ZOOMBOX);

			float zoomtexco[4];
			zoomtexco[0] = inzoombox[0] / (float)neww;
			zoomtexco[1] = inzoombox[1] / (float)newh;
			zoomtexco[2] = inzoombox[2] / (float)neww;
			zoomtexco[3] = inzoombox[3] / (float)newh;
			
			//g_width = oldw;
			//g_height = oldh;

			//Draw();

			DrawImage(screentex, 0, 0, g_width, g_height, 0, 0, 1, 1, g_gui.m_crop);
			DrawImage(screentex, zoombox[0], zoombox[1], zoombox[2], zoombox[3], zoomtexco[0], zoomtexco[1], zoomtexco[2], zoomtexco[3], g_gui.m_crop);
			DrawImage(screentex, zoombox[0], zoombox[1], zoombox[2], zoombox[3], zoomtexco[0], zoomtexco[3], zoomtexco[2], zoomtexco[1], g_gui.m_crop);
			//DrawImage(offtex, 0, 0, 100, 100, 0, 0, 1, 1, g_gui.m_crop);
			//DrawImage(offtex, 100, 0, 200, 100, 0, 1, 1, 0, g_gui.m_crop);

			//Draw();
			glFlush();

			DelFBO(&offtex, &offrb, &offfb, &offdepth);
			glDeleteTextures(1, &screentex);
			//DelFBO(0);
			SDL_GL_SwapWindow(g_window);
#endif
			
			
//#ifdef PLATFORM_MOBILE
#if 0
			/*
			GLint deffbo;
			//GL_FRAMEBUFFER_BINDING_OES
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &deffbo);
			
			uint32_t offtex;
			uint32_t offrb;
			uint32_t offfb;
			uint32_t offdepth;
			
			MakeFBO(&offtex, &offrb, &offfb, &offdepth, g_width, g_height);
			glBindFramebuffer(GL_FRAMEBUFFER, offfb);
			 */

			//g_zoomdrawframe = false;
			Draw();
			SDL_GL_SwapWindow(g_window);
#if 0
			if(!g_zoomdrawframe)
			{
				SDL_GL_SwapWindow(g_window);
				CaptureZoom();
			}
			
			//glBindFramebuffer(GL_FRAMEBUFFER, deffbo);
			
			//int32_t highw = Max2Pow(g_width);
			//int32_t highh = Max2Pow(g_height);
			
			//DrawImage(offtex, 0, 0, g_width-1, g_height-1, 0, 1, 1, 0, g_gui.m_crop);
			//DrawImage(offtex, 0, 0, g_width-1, g_height-1, 0, (float)g_width/(float)highw, (float)g_height/(float)highh, 0, g_gui.m_crop);
			else
			{
				DrawZoom();
				SDL_GL_SwapWindow(g_window);
			}
#elif 0
			if(g_zoomdrawframe)
			{
				DrawZoom();
				SDL_GL_SwapWindow(g_window);
				CaptureZoom();
			}
#endif
			
			g_zoomdrawframe = !g_zoomdrawframe;

			//DelFBO(&offtex, &offrb, &offfb, &offdepth);
			//glBindFramebuffer(GL_FRAMEBUFFER, deffbo);
			
			
			
#endif	//PLATFORM_MOBILE
			
			CHECKGLERROR();

			if(g_appmode == APPMODE_PLAY || g_appmode == APPMODE_EDITOR)
			{
#ifdef DEBUGLOG
				Log("main "<<__FILE__<<" "<<__LINE__);
				
#endif
				Scroll();
#ifdef DEBUGLOG
				Log("main "<<__FILE__<<" "<<__LINE__);
				
#endif
				UpdResTicker();
			}

			StopTimer(TIMER_DRAW);
		}

		//if((g_appmode == APPMODE_LOADING || g_appmode == APPMODE_RELOADING) || true /* UpdNextFrame(SIM_FRAME_RATE) */ )
		if((g_appmode == APPMODE_LOADING || g_appmode == APPMODE_RELOADING) || UpdNextFrame() )
		{
			//Try to avoid querying the timer, to maximize throughput
			static uint32_t lastupds = 1;
			static uint64_t lasttick = GetTicks();

			//Trick to allow running millions of times per frame by avoiding running the check for input events query
			while(true)
			{
				bool stopall = false;

				for(uint32_t upd=0; upd<lastupds; upd++)
				{

					StartTimer(TIMER_UPDATE);

#ifdef DEBUGLOG
					Log("main "<<__FILE__<<" "<<__LINE__);
					
#endif

					//TODO make even faster by reducing calls to GetTicks() (gettimeofday()) but only calling it every 100th or 1000th time and getting fps that way
					CalcUpdRate();
					Update();

					StopTimer(TIMER_UPDATE);
				}

				uint64_t now = GetTicks();
				uint64_t tickspassed = now - lasttick;

				if(tickspassed < 1000 / SIM_FRAME_RATE)
				{
					lastupds = (uint32_t)(lastupds * 1.5f);
				}
				else
				{
					lastupds = imax(1, (uint32_t)(lastupds * 0.9f));
				}

				if(!UpdNextFrame())
					goto updtime;

				if(g_speed == SPEED_FAST)
					goto updtime;

				continue;

updtime:
				lasttick = now;
				break;
			}
		}
		
		StopTimer(TIMER_FRAME);
	}
	
#ifdef PLATFORM_MOBILE
	g_width = width;
	g_height = height;
#endif
}

#ifdef USESTEAM
//-----------------------------------------------------------------------------
// Purpose: callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook( int32_t nSeverity, const char *pchDebugText )
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
#ifdef PLATFORM_WIN
	::OutputDebugString( pchDebugText );
#endif

	if(!g_applog)
		OpenLog("log.txt", APPVERSION);

	Log(pchDebugText);
	

	if ( nSeverity >= 1 )
	{
		// place to set a breakpoint for catching API errors
		int32_t x = 3;
		x = x;
	}
}
#endif

void Main()
{
	//*((int32_t*)0) = 0;

#ifdef USESTEAM

	if ( SteamAPI_RestartAppIfNecessary( k_uAppIdInvalid ) )
	{
		// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
		// local Steam client and also launches this game again.

		// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
		// removed steam_appid.txt from the game depot.

		return;
	}

	// Init Steam CEG
	if ( !Steamworks_InitCEGLibrary() )
	{
#ifdef PLATFORM_WIN
		OutputDebugString( "Steamworks_InitCEGLibrary() failed\n" );
#endif
		ErrMess( "Fatal Error", "Steam must be running to play this game (InitDrmLibrary() failed).\n" );
		return;
	}

	// Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
	// You don't necessarily have to though if you write your code to check whether all the Steam
	// interfaces are NULL before using them and provide alternate paths when they are unavailable.
	//
	// This will also load the in-game steam overlay dll into your process.  That dll is normally
	// injected by steam when it launches games, but by calling this you cause it to always load,
	// even when not launched via steam.
	if ( !SteamAPI_Init() )
	{
#ifdef PLATFORM_WIN
		OutputDebugString( "SteamAPI_Init() failed\n" );
#endif
		ErrMess( "Fatal Error", "Steam must be running to play this game (SteamAPI_Init() failed).\n" );
		return;
	}

	// set our debug handler
	SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

#endif

	Init();
		Log("1");
	
	MakeWin(TITLE);
	
#ifdef PLATFORM_MOBILE
	int32_t width = g_width;
	int32_t height = g_height;
	//halve for mobile small screen so it appears twice as big
	//g_width >>= 1;
	//g_height >>= 1;
#endif
	
	Log("11");
	
	SDL_ShowCursor(false);
	LoadSysRes();
	Queue();
	FillGUI();

	if(!LoadResCache(RESCACHE))
		MakeResCache(RESCACHE);
	
#ifdef PLATFORM_MOBILE
	g_width = width;
	g_height = height;
#endif

	EventLoop();
	
	Deinit();
	SDL_ShowCursor(true);
}

#ifdef PLATFORM_WIN
int32_t WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int32_t nCmdShow)
#else
int32_t main(int32_t argc, char* argv[])
#endif
{
#ifdef PLATFORM_IOS
	NSLog(@"running");
#endif
	
#if 0
	FILE* fp = fopen("log.txt", "rb");

	if(!fp)
		InfoMess("f!","f1");
	else
		InfoMess("s","s");

	int cb = 0;
	while(true)
	{
		char c1 = 0;
		
		++cb;
		fread(&c1, 1, 1, fp);

		if(c1 == 't')
		{
			fread(&c1, 1, 1, fp);

			if(c1 == 'm')
			{
				
				fread(&c1, 1, 1, fp);
				
				if(c1 == ' ')
				{
					fread(&c1, 1, 1, fp);
					
					if(c1 == '1')
					{
						fread(&c1, 1, 1, fp);

						if(c1 == '4')
						{
							InfoMess("14","14");
						}
					}
				}

			}

		}

		if(feof(fp))
			break;
	}

	fclose(fp);
	
	char m[123];
	sprintf(m, "r%d", cb);
	InfoMess(m,m);

	cb=0;
	fp = fopen("log.txt", "rb");

	
	if(!fp)
		InfoMess("f!","f12");

	//int cb = 0;
	while(true)
	{
		char c1 = 0;

		++cb;

		fread(&c1, 1, 1, fp);

		if(c1 == 't')
		{
			fread(&c1, 1, 1, fp);

			if(c1 == 'm')
			{
				
				fread(&c1, 1, 1, fp);
				
				if(c1 == ' ')
				{
					fread(&c1, 1, 1, fp);
					
					if(c1 == '1')
					{
						fread(&c1, 1, 1, fp);

						if(c1 == '3')
						{
							InfoMess("13","13");
						}
					}
				}

			}

		}

		if(feof(fp))
			break;
	}

	sprintf(m, "r%d", cb);
	InfoMess(m,m);
	fclose(fp);

#endif

	//g_applog << "Log start"    << std::endl; /* TODO, include date */
	//g_applog << "Init: "       << std::endl;
	//

#ifdef PLATFORM_WIN
	if ( IsDebuggerPresent() )
	{
		// We don't want to mask exceptions (or report them to Steam!) when debugging.
		// If you would like to step through the exception handler, attach a debugger
		// after running the game outside of the debugger.	

		Main();
		return 0;
	}
#endif

#ifdef PLATFORM_WIN
#ifdef USESTEAM
	_set_se_translator( MiniDumpFunction );

	try  // this try block allows the SE translator to work
	{
#endif
#endif
		Main();
#ifdef PLATFORM_WIN
#ifdef USESTEAM
	}
	catch( ... )
	{
		return -1;
	}
#endif
#endif
	
#ifdef PLATFORM_IOS
	exit(0);
#endif

	return 0;
}
