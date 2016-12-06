












#ifndef PACKETS_H
#define PACKETS_H

#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../math/vec2i.h"
#include "../math/vec2uc.h"
#include "../math/camera.h"
#include "../sim/simflow.h"
#include "../sim/player.h"

#ifndef MATCHMAKER
#include "../sim/resources.h"
#include "../sim/utype.h"
#include "../sim/bltype.h"
#include "../sim/conduit.h"
#include "../render/fogofwar.h"
#endif

class NetConn;

class OldPacket
{
public:
	char* buffer;
	int32_t len;
	uint64_t last;	//last time resent
	uint64_t first;	//first time sent
	bool expires;
	bool acked;	//used for outgoing packets
	uint64_t netfr;

	//sender/reciever
	IPaddress addr;
	void (*onackfunc)(OldPacket* op, NetConn* nc);

	void freemem()
	{
		if(len <= 0)
			return;

		if(buffer != NULL)
			delete [] buffer;
		buffer = NULL;
	}

	OldPacket()
	{
		len = 0;
		buffer = NULL;
		onackfunc = NULL;
		acked = false;
		netfr = g_netframe;
	}
	~OldPacket()
	{
		freemem();
	}

	OldPacket(const OldPacket& original)
	{
		len = 0;
		buffer = NULL;
		*this = original;
	}

	OldPacket& operator=(const OldPacket &original)
	{
		//corpdfix
		freemem();

		if(original.buffer && original.len > 0)
		{
			len = original.len;
			if(len > 0)
			{
				buffer = new char[len];
				memcpy((void*)buffer, (void*)original.buffer, len);
			}
			last = original.last;
			first = original.first;
			expires = original.expires;
			acked = original.acked;
			addr = original.addr;
			onackfunc = original.onackfunc;
			netfr = original.netfr;
#ifdef MATCHMAKER
			//ipaddr = original.ipaddr;
			//port = original.port;
			//memcpy((void*)&addr, (void*)&original.addr, sizeof(struct sockaddr_in));
#endif
		}
		else
		{
			buffer = NULL;
			len = 0;
			onackfunc = NULL;
			netfr = 0;
		}

		return *this;
	}
};

//TODO merge some of these into multi-purpose packet types
//TODO separate protocol/control packets from user/command packets

#define	PACKET_NULL						0
#define PACKET_DISCONNECT				1
#define PACKET_CONNECT					2
#define	PACKET_ACKNOWLEDGMENT			3
#define PACKET_PLACEBL					4
#define PACKET_NETTURN					5
#define PACKET_DONETURN					6
#define PACKET_JOIN						7
#define PACKET_ADDSV					8
#define PACKET_ADDEDSV					9
#define PACKET_KEEPALIVE				10
#define PACKET_GETSVLIST				11
#define PACKET_SVADDR					12
#define PACKET_SVINFO					13
#define PACKET_GETSVINFO				14
#define PACKET_SENDNEXTHOST				15
#define PACKET_NOMOREHOSTS				16
#define PACKET_ADDCLIENT				17
#define PACKET_SELFCLIENT				18
#define PACKET_SETCLNAME				19
#define PACKET_CLIENTLEFT				20
#define PACKET_CLIENTROLE				21
#define PACKET_DONEJOIN					22
#define PACKET_TOOMANYCL				23
#define PACKET_MAPCHANGE				24
#define PACKET_CHVAL					25
#define PACKET_CLDISCONNECTED			26
#define PACKET_CLSTATE					27
#define PACKET_NOCONN					28
#define PACKET_ORDERMAN					29
#define PACKET_CHAT						30
#define PACKET_MAPSTART					31
#define PACKET_GAMESTARTED				32
#define PACKET_WRONGVERSION				33
#define PACKET_MOVEORDER				34
#define PACKET_PLACECD					35
#define PACKET_NACK						36
#define PACKET_LANCALL					37
#define PACKET_LANANSWER				38
#define PACKET_SETSALEPROP				39
#define PACKET_BUYPROP					40
#define PACKET_DOWNMAP					41
#define PACKET_UPMAP					42
#define PACKET_DEMOLPROP				43
#define PACKET_UNIT						44
#define PACKET_BL						45
#define PACKET_CD						46
#define PACKET_FOL						47
#define PACKET_PY						48
#define PACKET_JAM						49
#define PACKET_VIS						50
#define PACKET_BORD						51
#define PACKET_GRAPHHEAD				52
#define PACKET_GRAPHPT					53
#define PACKET_TYPES					54

// byte-align structures
#pragma pack(push, 1)

struct PacketHeader
{
	uint16_t type;
	uint16_t ack;
	uint16_t senddock;
	uint16_t recvdock;
};

