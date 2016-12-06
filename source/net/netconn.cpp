












#include "../platform.h"
#include "netconn.h"
#include "lockstep.h"
#include "sendpackets.h"
#include "../sim/simflow.h"
#include "../sim/player.h"
#include "../save/savemap.h"
#include "../path/pathdebug.h"
#include "../utils.h"
#include "readpackets.h"

#ifndef MATCHMAKER
#include "client.h"
#include "../sim/bltype.h"
#include "../thread/thread.h"

#include "../app/appmain.h"
#include "../gui/layouts/chattext.h"
#include "../gui/layouts/messbox.h"
#include "../gui/widgets/spez/svlist.h"
#endif

bool g_sentsvinfo = false;	//did we send our hosted game's IP to the sv list?
//bool g_needsvlist = false;	//did we request a sv list?
//bool g_reqdsvlist = false;	//did we send out a request to get sv list?

UDPsocket g_sock = NULL;
std::list<NetConn> g_conn;
std::list<OldPacket> g_outgo;	//outgoing packets. sent are those that have arrived at the other side already.
std::list<OldPacket> g_recv;

NetConn* g_svconn = NULL;
NetConn* g_mmconn = NULL;	//matchmaker

uint16_t g_dock;

void NetConn::expirein(int32_t millis)
{
	uint64_t now = GetTicks();
	lastrecv = now - NETCONN_TIMEOUT + millis;
}

void OpenSock()
{
	//2015/10/27 now it doesn't log opening socket each time
	//Log("opening port/socket");
	
	int32_t r = (rand()*GetTicks());

#ifndef MATCHMAKER
	uint16_t startport = PORT + (r+0)%NPORTS;
#else
	uint16_t startport = PORT;
#endif

	if(g_sock)
	{
		//return;
		IPaddress* ip = SDLNet_UDP_GetPeerAddress(g_sock, -1);

		if(!ip)
			Log("SDLNet_UDP_GetPeerAddress: %s\r\n", SDLNet_GetError());
		else
			startport = SDL_SwapBE16(ip->port);

		SDLNet_UDP_Close(g_sock);
		g_sock = NULL;
	}
	else
		//g_dock = 0;
		g_dock = ( GetTicks() * rand() ) | 1;

	if(g_sock = SDLNet_UDP_Open(startport))
		return;

	//try 10 ports
#ifndef MATCHMAKER

	for(int32_t i=0; i<NPORTS; i++)
	{
		if(!(g_sock = SDLNet_UDP_Open(PORT+(r+i)%NPORTS)))
			continue;

		//char msg[128];
		//sprintf(msg, "show port %d", PORT+i);
		//InfoMess("p", msg);
		
		//SDLNet_UDP_SetPacketLoss(g_sock, 20);
		//SDLNet_UDP_SetPacketLoss(g_sock, 40);
		//SDLNet_UDP_SetPacketLoss(g_sock, 70);
		//SDLNet_UDP_SetPacketLoss(g_sock, 80);
		//SDLNet_UDP_SetPacketLoss(g_sock, 95);

		return;
	}
#endif

#ifndef MATCHMAKER
	char msg[1280];
	sprintf(msg, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
	Log(msg);
	ErrMess("Error", msg);
#endif
}

NetConn* Connect(const char* addrstr, uint16_t port, bool ismatch, bool isourhost, bool isclient, bool ishostinfo)
{
	IPaddress ip;

	if(SDLNet_ResolveHost(&ip, addrstr, port) == -1)
	{
		//char msg[1280];
		//sprintf(msg, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		//ErrMess("Error", msg);
		return NULL;
	}

#if 0
	if(!g_sock && !(g_sock = SDLNet_UDP_Open(0)))
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
		return NULL;
	}
#endif

	return Connect(&ip, ismatch, isourhost, isclient, ishostinfo);
}

