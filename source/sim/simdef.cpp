











#include "simdef.h"
#include "../trigger/console.h"
#include "conduit.h"
#include "building.h"
#include "truck.h"
#include "labourer.h"
#include "../sound/sound.h"
#include "../language.h"

#ifndef MATCHMAKER
#include "../gui/layouts/chattext.h"
#include "../gui/widgets/spez/lobby.h"
#include "../tool/rendersprite.h"
#include "../app/appmain.h"
#include "../sim/simtile.h"
#endif

void LoadSysRes()
{
	// Cursor types

#if 0
#define CU_NONE		0	//cursor off?
#define CU_DEFAULT	1
#define CU_MOVE		2	//move window
#define CU_RESZL	3	//resize width (horizontal) from left side
#define CU_RESZR	4	//resize width (horizontal) from right side
#define CU_RESZT	5	//resize height (vertical) from top side
#define CU_RESZB	6	//resize height (vertical) from bottom side
#define CU_RESZTL	7	//resize top left corner
#define CU_RESZTR	8	//resize top right corner
#define CU_RESZBL	9	//resize bottom left corner
#define CU_RESZBR	10	//resize bottom right corner
#define CU_WAIT		11	//shows a hourglass?
#define CU_DRAG		12	//drag some object between widgets?
#define CU_STATES	13
#endif

	LoadSprite("gui/transp", &g_cursor[CU_NONE], false, false);
	LoadSprite("gui/cursors/default", &g_cursor[CU_DEFAULT], false, false);
	LoadSprite("gui/cursors/move", &g_cursor[CU_MOVE], false, false);
	LoadSprite("gui/cursors/reszh", &g_cursor[CU_RESZL], false, false);
	LoadSprite("gui/cursors/reszh", &g_cursor[CU_RESZR], false, false);
	LoadSprite("gui/cursors/reszv", &g_cursor[CU_RESZT], false, false);
	LoadSprite("gui/cursors/reszv", &g_cursor[CU_RESZB], false, false);
	LoadSprite("gui/cursors/reszd2", &g_cursor[CU_RESZTL], false, false);
	LoadSprite("gui/cursors/reszd1", &g_cursor[CU_RESZTR], false, false);
	LoadSprite("gui/cursors/reszd1", &g_cursor[CU_RESZBL], false, false);
	LoadSprite("gui/cursors/reszd2", &g_cursor[CU_RESZBR], false, false);
	LoadSprite("gui/cursors/default", &g_cursor[CU_WAIT], false, false);
	LoadSprite("gui/cursors/default", &g_cursor[CU_DRAG], false, false);


	// Icons

#if 1
	DefI(ICON_DOLLARS, "gui/icons/cents.png", "\\$");
	DefI(ICON_PESOS, "gui/icons/pesos.png", "\\peso");
	DefI(ICON_EUROS, "gui/icons/euros.png", "\\euro");
	DefI(ICON_POUNDS, "gui/icons/pounds.png", "\\pound");
	DefI(ICON_FRANCS, "gui/icons/francs.png", "\\franc");
	DefI(ICON_YENS, "gui/icons/yens.png", "\\yen");
	DefI(ICON_RUPEES, "gui/icons/rupees.png", "\\rupee");
	DefI(ICON_ROUBLES, "gui/icons/roubles.png", "\\ruble");
	DefI(ICON_LABOUR, "gui/icons/labour.png", "\\labour");
	DefI(ICON_HOUSING, "gui/icons/housing.png", "\\housing");
	DefI(ICON_FARMPRODUCT, "gui/icons/farmproducts.png", "\\farmprod");
	DefI(ICON_WSFOOD, "gui/icons/wsfood.png", "\\wsfood");
	DefI(ICON_RETFOOD, "gui/icons/retfood.png", "\\retfood");
	DefI(ICON_CHEMICALS, "gui/icons/chemicals.png", "\\chemicals");
	DefI(ICON_ELECTRONICS, "gui/icons/electronics.png", "\\electronics");
	DefI(ICON_RESEARCH, "gui/icons/research.png", "\\research");
	DefI(ICON_PRODUCTION, "gui/icons/production.png", "\\production");
	DefI(ICON_IRONORE, "gui/icons/ironore.png", "\\ironore");
	DefI(ICON_URANIUMORE, "gui/icons/uraniumore.png", "\\uraniumore");
	DefI(ICON_STEEL, "gui/icons/steel.png", "\\steel");
	DefI(ICON_CRUDEOIL, "gui/icons/crudeoil.png", "\\crudeoil");
	DefI(ICON_WSFUEL, "gui/icons/fuelwholesale.png", "\\wsfuel");
	DefI(ICON_STONE, "gui/icons/stone.png", "\\stone");
	DefI(ICON_CEMENT, "gui/icons/cement.png", "\\cement");
	DefI(ICON_ENERGY, "gui/icons/energy.png", "\\energy");
	DefI(ICON_ENUR, "gui/icons/uranium.png", "\\enur");
	DefI(ICON_COAL, "gui/icons/coal.png", "\\coal");
	DefI(ICON_TIME, "gui/icons/time.png", "\\time");
	DefI(ICON_RETFUEL, "gui/icons/fuelretail.png", "\\retfuel");
	DefI(ICON_LOGS, "gui/icons/logs.png", "\\logs");
	DefI(ICON_LUMBER, "gui/icons/lumber.png", "\\lumber");
	DefI(ICON_WATER, "gui/icons/water.png", "\\water");
	DefI(ICON_EXCLAMATION, "gui/icons/exclamation.png", "\\exclam");
	DefI(ICON_CENTS, "gui/icons/cents.png", "\\cent");
	DefI(ICON_SMILEY, "gui/icons/smiley.png", ":)");
	DefI(ICON_FIRM, "gui/icons/firm.png", "\\firm");
	DefI(ICON_GOV, "gui/icons/gov.png", "\\gov");
#endif
	
#ifndef MATCHMAKER
	CreateTex(g_texstop, "gui/pause.png", true, false);
	CreateTex(g_texplay, "gui/play.png", true, false);
	CreateTex(g_texfast, "gui/fastforward.png", true, false);
#endif
}

