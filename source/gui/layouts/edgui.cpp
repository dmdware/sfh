










#include "../../app/appmain.h"
#include "../gui.h"
#include "../keymap.h"
#include "../../render/heightmap.h"
#include "../../math/camera.h"
#include "../../render/screenshot.h"
#include "../../save/savemap.h"
#include "appgui.h"
#include "../../sim/utype.h"
#include "edgui.h"
#include "../../save/savemap.h"
#include "../../sim/bltype.h"
#include "../../sim/player.h"
#include "messbox.h"
#include "../../path/collidertile.h"
#include "../../render/foliage.h"
#include "../../sim/unit.h"
#include "../../sim/utype.h"
#include "../../sim/bltype.h"
#include "../../sim/build.h"
#include "../../sim/building.h"
#include "../../sim/map.h"
#include "../../math/hmapmath.h"
#include "../../math/isomath.h"
#include "../../net/client.h"
#include "../../algo/random.h"
#include "../../trigger/condition.h"
#include "../../trigger/trigger.h"
#include "../../language.h"
#include "../../sim/border.h"
#include "../../sim/simdef.h"
#include "playgui.h"
#include "../../app/segui.h"

void Resize_ToolCat(Widget* w)
{
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + (g_font[w->m_font].gheight + 5) * 0;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + g_font[w->m_font].gheight;
}

void Resize_ToolAct(Widget* w)
{
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + (g_font[w->m_font].gheight + 5) * 1;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + g_font[w->m_font].gheight;
}

int32_t EdToolsSelection()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	ViewLayer* ed = (ViewLayer*)gui->get("ed");

	Widget* tool = ed->get("tool cat");

	int32_t selected = tool->m_selected;

	return selected;
}

void CloseEdTools()
{
	GUI* gui = &g_gui;
	ViewLayer* ed = (ViewLayer*)gui->get("ed");

	gui->hide("ed units");
	gui->hide("ed buildings");
	gui->hide("ed conduits");
	gui->hide("ed resources");
	gui->hide("ed triggers");
	gui->hide("ed borders");
	gui->hide("ed scripts");
	gui->hide("ed newmap");
    gui->hide("ed foliage");
    ed->hide("act layer");
    ed->hide("elev layer");
    ed->hide("owner layer");
    ed->hide("owner layer 2");
	gui->hide("ed terrain");

	g_build = BL_NONE;
    for(int32_t ctype=0; ctype<CD_TYPES; ctype++)
        ClearCdPlans(ctype);
}

int32_t GetEdTool()
{
	GUI* gui = &g_gui;

	ViewLayer* ed = (ViewLayer*)gui->get("ed");

	Widget* tool = ed->get("tool cat");

	int32_t selected = tool->m_selected;

	return selected;
}

int32_t GetEdAct()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	ViewLayer* ed = (ViewLayer*)gui->get("ed");
	ViewLayer* actlayer = (ViewLayer*)ed->get("act layer");

	Widget* tool = actlayer->get("tool act");

	int32_t act = tool->m_selected;

	return act;
}

int32_t GetElevAct()
{
    Player* py = &g_player[g_localP];
    GUI* gui = &g_gui;
    
    ViewLayer* ed = (ViewLayer*)gui->get("ed");
    ViewLayer* actlayer = (ViewLayer*)ed->get("elev layer");
    
    Widget* tool = actlayer->get("elev act");
    
    int32_t act = tool->m_selected;
    
    return act;
}

void RegenEdPy()
{
	GUI* gui = &g_gui;
	ViewLayer* view = (ViewLayer*)gui->get("ed players");
	DropList* itc = (DropList*)view->get("insttype check");
	
	DropList* pysel = (DropList*)view->get("py sel");
	int32_t pyi = pysel->m_selected;

	if(pyi < 0)
		return;

	Player* py = &g_player[pyi];
	itc->m_selected = py->insttype;
	
	DropList* pysel2 = (DropList*)view->get("py sel 2");
	pysel2->m_selected = py->parentst;

	CheckBox* oncheck = (CheckBox*)view->get("on check");
	oncheck->m_selected = py->on ? 1 : 0;
	
	CheckBox* aicheck = (CheckBox*)view->get("ai check");
	aicheck->m_selected = py->ai ? 1 : 0;
}