struct BasePacket
{
	PacketHeader header;
};

typedef BasePacket NoConnectionPacket;
typedef BasePacket DoneJoinPacket;
typedef BasePacket TooManyClPacket;
typedef BasePacket SendNextHostPacket;
typedef BasePacket NoMoreHostsPacket;
typedef BasePacket GetSvInfoPacket;
typedef BasePacket GetSvListPacket;
typedef BasePacket KeepAlivePacket;
typedef BasePacket AddSvPacket;
typedef BasePacket AddedSvPacket;
typedef BasePacket AckPacket;
typedef BasePacket NAckPacket;
typedef BasePacket MapStartPacket;
//typedef BasePacket GameStartedPacket;
typedef BasePacket LANCallPacket;
typedef BasePacket LANAnswerPacket;
typedef BasePacket DownMapPacket;

#ifndef MATCHMAKER

struct FolPacket
{
	PacketHeader header;
	int32_t fi;

	bool on;
	unsigned char type;
	Vec2i cmpos;
	float yaw;

	//Updraw
};

struct GraphHeadPacket
{
	PacketHeader header;
	uint64_t simframe;
	int32_t cycles;
	int8_t series;
	int8_t row;

#define SERIES_PROTECT	0
#define SERIES_HEALTH	1
};

struct GraphPtPacket
{
	PacketHeader header;
	int8_t series;
	int8_t row;
	float pt;
};

struct BlPacket
{
	PacketHeader header;
	int32_t bi;
	
	bool on;
	int32_t type;
	int32_t owner;

	Vec2i tpos;	//position in tiles
	Vec2f drawpos;	//drawing position in world pixels

	bool finished;

	int32_t stocked[RESOURCES];
	int32_t inuse[RESOURCES];

	int32_t conmat[RESOURCES];
	bool inoperation;

	int32_t price[RESOURCES];	//price of produced goods
	int32_t propprice;	//price of this property
	bool forsale;	//is this property for sale?
	bool demolish;	//was a demolition ordered?
	//TODO only one stocked[] and no conmat[]. set stocked[] to 0 after construction finishes, and after demolition ordered.

	int32_t conwage;
	int32_t opwage;
	int32_t cydelay;	//the frame delay between production cycles, when production target is renewed
	int16_t prodlevel;	//production target level of max RATIO_DENOM
	int16_t cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	uint64_t lastcy;	//last simframe of last production cycle
	
	int32_t manufprc[UNIT_TYPES];
	int16_t transporter[RESOURCES];

	int32_t hp;
	
	int32_t varcost[RESOURCES];	//variable cost for input resource ri
	int32_t fixcost;	//fixed cost for transport

	bool demolition;

	int16_t pownetw;
	int16_t crpipenetw;

	int8_t nroadnetw;
	int16_t ncapsup;
	int8_t noccup;
	int8_t nworker;
	char data[0];

	//std::list<int16_t> roadnetw;
	//std::list<CapSup> capsup;	//capacity suppliers
	//std::list<CycleHist> cyclehist;
	//std::list<int32_t> occupier;
	//std::list<int32_t> worker;
	//std::list<ManufJob> manufjob;

	//UpDraw
};

struct CdPacket
{
	PacketHeader header;
	
	int8_t cdx;
	int8_t cdy;
	int8_t cdti;
	
	bool on;
	unsigned char conntype;
	bool finished;
	unsigned char owner;
	int32_t conmat[RESOURCES];
	int16_t netw;	//network
	//bool inaccessible;
	int16_t transporter[RESOURCES];
	Vec2f drawpos;
	//int32_t maxcost[RESOURCES];
	int32_t conwage;

	bool selling;

	//Updraw
};

struct PyPacket
{
	PacketHeader header;

	int32_t pyi;
	bool on;
	bool ai;
	
	int32_t local[RESOURCES];	// used just for counting; cannot be used
	int32_t global[RESOURCES];
	int32_t resch[RESOURCES];	//resource changes/deltas
	int32_t truckwage;	//truck driver wage per second
	int32_t transpcost;	//transport cost per second
	uint64_t util;
	uint64_t gnp;
	
	uint64_t lastthink;	//AI simframe timer

#if 0
	signed char insttype;	//institution type
	signed char instin;	//institution index
	int32_t parentst;	//parent state player index
	
	bool protectionism;
	int32_t imtariffratio;	//import tariff ratio
	int32_t extariffratio;	//export tariff ratio
	
	//TODO check if to include +1 or if thats part of pynamelen
	char username[PYNAME_LEN+1];
	char password[PYNAME_LEN+1];
#endif
};

