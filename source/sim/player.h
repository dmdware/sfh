











#ifndef PLAYER_H
#define PLAYER_H

#include "../platform.h"
//#include "../net/netconn.h"
#include "resources.h"
#include "../gui/richtext.h"
#include "../econ/institution.h"

#ifndef MATCHMAKER
#include "../gui/gui.h"
#include "../math/camera.h"
#include "selection.h"
//#include "../../script/objectscript.h"
#endif

struct PlayerColor
{
	unsigned char color[3];
	char name[32];
};

#define PLAYER_COLORS	48

extern PlayerColor g_pycols[PLAYER_COLORS];

#define PYNAME_LEN		63	//not counting null (=>64)

#ifndef MATCHMAKER

class Player
{
public:
	bool on;
	bool ai;

	int32_t local[RESOURCES];	// used just for counting; cannot be used
	int32_t global[RESOURCES];
	int32_t resch[RESOURCES];	//resource changes/deltas
	int32_t truckwage;	//truck driver wage per second
	int32_t transpcost;	//transport cost per second
	uint64_t util;
	uint64_t gnp;

	float color[4];
	RichText name;
	int32_t client;	//for server

	signed char insttype;	//institution type
	signed char instin;	//institution index
	int32_t parentst;	//parent state player index

	bool protectionism;
	int32_t imtariffratio;	//import tariff ratio
	int32_t extariffratio;	//export tariff ratio

	uint64_t lastthink;	//AI simframe timer

	//TODO check if to include +1 or if thats part of pynamelen
	char username[PYNAME_LEN+1];
	char password[PYNAME_LEN+1];

#ifndef MATCHMAKER
	Player();
	~Player();
#endif
};

//#define PLAYERS 12
#define PLAYERS ARRSZ(g_pycols)
//#define PLAYERS	6	//small number of AI players so it doesn't freeze (as much)

extern Player g_player[PLAYERS];
extern int32_t g_localP;
extern int32_t g_playerm;
extern bool g_diplomacy[PLAYERS][PLAYERS];

void FreePys();
void AssocPy(int32_t player, int32_t client);
int32_t NewPlayer();
int32_t NewClPlayer();
void DefP(int32_t ID, float red, float green, float blue, float alpha, RichText name);
void DrawPy();
void Bankrupt(int32_t player, const char* reason);
void IniRes();

#endif

#endif
