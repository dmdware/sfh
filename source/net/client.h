












#ifndef CLIENT_H
#define CLIENT_H

#include "../gui/richtext.h"
#include "../sim/player.h"
#include "netconn.h"
#include "../sim/simflow.h"
#include "../sim/simstate.h"
#include "packets.h"
#include "lockstep.h"

//a client is like a player, but concerns networking.
//a client must be a human player.
//a client controls a player slot.
class Client
{
public:
	bool on;
	int32_t player;
	RichText name;
	//unsigned char color;
	NetConn* nc;
	bool unresp;	//unresponsive?
	bool ready;
	int16_t ping;	//for client use; server keeps it in NetConn's
	unsigned char speed;
	uint64_t curnetfr;	//not so much the frame as the net turn
	int32_t downin;	//download file byte index
	unsigned char clplat;	//client platform

	//information to send to join mid-session
	uint64_t joinsimf;	//join sim frame
	SimState* joinstate;	//simstate at the start of joinsimf
	std::list<NetTurn> afterjoin;	//buffer of NetTurn commands on and after joinsimf

	Client()
	{
		nc = NULL;
		speed = SPEED_PLAY;
		joinstate = NULL;
	}
};

#define CLIENTS	PLAYERS

extern Client g_client[CLIENTS];
extern int32_t g_localC;
extern RichText g_name;

void ResetCls();
void ResetCl(Client* c);
bool AddClient(NetConn* nc, RichText name, int32_t* retci, unsigned char clplati);
void AddClient(NetConn* nc, RichText name, int32_t ci, unsigned char clplati);

#endif