struct UnitPacket
{
	PacketHeader header;
	int32_t ui;
	bool on;
	int32_t type;
	int32_t owner;
	Vec2i cmpos;
	Vec3f facing;	//used for tank turret
	Vec3f rotation;
	float frame[2];	//BODY_LOWER and BODY_UPPER
	Vec2i goal;
	int32_t target;
	int32_t target2;
	bool targetu;
	bool underorder;
	int32_t fuelstation;
	int32_t belongings[RESOURCES];
	int32_t hp;
	bool passive;
	Vec2i prevpos;
	int32_t taskframe;
	bool pathblocked;
	int32_t jobframes;
	int32_t supplier;
	int32_t reqamt;
	int32_t targtype;
	int32_t home;
	int32_t car;
	int32_t cargoamt;
	int32_t cargotype;
	int32_t cargoreq;	//req amt
	//std::vector<TransportJob> bids;
	Vec2i subgoal;
	unsigned char mode;
	int32_t pathdelay;
	uint64_t lastpath;

	bool threadwait;

	// used for debugging - don't save to file
	bool collided;

	signed char cdtype;	//conduit type for mode (going to construction)
	int32_t driver;
	//int16_t framesleft;
	int32_t cyframes;	//cycle frames (unit cycle of consumption and work)
	int32_t opwage;	//transport driver wage	//edit: NOT USED, set globally in Player class
	//std::list<TransportJob> bids;	//for trucks

	int32_t exputil;	//expected utility on arrival

	bool forsale;
	int32_t price;

	int32_t incomerate;

	uint16_t npath;
	uint16_t ntpath;
	char pathdata[0];

	//std::list<TrCycleHist> cyclehist;

	//UpDraw
};

struct JamPacket
{
	PacketHeader header;
	uint8_t jam;
	uint8_t tx;
	uint8_t ty;
};

struct VisPacket
{
	PacketHeader header;
	int8_t tx;
	int8_t ty;
	VisTile vt;
};

struct BordPacket
{
	PacketHeader header;
	int8_t tx;
	int8_t ty;
	int8_t owner;
};

#endif

struct GameStartedPacket
{
	PacketHeader header;
	uint64_t simframe;
	uint64_t netframe;
	Vec2uc mapsz;
};

struct UpMapPacket
{
	PacketHeader header;
	int32_t paysz;
	char data[0];
};

//set property selling state
//TODO set selling conduits
struct SetSalePropPacket
{
	PacketHeader header;
	int32_t propi;
	bool selling;	//TODO custom ecbool typedef
	int32_t price;
	int32_t proptype;
	signed char tx;
	signed char ty;
	//int32_t pi;	//should match owner at the time. necessary?
};

#define PROP_BL_BEG		0
#define PROP_BL_END		(BL_TYPES)
#define PROP_CD_BEG		(PROP_BL_END)
#define PROP_CD_END		(PROP_CD_BEG+CD_TYPES)
#define PROP_U_BEG		(PROP_CD_END)
#define PROP_U_END		(PROP_U_BEG+UNIT_TYPES)

//TODO sell off and set for jobs for individual trucks and individual deals apart from std::list<Widget*>::iterator manager
//buy property
//TODO implement buying conduits
//TODO drop-down tree view for all buildings and conduits
//TODO selection box for conduits
struct BuyPropPacket
{
	PacketHeader header;
	int32_t propi;
	int32_t pi;
	int32_t proptype;
	signed char tx;
	signed char ty;
};

struct DemolPropPacket
{
	PacketHeader header;
	int32_t propi;
	int32_t pi;
	int32_t proptype;
	signed char tx;
	signed char ty;
};

//TODO order demolish property and conduit

//variable length depending on number of tiles involved
struct PlaceCdPacket
{
	PacketHeader header;
	int16_t player;
	unsigned char cdtype;
	int16_t ntiles;
	Vec2uc place[0];
};

//variable length depending on number of units ordered
struct MoveOrderPacket
{
	PacketHeader header;
	Vec2i mapgoal;
	uint16_t nunits;
	uint16_t units[0];
};

//order unit manufacture at building
struct OrderManPacket
{
	PacketHeader header;
	int32_t utype;
	int32_t player;
	int32_t bi;
};

#define MAX_CHAT			711

struct ChatPacket
{
	PacketHeader header;
	int32_t client;
	char msg[MAX_CHAT+1];
};

#define CLCH_UNRESP			0	//client became unresponsive
#define CLCH_RESP			1	//became responsive again
#define CLCH_PING			2
#define CLCH_READY			3	//client became ready to start
#define CLCH_NOTREADY		4	//client became not ready
#define CLCH_PAUSE			5	//pause speed
#define CLCH_PLAY			6	//normal play speed
#define CLCH_FAST			7	//fast forward speed