// Queue all the game resources and define objects
void Queue()
{
	//return;

	// Resource types

#if 0
#define RES_DOLLARS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_CHEMICALS		5
#define RES_ELECTRONICS		6
#define RES_IRONORE			7
#define RES_METAL			8
#define RES_STONE			9
#define RES_CEMENT			10
#define RES_COAL			11
#define RES_URANIUM			12
#define RES_PRODUCTION		13
#define RES_CRUDEOIL		15
#define RES_RETFUEL			16
#define RES_ENERGY			17
#define RESOURCES			18
#endif

#if 0
	void DefR(int32_t resi, const char* n, const char* depn, int32_t iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a)
#endif

#if 1
		
	//2016/04/25 added resource decay half-life
	DefR(RES_DOLLARS,		STRTABLE[STR_RESFUNDS].rawstr().c_str(),				"",						ICON_DOLLARS,		true,	false,	false,	0.0f,1.0f,0.0f,1.0f,	CD_NONE, STRTABLE[STR_MEASDOLLARS].rawstr().c_str(), 0);
	DefR(RES_LABOUR,		STRTABLE[STR_RESLABOUR].rawstr().c_str(),				"",						ICON_LABOUR,		true,	false,	false,	0.0f,0.0f,1.0f,1.0f,	CD_NONE, STRTABLE[STR_MEASLABSECS].rawstr().c_str(), 0);
	DefR(RES_HOUSING,		STRTABLE[STR_RESHOUSING].rawstr().c_str(),				"",						ICON_HOUSING,		true,	true,	false,	1.0f,0.0f,0.0f,1.0f,	CD_ROAD, STRTABLE[STR_MEASOCC].rawstr().c_str(), 0);
	DefR(RES_FARMPRODUCTS,	STRTABLE[STR_RESFARMPRODS].rawstr().c_str(),			"Fertile",				ICON_FARMPRODUCT,	true,	false,	false,	1.0f,1.0f,0.0f,1.0f,	CD_ROAD, STRTABLE[STR_MEASBUSHELS].rawstr().c_str(), SIM_FRAME_RATE * 60 * 60 * 24 * 30);
	DefR(RES_PRODUCTION,	STRTABLE[STR_RESPROD].rawstr().c_str(),					"",						ICON_PRODUCTION,	true,	false,	false,	0.0f,1.0f,1.0f,1.0f,	CD_ROAD, STRTABLE[STR_MEASPROD].rawstr().c_str(), 0);
	DefR(RES_RETFOOD,		STRTABLE[STR_RESRETFOOD].rawstr().c_str(),				"",						ICON_RETFOOD,		true,	false,	false,	1.0f,0.0f,1.0f,1.0f,	CD_ROAD, STRTABLE[STR_MEASBUSHELS].rawstr().c_str(), SIM_FRAME_RATE * 60 * 60 * 24 * 30);
	DefR(RES_CRUDEOIL,		STRTABLE[STR_RESCROIL].rawstr().c_str(),				"Oil Deposit",			ICON_CRUDEOIL,		true,	false,	false,	0.3f,0.3f,0.3f,1.0f,	CD_CRPIPE, STRTABLE[STR_MEASGALLONS].rawstr().c_str(), 0);
	DefR(RES_WSFUEL,		STRTABLE[STR_RESWSFUEL].rawstr().c_str(),				"",						ICON_WSFUEL,		true,	false,	false,	0.5f,0.5f,0.5f,1.0f,	CD_ROAD, STRTABLE[STR_MEASGALLONS].rawstr().c_str(), 0);
	DefR(RES_RETFUEL,		STRTABLE[STR_RESRETFUEL].rawstr().c_str(),				"",						ICON_RETFUEL,		true,	false,	false,	0.7f,0.7f,0.7f,1.0f,	CD_ROAD, STRTABLE[STR_MEASGALLONS].rawstr().c_str(), 0);
	DefR(RES_ENERGY,		STRTABLE[STR_RESENERGY].rawstr().c_str(),				"",						ICON_ENERGY,		false,	true,	false,	0.0f,0.5f,0.5f,1.0f,	CD_POWL, STRTABLE[STR_MEASKWATTS].rawstr().c_str(), 0);
	DefR(RES_CHEMICALS,		STRTABLE[STR_RESCHEMS].rawstr().c_str(),				"",						ICON_CHEMICALS,		true,	false,	false,	0.5f,0.0f,0.5f,1.0f,	CD_ROAD, STRTABLE[STR_MEASLITERS].rawstr().c_str(), 0);
	//DefR(RES_ELECTRONICS,	"Electronics",			"",								ICON_ELECTRONICS,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CD_ROAD);
	DefR(RES_IRONORE,		STRTABLE[STR_RESIRONORE].rawstr().c_str(),				"",						ICON_IRONORE,		true,	false,	false,	0.5f,0.5f,0.0f,1.0f,	CD_ROAD, STRTABLE[STR_MEASKGRAMS].rawstr().c_str(), 0);
	DefR(RES_METAL,			STRTABLE[STR_RESMETAL].rawstr().c_str(),				"",						ICON_STEEL,			true,	false,	false,	0.5f,0.0f,0.5f,1.0f,	CD_ROAD, STRTABLE[STR_MEASKGRAMS].rawstr().c_str(), 0);
	DefR(RES_STONE,			STRTABLE[STR_RESSTONE].rawstr().c_str(),				"",						ICON_STONE,			true,	false,	false,	0.3f,0.7f,0.7f,1.0f,	CD_ROAD, STRTABLE[STR_MEASKGRAMS].rawstr().c_str(), 0);
	DefR(RES_CEMENT,		STRTABLE[STR_RESCEMENT].rawstr().c_str(),				"",						ICON_CEMENT,		true,	false,	false,	0.7f,0.3f,0.7f,1.0f,	CD_ROAD, STRTABLE[STR_MEASKGRAMS].rawstr().c_str(), 0);
	DefR(RES_COAL,			STRTABLE[STR_RESCOAL].rawstr().c_str(),					"",						ICON_COAL,			true,	false,	false,	0.7f,0.7f,0.3f,1.0f,	CD_ROAD, STRTABLE[STR_MEASKGRAMS].rawstr().c_str(), 0);
	DefR(RES_URANIUM,		STRTABLE[STR_RESURAN].rawstr().c_str(),					"",						ICON_ENUR,			true,	false,	false,	0.3f,0.3f,0.7f,1.0f,	CD_ROAD, STRTABLE[STR_MEASGRAMS].rawstr().c_str(), 0);


	// Various environment textures
	
	//AddTile("textures/marsdirt/MarsViking1Lander-BigJoeRock-19780211.jpg");
	//PrepareRender(RENDER_TERRTILE, (int32_t*)&g_ground,
	//	false, true, false, false, 1, 1, true, false, false);
	//FreeTile();

	//g_appmode = APPMODE_LOADING;
	///GUI* gui = &g_gui;
	///gui->hideall();
	///gui->show("loading");
	//gui->get("loading")->showall();

	MakeTiles();

	//QueueRend("models/2016/10/1000sph.ms3d", RENDER_MODEL,
	//	&g_100sph, false, false, false, false, 0, 0,
	//	true, false, false, true, 0);

	QueueTex(&g_fog0, "billboards/exhaust2.png", false, false);
	QueueTex(&g_fog1, "billboards/exhaust.png", false, false);

	//QueueRend("textures/marsdirt/MarsViking1Lander-BigJoeRock-19780211.jpg",
	//QueueRend("textures/2015/12/mars.jpg",
	//QueueRend("textures/old/g.jpg",
	//QueueRend("textures/terr/grass0/grass0.jpg",
	//QueueRend("models/industry/gresikacid/bigpiper.jpg",
	//	RENDER_TERRTILE, &g_ground, false, true, false, false, 1, 1, true, true, true, true,
	//	5);

	//DefTl("sprites/tiles/dirt/dirt");
	//DefTl("sprites/tiles/tg/tg");
	//DefTl("sprites/tiles/gengrass - Copy/gg");
	DefTl("sprites/tiles/grass/grass");
	//DefTl("sprites/tiles/w45grass2/w45grass2");
	//DefTl("sprites/tiles/g0/g0");
#endif
	//return;

	QueueTex(&g_circle, "gui/circle.png", true, true);

	LoadParticles();

	//return;

	// Players

#if 1
	for(int32_t i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];
		PlayerColor* pyc = &g_pycols[i];

		char name[64];
		sprintf(name, "AI%d (%s)", i, pyc->name);

		DefP(i, pyc->color[0]/255.0f, pyc->color[1]/255.0f, pyc->color[2]/255.0f, 1, RichText(name));

		//SubmitConsole(&p->name);
	}
#endif

#if 00001

	// Unit types

#if 0001
	/*
	IMPORTANT: because of "jams" variable measured in unsigned char, max unit width is 255 for now.
	divide unit width by 2 or more when subtracting from jams to get bigger unit allowable size.
	*/

	//DefU(UNIT_LABOURER, "models/labourer/labourer.ms3d", Vec3i(1,1,1)*182.0f/100.0f, Vec3i(0,0,0)*182.0f/100.0f, Vec3i(125, 250, 125), "Labourer", 100, true, true, false, false, false, 6, false);
	DefU(UNIT_LABOURER, "sprites/units/lab00/h1", 30, Vec2s(PATHNODE_SIZE, PATHNODE_SIZE*2), STRTABLE[STR_LABOURER].rawstr().c_str(), 100, 
	//DefU(UNIT_LABOURER, "sprites/units/lab00/lab00", 3, Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), STRTABLE[STR_LABOURER].rawstr().c_str(), 100, 
		true, true, false, false, false, 6, false, 5*TILE_SIZE,
		0);
	//DefU(UNIT_LABOURER, "sprites/units/truck/truck", 1, Vec2s(50, 150), "Labourer", 100, true, true, false, false, false, 6, false, 5*TILE_SIZE);
	
	DefU(UNIT_TRUCK, "sprites/units/truck/truck", 1, Vec2s(PATHNODE_SIZE*3, PATHNODE_SIZE*2), STRTABLE[STR_TRUCK].rawstr().c_str(), 100,
	//DefU(UNIT_TRUCK, "sprites/units/truck/truck", 1, Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), STRTABLE[STR_TRUCK].rawstr().c_str(), 100, 
		true, false, true, false, false, TRUCK_SPEED, false, 5*TILE_SIZE,
		RATIO_DENOM * 14 / 70);
	UCost(UNIT_TRUCK, RES_PRODUCTION, (18000/3));
	

