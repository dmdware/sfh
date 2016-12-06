












#include "client.h"

#ifndef MATCHMAKER
#include "../gui/layouts/chattext.h"
#include "../language.h"
#endif

Client g_client[CLIENTS];
int32_t g_localC;
RichText g_name = RichText("Player");

void ResetCls()
{
	g_localC = -1;
	g_localP = -1;
	g_speed = SPEED_PLAY;

	for(int32_t i=0; i<CLIENTS; i++)
	{
		Client* c = &g_client[i];

		c->on = false;
		c->name = RichText("Player");
		//c->color = 0;

		if(c->nc)
		{
			Disconnect(c->nc);
			c->nc->client = -1;
			c->nc = NULL;
		}

		//c->nc = NULL;
		c->ready = false;
		c->unresp = false;
		c->speed = SPEED_PLAY;
	}

	for(int32_t i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];

		py->client = -1;
	}
}

void ResetCl(Client* c)
{
	c->on = false;
	c->name = RichText("Player");
	c->ready = false;
	c->unresp = false;
	c->speed = SPEED_PLAY;

	if(c->nc)
	{
		c->nc->client = -1;
		c->nc = NULL;
	}

	if(c->player >= 0)
	{
		Player* py = &g_player[c->player];
		py->client = -1;
		//py->on = false;
		c->player = -1;
	}

	//if(c - g_client == g_localC)
	//	g_speed = SPEED_PLAY;

	delete c->joinstate;
	c->joinstate = NULL;
	c->afterjoin.clear();

	UpdSpeed();
}

int32_t NewClient()
{
	for(int32_t i=0; i<CLIENTS; i++)
		if(!g_client[i].on)
			return i;

	return -1;
}

//version where we already have a client index
void AddClient(NetConn* nc, RichText name, int32_t ci, unsigned char clplati)
{
	if(nc)
		nc->client = ci;

	Client* c = &g_client[ci];

	c->on = true;
	c->player = -1;
	c->name = name;
	c->nc = nc;
	c->speed = SPEED_PLAY;
	c->downin = -1;
	c->clplat = clplati;

	if(nc)
		nc->client = ci;

#ifndef MATCHMAKER
	RichText clplat = RichText("?");

	if(clplati < CLPLAT_TYPES)
		clplat = RichText(CLPLATSTR[clplati]);

	RichText msg = name + RichText(" ") + STRTABLE[STR_JOINEDGAME] + RichText(" ") + clplat + RichText(".");
	AddNotif(&msg);
#endif
}

//version that gets a new client index from the slots
bool AddClient(NetConn* nc, RichText name, int32_t* retci, unsigned char clplati)
{
	int32_t ci = NewClient();

	if(ci < 0)
		return false;

	//calls the other version here
	AddClient(nc, name, ci, clplati);

	if(retci)
		*retci = ci;

	return true;
}