struct ClStatePacket
{
	PacketHeader header;
	unsigned char chtype;
	int16_t client;
	float ping;
	int32_t downin;
};

/*
The difference between client left
and disconnected is that ClDisc*
means it was done by server (kicked? timed out?)
*/

struct ClientLeftPacket
{
	PacketHeader header;
	int16_t client;
};

struct ClDisconnectedPacket
{
	PacketHeader header;
	int16_t client;
	bool timeout;
};

#define CHVAL_BLPRICE					0
#define CHVAL_BLWAGE					1
#define CHVAL_TRWAGE					2
#define CHVAL_TRPRICE					3
#define CHVAL_CSTWAGE					4
#define CHVAL_PRODLEV					5
#define CHVAL_CDWAGE					6
#define CHVAL_MANPRICE					7

struct ChValPacket
{
	PacketHeader header;
	unsigned char chtype;
	int32_t value;
	unsigned char player;
	unsigned char res;
	uint16_t bi;
	unsigned char x;
	unsigned char y;
	unsigned char cdtype;
	unsigned char utype;
};

//not counting null terminator
#define MAPNAME_LEN		127
#define SVNAME_LEN		63

struct MapChangePacket
{
	PacketHeader header;
	uint32_t checksum;
	int32_t filesz;
	char map[MAPNAME_LEN+1];
	bool live;
};

#define CLPLAT_WIN	0	//windows
#define CLPLAT_MAC	1	//mac
#define CLPLAT_LIN	2	//linux
#define CLPLAT_IPH	3	//iphone
#define CLPLAT_IPA	4	//ipad
#define CLPLAT_AND	5	//android
#define CLPLAT_TYPES	6

#if defined( PLATFORM_WIN )
#define CLPLATI		CLPLAT_WIN
#elif defined( PLATFORM_IPHONE )
#define CLPLATI		CLPLAT_IPH
#elif defined( PLATFORM_IPAD )
#define CLPLATI		CLPLAT_IPA
#elif defined( PLATFORM_ANDROID )
#define CLPLATI		CLPLAT_AND
#elif defined( PLATFORM_MAC )
#define CLPLATI		CLPLAT_MAC
#elif defined( PLATFORM_LINUX )
#define CLPLATI		CLPLAT_LIN
#else
#define CLPLATI		-1
#endif

extern char* CLPLATSTR[CLPLAT_TYPES];

struct JoinPacket
{
	PacketHeader header;
	uint32_t version;
	unsigned char clplat;
	char name[PYNAME_LEN+1];
};

struct WrongVersionPacket
{
	PacketHeader header;
	uint32_t correct;
};

struct AddClientPacket
{
	PacketHeader header;
	signed char client;
	signed char player;
	char name[PYNAME_LEN+1];
	bool ishost;
	bool ready;
	unsigned char clplat;
	uint8_t speed;
};

struct SelfClientPacket
{
	PacketHeader header;
	int32_t client;
};

struct SetClNamePacket
{
	PacketHeader header;
	int32_t client;
	char name[PYNAME_LEN+1];
};

struct ClientRolePacket
{
	PacketHeader header;
	signed char client;
	signed char player;
};

struct SvAddrPacket
{
	PacketHeader header;
	IPaddress addr;
	uint16_t dock;
};

class SendSvInfo	//sendable
{
public:
	IPaddress addr;
	char svname[SVNAME_LEN+1];
	int16_t nplayers;
	int16_t maxpys;
	char mapname[MAPNAME_LEN+1];
	bool started;
};

struct SvInfoPacket
{
	PacketHeader header;
	SendSvInfo svinfo;
};

struct NetTurnPacket
{
	PacketHeader header;
	/*
	Is this the net frame upon which it is delivered?
	Or the frame when this will be executed?
	Let's make it the frame that will be executed.
	*/
	uint64_t fornetfr;	//for net fr #..
	uint16_t loadsz;
	//commands go after
};

struct DoneTurnPacket
{
	PacketHeader header;
	uint64_t curnetfr;	//current net fr #.. (start of cl's current turn)
	int16_t client;	//should match sender
};

struct PlaceBlPacket
{
	PacketHeader header;
	int32_t btype;
	Vec2i tpos;
	int32_t player;
};

struct ConnectPacket
{
	PacketHeader header;
	bool reconnect;
	uint16_t yourlastrecvack;
	uint16_t yournextrecvack;
	uint16_t yourlastsendack;
};

struct DisconnectPacket
{
	PacketHeader header;
	bool reply;
};

// Default alignment
#pragma pack(pop)

#endif



