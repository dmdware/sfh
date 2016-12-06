












#ifndef SENDPACKETS_H
#define SENDPACKETS_H

#include "../platform.h"
#include "packets.h"

class NetConn;

void SendAll(char* data, int32_t size, bool reliable, bool expires, IPaddress* exaddr, uint16_t exdock);
void SendData(char* data, int32_t size, IPaddress * paddr, bool reliable, bool expires, NetConn* nc, UDPsocket* sock, int32_t msdelay, void (*onackfunc)(OldPacket* p, NetConn* nc));
void Acknowledge(uint16_t ack, NetConn* nc, IPaddress* addr, UDPsocket* sock, char* buffer, int32_t bytes);
void ResendPacks();
void Register(char* username, char* password, char* email);
void Login(char* username, char* password);

#endif	//SENDPACKETS_H