#if 0
	DefU(UNIT_CARLYLE, "sprites/units/carlyle/carlyle", 1, Vec2s(PATHNODE_SIZE*6, PATHNODE_SIZE*2), STRTABLE[STR_CARLYLE].rawstr().c_str(), 100,
	//DefU(UNIT_CARLYLE, "sprites/units/carlyle/carlyle", 1,  Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), STRTABLE[STR_CARLYLE].rawstr().c_str(), 100, 
		true, true, false, false, false, 16, true, 5*TILE_SIZE,
		0);
	UCost(UNIT_CARLYLE, RES_PRODUCTION, 15);
	
	DefU(UNIT_BATTLECOMP, "sprites/units/bcomp/bcomp", 1, Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), STRTABLE[STR_BATTLECOMP].rawstr().c_str(), 100,
	//DefU(UNIT_BATTLECOMP, "sprites/units/bcomp/bcomp", 1, Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), STRTABLE[STR_BATTLECOMP].rawstr().c_str(), 100, 
		true, true, false, false, false, 6, true, 5*TILE_SIZE,
		0);
	UCost(UNIT_BATTLECOMP, RES_PRODUCTION, 10);
#endif

#else
	//large sizes

	//DefU(UNIT_LABOURER, "models/labourer/labourer.ms3d", Vec3i(1,1,1)*182.0f/100.0f, Vec3i(0,0,0)*182.0f/100.0f, Vec3i(125, 250, 125), "Labourer", 100, true, true, false, false, false, 6, false);
	DefU(UNIT_LABOURER, "sprites/units/lab00/lab00", 3, Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), "Labourer", 100, true, true, false, false, false, 6, false, 5*TILE_SIZE);
	//DefU(UNIT_LABOURER, "sprites/units/truck/truck", 1, Vec2s(50, 150), "Labourer", 100, true, true, false, false, false, 6, false, 5*TILE_SIZE);

	DefU(UNIT_TRUCK, "sprites/units/truck/truck", 1, Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), "Truck", 100, true, false, true, false, false, 30, false, 5*TILE_SIZE);
	UCost(UNIT_TRUCK, RES_PRODUCTION, 1);

	DefU(UNIT_CARLYLE, "sprites/units/carlyle/carlyle", 1,  Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), "Carlyle", 100, true, true, false, false, false, 16, true, 5*TILE_SIZE);
	UCost(UNIT_CARLYLE, RES_PRODUCTION, 15);

	DefU(UNIT_BATTLECOMP, "sprites/units/bcomp/bcomp", 1, Vec2s(PATHNODE_SIZE, PATHNODE_SIZE), "Droid", 100, true, true, false, false, false, 6, true, 5*TILE_SIZE);
	UCost(UNIT_BATTLECOMP, RES_PRODUCTION, 10);
#endif

#if 0
	DefU(UNIT_CARLYLE, "models/carlyle/carlyle.ms3d", Vec2s(250, 250), "Tank", 100, true, true, false, false, false, 16, true);
	UCost(UNIT_CARLYLE, RES_PRODUCTION, 15);

	//DefU(UNIT_TRUCK, "models/truck/truck.ms3d", Vec3i(1,1,1)*30.0f, Vec3i(0,0,0), Vec3i(125, 250, 125), "Truck", 100, true, false, true, false, false, 30, false);
	DefU(UNIT_TRUCK, "models/truck/truck.ms3d", Vec2s(100, 250), "Truck", 100, true, false, true, false, false, 30, false);
	UCost(UNIT_TRUCK, RES_PRODUCTION, 1);
#endif

	// Foliage types

#if 0
	DefF(FL_BARETREE1, "sprites/fol/bare1/bare1", Vec3i(20,20,20), Vec3i(0,0,0), Vec2s(125, 200)*13);
	DefF(FL_BARETREE2, "sprites/fol/bare2/bare2", Vec3i(20,20,20), Vec3i(0,0,0), Vec2s(125, 200)*13);
	DefF(FL_BARETREE3, "sprites/fol/bare3/bare3", Vec3i(20,20,20), Vec3i(0,0,0), Vec2s(125, 200)*13);
	DefF(FL_EURHACKBERRY1, "sprites/fol/eurhackberry1/eurhackberry", Vec3i(20,20,20), Vec3i(0,0,0), Vec2s(125, 200)*13);
	DefF(FL_EURHACKBERRY2, "sprites/fol/eurhackberry2/eurhackberry2", Vec3i(20,20,20), Vec3i(0,0,0), Vec2s(125, 200)*13);
	DefF(FL_EURHACKBERRY3, "sprites/fol/eurhackberry3/eurhackberry3", Vec3i(20,20,20), Vec3i(0,0,0), Vec2s(125, 200)*13);
#else
	DefF(FL_SPRUCE1, "sprites/fol/spruce1/spruce1", Vec2s(250, 1050));
	DefF(FL_SPRUCE2, "sprites/fol/spruce2/spruce2", Vec2s(250, 1050));
	DefF(FL_SPRUCE3, "sprites/fol/spruce3/spruce3", Vec2s(250, 1050));
	//DefF(FL_EUHB1, "sprites/fol/euhb/euhb", Vec3i(20,20,20), Vec3i(0,0,0), Vec2s(250, 1050));
	//DefF(FL_EUHB1, "sprites/fol/spruce3/spruce3", Vec3i(20,20,20), Vec3i(0,0,0), Vec2s(250, 1050));
#endif

	// Building types

	//realistic start
#if 0
	

	DefB(BL_BRK, STRTABLE[STR_HOUSE1].rawstr().c_str(),
		Vec2i(1,1),  false,
		"sprites/bl/brk/brk",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		0,
		FLAG_BLOWN_FIRM,
		0, 0);
	BMat(BL_BRK, RES_CEMENT, 4);
	BMat(BL_BRK, RES_LABOUR, 4);
	BMat(BL_BRK, RES_STONE, 2);
	BOut(BL_HOUSE1, RES_HOUSING, 3, 600);
	BDes(BL_HOUSE1, STRTABLE[STR_HOUSEDESC].rawstr().c_str());

	//DefB(BL_HOUSE1, "Apartments", Vec2i(2,1), "sprites/bl/apartment1/basebuilding.ms3d", Vec3i(100,100,100), Vec3i(0,0,0), "sprites/bl/apartment1/basebuilding.ms3d", Vec3i(100,100,100), Vec3i(0,0,0), FD_LAND, RES_NONE);
	DefB(BL_HOUSE1, STRTABLE[STR_HOUSE1].rawstr().c_str(),
		Vec2i(1,1),  false,
		"sprites/bl/house1/house1",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		0,
		FLAG_BLOWN_FIRM,
		40, RATIO_DENOM * 12);
	BMat(BL_HOUSE1, RES_CEMENT, 45359);
	BMat(BL_HOUSE1, RES_LABOUR, 1200);
	//BMat(BL_HOUSE1, RES_STONE, 2);
	BIn(BL_HOUSE1, RES_ENERGY, 1000);
	BOut(BL_HOUSE1, RES_HOUSING, 4, 600);
	BDes(BL_HOUSE1, STRTABLE[STR_HOUSEDESC].rawstr().c_str());
	//BSon(BL_HOUSE1, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_HOUSE1, BLSND_PROD, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE1, BLSND_FINI, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE1, BLSND_SEL, "sounds/bl/doorbell1.wav");
	BSon(BL_HOUSE1, BLSND_CSEL, "sounds/bl/doorbell1.wav");

#if 1
	DefB(BL_HOUSE2, STRTABLE[STR_HOUSE1].rawstr().c_str(),
		Vec2i(1,1),  false,
		"sprites/bl/house2/house2",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		0,
		FLAG_BLOWN_FIRM,
		35, RATIO_DENOM * 15);
	BMat(BL_HOUSE2, RES_CEMENT, 45359);
	BMat(BL_HOUSE2, RES_LABOUR, 1300);
	//BMat(BL_HOUSE2, RES_STONE, 2);
	BIn(BL_HOUSE2, RES_ENERGY, 1000);
	BOut(BL_HOUSE2, RES_HOUSING, 5, 600);
	BDes(BL_HOUSE2, STRTABLE[STR_HOUSEDESC].rawstr().c_str());
	BSon(BL_HOUSE2, BLSND_PROD, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE2, BLSND_FINI, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE2, BLSND_SEL, "sounds/bl/doorbell1.wav");
	BSon(BL_HOUSE2, BLSND_CSEL, "sounds/bl/doorbell1.wav");
