













#include "simflow.h"

#ifndef MATCHMAKER
#include "../net/client.h"
#endif

//TODO does g_netframe need to be uint64_t?

uint64_t g_simframe = 0;
uint64_t g_netframe = 0;	//net frames follow sim frames except when there's a pause, simframes will stop but netframes will continue
unsigned char g_speed = SPEED_PLAY;
bool g_gameover = false;

#ifndef MATCHMAKER
void UpdSpeed()
{
	int16_t fast = 0;
	int16_t play = 0;
	int16_t pause = 0;

	/*
	We need a separate counter for net frames and sim frames,
	because net frames continue while sim frames are paused.
	*/

	//If anybody's paused, we can't continue.
	for(int32_t ci=0; ci<CLIENTS; ci++)
	{
		Client* c = &g_client[ci];

		if(!c->on)
			continue;

		switch(c->speed)
		{
		case SPEED_FAST:
			fast++;
			break;
		case SPEED_PLAY:
			play++;
			break;
		case SPEED_PAUSE:
			pause++;
			break;
		default:
			break;
		}
	}

	if(pause)
		g_speed = SPEED_PAUSE;
	else if(play)
		g_speed = SPEED_PLAY;
	else if(fast)
		g_speed = SPEED_FAST;

#if 0
	if(g_localC >= 0)
	{
		Client* c = &g_client[g_localC];
		c->speed = g_speed;
	}
#endif
}
#endif