












#ifndef NET_H
#define NET_H

#include "../platform.h"
#include "packets.h"

#define PORT		50420
#define NPORTS		1024
#define SV_ADDR		"23.226.230.153"	//live server
//#define SV_ADDR			"23.226.224.175"		//vps
//#define SV_ADDR		"54.221.229.124"	//corp1 aws
//#define SV_ADDR			"192.168.1.100"		//home local server ip
//#define SV_ADDR			"192.168.1.103"		//home local server ip
//#define SV_ADDR			"174.6.61.178"		//home public server ip

#define RESEND_DELAY	30
#define RESEND_EXPIRE	(5000)
#define NETCONN_TIMEOUT	(30*1000)
#define NETCONN_UNRESP	(NETCONN_TIMEOUT/3)
#define QUIT_DELAY		(10*1000)
//#define SLIDING_WIN		3	//max resend outgoing packets ahead
#define SLIDING_WIN		10	//max resend outgoing packets ahead	//corpd fix
#define RECV_BUFFER		10	//max packets ahead to buffer recieved out of sequence

uint16_t NextAck(uint16_t ack);
uint16_t PrevAck(uint16_t ack);
bool PastAck(uint16_t test, uint16_t current);
bool PastFr(uint64_t test, uint64_t current);

//extern uint64_t g_lastS;  //last sent
//extern uint64_t g_lastR;  //last recieved

extern int32_t g_netmode;
extern bool g_lanonly;

extern char g_mapname[MAPNAME_LEN+1];
extern char g_svname[SVNAME_LEN+1];
extern uint32_t g_mapcheck;
extern int32_t g_mapfsz;

#ifdef MATCHMAKER
extern uint32_t g_transmitted;
#endif

#define NETM_SINGLE			0	//single player
#define NETM_HOST			1	//hosting
#define NETM_CLIENT			2	//client
//#define NET_SPECT			3	//spectator client
#define NETM_TUT			4	//bit

//#define NET_DEBUG	//debug messages for packets recvd

extern double g_sendrate;

#define SENDRATE	(1024*100)	//Bps

void UpdNet();
void ClearPackets();
void CheckAddSv();
bool Same(IPaddress* a, uint16_t ad, IPaddress* b, uint16_t bd);
bool NetQuit();

#ifndef MATCHMAKER
extern uint32_t g_transmitted;
#endif

#endif