#endif

	DefB(BL_TRFAC, STRTABLE[STR_TRFAC].rawstr().c_str(),
		Vec2i(1,1),  false,
		"sprites/bl/trfac/trfac",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		DEFL_CSWAGE,
		FLAG_BLOWN_FIRM,
		13, RATIO_DENOM * 2*1);
	BMat(BL_HOUSE2, RES_CEMENT, 45359);
	BMat(BL_HOUSE2, RES_LABOUR, 1300);
	BMat(BL_TRFAC, RES_METAL, 5550);
	//BMat(BL_TRFAC, RES_ELECTRONICS, 10);
	BIn(BL_TRFAC, RES_LABOUR, 10);
	BIn(BL_TRFAC, RES_CHEMICALS, 3);
	BIn(BL_TRFAC, RES_METAL, 3175);
	BIn(BL_TRFAC, RES_ENERGY, 5000);
	BOut(BL_TRFAC, RES_PRODUCTION, 18, 10);
	BDes(BL_TRFAC, STRTABLE[STR_FACDESC].rawstr().c_str());
	//BSon(BL_TRFAC, BLSND_PROD, "sounds/notif/button-39.wav");
	//BSon(BL_TRFAC, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_TRFAC, BLSND_PROD, "sounds/bl/factory-short.wav");
	BSon(BL_TRFAC, BLSND_FINI, "sounds/bl/factory-short.wav");
	BSon(BL_TRFAC, BLSND_SEL, "sounds/bl/factory.wav");
	BSon(BL_TRFAC, BLSND_CSEL, "sounds/bl/factory.wav");
	BMan(BL_TRFAC, UNIT_TRUCK);
	BMan(BL_TRFAC, UNIT_BATTLECOMP);
	BMan(BL_TRFAC, UNIT_CARLYLE);

#endif	// /realistic start

#if 0
	DefB(BL_BRK, STRTABLE[STR_HOUSE1].rawstr().c_str(), "gui/brbut/barracks.png",
		Vec2i(1,1),  false,
		"sprites/bl/brk/brk",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		0,
		FLAG_BLOWN_FIRM,
		0, 0);
	BMat(BL_BRK, RES_CEMENT, 4);
	BMat(BL_BRK, RES_LABOUR, 4* ((1000))*(10*1));
	BMat(BL_BRK, RES_STONE, 2);
	//BOut(BL_HOUSE1, RES_HOUSING, 15, 600);
	//BDes(BL_HOUSE1, STRTABLE[STR_HOUSEDESC].rawstr().c_str());
#endif

	//TODO fail safe sprite loading smart

	//DefB(BL_HOUSE1, "Apartments", Vec2i(2,1), "sprites/bl/apartment1/basebuilding.ms3d", Vec3i(100,100,100), Vec3i(0,0,0), "sprites/bl/apartment1/basebuilding.ms3d", Vec3i(100,100,100), Vec3i(0,0,0), FD_LAND, RES_NONE);
	DefB(BL_HOUSE1, STRTABLE[STR_HOUSE1].rawstr().c_str(), "gui/brbut/apartment2.png",
		Vec2i(1,1),  false,
		//"sprites/2016/09/aaa000",
		"sprites/bl/house1/house1",
		30,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		0,
		FLAG_BLOWN_FIRM,
		110, RATIO_DENOM * 2*1,
		STOCKING_RATE);
	BMat(BL_HOUSE1, RES_CEMENT, 4);
	BMat(BL_HOUSE1, RES_LABOUR, 4* ((1000))*1);
	BMat(BL_HOUSE1, RES_STONE, 2);
	BIn(BL_HOUSE1, RES_ENERGY, 2 *(((10*1))));
	BOut(BL_HOUSE1, RES_HOUSING, 5*(((1*1))), 600);
	BDes(BL_HOUSE1, STRTABLE[STR_HOUSEDESC].rawstr().c_str());
	//BSon(BL_HOUSE1, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_HOUSE1, BLSND_PROD, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE1, BLSND_FINI, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE1, BLSND_SEL, "sounds/bl/doorbell1.wav");
	BSon(BL_HOUSE1, BLSND_CSEL, "sounds/bl/doorbell1.wav");

	DefB(BL_HOUSE2, STRTABLE[STR_HOUSE1].rawstr().c_str(), "gui/brbut/apartment2.png",
		Vec2i(2,2),  false,
		"sprites/bl/b1911/b1911",
		1,
		"sprites/bl/cstr2x2/cstr2x2",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		0,
		FLAG_BLOWN_FIRM,
		0, RATIO_DENOM * 6*1,
		STOCKING_RATE);
	BMat(BL_HOUSE2, RES_CEMENT, 40);
	BMat(BL_HOUSE2, RES_LABOUR, 40* ((1000))*1);
	BMat(BL_HOUSE2, RES_STONE, 20);
	BIn(BL_HOUSE2, RES_ENERGY, 1000*(((10*1))));
	BOut(BL_HOUSE2, RES_HOUSING, 150*(((1*1))), 600);
	BDes(BL_HOUSE2, STRTABLE[STR_HOUSEDESC].rawstr().c_str());
	//BSon(BL_HOUSE2, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_HOUSE2, BLSND_PROD, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE2, BLSND_FINI, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE2, BLSND_SEL, "sounds/bl/doorbell1.wav");
	BSon(BL_HOUSE2, BLSND_CSEL, "sounds/bl/doorbell1.wav");

#if 0
	DefB(BL_HOUSE2, STRTABLE[STR_HOUSE1].rawstr().c_str(),
		Vec2i(1,1),  false,
		"sprites/bl/house2/house2",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		0,
		FLAG_BLOWN_FIRM,
		14, RATIO_DENOM * 2*1);
	BMat(BL_HOUSE2, RES_CEMENT, 4);
	BMat(BL_HOUSE2, RES_LABOUR, 4);
	BMat(BL_HOUSE2, RES_STONE, 2);
	BOut(BL_HOUSE2, RES_HOUSING, 15, 600);
	BDes(BL_HOUSE2, STRTABLE[STR_HOUSEDESC].rawstr().c_str());
	BSon(BL_HOUSE2, BLSND_PROD, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE2, BLSND_FINI, "sounds/bl/doorbell1-short.wav");
	BSon(BL_HOUSE2, BLSND_SEL, "sounds/bl/doorbell1.wav");
	BSon(BL_HOUSE2, BLSND_CSEL, "sounds/bl/doorbell1.wav");
#endif

	DefB(BL_TRFAC, STRTABLE[STR_TRFAC].rawstr().c_str(), "gui/brbut/factory3.png",
		Vec2i(1,1),  false,
		"sprites/bl/trfac/trfac",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		DEFL_CSWAGE,
		FLAG_BLOWN_FIRM,
		3, RATIO_DENOM * 20*1,
		STOCKING_RATE);
	BMat(BL_TRFAC, RES_LABOUR, 4* ((1000))*1);
	BMat(BL_TRFAC, RES_METAL, 2);
	BMat(BL_TRFAC, RES_CEMENT, 3);
	//BMat(BL_TRFAC, RES_ELECTRONICS, 10);
	BIn(BL_TRFAC, RES_LABOUR, 10* ((1000))*(10*1));
	BIn(BL_TRFAC, RES_ENERGY, 100*(((10*1))));
	BIn(BL_TRFAC, RES_CHEMICALS, 3*(((10*1))));
	BIn(BL_TRFAC, RES_METAL, 182*(((10*1))));
	BOut(BL_TRFAC, RES_PRODUCTION, (1800/3)*(((10*1))), 30);
	BDes(BL_TRFAC, STRTABLE[STR_FACDESC].rawstr().c_str());
	//BSon(BL_TRFAC, BLSND_PROD, "sounds/notif/button-39.wav");
	//BSon(BL_TRFAC, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_TRFAC, BLSND_PROD, "sounds/bl/factory-short.wav");
	BSon(BL_TRFAC, BLSND_FINI, "sounds/bl/factory-short.wav");
	BSon(BL_TRFAC, BLSND_SEL, "sounds/bl/factory.wav");
	BSon(BL_TRFAC, BLSND_CSEL, "sounds/bl/factory.wav");
	BMan(BL_TRFAC, UNIT_TRUCK);
	//BMan(BL_TRFAC, UNIT_BATTLECOMP);
	//BMan(BL_TRFAC, UNIT_CARLYLE);

	DefB(BL_OILREF, STRTABLE[STR_OILREF].rawstr().c_str(), "gui/brbut/refinery2.png",
		Vec2i(1,1),  false,
		"sprites/bl/oilref/oilref",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE, 
		DEFL_CSWAGE,
		FLAG_BLOWN_FIRM,
		7, RATIO_DENOM * 2*1,
		STOCKING_RATE);
	BMat(BL_OILREF, RES_CEMENT, 5);
	BMat(BL_OILREF, RES_LABOUR, 10* ((1000))*1);
	BIn(BL_OILREF, RES_LABOUR, 10 * ((1000))*(10*1));
	BIn(BL_OILREF, RES_ENERGY, 5*(((10*1))));
	BIn(BL_OILREF, RES_CRUDEOIL, 45*(((10*1)))*3785);
	BOut(BL_OILREF, RES_WSFUEL, 42*(((10*1)))*3785, 1);
	BEmit(BL_OILREF, 0, PARTICLE_EXHAUST, Vec3i(0.42f*TILE_SIZE, 4.0f*TILE_SIZE, TILE_SIZE*-0.22f));
	BEmit(BL_OILREF, 1, PARTICLE_EXHAUST2, Vec3i(0.42f*TILE_SIZE, 4.0f*TILE_SIZE, TILE_SIZE*-0.22f));
	//BEmit(BL_OILREF, 2, PARTICLE_EXHAUST, Vec3i(TILE_SIZE*-4.5/10, TILE_SIZE*1.75, TILE_SIZE*3.0f/10));
	//BEmit(BL_OILREF, 3, PARTICLE_EXHAUST2, Vec3i(TILE_SIZE*-4.5/10, TILE_SIZE*1.75, TILE_SIZE*3.0f/10));
	BDes(BL_OILREF, STRTABLE[STR_OILREFDESC].rawstr().c_str());
	//BSon(BL_OILREF, BLSND_PROD, "sounds/notif/beep-23.wav");
	//BSon(BL_OILREF, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_OILREF, BLSND_PROD, "sounds/bl/oilref-short.wav");
	BSon(BL_OILREF, BLSND_FINI, "sounds/bl/oilref-short.wav");
	BSon(BL_OILREF, BLSND_SEL, "sounds/bl/oilref.wav");
	BSon(BL_OILREF, BLSND_CSEL, "sounds/bl/oilref.wav");