void Change_ToolCat()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* ed = (ViewLayer*)gui->get("ed");
	int32_t selected = GetEdTool();

	int32_t act = GetEdAct();
	CloseEdTools();
	
	switch(selected)
	{
	case TOOLCAT_NEWMAP:
		gui->get("ed newmap")->show();
		break;
	case TOOLCAT_ELEVATION:
        ed->get("elev layer")->show();
		break;
	case TOOLCAT_TERRAIN:
		//gui->get("ed terrain")->show();
		break;
	case TOOLCAT_FOLIAGE:
		//gui->get("ed foliage")->show();
		ed->get("act layer")->show();
		break;
	case TOOLCAT_UNITS:
		gui->get("ed units")->show();
        ed->get("act layer")->show();
        ed->get("owner layer")->show();
		break;
	case TOOLCAT_BUILDINGS:
		gui->get("ed buildings")->show();
        ed->get("act layer")->show();
        ed->get("owner layer")->show();
		if(act == TOOLACT_PLACE)
			g_build = GetEdBlType();
		break;
	case TOOLCAT_CONDUITS:
		gui->get("ed conduits")->show();
        ed->get("act layer")->show();
        ed->get("owner layer")->show();
		break;
	case TOOLCAT_RESOURCES:
        //gui->get("ed resources")->show();
        ed->get("owner layer")->show();
		break;
	case TOOLCAT_TRIGGERS:
		//gui->get("ed triggers")->show();
		break;
	case TOOLCAT_BORDERS:
		//gui->get("ed borders")->show();
        ed->get("owner layer")->show();
		break;
	case TOOLCAT_PLAYERS:
        ed->get("owner layer")->show();
        ed->get("owner layer 2")->show();
        gui->get("ed players")->show();
		RegenEdPy();
		break;
	case TOOLCAT_SCRIPTS:
		//gui->get("ed scripts")->show();
		break;
	default:
		break;
	};

#if 0
	// Delete Objects
	else if(selected == TOOLCAT_DELETEOBJECTS)
	{
		gui->show("delete objects");
	}
	else if(selected == TOOLCAT_PLACEROADS)
	{
		g_build = BL_ROAD;
	}
	else if(selected == TOOLCAT_PLACECRUDEPIPES)
	{
		g_build = BL_CRPIPE;
	}
	else if(selected == TOOLCAT_PLACEPOWERLINES)
	{
		g_build = BL_POWL;
	}
#endif
#if 0
#define TOOLCAT_PLACEROADS		4
#define TOOLCAT_PLACEPOWERLINES	5
#define TOOLCAT_PLACECRUDEPIPES	6
#endif
}

