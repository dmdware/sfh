












#ifndef LOCKSTEP_H
#define LOCKSTEP_H

#include "../platform.h"
#include "net.h"
#include "packets.h"

#define NETTURN	(200/SIM_FRAME_RATE)	//200 ms delay between net turns
//#define NETTURN		(1000/SIM_FRAME_RATE)	//1000 ms delay between net turns
#define FASTTURN	(NETTURN*10)			//a fast-forward net turn is 10x normal net turn

class NetTurn 
{
public:
	uint64_t startnetfr;
	std::list<PacketHeader*> cmds;
	bool canturn;	//for client use only

	NetTurn()
	{
		canturn = false;
	}
};

#if 0
extern std::list<PacketHeader*> g_nextcmd;
extern std::list<PacketHeader*> g_nextnextcmd;
extern bool g_canturn;
extern bool g_canturn2;
#else
extern NetTurn g_next;
extern NetTurn g_next2;
extern std::list<NetTurn> g_next3;
#endif

void Cl_StepTurn();
void Sv_StepTurn();
void SP_StepTurn();
void FreeCmds(std::list<PacketHeader*>* q);
void AppendCmds(std::list<PacketHeader*>* q, NetTurnPacket* ntp);
void AppendCmd(std::list<PacketHeader*>* q, PacketHeader* p, int16_t sz);
bool CanStep();
void UpdTurn();
void LockCmd(PacketHeader* pack);
uint32_t TurnFrames();
void FillNetTurnPacket(NetTurnPacket** pp, std::list<PacketHeader*>* q);

#endif