#if 0
	DefB(BL_OILREF, "Gas Station",
		Vec2i(2,2),  false, "sprites/bl/refinery2/refinery2",
		Vec3i(1,1,1), Vec3i(0,0,0), "sprites/bl/refinery2/refinery2",
		Vec3i(1,1,1), Vec3i(0,0,0), FD_LAND, RES_NONE, 1000);
	BMat(BL_OILREF, RES_CEMENT, 5);
	BMat(BL_OILREF, RES_LABOUR, 10);
	BIn(BL_OILREF, RES_ENERGY, 50);
	BIn(BL_OILREF, RES_WSFUEL, 5);
	BOut(BL_OILREF, RES_RETFUEL, 5);
	BDes(BL_OILREF, "Turn wholesale fuel into retail fuel, generated at refineries.");
#endif

	DefB(BL_COALPOW, STRTABLE[STR_COALPOW].rawstr().c_str(), "gui/brbut/coalpow.png",
		Vec2i(1,1), false,
		"sprites/bl/coalpow/coalpow",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		DEFL_CSWAGE,
		FLAG_BLOWN_STATE,
		10, RATIO_DENOM * 2*1,
		STOCKING_RATE*10);
	BMat(BL_COALPOW, RES_CEMENT, 5);
	BMat(BL_COALPOW, RES_LABOUR, 10* ((1000))*1);
	BIn(BL_COALPOW, RES_COAL, 5*(((10*1))));
	BIn(BL_COALPOW, RES_LABOUR, 5* ((1000))*(10*1));
	BOut(BL_COALPOW, RES_ENERGY, 500*(((10*1))), 4);
	BEmit(BL_COALPOW, 0, PARTICLE_EXHAUST2, Vec3i(-0.3f*TILE_SIZE, 3.3f*TILE_SIZE, 0.2f*TILE_SIZE));
	BEmit(BL_COALPOW, 1, PARTICLE_EXHAUST, Vec3i(-0.3f*TILE_SIZE, 3.3f*TILE_SIZE, 0.2f*TILE_SIZE));
	//BEmit(BL_COALPOW, 2, PARTICLE_EXHAUST2, Vec3i(-9.5f - 1, 23.4f, 10.6f - 1)*TILE_SIZE/32.0f*2.0f);
	//BEmit(BL_COALPOW, 3, PARTICLE_EXHAUST2, Vec3i(-9.9f + 1, 23.4f, -10.0f - 1)*TILE_SIZE/32.0f*2.0f);
	BDes(BL_COALPOW, STRTABLE[STR_COALPOWDESC].rawstr().c_str());
	//BSon(BL_COALPOW, BLSND_PROD, "sounds/notif/beep-24.wav");
	//BSon(BL_COALPOW, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_COALPOW, BLSND_PROD, "sounds/bl/coalpow2-short.wav");
	BSon(BL_COALPOW, BLSND_FINI, "sounds/bl/coalpow2-short.wav");
	BSon(BL_COALPOW, BLSND_SEL, "sounds/bl/coalpow2.wav");
	BSon(BL_COALPOW, BLSND_CSEL, "sounds/bl/coalpow2.wav");

	DefB(BL_CHEMPL, STRTABLE[STR_CHEMPL].rawstr().c_str(), "gui/brbut/chemplant.png",
		Vec2i(1,1), false,
		"sprites/bl/chempl/chempl",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		DEFL_CSWAGE,
		FLAG_BLOWN_FIRM,
		10, RATIO_DENOM * 2*1,
		STOCKING_RATE*3);
	BMat(BL_CHEMPL, RES_CEMENT, 5);
	BMat(BL_CHEMPL, RES_LABOUR, 10 * ((1000))*1);
	BIn(BL_CHEMPL, RES_IRONORE, 5*(((10*1))));
	BIn(BL_CHEMPL, RES_ENERGY, 5*(((10*1))));
	BIn(BL_CHEMPL, RES_LABOUR, 5  * ((1000))*(10*1));
	BOut(BL_CHEMPL, RES_CHEMICALS, 100*(((10*1))), 10);
	BDes(BL_CHEMPL, STRTABLE[STR_CHEMPLDESC].rawstr().c_str());
	//BSon(BL_CHEMPL, BLSND_PROD, "sounds/notif/beep-25.wav");
	//BSon(BL_CHEMPL, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_CHEMPL, BLSND_PROD, "sounds/bl/chempl2-short.wav");
	BSon(BL_CHEMPL, BLSND_FINI, "sounds/bl/chempl2-short.wav");
	BSon(BL_CHEMPL, BLSND_SEL, "sounds/bl/chempl2.wav");
	BSon(BL_CHEMPL, BLSND_CSEL, "sounds/bl/chempl2.wav");

#if 0
	DefB(BL_ELECPL, "Electronics Plant",
		Vec2i(1,1), true,
		"sprites/bl/cstr/cstr",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE);
	BMat(BL_ELECPL, RES_CEMENT, 5);
	BMat(BL_ELECPL, RES_LABOUR, 10);
	BIn(BL_ELECPL, RES_IRONORE, 5);
	BIn(BL_ELECPL, RES_ENERGY, 5);
	BIn(BL_ELECPL, RES_LABOUR, 5);
	BOut(BL_ELECPL, RES_ELECTRONICS, 10);
	BDes(BL_ELECPL, "Produces electronics necessary for units.");
	BSon(BL_ELECPL, BLSND_PROD, "sounds/notif/beep-26.wav");
	BSon(BL_ELECPL, BLSND_FINI, "sounds/notif/beep-22.wav");