void Resize_UnitDropDownSelector(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 1;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_PySelector(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 2;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_EdPy_OnCheck(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 0;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_EdPy_AICheck(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 1;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_PySelector2(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 4;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_UnitCompanyDropDownSelector(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 3;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_BuildingDropDownSelector(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 1;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Change_UnitDropDownSelector()
{
}

void FillEdUnits()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->add(new ViewLayer(gui, "ed units"));
	ViewLayer* uview = (ViewLayer*)gui->get("ed units");

	uview->add(new DropList(uview, "utype", MAINFONT16, Resize_UnitDropDownSelector, NULL));

	Widget* utype = uview->get("utype");

	for(int32_t i=0; i<UNIT_TYPES; i++)
		utype->m_options.push_back(RichText(g_utype[i].name));

	//utype->m_options.push_back(RichText("Labourer"));
	//utype->m_options.push_back(RichText("Truck"));
	//utype->m_options.push_back(RichText("Infantry"));

	//utype->select(0);

#if 0
	uview->add(new DropList(uview, "unit owner selector", MAINFONT16, Resize_UnitCountryDropDownSelector, NULL));

	Widget* unitcountryselector = uview->get("unit owner selector");

	//unitcountryselector->m_options.push_back(RichText("No player"));
	for(int32_t i=0; i<PLAYERS; i++)
	{
		char num[128];
		sprintf(num, "%d. ", i);
		unitcountryselector->m_options.push_back(RichText(num) + g_player[i].name);
	}
#endif

#if 0
	uview->add(new DropList(NULL, "unit company selector", MAINFONT8, Resize_UnitCompanyDropDownSelector, NULL));

	Widget* unitcompanyselector = uview->get("unit company selector");

	unitcompanyselector->m_options.push_back(RichText("No cmpany"));
#endif
}

int32_t GetEdUType()
{
	GUI* gui = &g_gui;

	ViewLayer* uview = (ViewLayer*)gui->get("ed units");

	Widget* utype = uview->get("utype");

	return utype->m_selected;
}

int32_t GetPlaceOwner()
{
	GUI* gui = &g_gui;
    
    ViewLayer* ed = (ViewLayer*)gui->get("ed");
	ViewLayer* view = (ViewLayer*)ed->get("owner layer");

	Widget* pysel = view->get("py sel");

	return pysel->m_selected;
}

int32_t GetPlaceOwner2()
{
	GUI* gui = &g_gui;
    
    ViewLayer* ed = (ViewLayer*)gui->get("ed");
	ViewLayer* view = (ViewLayer*)ed->get("owner layer 2");

	Widget* pysel = view->get("py sel");

	return pysel->m_selected;
}

int32_t GetEdCdType()
{
    GUI* gui = &g_gui;
    
    ViewLayer* view = (ViewLayer*)gui->get("ed conduits");
    
    Widget* csel = view->get("cd sel");
    
    return csel->m_selected;
}

int32_t GetPlaceUnitCompany()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	ViewLayer* uview = (ViewLayer*)gui->get("ed units");

	Widget* unitcompanyselector = uview->get("unit company selector");

	return unitcompanyselector->m_selected - 1;
}

void Resize_EdNewMap_Table(Widget* w)
{
#if 0
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 20;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + g_font[w->m_font].gheight + 5;
#endif

	BmpFont* f = &g_font[MAINFONT16];

	std::string* name = &w->m_name;
	int32_t row, col;
	sscanf(name->c_str(), "%d %d", &col, &row);
	
	w->m_pos[0] = LEFT_PANEL_WIDTH / 2 * col;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 32 + (f->gheight + 5) * row;
	w->m_pos[2] = LEFT_PANEL_WIDTH / 2 * (col+1);
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Click_EdNewMap_Gen()
{
	GUI* gui = &g_gui;
	ViewLayer* mapview = (ViewLayer*)gui->get("ed newmap");
	
	EditBox* sbox = (EditBox*)mapview->get("1 3");
	//EditBox* s1box = (EditBox*)mapview->get("1 4");
	//EditBox* s2box = (EditBox*)mapview->get("1 5");
	EditBox* xbox = (EditBox*)mapview->get("1 0");
	EditBox* ybox = (EditBox*)mapview->get("1 1");
	CheckBox* rbox = (CheckBox*)mapview->get("0 2");
	
	std::string sstr = sbox->m_value.rawstr();
	//std::string s1str = s1box->m_value.rawstr();
	//std::string s2str = s2box->m_value.rawstr();
	std::string xstr = xbox->m_value.rawstr();
	std::string ystr = ybox->m_value.rawstr();
	
	uint32_t seed;
	uint32_t seed1;
	uint32_t seed2;
	int32_t widx;
	int32_t widy;
	
	sscanf(sstr.c_str(), "%u", &seed);
	//sscanf(s1str.c_str(), "%u", &seed1);
	//sscanf(s2str.c_str(), "%u", &seed2);
	sscanf(xstr.c_str(), "%d", &widx);
	sscanf(ystr.c_str(), "%d", &widy);

	if(widx < 4 || 
		widy < 4 ||
		widx > MAX_MAP ||
		widy > MAX_MAP)
	{
		char cmess[128];
		sprintf(cmess, "%s 4 %s %d", STRTABLE[STR_WIDTHMUSTBE].rawstr().c_str(), STRTABLE[STR_AND].rawstr().c_str(), MAX_MAP);
		RichText mess = RichText(cmess);
		Mess(&mess);
		return;
	}

#if 0
	if(seed1 == 0 ||
		seed2 == 0)
	{
		char cmess[128];
		sprintf(cmess, "Seed cannot be 0.");
		RichText mess = RichText(cmess);
		Mess(&mess);
		return;
	}

	if(seed1 == 0x464fffff)
	{
		char cmess[128];
		sprintf(cmess, "Seed 1 cannot be %u", 0x464fffff);
		RichText mess = RichText(cmess);
		Mess(&mess);
		return;
	}

	if(seed2 == 0x9068ffff)
	{
		char cmess[128];
		sprintf(cmess, "Seed 2 cannot be %u", 0x9068ffff);
		RichText mess = RichText(cmess);
		Mess(&mess);
		return;
	}
#endif

	bool r = (bool)rbox->m_selected;
	
	FreeMap();

	g_hmap.alloc(widx, widy);

	if(!r)
	{
		
		for(int32_t x=0; x<=g_mapsz.x; x++)
			for(int32_t y=0; y<=g_mapsz.y; y++)
				g_hmap.setheight(x, y, 1);

        g_hmap.lowereven();
		g_hmap.remesh();
		//AllocPathGrid((g_mapsz.x+1)*TILE_SIZE-1, (g_mapsz.y+1)*TILE_SIZE-1);
		AllocPathGrid((g_mapsz.x)*TILE_SIZE, (g_mapsz.y)*TILE_SIZE);
		AllocGrid((int32_t)g_mapsz.x+0, (int32_t)g_mapsz.y+0);
		FillColliderGrid();
	}
	else
	{
		srand3(seed);
		//srand4(seed1, seed2);

#if 0
		char msg[2048] = "";
		for(int32_t i=0; i<25; i++)
		{
			char add[54];
			sprintf(add, "%u\n", rand3());
			strcat(msg, add);
		}
		RichText mess(msg);
		Mess(&mess);
#elif 0
		
		std::list<uint32_t> repeats;
		
		for(uint32_t i=0; i<66*1000; i++)
		//for(uint32_t i=0; i<100*1000; i++)
		{
			uint32_t now = rand3();

			bool found = false;

			for(std::list<Widget*>::iterator rit=repeats.begin(); rit!=repeats.end(); rit++)
			{
				if(*rit != now)
					continue;

				found = true;
				break;
			}

			if(!found)
			{
				repeats.push_back(now);
				continue;
			}

			char cmess[128];
			sprintf(cmess, "period %d, val=%u", i, now);
			RichText mess = RichText(cmess);
			Mess(&mess);
			break;
		}
#endif

		for(int32_t x=0; x<=g_mapsz.x; x++)
			for(int32_t y=0; y<=g_mapsz.y; y++)
			{
				g_hmap.setheight(x, y, rand3()%7);
			//	g_hmap.setheight(x, y, 0);
			}
        
        g_hmap.lowereven();
		g_hmap.remesh();
		
		for(int32_t x=0; x<=g_mapsz.x; x++)
			for(int32_t y=0; y<=g_mapsz.y; y++)
			{
				g_hmap.setheight(x, y, g_hmap.getheight(x, y) / 2 + 1);
			//	g_hmap.setheight(x, y, 0);
			}
        
        g_hmap.lowereven();
		g_hmap.remesh();
		
		//AllocPathGrid((g_mapsz.x+1)*TILE_SIZE-1, (g_mapsz.y+1)*TILE_SIZE-1);
		AllocPathGrid((g_mapsz.x)*TILE_SIZE, (g_mapsz.y)*TILE_SIZE);
		AllocGrid((int32_t)g_mapsz.x+0, (int32_t)g_mapsz.y+0);
		FillColliderGrid();

		FillForest(rand3());

		for(int32_t i=0; i < 2 * (1); i++)
			//for(int32_t j=0; j<10; j++)
		//for(int32_t i=0; i<15; i++)
			for(int32_t j=0; j < 2 * (1); j++)
		//for(int32_t i=0; i<1; i++)
			//for(int32_t j=0; j<15; j++)
			{

				//Vec3i cmpos((g_mapsz.x+4)*TILE_SIZE/2 + (i+2)*PATHNODE_SIZE, 0, g_mapsz.y*TILE_SIZE/2 + (j+2)*PATHNODE_SIZE);
				//cmpos.y = g_hmap.accheight(cmpos.x, cmpos.z);
				Vec2i cmpos(4*TILE_SIZE + (i+2)*PATHNODE_SIZE*4, 4*TILE_SIZE + (j+2)*PATHNODE_SIZE*4);

				//if(rand()%2 == 1)
				//	PlaceUnit(UNIT_BATTLECOMP, cmpos, 0);
				//else
				PlaceUnit(UNIT_LABOURER, cmpos, 0, NULL);
				//PlaceUnit(UNIT_TRUCK, cmpos, rand()%PLAYERS, NULL);

				ClearFol(cmpos.x - TILE_SIZE, cmpos.y - TILE_SIZE, cmpos.x + TILE_SIZE, cmpos.y + TILE_SIZE);
			}
	}

	Vec3i cmscroll;
	cmscroll.x = 4 * TILE_SIZE;
	cmscroll.y = 4 * TILE_SIZE;
	cmscroll.z = Bilerp(&g_hmap, cmscroll.x, cmscroll.y);
	g_scroll = CartToIso(cmscroll) - Vec2i(g_width,g_height)/2;

	//g_localP = NewPlayer();
	g_localP = 0;
	AddClient(NULL, g_name, &g_localC, CLPLATI);
	AssocPy(g_localP, g_localC);

	//return;

	CHECKGLERROR();
	IniRes();
}

void FillEdNewMap()
{
	GUI* gui = &g_gui;

	gui->add(new ViewLayer(gui, "ed newmap"));
	ViewLayer* mapview = (ViewLayer*)gui->get("ed newmap");
	
	mapview->add(new Text(mapview, "0 0", STRTABLE[STR_TILEWX], MAINFONT16, Resize_EdNewMap_Table));
	mapview->add(new Text(mapview, "0 1", STRTABLE[STR_TILEWY], MAINFONT16, Resize_EdNewMap_Table));
	
	char maxmap[32];
	sprintf(maxmap, "%d", MAX_MAP);

	mapview->add(new EditBox(mapview, "1 0", RichText(maxmap), MAINFONT16, Resize_EdNewMap_Table, false, 3, NULL, NULL, -1));
	mapview->add(new EditBox(mapview, "1 1", RichText(maxmap), MAINFONT16, Resize_EdNewMap_Table, false, 3, NULL, NULL, -1));

	mapview->add(new CheckBox(mapview, "0 2", STRTABLE[STR_RANDOM], MAINFONT16, Resize_EdNewMap_Table, 0));
	
	mapview->add(new Text(mapview, "0 3", STRTABLE[STR_RANDSEED], MAINFONT16, Resize_EdNewMap_Table));
	mapview->add(new EditBox(mapview, "1 3", RichText("0"), MAINFONT16, Resize_EdNewMap_Table, false, 16, NULL, NULL, -1));
	//mapview->add(new EditBox(mapview, "1 4", RichText("1"), MAINFONT16, Resize_EdNewMap_Table, false, 16, NULL, NULL, -1));
	//mapview->add(new EditBox(mapview, "1 5", RichText("1"), MAINFONT16, Resize_EdNewMap_Table, false, 16, NULL, NULL, -1));

	mapview->add(new Button(mapview, "0 6", "gui/transp.png", STRTABLE[STR_GENERATE], RichText(), MAINFONT16, BUST_LINEBASED, Resize_EdNewMap_Table, Click_EdNewMap_Gen, NULL, NULL, NULL, NULL, -1, NULL));
}

void Change_Ed_BlType()
{
	int32_t act = GetEdAct();

	if(act == TOOLACT_PLACE)
		g_build = GetEdBlType();
	else
		g_build = BL_NONE;
}

void FillEdBl()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->add(new ViewLayer(gui, "ed buildings"));
	ViewLayer* view = (ViewLayer*)gui->get("ed buildings");

	view->add(new DropList(view, "building selector", MAINFONT16, Resize_BuildingDropDownSelector, Change_Ed_BlType));

	Widget* bsel = view->get("building selector");

	for(int32_t i=0; i<BL_TYPES; i++)
	{
		bsel->m_options.push_back(RichText(g_bltype[i].name));
	}

	//placebuildingsview->add(new DropList(NULL, "owner owner selector", MAINFONT8, Resize_BuildingCountryDropDownSelector, NULL));

	//placebuildingsview->add(new DropList(NULL, "company owner selector", MAINFONT8, Resize_BuildingCompanyDropDownSelector, NULL));
}

void Change_Ed_CdType()
{
    int32_t act = GetEdAct();
    
    if(act == TOOLACT_PLACE)
        g_build = BL_TYPES + GetEdCdType();
    else
        g_build = BL_NONE;
    
    for(int32_t ctype=0; ctype<CD_TYPES; ctype++)
        ClearCdPlans(ctype);
}

void FillEdCd()
{
    Player* py = &g_player[g_localP];
    GUI* gui = &g_gui;
    
    gui->add(new ViewLayer(gui, "ed conduits"));
    ViewLayer* view = (ViewLayer*)gui->get("ed conduits");
    
    view->add(new DropList(view, "cd sel", MAINFONT16, Resize_BuildingDropDownSelector, Change_Ed_CdType));
    
    Widget* cdsel = view->get("cd sel");
    
    for(int32_t i=0; i<CD_TYPES; i++)
    {
		CdType* ct = &g_cdtype[i];

#if 0
		if(!ct->on)
		{
			char n[64];
			sprintf(n, "c%d", i);
			cdsel->m_options.push_back(RichText(n));
			continue;
		}
#endif

        cdsel->m_options.push_back(RichText(g_cdtype[i].name));
    }
    
    //placebuildingsview->add(new DropList(NULL, "owner owner selector", MAINFONT8, Resize_BuildingCountryDropDownSelector, NULL));
    
    //placebuildingsview->add(new DropList(NULL, "company owner selector", MAINFONT8, Resize_BuildingCompanyDropDownSelector, NULL));
}

void Resize_TrigList(Widget *w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 1;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight * 9;
}

void Resize_TrigName(Widget *w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 10;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight * 1;
}

void Resize_AddTrig(Widget *w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 11;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight * 1;
}

void Click_AddTrig()
{
	GUI *gui = &g_gui;
	ViewLayer *view = (ViewLayer*)gui->get("ed triggers");
	EditBox *namebox = (EditBox*)view->get("trig name");
	std::string newname = namebox->m_value.rawstr();
	
	if(!UniqueTrigger(newname.c_str()))
	{
		RichText rmess = RichText("Trigger name must be unique.");
		Mess(&rmess);
		return;
	}
	
	//TODO return Trigger** to simplify logic in case of NULL scripthead
	Trigger *ptail = GetScriptTail();
	Trigger **pptail;
	
	if(!ptail)
		pptail = &g_scripthead;
	else
		pptail = &ptail->next;
	
	
	
}

void Resize_EditTrig(Widget *w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 12;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight * 1;
}

void Click_EditTrig()
{
	
}

void Resize_DelTrig(Widget *w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 13;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight * 1;
}

void Click_DelTrig()
{
	
}

void FillEdTrig()
{
	GUI* gui = &g_gui;
	
	gui->add(new ViewLayer(gui, "ed triggers"));
	ViewLayer* view = (ViewLayer*)gui->get("ed triggers");
	
	view->add(new ListBox(view, "trig list", MAINFONT16, Resize_TrigList, NULL));
	
	view->add(new EditBox(view, "trig name", RichText(), MAINFONT16, Resize_TrigName, false, CONDITION_LEN, NULL, NULL, -1));
	
	view->add(new Button(view, "trig add", "gui/transp.png", RichText("Add"), RichText(), MAINFONT16, BUST_LINEBASED, Resize_AddTrig, Click_AddTrig, NULL, NULL, NULL, NULL, -1, NULL));
	
	view->add(new Button(view, "trig edit", "gui/transp.png", RichText("Edit"), RichText(), MAINFONT16, BUST_LINEBASED, Resize_EditTrig, Click_EditTrig, NULL, NULL, NULL, NULL, -1, NULL));
	
	view->add(new Button(view, "trig add", "gui/transp.png", RichText("Delete"), RichText(), MAINFONT16, BUST_LINEBASED, Resize_DelTrig, Click_DelTrig, NULL, NULL, NULL, NULL, -1, NULL));
}

void Resize_ParStText(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 3;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_InstTypeText(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 6;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_InstTypeCheck(Widget* w)
{
	BmpFont* f = &g_font[MAINFONT16];
	w->m_pos[0] = 5;
	w->m_pos[1] = 5 + 64 + SCROLL_BORDER + 90 + (f->gheight + 5) * 7;
	w->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Change_EdPy_PySel()
{
	GUI* gui = &g_gui;
	ViewLayer* view = (ViewLayer*)gui->get("ed players");
    Widget* osel = view->get("py sel");
	Player* py = &g_player[osel->m_selected];
	RegenEdPy();
}

void Change_EdPy_OnCheck()
{
	GUI* gui = &g_gui;
	ViewLayer* view = (ViewLayer*)gui->get("ed players");
    Widget* osel = view->get("py sel");
	Player* py = &g_player[osel->m_selected];
    Widget* oncheck = view->get("on check");
	py->on = (bool)oncheck->m_selected;
}

void Change_EdPy_AICheck()
{
	GUI* gui = &g_gui;
	ViewLayer* view = (ViewLayer*)gui->get("ed players");
    Widget* osel = view->get("py sel");
	Player* py = &g_player[osel->m_selected];
    Widget* aicheck = view->get("ai check");
	py->ai = (bool)aicheck->m_selected;
}


void Change_EdPy_PySel2()
{
	GUI* gui = &g_gui;
	ViewLayer* view = (ViewLayer*)gui->get("ed players");
    Widget* osel = view->get("py sel");
	Player* py = &g_player[osel->m_selected];
	py->parentst = osel->m_selected;
}

void Change_EdPy_InstType()
{
	GUI* gui = &g_gui;
	ViewLayer* view = (ViewLayer*)gui->get("ed players");
    Widget* osel = view->get("py sel");
	DropList* itc = (DropList*)view->get("insttype check");
	Player* py = &g_player[osel->m_selected];
	py->insttype = itc->m_selected;
}

void FillEdPy()
{
	GUI* gui = &g_gui;
	
	gui->add(new ViewLayer(gui, "ed players"));
	ViewLayer* view = (ViewLayer*)gui->get("ed players");

    view->add(new DropList(view, "py sel", MAINFONT16, Resize_PySelector, Change_EdPy_PySel));
    Widget* osel = view->get("py sel");
    for(int32_t i=0; i<PLAYERS; i++)
    {
        char num[128];
        sprintf(num, "%d. ", i);
        Player* py = &g_player[i];
        osel->m_options.push_back(RichText(num) + py->name);
    }

	view->add(new CheckBox(view, "on check", (STRTABLE[STR_PLAYSLOTON]), MAINFONT16, Resize_EdPy_OnCheck, 0, 1, 1, 1, 1, Change_EdPy_OnCheck));
	view->add(new CheckBox(view, "ai check", (STRTABLE[STR_ISAIPLAYER]), MAINFONT16, Resize_EdPy_AICheck, 0, 1, 1, 1, 1, Change_EdPy_AICheck));

    view->add(new DropList(view, "py sel 2", MAINFONT16, Resize_PySelector2, Change_EdPy_PySel2));
    osel = view->get("py sel 2");
    for(int32_t i=0; i<PLAYERS; i++)
    {
        char num[128];
        sprintf(num, "%d. ", i);
        Player* py = &g_player[i];
        osel->m_options.push_back(RichText(num) + py->name);
    }

	view->add(new Text(view, "parst text", STRTABLE[STR_PARENTSTATE], MAINFONT16, Resize_ParStText));
	
	
	view->add(new Text(view, "insttype text", STRTABLE[STR_INSTTYPE], MAINFONT16, Resize_InstTypeText));
	view->add(new DropList(view, "insttype check", MAINFONT16, Resize_InstTypeCheck, Change_EdPy_InstType));
	
#if 0
	view->get("ai check")->hide();
	view->get("parst text")->hide();
	view->get("insttype text")->hide();
	view->get("insttype check")->hide();
#endif

	DropList* itc = (DropList*)view->get("insttype check");

	for(int32_t iti=0; iti<INST_TYPES; iti++)
	{
		itc->m_options.push_back(RichText(INST_STR[iti]));
	}
}

int32_t GetEdBlType()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	ViewLayer* placebuildingsview = (ViewLayer*)gui->get("ed buildings");

	Widget* buildingselector = placebuildingsview->get("building selector");

	return buildingselector->m_selected;
}

void Resize_LoadMapButton(Widget* w)
{
	w->m_pos[0] = 5 + 64*0;
	w->m_pos[1] = 5 + SCROLL_BORDER;
	w->m_pos[2] = 5 + 64*1;
	w->m_pos[3] = w->m_pos[1] + 64;
}

void Resize_SaveMapButton(Widget* w)
{
	w->m_pos[0] = 5 + 64*1;
	w->m_pos[1] = 5 + SCROLL_BORDER;
	w->m_pos[2] = 5 + 64*2;
	w->m_pos[3] = w->m_pos[1] + 64;
}

void Resize_QSaveMapButton(Widget* w)
{
	w->m_pos[0] = 5 + 64*2;
	w->m_pos[1] = 5 + SCROLL_BORDER;
	w->m_pos[2] = 5 + 64*3;
	w->m_pos[3] = w->m_pos[1] + 64;
}

void Change_ToolAct()
{
	g_build = BL_NONE;

	Change_ToolCat();
}

void LowerTerr(Vec2i from, Vec2i to)
{
    from.x = imin(g_mapsz.x,from.x);
    from.y = imin(g_mapsz.y,from.y);
    to.x = imax(0,to.x);
    to.y = imax(0,to.y);
    
    int32_t tminx = imin(from.x,to.x);
    int32_t tminy = imin(from.y,to.y);
    int32_t tmaxx = imax(from.x,to.x);
    int32_t tmaxy = imax(from.y,to.y);
    
    for(int32_t x=tminx; x<=tmaxx; x++)
        for(int32_t y=tminy; y<=tmaxy; y++)
        {
            int32_t h = g_hmap.getheight(x, y);
            h = imax(0, h-1);
            g_hmap.setheight(x, y, (unsigned char)h);
        }
    
    g_hmap.lowereven();
    g_hmap.remesh();
    RecheckStand();
}

void RaiseTerr(Vec2i from, Vec2i to)
{
    from.x = imin(g_mapsz.x,from.x);
    from.y = imin(g_mapsz.y,from.y);
    to.x = imax(0,to.x);
    to.y = imax(0,to.y);
    
    int32_t tminx = imin(from.x,to.x);
    int32_t tminy = imin(from.y,to.y);
    int32_t tmaxx = imax(from.x,to.x);
    int32_t tmaxy = imax(from.y,to.y);
    
    for(int32_t x=tminx; x<=tmaxx; x++)
        for(int32_t y=tminy; y<=tmaxy; y++)
        {
            int32_t h = g_hmap.getheight(x, y);
            h = imin(6, h+1);
            g_hmap.setheight(x, y, (unsigned char)h);
        }
    
    g_hmap.highereven();
    g_hmap.remesh();
    RecheckStand();
}

void SpreadTerr(Vec2i from, Vec2i to)
{
    from.x = imin(g_mapsz.x,from.x);
    from.y = imin(g_mapsz.y,from.y);
    to.x = imax(0,to.x);
    to.y = imax(0,to.y);
    
    int32_t tminx = imin(from.x,to.x);
    int32_t tminy = imin(from.y,to.y);
    int32_t tmaxx = imax(from.x,to.x);
    int32_t tmaxy = imax(from.y,to.y);
    
    unsigned char h = g_hmap.getheight(from.x, from.y);
    
    for(int32_t x=tminx; x<=tmaxx; x++)
        for(int32_t y=tminy; y<=tmaxy; y++)
        {
            g_hmap.setheight(x, y, (unsigned char)h);
        }
    
    g_hmap.lowereven();
    g_hmap.remesh();
    RecheckStand();
}

//TODO clean up here ed, and change names to something consistent
void EdPlaceBords()
{
	int32_t owner = GetPlaceOwner();

	//if(owner % (FIRMSPERSTATE+1) != 0)
	//	return;
	
	//TODO don't need imin imax fmin fmax funcs, use macros, less compiled symbols, less cache use
	int32_t tminx = imin(g_vdrag[0].x, g_vdrag[1].x) / TILE_SIZE;
	int32_t tminy = imin(g_vdrag[0].y, g_vdrag[1].y) / TILE_SIZE;
	int32_t tmaxx = imax(g_vdrag[0].x, g_vdrag[1].x) / TILE_SIZE;
	int32_t tmaxy = imax(g_vdrag[0].y, g_vdrag[1].y) / TILE_SIZE;

	tminx = imax(0, tminx);
	tminy = imax(0, tminy);
	tmaxx = imin(g_mapsz.x, tmaxx);
	tmaxy = imin(g_mapsz.y, tmaxy);

	for(int32_t tx=tminx; tx<tmaxx; tx++)
		for(int32_t ty=tminy; ty<tmaxy; ty++)
		{
			int32_t i = tx + ty * (g_mapsz.x+0);
			g_border[i] = (signed char)owner;
		}
}

void FillEd()
{
	g_lastsave[0] = '\0';

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->add(new ViewLayer(gui, "ed"));
	ViewLayer* ed = (ViewLayer*)gui->get("ed");
	
	ed->add(new Image(ed, "", "gui/filled.jpg", true, Resize_LeftPanel));
	ed->add(new TouchListener(ed, "", Resize_LeftPanel, NULL, NULL, NULL, -1));

#if 0
	Button(Widget* parent, const char* filepath, const RichText t, int32_t f, int32_t style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int32_t p), void (*overf)(), void (*overf2)(int32_t p), void (*out)(), int32_t parm)
#endif

	ed->add(new Button(ed, "name", "gui/edload.png", RichText(""), (STRTABLE[STR_LOADMAP]), MAINFONT16, BUST_LEFTIMAGE, Resize_LoadMapButton, Click_LoadMapButton, NULL, NULL, NULL, NULL, -1, NULL));
	ed->add(new Button(ed, "name", "gui/edsave.png", RichText(""), (STRTABLE[STR_SAVEMAP]), MAINFONT16, BUST_LEFTIMAGE, Resize_SaveMapButton, Click_SaveMapButton, NULL, NULL, NULL, NULL, -1, NULL));
	ed->add(new Button(ed, "name", "gui/qsave.png", RichText(""), (STRTABLE[STR_QSAVE]), MAINFONT16, BUST_LEFTIMAGE, Resize_QSaveMapButton, Click_QSaveMapButton, NULL, NULL, NULL, NULL, -1, NULL));

	ed->add(new DropList(ed, "tool cat", MAINFONT16, Resize_ToolCat, Change_ToolCat));

	Widget* toolcat = ed->get("tool cat");

#if 0
#define TOOLCAT_BORDERS			0
#define TOOLCAT_PLACEUNITS		1
#define TOOLCAT_PLACEBUILDINGS	2
#define TOOLCAT_DELETEOBJECTS	3
#endif

#if 0
	toolcat->m_options.push_back(RichText("Borders (Reserved)"));
	toolcat->m_options.push_back(RichText("Place Units"));
	toolcat->m_options.push_back(RichText("Place Buildings"));
	toolcat->m_options.push_back(RichText("Delete Objects"));
	toolcat->m_options.push_back(RichText("Place Roads"));
	toolcat->m_options.push_back(RichText("Place Powerline"));
	toolcat->m_options.push_back(RichText("Place Crude Oil Pipeline"));
#endif
	
	toolcat->m_options.push_back((STRTABLE[STR_NEWMAP]));
	toolcat->m_options.push_back((STRTABLE[STR_ELEVATION]));
	toolcat->m_options.push_back((STRTABLE[STR_TERRAIN]));
	toolcat->m_options.push_back((STRTABLE[STR_FOLIAGE]));
	toolcat->m_options.push_back((STRTABLE[STR_UNITS]));
	toolcat->m_options.push_back((STRTABLE[STR_BUILDINGS]));
	toolcat->m_options.push_back((STRTABLE[STR_CONDUITS]));
	toolcat->m_options.push_back((STRTABLE[STR_RESOURCES]));
	toolcat->m_options.push_back((STRTABLE[STR_TRIGGERS]));
	toolcat->m_options.push_back((STRTABLE[STR_BORDERS]));
	toolcat->m_options.push_back((RichText("Players")));
	//toolcat->m_options.push_back(RichText("Scripts"));

	//toolcat->select(1);

	toolcat->m_selected = -1;
	
	ed->add(new ViewLayer(ed, "act layer"));
	ViewLayer* actlayer = (ViewLayer*)ed->get("act layer");
	actlayer->add(new DropList(actlayer, "tool act", MAINFONT16, Resize_ToolAct, Change_ToolAct));
	Widget* toolact = actlayer->get("tool act");
	toolact->m_options.push_back((STRTABLE[STR_PLACE]));
	toolact->m_options.push_back((STRTABLE[STR_DELETE]));
    
    ed->add(new ViewLayer(ed, "elev layer"));
    ViewLayer* elevlayer = (ViewLayer*)ed->get("elev layer");
    elevlayer->add(new DropList(actlayer, "elev act", MAINFONT16, Resize_ToolAct, NULL));
    Widget* elevact = elevlayer->get("elev act");
    elevact->m_options.push_back((STRTABLE[STR_RAISE]));
    elevact->m_options.push_back((STRTABLE[STR_LOWER]));
    elevact->m_options.push_back((STRTABLE[STR_SPREAD]));
    
    ed->add(new ViewLayer(ed, "owner layer"));
    ViewLayer* ownlayer = (ViewLayer*)ed->get("owner layer");
    ownlayer->add(new DropList(ownlayer, "py sel", MAINFONT16, Resize_PySelector, NULL));
    Widget* osel = ownlayer->get("py sel");
    for(int32_t i=0; i<PLAYERS; i++)
    {
        char num[128];
        sprintf(num, "%d. ", i);
        Player* py = &g_player[i];
        osel->m_options.push_back(RichText(num) + py->name);
    }

    ed->add(new ViewLayer(ed, "owner layer 2"));
    ViewLayer* ownlayer2 = (ViewLayer*)ed->get("owner layer 2");
    ownlayer2->add(new DropList(ownlayer, "py sel", MAINFONT16, Resize_PySelector2, NULL));
    osel = ownlayer2->get("py sel");
    for(int32_t i=0; i<PLAYERS; i++)
    {
        char num[128];
        sprintf(num, "%d. ", i);
        Player* py = &g_player[i];
        osel->m_options.push_back(RichText(num) + py->name);
    }

	FillEdNewMap();
	FillEdUnits();
	FillEdBl();
    FillEdCd();
	FillEdTrig();
	FillEdPy();
}
