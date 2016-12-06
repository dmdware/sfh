

#include "session.h"
#include "../thread/thread.h"
#include "../sim/build.h"
#include "../net/lockstep.h"
#include "../net/client.h"
#include "../sim/map.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../save/savemap.h"
#include "../path/pathdebug.h"

#ifndef MATCHMAKER
#include "../app/appmain.h"
#include "../gui/layouts/chattext.h"
#include "../gui/layouts/messbox.h"
#include "../gui/widgets/spez/svlist.h"
#include "../net/download.h"
#endif

#ifndef MATCHMAKER

//used to clear cmd queues
void EndSess(bool switchmode)
{
#if 0
	for(int32_t i=0; i<THREADS; i++)
	{
		Thread* t = &g_thread[i];
		SDL_mutexP(t->mutex);
		t->quit = true;
		SDL_mutexV(t->mutex);
		//t->handle = SDL_CreateThread(ThreadFun, "t", (void*)t);
		SDL_WaitThread(t->handle, NULL);
		t->handle = NULL;
		SDL_DestroyMutex(t->mutex);
		t->mutex = NULL;
	}
#endif

	if(g_downmap)
	{
		char maprelative[WF_MAX_PATH+1];
		sprintf(maprelative, "maps/%s", g_downfile.c_str());
		char full[WF_MAX_PATH+1];
		FullWritePath(maprelative, full);
		unlink(full);
		g_downmap = false;
	}

	g_build = BL_NONE;
	
	//FreeCmds(&g_nextnextcmd);
	//FreeCmds(&g_nextcmd);
	FreeCmds(&g_next.cmds);
	FreeCmds(&g_next2.cmds);
	for(std::list<NetTurn>::iterator tit=g_next3.begin(); tit!=g_next3.end(); tit++)
		FreeCmds(&tit->cmds);
	g_next3.clear();
	
	//ResetCls();
	FreeMap();
	FreePys();
	
	if(switchmode)
	{
#ifndef MATCHMAKER
		Player* py = &g_player[g_localP];
		GUI* gui = &g_gui;
		gui->hideall();
		gui->show("main");
#endif

		if(g_netmode == NETM_HOST)
		{
			if(g_mmconn)
				Disconnect(g_mmconn);
		}

		g_appmode = APPMODE_MENU;
		g_netmode = NETM_SINGLE;
	}

	if(g_svconn)
		Disconnect(g_svconn);

#ifndef MATCHMAKER
	ClearChat();

	delete g_joinstate;
	g_joinstate = NULL;
#endif
}

//used to clear cmd queues
void BegSess(bool switchmode)
{
	//FreeCmds(&g_nextnextcmd);
	//FreeCmds(&g_nextcmd);
	FreeCmds(&g_next.cmds);
	FreeCmds(&g_next2.cmds);
	for(std::list<NetTurn>::iterator tit=g_next3.begin(); tit!=g_next3.end(); tit++)
		FreeCmds(&tit->cmds);
	g_next3.clear();


	g_simframe = 0;
	g_netframe = 0;
	g_speed = SPEED_PLAY;
	g_gameover = false;
	//g_canturn = true;	//no NetTurnPacket for turn 1 (which would've been recv'd on turn 0) for cl's
	//g_canturn2 = false;
	g_next.canturn = true;
	g_next2.canturn = false;

	for(int32_t ci=0; ci<CLIENTS; ci++)
	{
		Client* c = &g_client[ci];
		//c->speed = SPEED_PLAY;
		c->curnetfr = 0;
	}

	g_gridvecs.clear();

	int32_t turnframes = TurnFrames();
	g_next.startnetfr = turnframes;

#if 0
	for(int32_t i=0; i<THREADS; i++)
	{
		Thread* t = &g_thread[i];
		t->quit = false;
		t->mutex = SDL_CreateMutex();
		t->handle = SDL_CreateThread(ThreadFun, "t", (void*)t);
	}
#endif
}

#endif