#endif

	DefB(BL_GASST, STRTABLE[STR_GASSTN].rawstr().c_str(), "gui/brbut/gasstation2.png",
		Vec2i(1,1), false,
		"sprites/bl/gasstn/gasstn",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		DEFL_CSWAGE,
		FLAG_BLOWN_FIRM,
		9, RATIO_DENOM * 2*1,
		STOCKING_RATE);
	BMat(BL_GASST, RES_CEMENT, 5);
	BMat(BL_GASST, RES_LABOUR, 10* ((1000))*1);
	BIn(BL_GASST, RES_WSFUEL, 20*(((10*1)))*3785);
	BIn(BL_GASST, RES_ENERGY, 5*(((10*1))));
	BIn(BL_GASST, RES_LABOUR, 2* ((1000))*(10*1));
	BOut(BL_GASST, RES_RETFUEL, 20*(((10*1)))*3785, 100);
	BDes(BL_GASST, STRTABLE[STR_GASSTNDESC].rawstr().c_str());
	//BSon(BL_CEMPL, BLSND_PROD, "sounds/notif/beep-027.wav");
	//BSon(BL_CEMPL, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_GASST, BLSND_PROD, "sounds/bl/gasstn-short.wav");
	BSon(BL_GASST, BLSND_FINI, "sounds/bl/gasstn-short.wav");
	BSon(BL_GASST, BLSND_SEL, "sounds/bl/gasstn.wav");
	BSon(BL_GASST, BLSND_CSEL, "sounds/bl/gasstn.wav");

	DefB(BL_CEMPL, STRTABLE[STR_CEMPL].rawstr().c_str(), "gui/brbut/cemplant.png",
		Vec2i(1,1), false,
		"sprites/bl/cempl/cempl",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		 DEFL_CSWAGE,
		FLAG_BLOWN_FIRM,
		9, RATIO_DENOM * 2*1,
		STOCKING_RATE);
	BMat(BL_CEMPL, RES_CEMENT, 5);
	BMat(BL_CEMPL, RES_LABOUR, 10* ((1000))*1);
	BIn(BL_CEMPL, RES_STONE, 5*(((10*1))));
	BIn(BL_CEMPL, RES_ENERGY, 5*(((10*1))));
	BIn(BL_CEMPL, RES_LABOUR, 5* ((1000))*(10*1));
	BOut(BL_CEMPL, RES_CEMENT, 10*(((10*1))), 100);
	BDes(BL_CEMPL, STRTABLE[STR_CEMPLDESC].rawstr().c_str());
	//BSon(BL_CEMPL, BLSND_PROD, "sounds/notif/beep-027.wav");
	//BSon(BL_CEMPL, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_CEMPL, BLSND_PROD, "sounds/bl/cempl-short.wav");
	BSon(BL_CEMPL, BLSND_FINI, "sounds/bl/cempl-short.wav");
	BSon(BL_CEMPL, BLSND_SEL, "sounds/bl/cempl.wav");
	BSon(BL_CEMPL, BLSND_CSEL, "sounds/bl/cempl.wav");

#if 0
#if 1
	DefB(BL_QUARRY, "Quarry",
		Vec2i(1,1), true,
		"sprites/bl/quarry/quarry.ms3d",
		Vec3i(1,1,1)/32.0f*TILE_SIZE, Vec3i(0,0,0),
		"sprites/bl/quarry/quarry.ms3d",
		Vec3i(1,1,1)/32.0f*TILE_SIZE, Vec3i(0,0,0),
		FD_LAND, RES_NONE, 1000,
		 DEFL_CSWAGE);
#else
	DefB(BL_QUARRY, "Quarry",
		Vec2i(1,1), false,
		"sprites/bl/box/quarry/basebuilding.ms3d",
		Vec3i(1,1,1)*100, Vec3i(0,0,0),
		"sprites/bl/box/quarry_c/basebuilding.ms3d",
		Vec3i(1,1,1)*100, Vec3i(0,0,0),
		FD_LAND, RES_NONE, 1000);
#endif
	BMat(BL_QUARRY, RES_CEMENT, 5);
	BMat(BL_QUARRY, RES_LABOUR, 10);
	BIn(BL_QUARRY, RES_ENERGY, 5);
	BIn(BL_QUARRY, RES_LABOUR, 5);
	BOut(BL_QUARRY, RES_STONE, 10);
	BDes(BL_QUARRY, "Extracts stone.");
	BSon(BL_QUARRY, BLSND_PROD, "sounds/notif/beep-28.wav");
	BSon(BL_QUARRY, BLSND_FINI, "sounds/notif/beep-22.wav");