//Safe to call more than once, if connection already established, this will just
//update NetConn booleans.
//Edit: Maybe not safe anymore. Connection will reset to 0 on additional call.
//Necesssary because other side might have lost connection (timeout) and if we
//send using the last acks we'll just get acks back but other side won't read messages.
//Edit: probably unlikely that one side will time out significantly before the other. Still safe.
//Edit: made it flush prev in/out packets and reset ack/seq num's on additional call.
NetConn* Connect(IPaddress* ip, bool ismatch, bool isourhost, bool isclient, bool ishostinfo)
{
	if(!g_sock)
		OpenSock();

	//Log("connect();");

#if 0
	if(SDLNet_UDP_Bind(g_sock, 0, ip) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
		return NULL;
	}
#endif

	NetConn* nc = Match(ip, 0);

	NetConn newnc;
	bool isnew = false;

	if(!nc)
	{
		isnew = true;
		newnc.addr = *ip;
		newnc.handshook = false;
		newnc.dock = 0;
		newnc.lastrecv = GetTicks();
		newnc.lastsent = newnc.lastrecv;
		//important - reply ConnectPacket with ack=0 will be
		//ignored as copy (even though it is original) if new NetConn's lastrecvack=0.
		newnc.lastrecvack = USHRT_MAX;
		newnc.nextsendack = 0;
		newnc.closed = false;
		g_conn.push_back(newnc);

#if 0
		{
			std::list<Widget*>::iterator ci1 = g_conn.begin();
			std::list<Widget*>::iterator ci2 = g_conn.rbegin();

			if(g_conn.size() > 1 &&
				/* ci1->addr.host == ci2->addr.host &&
				ci1->addr.port == ci2->addr.port */
				//memcmp(&ci1->addr, &ci2->addr, sizeof(IPaddress)) == 0
				Same(&ci1->addr, &ci2->addr)
				)
			{
				char msg[128];
				sprintf(msg, "mult c same at f%s, l%d", __FILE__, __LINE__);
				InfoMess("e", msg);
			}
		}
#endif

		nc = &*g_conn.rbegin();
	}
	else
	{
		//force reconnect (sending ConnectPacket).
		//also important for Click_SL_Join to know that we
		//can't send a JoinPacket immediately after this function,
		//but must wait for a reply ConnectPacket.
		//edit: never do this; never set handshook=false if already handshook
		if(nc->closed)
			nc->handshook = false;

#if 0
		//corpd fix
		//Disconnect(nc);
		FlushPrev(ip);
		nc->lastrecv = GetTicks();
		nc->lastsent = newnc.lastrecv;
		//important - reply ConnectPacket with ack=0 will be
		//ignored as copy (even though it is original) if new NetConn's lastrecvack=0.
		nc->lastrecvack = USHRT_MAX;
		nc->nextsendack = 0;
		nc->handshook = false;
#endif
	}

	bool disconnecting = false;

	for(std::list<OldPacket>::iterator pit=g_outgo.begin(); pit!=g_outgo.end(); pit++)
	{
		OldPacket* op = &*pit;
		PacketHeader* ph = (PacketHeader*)op->buffer;

		if(!Same(&op->addr, ph->recvdock, &nc->addr, nc->dock))
			continue;

		if(ph->type != PACKET_DISCONNECT)
			continue;

		disconnecting = true;
		break;
	}

	if(disconnecting)
	{
		//edit: never do this; never set handshook=false if already handshook
		nc->handshook = false;
		FlushPrev(&nc->addr, nc->dock);
	}

	//nc.ctype = CONN_HOST;
	//nc.isourhost = true;
	//g_conn.push_back(nc);

	//only "true" it, or retain current state of nc->...
	nc->isclient = isclient ? true : nc->isclient;
	nc->isourhost = isourhost ? true : nc->isourhost;
	nc->ismatch = ismatch ? true : nc->ismatch;
	nc->ishostinfo = ishostinfo ? true : nc->ishostinfo;

	if(isourhost)
		g_svconn = nc;
	if(ismatch)
		g_mmconn = nc;

	//see if we need to connect for realsies.
	//i.e., send a connect packet and clean prev packets.
	if(!nc->handshook /* ||
		GetTicks() - nc->lastrecv > NETCONN_TIMEOUT ||
		nc->closed */ )
	{
		//if(!nc->handshook)
		//	InfoMess("!has", "!hsho");

		//if(GetTicks() - nc->lastrecv > NETCONN_TIMEOUT)
		//	InfoMess("GetTicks() - nc->lastrecv > NETCONN_TIMEOUT", "GetTicks() - nc->lastrecv > NETCONN_TIMEOUT");

		bool sending = false;	//sending ConnectPacket?
		uint16_t yourlastrecvack = PrevAck(nc->nextsendack);

		for(std::list<OldPacket>::iterator pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
		{
			PacketHeader* ph = (PacketHeader*)pi->buffer;

			if(!Same(&pi->addr, ph->recvdock, &nc->addr, nc->dock) &&
				!Same(&pi->addr, ph->recvdock, &nc->addr, 0))
				continue;

			//if(PastAck(PrevAck(ph->ack), yourlastrecvack))
			//	yourlastrecvack = PrevAck(ph->ack);

			if(ph->type != PACKET_CONNECT)
				continue;

			sending = true;
			break;
		}

		if(!sending)
		{
			//don't flush prev packs. maybe this was a gethostinfo
			//connection and now it's also becoming an ourhost connection.
			//actually, check if we're already handshook.
			//FlushPrev(ip);

			ConnectPacket cp;
			cp.header.type = PACKET_CONNECT;
			//cp.reconnect = !isnew;
			cp.reconnect = false;	//corpd fix
			cp.yourlastrecvack = yourlastrecvack;
			cp.yournextrecvack = nc->nextsendack;
			cp.yourlastsendack = nc->lastrecvack;
			//SendData((char*)&cp, sizeof(ConnectPacket), ip, true, false, nc, &g_sock, 0, OnAck_Connect);
			SendData((char*)&cp, sizeof(ConnectPacket), ip, isnew, false, nc, &g_sock, 0, OnAck_Connect);
		}
	}

	nc->closed = false;

	return nc;
}

//flush all prev incoming and outgoing packets from this addr
void FlushPrev(IPaddress* from, uint16_t dock)
{
	std::list<OldPacket>::iterator it = g_outgo.begin();

	while(it!=g_outgo.end())
	{
		PacketHeader* ph = (PacketHeader*)it->buffer;

		//if(memcmp(&it->addr, from, sizeof(IPaddress)) != 0)
		if(!Same(&it->addr, ph->recvdock, from, dock) &&
			!Same(&it->addr, ph->recvdock, from, 0) &&
			!Same(&it->addr, 0, from, dock))
		{
			it++;
			continue;
		}

		//it->freemem();
		it = g_outgo.erase(it);
	}

	it = g_recv.begin();

	while(it!=g_recv.end())
	{
		PacketHeader* ph = (PacketHeader*)it->buffer;

		//if(memcmp(&it->addr, from, sizeof(IPaddress)) != 0)
		if(!Same(&it->addr, ph->senddock, from, dock) &&
			!Same(&it->addr, ph->senddock, from, 0) &&
			!Same(&it->addr, 0, from, dock))
		{
			it++;
			continue;
		}

		//it->freemem();
		it = g_recv.erase(it);
	}
}

//keep expiring connections alive (try to)
void KeepAlive()
{
	//return;

#ifdef MATCHMAKER
	//return;
#endif

	uint64_t nowt = GetTicks();
	std::list<NetConn>::iterator ci = g_conn.begin();

#if 0
	Log("ka 1");
	
#endif

	while(g_conn.size() > 0 && ci != g_conn.end())
	{

#if 0
			Log("g_conn.size()="<<g_conn.size());
			
#endif

#if 0
	Log("ka 2");
	
#endif

		if(!ci->handshook || ci->closed)
		{
#if 0
	Log("ka 3");
	
#endif
			ci++;
			continue;
		}
#if 0
	Log("ka 4");
	
#endif

		if(nowt - ci->lastrecv > NETCONN_TIMEOUT/4)
		{
#if 0
	Log("ka 5 "<<nowt<<" - "<<ci->lastrecv<<" = "<<(nowt-ci->lastrecv)<<" > "<<(NETCONN_TIMEOUT/2));
	
#endif
			//check if we're already trying to send a packet to get a reply
			bool outgoing = false;

#if 0
			Log("g_outgo.size()="<<g_outgo.size());
			
#endif

			for(std::list<OldPacket>::iterator pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
			{
				PacketHeader* ph = (PacketHeader*)pi->buffer;
#if 0
	Log("ka 6");
	
#endif
				//if(memcmp(&pi->addr, &ci->addr, sizeof(IPaddress)) != 0)
				if(!Same(&pi->addr, ph->recvdock, &ci->addr, ci->dock) &&
					!Same(&pi->addr, ph->recvdock, &ci->addr, 0))
				{
					continue;
				}

#if 0
	Log("ka 7");
	
#endif

				outgoing = true;
				break;
			}

#if 0
	Log("ka 8");
	
#endif
			if(outgoing)
			{
				ci++;
				continue;
			}
#if 0
			Log("kap");
			//InfoMess("kap", "kap");

			Log("g_conn.size()="<<g_conn.size());
			
#endif
			KeepAlivePacket kap;
			kap.header.type = PACKET_KEEPALIVE;
			SendData((char*)&kap, sizeof(KeepAlivePacket), &ci->addr, true, false, &*ci, &g_sock, 0, NULL);
		}


#if 0
	Log("ka 9");
	
#endif

		ci++;
	}

#if 0
	Log("ka 10");
	
#endif
}

void CheckConns()
{
	//return;

	uint64_t now = GetTicks();

#ifndef MATCHMAKER
	static uint64_t pingsend = GetTicks();
	//pingsend += (float)g_drawfrinterval;	//corpd fix
	//send out client pings
	if(g_netmode == NETM_HOST &&
		//(uint32_t)(GetTicks()) % NETCONN_UNRESP == 0
		//pingsend > (float)(NETCONN_UNRESP/2/1000
		now - pingsend > (NETCONN_UNRESP/2)
		)
	{
		pingsend = now;

		for(int32_t i=0; i<CLIENTS; i++)
		{
			Client* c = &g_client[i];

			if(!c->on)
				continue;

			if(i == g_localC)
				continue;	//clients will have their own ping for the host

			NetConn* nc = c->nc;

			if(!nc)
				continue;

			ClStatePacket csp;
			csp.header.type = PACKET_CLSTATE;
			csp.chtype = CLCH_PING;
			csp.ping = nc->ping;
			csp.client = i;
			csp.downin = c->downin;
			SendAll((char*)&csp, sizeof(ClStatePacket), true, false, NULL, 0);
		}
	}
#endif
	
	std::list<NetConn>::iterator ci = g_conn.begin();

	while(g_conn.size() > 0 && ci != g_conn.end())
	{
		//get rid of timed out connections
		if(!ci->closed && now - ci->lastrecv > NETCONN_TIMEOUT)
		{
			//TO DO any special condition handling, inform user about sv timeout, etc.

#ifndef MATCHMAKER
			if(ci->ismatch)
			{
#ifdef NET_DEBUG
				Log("time out conn (now - ci->lastrecv > NETCONN_TIMEOUT = "<<now<<" - "<<ci->lastrecv<<" = "<<(now - ci->lastrecv)<<" > "<<NETCONN_TIMEOUT<<")"<<ci->addr.host<<" "<<DateTime()<<" msec"<<GetTicks());
				
#endif
#if 0
				uint64_t passed = now - ci->lastrecv;
				char msg[1280];
				sprintf(msg, "Connection to matchmaker server timed out (%f seconds, num conn = %d).", (float)(passed/1000.0f), (int32_t)g_conn.size());

				for(std::list<Widget*>::iterator ci2 = g_conn.begin(); ci2 != g_conn.end(); ci2++)
				{
					char add[128];
					sprintf(add, "\r\n ci2: ip%u,po%u ?shook:%d", ci2->addr.host, (uint32_t)ci2->addr.port, (int32_t)ci2->handshook);
					strcat(msg, add);
				}

				ErrMess("Error", msg);
#else
				//ErrMess("Error", "Connection to matchmaker server timed out.");
#endif
				g_sentsvinfo = false;
			}
			else if(ci->isourhost)
			{
#if 0
				uint64_t passed = now - ci->lastrecv;
				char msg[1280];
				sprintf(msg, "Connection to game host timed out (%f seconds, num conn = %d).", (float)(passed/1000.0f), (int32_t)g_conn.size());

				for(std::list<Widget*>::iterator ci2 = g_conn.begin(); ci2 != g_conn.end(); ci2++)
				{
					char add[128];
					sprintf(add, "\r\n ci2: ip%u,po%u ?shook:%d", ci2->addr.host, (uint32_t)ci2->addr.port, (int32_t)ci2->handshook);
					strcat(msg, add);
				}

				ErrMess("Error", msg);
#else
				//ErrMess("Error", "Connection to game host timed out.");
				//Log("Connection to game host timed out.");
#endif

				EndSess();
				RichText mess = RichText("ERROR: Connection to host timed out.");
				Mess(&mess);

				//return;
			}
			else if(ci->ishostinfo)
				;	//ErrMess("Error", "Connection to prospective game host timed out.");
			else if(ci->isclient)
			{
				//ErrMess("Error", "Connection to client timed out.");

				/*
				TODO
				combine ClDisconnectedPacket and ClientLeftPacket.
				use params to specify conditions of leaving:
				- of own accord
				- timed out
				- kicked by host
				*/

				//TODO inform other clients?
				ClDisconnectedPacket cdp;
				cdp.header.type = PACKET_CLDISCONNECTED;
				cdp.client = ci->client;
				cdp.timeout = true;
				SendAll((char*)&cdp, sizeof(ClDisconnectedPacket), true, false, &ci->addr, ci->dock);
				
				Client* c = &g_client[ci->client];
				RichText msg = c->name + STRTABLE[STR_TIMEDOUT];
				AddNotif(&msg);
			}
#if 0
			if(ci->ismatch)
			{
				g_sentsvinfo = false;
				g_mmconn = NULL;
			}

			if(ci->isourhost)
			{
				g_svconn = NULL;
			}
#endif
#else
			//Log(DateTime()<<" timed out");
			
#endif

			//FlushPrev(&ci->addr);

			//ci = g_conn.erase(ci);

			ci->closed = true;	//Close it using code below
			//ci++;
			//continue;
		}

		//get rid of closed connections
		if(ci->closed)
		{
			if(&*ci == g_mmconn)
			{
				g_sentsvinfo = false;
				g_mmconn = NULL;
			}
			if(&*ci == g_svconn)
				g_svconn = NULL;
#ifndef MATCHMAKER
			for(int32_t cli=0; cli<CLIENTS; cli++)
			{
				Client* c = &g_client[cli];

				if(!c->on)
					continue;

				if(c->nc == &*ci)
				{
					if(g_netmode == NETM_HOST)
					{
					/*
						//disconnect inform others? on the occasion of connection being closed by cl side
						ClientLeftPacket clp;
						clp.header.type = PACKET_CLIENTLEFT;
						clp.client = cli;
						//TODO processing for client left packet
						SendAll((char*)&clp, sizeof(ClientLeftPacket), true, false, &ci->addr);
					*/

						/*
						TODO this might or might not be because of a ReadDisconnectPacket from client.
						React accordingly to timeout etc. sending other clients info.
						*/

						//Done in ReadDisconnectPacket(); and above in timeout code
						//RichText msg = c->name + RichText(" left.");
						//AddNotif(&msg);
					}

					if(c->player >= 0)
					{
						Player* py = &g_player[c->player];
						py->on = false;
						py->client = -1;
						//TODO sell assets?
					}

					c->player = -1;
					c->on = false;
				}
			}
#endif

			//necessary to flush? already done in ReadDisconnectPacket();
			//might be needed if connection can become ->closed another way.
			FlushPrev(&ci->addr, ci->dock);
			ci = g_conn.erase(ci);
			continue;
		}

		//inform other clients of unresponsive clients
		//or inform local player or unresponsive host
		if(now - ci->lastrecv > NETCONN_UNRESP &&
			ci->isclient)	//make sure this is not us or a matchmaker
		{
#ifndef MATCHMAKER
			NetConn* nc = &*ci;

			Client* c = NULL;

			if(nc->client >= 0)
				c = &g_client[nc->client];

			if(g_netmode == NETM_CLIENT &&
				nc->isourhost)
			{
				//inform local player TODO
				c->unresp = true;
			}
			else if(g_netmode == NETM_HOST &&
				nc->isclient &&
				c)
			{
				//inform others
				if(c->unresp)
				{
					ci++;	//corpc fix
					continue; //already informed
				}

				c->unresp = true;

				ClStatePacket csp;
				csp.header.type = PACKET_CLSTATE;
				csp.chtype = CLCH_UNRESP;
				csp.client = c - g_client;
				csp.downin = c->downin;
				SendAll((char*)&csp, sizeof(ClStatePacket), true, false, &nc->addr, nc->dock);
			}
#endif
		}

		ci++;
	}
}

NetConn* Match(IPaddress* addr, uint16_t dock)
{
	if(!addr)
		return NULL;

	for(std::list<NetConn>::iterator ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(Same(&ci->addr, ci->dock, addr, dock))
		//if(memcmp((void*)&ci->addr, (void*)addr, sizeof(IPaddress)) == 0)
			return &*ci;

	if(!dock)
		for(std::list<NetConn>::iterator ci=g_conn.begin(); ci!=g_conn.end(); ci++)
			if(Same(&ci->addr, 0, addr, dock))
			//if(memcmp((void*)&ci->addr, (void*)addr, sizeof(IPaddress)) == 0)
				return &*ci;

	return NULL;
}

void Disconnect(NetConn* nc)
{
	//corpd fix
	nc->disconnecting = true;

	//check if we already called Disconnect on this connection
	//and have an outgoing DisconnectPacket
	bool out = false;

	for(std::list<OldPacket>::iterator pit=g_outgo.begin(); pit!=g_outgo.end(); pit++)
	{
		PacketHeader* ph = (PacketHeader*)pit->buffer;

		if(!Same(&pit->addr, ph->recvdock, &nc->addr, nc->dock) &&
			!Same(&pit->addr, ph->recvdock, &nc->addr, 0) &&
			!Same(&pit->addr, 0, &nc->addr, nc->dock))
			continue;

		if(ph->type != PACKET_DISCONNECT)
			continue;

		out = true;
		break;
	}

	if(!out)
	{
		DisconnectPacket dp;
		dp.header.type = PACKET_DISCONNECT;
		SendData((char*)&dp, sizeof(DisconnectPacket), &nc->addr, true, false, nc, &g_sock, 0, OnAck_Disconnect);
	}
	//nc->closed = true;	//Do this OnAck_Disconnect
	/*
	Following is necessary so that
	for example if we try to connect
	to an unreachable host (setting nc->isourhost)
	and click "Cancel" when it doesn't do anything,
	we won't be taken to single player screen
	when it times out and sees that it ->isourhost.
	*/
	/*
	Going to comment this out because it seems
	if the host stops hosting and restarts, it
	forgets that this connection is the matchmaker,
	and thinks that it isn't connected. 
	Actually, the problem is !handshook.
	//nc->ismatch = false;
	*/
	/*
	//maybe these are all unecessary
	nc->isourhost = false;
	nc->isclient = false;
	nc->ishostinfo = false;
	*/
	//nc->expirein(RESEND_DELAY*2);

	//InfoMess("ds","ds");

#if 0
	FlushPrev(&nc->addr);

	//will be removed at the end of UpdNet();.
	std::list<Widget*>::iterator cit = g_conn.begin();
	while(cit != g_conn.end())
	{
		if(&*cit != nc)
			continue;

		cit = g_conn.erase(cit);
		break;
	}
#endif
}