#endif

	DefB(BL_IRONSM, STRTABLE[STR_IRONSM].rawstr().c_str(), "gui/brbut/smelter.png",
		Vec2i(1,1), false,
		"sprites/bl/ironsm/ironsm",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		 DEFL_CSWAGE,
		FLAG_BLOWN_FIRM,
		11, RATIO_DENOM * 2*1,
		STOCKING_RATE*2);
	BMat(BL_IRONSM, RES_LABOUR, 4* ((1000))*1);
	BMat(BL_IRONSM, RES_CEMENT, 3);
	BMat(BL_IRONSM, RES_STONE, 1);
	BIn(BL_IRONSM, RES_LABOUR, 10* ((1000))*(10*1));
	BIn(BL_IRONSM, RES_IRONORE, 125*(((10*1))));
	BIn(BL_IRONSM, RES_CHEMICALS, 10*(((10*1))));
	BOut(BL_IRONSM, RES_METAL, 125*(((10*1))), 1);
	BDes(BL_IRONSM, STRTABLE[STR_IRONSMDESC].rawstr().c_str());
	//BSon(BL_IRONSM, BLSND_PROD, "sounds/notif/beep-29.wav");
	//BSon(BL_IRONSM, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_IRONSM, BLSND_PROD, "sounds/bl/ironsm-short.wav");
	BSon(BL_IRONSM, BLSND_FINI, "sounds/bl/ironsm-short.wav");
	BSon(BL_IRONSM, BLSND_SEL, "sounds/bl/ironsm.wav");
	BSon(BL_IRONSM, BLSND_CSEL, "sounds/bl/ironsm.wav");

	DefB(BL_NUCPOW, STRTABLE[STR_NUCPOW].rawstr().c_str(), "gui/brbut/nucpow2.png",
		Vec2i(1,1), false,
		"sprites/bl/nucpow/nucpow",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		 DEFL_CSWAGE,
		FLAG_BLOWN_STATE,
		1, RATIO_DENOM * 20*1,
		STOCKING_RATE*10);
	BMat(BL_NUCPOW, RES_CEMENT, 5);
	BMat(BL_NUCPOW, RES_LABOUR, 10* ((1000))*1);
	BIn(BL_NUCPOW, RES_URANIUM, 5*(((10*1))));
	BIn(BL_NUCPOW, RES_LABOUR, 5* ((1000))*(10*1));
	BOut(BL_NUCPOW, RES_ENERGY, 400*(((10*1))), 4);
	BEmit(BL_NUCPOW, 0, PARTICLE_EXHAUSTBIG, Vec3i(TILE_SIZE*-0.25f, TILE_SIZE*1.0f, TILE_SIZE*-0.25f));
	BEmit(BL_NUCPOW, 1, PARTICLE_EXHAUSTBIG, Vec3i(TILE_SIZE*0.25f, TILE_SIZE*1.0f, TILE_SIZE*-0.25f));
	BDes(BL_NUCPOW, STRTABLE[STR_NUCPOWDESC].rawstr().c_str());
	//BSon(BL_NUCPOW, BLSND_PROD, "sounds/notif/beep-30b.wav");
	//BSon(BL_NUCPOW, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_NUCPOW, BLSND_PROD, "sounds/bl/nucpow2-short.wav");
	BSon(BL_NUCPOW, BLSND_FINI, "sounds/bl/nucpow2-short.wav");
	BSon(BL_NUCPOW, BLSND_SEL, "sounds/bl/nucpow2.wav");
	BSon(BL_NUCPOW, BLSND_CSEL, "sounds/bl/nucpow2.wav");

	DefB(BL_FARM, STRTABLE[STR_FARM].rawstr().c_str(), "gui/brbut/farm2.png",
		Vec2i(1,1), true,
		"sprites/bl/farm/farm",
		1,
		"sprites/bl/farm/farm_c",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		 DEFL_CSWAGE,
		FLAG_BLOWN_FIRM,
		83, RATIO_DENOM * 2*1,
		STOCKING_RATE/STOCKING_RATE*RATIO_DENOM);
	BMat(BL_FARM, RES_LABOUR, 4* ((1000))*1);
	BIn(BL_FARM, RES_LABOUR, 1 * ((1000))*(1*1) / 100);
	BIn(BL_FARM, RES_ENERGY, 0*(((10*1))));
	BIn(BL_FARM, RES_CHEMICALS, 0*(((10*1))));
	BOut(BL_FARM, RES_FARMPRODUCTS, 684*(((1*1))), 1);
	BDes(BL_FARM, STRTABLE[STR_FARMDESC].rawstr().c_str());
	//BSon(BL_FARM, BLSND_PROD, "sounds/notif/button-16.wav");
	//BSon(BL_FARM, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_FARM, BLSND_PROD, "sounds/bl/farm-short.wav");
	BSon(BL_FARM, BLSND_FINI, "sounds/bl/farm-short.wav");
	BSon(BL_FARM, BLSND_SEL, "sounds/bl/farm.wav");
	BSon(BL_FARM, BLSND_CSEL, "sounds/bl/farm.wav");

	DefB(BL_STORE, STRTABLE[STR_STORE].rawstr().c_str(), "gui/brbut/store1.png",
		Vec2i(1,1), false,
		"sprites/bl/store/store",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_NONE, 1000,
		5*TILE_SIZE,
		 DEFL_CSWAGE,
		 FLAG_BLOWN_FIRM,
		 4, RATIO_DENOM * 9*1,
		STOCKING_RATE * 10);
	BMat(BL_STORE, RES_CEMENT, 5);
	BMat(BL_STORE, RES_LABOUR, 10* ((1000))*1);
	BIn(BL_STORE, RES_LABOUR, 10* ((1000))*(10*1));
	BIn(BL_STORE, RES_ENERGY, 10*(((10*1))));
	//BIn(BL_STORE, RES_FARMPRODUCTS, 3600);
	BIn(BL_STORE, RES_FARMPRODUCTS, 210000*(((10*1))));
	BIn(BL_STORE, RES_PRODUCTION, 600);
	BIn(BL_STORE, RES_ENERGY, 600);
	BOut(BL_STORE, RES_RETFOOD, 210000*(((10*1))), 5);
	//BOut(BL_STORE, RES_RETFOOD, 2000, 105);
	//BOut(BL_STORE, RES_RETFOOD, 20 * CYCLE_FRAMES/FOOD_CONSUM_DELAY_FRAMES * LABOURER_FOODCONSUM , DEFL_CSWAGE * STARTING_LABOUR / 2 / (20 * CYCLE_FRAMES/FOOD_CONSUM_DELAY_FRAMES * LABOURER_FOODCONSUM));
	BDes(BL_STORE, STRTABLE[STR_STOREDESC].rawstr().c_str());
	//BSon(BL_STORE, BLSND_PROD, "sounds/notif/button-19.wav");
	//BSon(BL_STORE, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_STORE, BLSND_PROD, "sounds/bl/store-short.wav");
	BSon(BL_STORE, BLSND_FINI, "sounds/bl/store-short.wav");
	BSon(BL_STORE, BLSND_SEL, "sounds/bl/store.wav");
	BSon(BL_STORE, BLSND_CSEL, "sounds/bl/store.wav");

	DefB(BL_OILWELL, STRTABLE[STR_OILWELL].rawstr().c_str(), "gui/brbut/oilwell2.png",
		Vec2i(1,1), false,
		"sprites/bl/oilwell/oilwell",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, RES_CRUDEOIL, 1000,
		5*TILE_SIZE,
		 DEFL_CSWAGE,
		 FLAG_BLOWN_FIRM,
		 16, RATIO_DENOM * 2*1,
		STOCKING_RATE);
	BMat(BL_OILWELL, RES_CEMENT, 5);
	BMat(BL_OILWELL, RES_LABOUR, 1* ((1000))*1);
	BIn(BL_OILWELL, RES_ENERGY, 5*(((10*1))));
	BIn(BL_OILWELL, RES_LABOUR, 5* ((1000))*(10*1));
	BOut(BL_OILWELL, RES_CRUDEOIL, 10*(((10*1)))*3785, 1);
	BDes(BL_OILWELL, STRTABLE[STR_OILWELLDESC].rawstr().c_str());
	//BSon(BL_OILWELL, BLSND_PROD, "sounds/notif/button-31.wav");
	//BSon(BL_OILWELL, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_OILWELL, BLSND_PROD, "sounds/bl/oilwell-short.wav");
	BSon(BL_OILWELL, BLSND_FINI, "sounds/bl/oilwell-short.wav");
	BSon(BL_OILWELL, BLSND_SEL, "sounds/bl/oilwell.wav");
	BSon(BL_OILWELL, BLSND_CSEL, "sounds/bl/oilwell.wav");

	DefB(BL_SHMINE, STRTABLE[STR_SHMINE].rawstr().c_str(), "gui/brbut/mine.png",
		Vec2i(1,1), false,
		"sprites/bl/shmine/shmine",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, -1, 1000,
		5*TILE_SIZE,
		 DEFL_CSWAGE,
		 FLAG_BLOWN_FIRM,
		 10, RATIO_DENOM * 2*1,
		STOCKING_RATE);
	BMat(BL_SHMINE, RES_LABOUR, 4* ((1000))*1);
	BMat(BL_SHMINE, RES_CEMENT, 3);
	//BIn(BL_SHMINE, RES_LABOUR, 10);
	BIn(BL_SHMINE, RES_LABOUR, 5* ((1000))*(10*1));
	BOut(BL_SHMINE, RES_IRONORE, 70*(((10*1))), 100);
	BOut(BL_SHMINE, RES_STONE, 100*(((10*1))), 100);
	BOut(BL_SHMINE, RES_URANIUM, 50*(((10*1))), 100);
	BOut(BL_SHMINE, RES_COAL, 50*(((10*1))), 100);
	BDes(BL_SHMINE, STRTABLE[STR_SHMINEDESC].rawstr().c_str());
	//BSon(BL_SHMINE, BLSND_PROD, "sounds/notif/button-32.wav");
	//BSon(BL_SHMINE, BLSND_FINI, "sounds/notif/beep-22.wav");
	BSon(BL_SHMINE, BLSND_PROD, "sounds/bl/shovel-short.wav");
	BSon(BL_SHMINE, BLSND_FINI, "sounds/bl/shovel-short.wav");
	BSon(BL_SHMINE, BLSND_SEL, "sounds/bl/shovel.wav");
	BSon(BL_SHMINE, BLSND_CSEL, "sounds/bl/shovel.wav");
	
#if 1
	DefB(BL_BANK, STRTABLE[STR_BANK].rawstr().c_str(), "gui/brbut/bank.png",
		Vec2i(1,1), false,
		"sprites/bl/bank/bank",
		1,
		"sprites/bl/cstr/cstr",
		1,
		FD_LAND, -1, 1000,
		5*TILE_SIZE,
		 DEFL_CSWAGE,
		 FLAG_BLOWN_FIRM,
		 1, RATIO_DENOM * 2*1,
		STOCKING_RATE);
	BMat(BL_BANK, RES_LABOUR, 4* ((1000))*(10*1));
	BMat(BL_BANK, RES_CEMENT, 3);
	//BIn(BL_BANK, RES_LABOUR, 10);
	//BIn(BL_BANK, RES_LABOUR, 5* ((1000))*(10*1));
	//BOut(BL_BANK, RES_IRONORE, 7, 100);
	//BOut(BL_BANK, RES_STONE, 10, 100);
	//BOut(BL_BANK, RES_URANIUM, 5, 100);
	//BOut(BL_BANK, RES_COAL, 5, 100);
	BDes(BL_BANK, STRTABLE[STR_BANKDESC].rawstr().c_str());
	//BSon(BL_SHMINE, BLSND_PROD, "sounds/notif/button-32.wav");
	//BSon(BL_SHMINE, BLSND_FINI, "sounds/notif/beep-22.wav");
	//BSon(BL_BANK, BLSND_PROD, "sounds/bl/shovel-short.wav");
	//BSon(BL_BANK, BLSND_FINI, "sounds/bl/shovel-short.wav");
	//BSon(BL_BANK, BLSND_SEL, "sounds/bl/shovel.wav");
	//BSon(BL_BANK, BLSND_CSEL, "sounds/bl/shovel.wav");
#endif

	// Conduit types

	//TODO shouldn't need offsetof(..) for modding

	DefCd(CD_ROAD, STRTABLE[STR_ROAD].rawstr().c_str(),  "gui/brbut/road.png",
		offsetof(Building,roadnetw), offsetof(Selection,roads),
		false, false, Vec2i(TILE_SIZE/2, TILE_SIZE/2), Vec3i(-TILE_SIZE, -TILE_SIZE, 0),
		"gui/hover/noroad.png",
		2, 2,
		false,
		 FLAG_BLOWN_STATE);
	CdDes(CD_ROAD, STRTABLE[STR_ROADDESC].rawstr().c_str());
	CdMat(CD_ROAD, RES_LABOUR, 1* ((1000))*1);
	CdMat(CD_ROAD, RES_CEMENT, 1);
#if 1
	DefConn(CD_ROAD, CONNECTION_NOCONNECTION, CONSTRUCTION, "sprites/cd/road/1_c/1_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTH, CONSTRUCTION, "sprites/cd/road/n_c/n_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_EAST, CONSTRUCTION, "sprites/cd/road/e_c/e_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_SOUTH, CONSTRUCTION, "sprites/cd/road/s_c/s_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_WEST, CONSTRUCTION, "sprites/cd/road/w_c/w_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHEAST, CONSTRUCTION, "sprites/cd/road/ne_c/ne_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHSOUTH, CONSTRUCTION, "sprites/cd/road/ns_c/ns_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_EASTSOUTH, CONSTRUCTION, "sprites/cd/road/es_c/es_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHWEST, CONSTRUCTION, "sprites/cd/road/nw_c/nw_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_EASTWEST, CONSTRUCTION, "sprites/cd/road/ew_c/ew_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_SOUTHWEST, CONSTRUCTION, "sprites/cd/road/sw_c/sw_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "sprites/cd/road/esw_c/esw_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "sprites/cd/road/nsw_c/nsw_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHEASTWEST, CONSTRUCTION, "sprites/cd/road/new_c/new_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "sprites/cd/road/nes_c/nes_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "sprites/cd/road/nesw_c/nesw_c", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NOCONNECTION, FINISHED, "sprites/cd/road/1/1", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTH, FINISHED, "sprites/cd/road/n/n", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_EAST, FINISHED, "sprites/cd/road/e/e", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_SOUTH, FINISHED, "sprites/cd/road/s/s", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_WEST, FINISHED, "sprites/cd/road/w/w", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHEAST, FINISHED, "sprites/cd/road/ne/ne", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHSOUTH, FINISHED, "sprites/cd/road/ns/ns", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_EASTSOUTH, FINISHED, "sprites/cd/road/es/es", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHWEST, FINISHED, "sprites/cd/road/nw/nw", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_EASTWEST, FINISHED, "sprites/cd/road/ew/ew", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_SOUTHWEST, FINISHED, "sprites/cd/road/sw/sw", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_EASTSOUTHWEST, FINISHED, "sprites/cd/road/esw/esw", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHSOUTHWEST, FINISHED, "sprites/cd/road/nsw/nsw", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHEASTWEST, FINISHED, "sprites/cd/road/new/new", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHEASTSOUTH, FINISHED, "sprites/cd/road/nes/nes", RENDER_MODEL);
	DefConn(CD_ROAD, CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "sprites/cd/road/nesw/nesw", RENDER_MODEL);
#endif

#if 1
	DefCd(CD_POWL, STRTABLE[STR_POWL].rawstr().c_str(), "gui/brbut/powerline.png",
		offsetof(Building,pownetw), offsetof(Selection,powls),
		true, true, Vec2i(TILE_SIZE, TILE_SIZE), Vec3i(0, 0, 0),
		"gui/hover/noelec.png",
		2, 2,
		true,
		 FLAG_BLOWN_STATE);
	CdDes(CD_POWL, STRTABLE[STR_POWLDESC].rawstr().c_str());
	CdMat(CD_POWL, RES_LABOUR, 1* ((1000))*1);
	CdMat(CD_POWL, RES_CEMENT, 1);
	DefConn(CD_POWL, CONNECTION_NOCONNECTION, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTH, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_EAST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_SOUTH, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_WEST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHEAST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHSOUTH, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_EASTSOUTH, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHWEST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_EASTWEST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_SOUTHWEST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHEASTWEST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NOCONNECTION, FINISHED, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTH, FINISHED, "sprites/cd/powl/n/n", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_EAST, FINISHED, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_SOUTH, FINISHED, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_WEST, FINISHED, "sprites/cd/powl/w/w", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHEAST, FINISHED, "sprites/cd/powl/n/n", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHSOUTH, FINISHED, "sprites/cd/powl/n/n", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_EASTSOUTH, FINISHED, "sprites/cd/powl/1/1", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHWEST, FINISHED, "sprites/cd/powl/nw/nw", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_EASTWEST, FINISHED, "sprites/cd/powl/w/w", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_SOUTHWEST, FINISHED, "sprites/cd/powl/w/w", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_EASTSOUTHWEST, FINISHED, "sprites/cd/powl/w/w", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHSOUTHWEST, FINISHED, "sprites/cd/powl/nw/nw", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHEASTWEST, FINISHED, "sprites/cd/powl/nw/nw", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHEASTSOUTH, FINISHED, "sprites/cd/powl/n/n", RENDER_UNSPEC);
	DefConn(CD_POWL, CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "sprites/cd/powl/nw/nw", RENDER_UNSPEC);
#endif

	DefCd(CD_CRPIPE, STRTABLE[STR_CRPIPE].rawstr().c_str(), "gui/brbut/crudepipeline.png",
		offsetof(Building,crpipenetw), offsetof(Selection,crpipes),
		true, true, Vec2i(TILE_SIZE, TILE_SIZE), Vec3i(0, 0, 0),
		"gui/hover/nocrude.png",
		2, 2,
		true,
		 FLAG_BLOWN_FIRM);
	CdDes(CD_CRPIPE, STRTABLE[STR_CRPIPEDESC].rawstr().c_str());
	CdMat(CD_CRPIPE, RES_LABOUR, 1* ((1000))*1);
	CdMat(CD_CRPIPE, RES_CEMENT, 1);
#if 1
	DefConn(CD_CRPIPE, CONNECTION_NOCONNECTION, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTH, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_EAST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_SOUTH, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_WEST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHEAST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHSOUTH, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_EASTSOUTH, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHWEST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_EASTWEST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_SOUTHWEST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHEASTWEST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NOCONNECTION, FINISHED, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTH, FINISHED, "sprites/cd/crpipe/n/n", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_EAST, FINISHED, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_SOUTH, FINISHED, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_WEST, FINISHED, "sprites/cd/crpipe/w/w", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHEAST, FINISHED, "sprites/cd/crpipe/n/n", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHSOUTH, FINISHED, "sprites/cd/crpipe/n/n", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_EASTSOUTH, FINISHED, "sprites/cd/crpipe/1/1", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHWEST, FINISHED, "sprites/cd/crpipe/nw/nw", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_EASTWEST, FINISHED, "sprites/cd/crpipe/w/w", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_SOUTHWEST, FINISHED, "sprites/cd/crpipe/w/w", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_EASTSOUTHWEST, FINISHED, "sprites/cd/crpipe/w/w", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHSOUTHWEST, FINISHED, "sprites/cd/crpipe/nw/nw", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHEASTWEST, FINISHED, "sprites/cd/crpipe/nw/nw", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHEASTSOUTH, FINISHED, "sprites/cd/crpipe/n/n", RENDER_UNSPEC);
	DefConn(CD_CRPIPE, CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "sprites/cd/crpipe/nw/nw", RENDER_UNSPEC);
#endif
#endif

	// Sounds

#if 0
#define TRSND_NEWJOB	0
#define TRSND_DONEJOB	1
#define TRSND_WORK		2
#define TR_SOUNDS		3

extern int16_t g_trsnd[TR_SOUNDS];
#endif

//TODO make moddable based on unit types defined in mod

	//LoadSound("sounds/notif/button-33a.wav", &g_trsnd[TRSND_NEWJOB]);
	//LoadSound("sounds/notif/button-34.wav", &g_trsnd[TRSND_DONEJOB]);
	//LoadSound("sounds/notif/button-35.wav", &g_trsnd[TRSND_WORK]);
	LoadSound("sounds/truck/truck.wav", &g_trsnd[TRSND_NEWJOB]);
	LoadSound("sounds/truck/truck.wav", &g_trsnd[TRSND_DONEJOB]);
	LoadSound("sounds/truck/truck.wav", &g_trsnd[TRSND_WORK]);

	//LoadSound("sounds/notif/button-37.wav", &g_labsnd[LABSND_WORK]);
	LoadSound("sounds/notif/button-41.wav", &g_labsnd[LABSND_WORK]);
	LoadSound("sounds/notif/button-38.wav", &g_labsnd[LABSND_SHOP]);
	LoadSound("sounds/notif/beep-22.wav", &g_labsnd[LABSND_REST]);
	
	LoadSound("sounds/notif/chat.wav", &g_chat);

	LoadSound("sounds/notif/button-32.wav", &g_beep);

#if 0
	g_ordersnd.clear();
	g_ordersnd.push_back(Sound("sounds/aaa000/gogogo.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/moveout2.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/spreadout.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/wereunderattack3.wav"));
	//g_zpainSnd.push_back(Sound("sounds/zpain.wav"));
#endif
#if 0
	g_ordersnd.clear();
	g_ordersnd.push_back(Sound());
	g_ordersnd[0] = Sound("sounds/aaa000/gogogo.wav");
#endif
}
