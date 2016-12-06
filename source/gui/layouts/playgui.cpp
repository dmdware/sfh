










#include "../../app/appmain.h"
#include "../../gui/gui.h"
#include "../../gui/widgets/winw.h"
#include "../keymap.h"
#include "../../render/heightmap.h"
#include "../../render/transaction.h"
#include "../../math/camera.h"
#include "../../render/screenshot.h"
#include "../../save/savemap.h"
#include "appgui.h"
#include "playgui.h"
#include "../../gui/icon.h"
#include "../../gui/widgets/spez/resticker.h"
#include "../../gui/widgets/spez/botpan.h"
#include "../../gui/widgets/spez/blpreview.h"
#include "../../gui/widgets/spez/blview.h"
#include "../../gui/widgets/spez/blgraphs.h"
#include "../../gui/widgets/spez/gengraphs.h"
#include "../../gui/widgets/spez/pygraphs.h"
#include "../../gui/widgets/spez/cstrview.h"
#include "../../gui/widgets/spez/svlist.h"
#include "../../gui/widgets/spez/saveview.h"
#include "../../gui/widgets/spez/loadview.h"
#include "../../gui/widgets/spez/truckmgr.h"
#include "appviewport.h"
#include "../../sim/bltype.h"
#include "../../sim/building.h"
#include "../../sim/player.h"
#include "../../sim/conduit.h"
#include "../../sim/simflow.h"
#include "chattext.h"
#include "../../net/client.h"
#include "../../net/lockstep.h"
#include "../../net/sendpackets.h"
#include "messbox.h"
#include "../../math/hmapmath.h"
#include "../../sim/map.h"

Vec3i g_vdrag[2];

void Resize_ResNamesTextBlock(Widget* w)
{
	Player* py = &g_player[g_localP];
	w->m_pos[0] = 0;
	w->m_pos[1] = 10;
	w->m_pos[2] = g_width;
	w->m_pos[3] = g_height;
}

void Resize_ResAmtsTextBlock(Widget* w)
{
	Player* py = &g_player[g_localP];
	w->m_pos[0] = 150;
	w->m_pos[1] = 10;
	w->m_pos[2] = g_width;
	w->m_pos[3] = g_height;
}

void Resize_ResDeltasTextBlock(Widget* w)
{
	Player* py = &g_player[g_localP];
	w->m_pos[0] = 250;
	w->m_pos[1] = 10;
	w->m_pos[2] = g_width;
	w->m_pos[3] = g_height;
}

void Out_Build()
{
}

void Resize_ResTicker(Widget* w)
{
	Player* py = &g_player[g_localP];
	w->m_pos[0] = 0;
	w->m_pos[1] = 0;
	w->m_pos[2] = g_width;
	w->m_pos[3] = g_font[MAINFONT16].gheight+5;
	w->m_tpos[0] = 0;
	w->m_tpos[1] = 0;
}

void UpdResTicker()
{
	//return;

	static float tickerpos = 0;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");
	ResTicker* restickerw = (ResTicker*)playview->get("res ticker");
	Widget* restickertw = &restickerw->restext;
	RichText restext;

#if 0
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_DOLLARS));
	restext.m_part.push_back(RichPart(" Funds: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_HOUSING));
	restext.m_part.push_back(RichPart(" Housing: 100/120"));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_FARMPRODUCT));
	restext.m_part.push_back(RichPart(" Farm Products: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_RETFOOD));
	restext.m_part.push_back(RichPart(" Retail Food: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_PRODUCTION));
	restext.m_part.push_back(RichPart(" Production: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_IRONORE));
	restext.m_part.push_back(RichPart(" Minerals: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_CRUDEOIL));
	restext.m_part.push_back(RichPart(" Crude Oil: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_WSFUEL));
	restext.m_part.push_back(RichPart(" Wholesale Fuel: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_RETFUEL));
	restext.m_part.push_back(RichPart(" Retail Fuel: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_ENERGY));
	restext.m_part.push_back(RichPart(" Energy: 100/120"));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_ENUR));
	restext.m_part.push_back(RichPart(" Uranium: 100 +1/"));
	restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
#else
	for(int32_t i=0; i<RESOURCES; i++)
	{
		if(i == RES_LABOUR)
			continue;

		Resource* r = &g_resource[i];

		if(r->capacity && py->local[i] <= 0)
			continue;
		else if(!r->capacity && py->local[i] + py->global[i] <= 0)
			continue;

		restext.m_part.push_back(RichPart(RICH_ICON, r->icon));

		char cstr1[128];
		sprintf(cstr1, " %s: ", r->name.c_str());
		char cstr2[64];

#if 0	//with reschanges?
		if(r->capacity)
			sprintf(cstr2, "%d/%d", py->local[i], py->global[i]);
		else
			sprintf(cstr2, "%d %+d/", py->local[i] + py->global[i], py->resch[i]);

		strcat(cstr1, cstr2);
		restext.m_part.push_back(RichPart(cstr1));

		if(!r->capacity)
			restext.m_part.push_back(RichPart(RICH_ICON, ICON_TIME));
#else
#if 0
		if(r->capacity)
			sprintf(cstr2, "%d/%d", py->local[i], py->global[i]);
		else
			sprintf(cstr2, "%d", py->local[i] + py->global[i]);
#else
		if(r->capacity)
		{
			std::string local = iform(py->local[i]);
			std::string global = iform(py->global[i]);
			sprintf(cstr2, "%s/%s", local.c_str(), global.c_str());
		}
		else
		{
			std::string total = iform(py->local[i] + py->global[i]);
			sprintf(cstr2, "%s", total.c_str());
		}
#endif

		strcat(cstr1, cstr2);
		restext.m_part.push_back(RichPart(cstr1));
#endif

		restext.m_part.push_back(RichPart("    "));
	}
#endif

	int32_t len = restext.texlen();

	tickerpos += 0.5f * g_drawfrinterval * 20;

	if((int32_t)tickerpos > len)
		tickerpos = 0;

	int32_t endx = EndX(&restext, restext.rawlen(), MAINFONT16, 0, 0);

	if(endx > g_width)
	{
		RichText restext2 = restext.substr((int32_t)tickerpos, len-(int32_t)tickerpos) + restext.substr(0, (int32_t)tickerpos);
		restickertw->m_text = restext2;
	}
	else
		restickertw->m_text = restext;
}

void Resize_BottomPanel(Widget* w)
{
	Player* py = &g_player[g_localP];

	w->m_pos[0] = 0;
	w->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32);
	w->m_pos[2] = (float)g_width;
	w->m_pos[3] = (float)g_height;
}

void Out_BuildButton()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->hide("bl preview");
}

void Click_BuildButton(int32_t bwhat)
{
	Player* py = &g_player[g_localP];
	g_build = bwhat;
	//Log("b "<<g_build);
	//char msg[128];
	//sprintf(msg, "b %d", bwhat);
	//InfoMess("t", msg);

	//TODO for mobile, display placement confirmation and show planned/preview bl

#ifdef PLATFORM_MOBILE
	Vec3i cmpos;
	Vec3f intersect;
	Vec2i pixpos = g_scroll + Vec2i(g_width/2,g_height/2);
	Vec3f ray;
	Vec3f point;
	
	IsoToCart(pixpos, &ray, &point);
	
	//if(!MapInter(&g_hmap, ray, point, &cmpos))
	//	return;
	
	Vec3f line[2];
	Vec3f fint;
	line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
	line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
	
	FastMapIntersect(&g_hmap, g_mapsz, line, &fint);
	
	cmpos.x = (int32_t)fint.x;
	cmpos.y = (int32_t)fint.y;
	cmpos.z = (int32_t)fint.z;
	
	g_vdrag[0] = cmpos;
	g_vdrag[1] = cmpos;
	
	//g_placecnt = 0;
#endif
	
	Out_BuildButton();
}

void Over_BuildButton(int32_t bwhat)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	if(gui->get("cs view")->m_opened)
		return;

	if(gui->get("bl view")->m_opened)
		return;

	g_bptype = bwhat;

	gui->show("bl preview");
	//BlPreview* bp = (BlPreview*)gui->get("bl preview")->get("bl preview");
	BlPreview* bp = (BlPreview*)gui->get("bl preview");
	Text* tl = (Text*)bp->get("title");

	std::string bname;
	RichText cb;	//conmat block
	RichText ib;	//inputs block
	RichText ob;	//outputs block
	RichText db;	//description block

	TextBlock* cbw = (TextBlock*)bp->get("conmat block");
	TextBlock* ibw = (TextBlock*)bp->get("input block");
	TextBlock* obw = (TextBlock*)bp->get("output block");
	TextBlock* dbw = (TextBlock*)bp->get("desc block");

	//cb.m_part.push_back(RichPart(UStr("CONSTRUCTION REQUISITES:")));
	cb = cb + STRTABLE[STR_CONREQS];

	if(bwhat < 0)
		;
	else if(bwhat < BL_TYPES)
	{
		//ib.m_part.push_back(RichPart(UStr("INPUTS:")));
		//ob.m_part.push_back(RichPart(UStr("OUTPUTS:")));
		ib = ib + STRTABLE[STR_INPUTS];
		ob = ob + STRTABLE[STR_OUTPUTS];

		BlType* bt = &g_bltype[bwhat];
		bname = bt->name;

		db.m_part.push_back(RichPart(UStr(bt->desc.c_str())));

		int32_t ns = 0;
		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			if(bt->conmat[ri] <= 0)
				continue;

			ns ++;
			Resource* r = &g_resource[ri];

			cb.m_part.push_back(RichPart(UStr("\n")));
			cb.m_part.push_back(RichPart(UStr(r->name.c_str())));
			cb.m_part.push_back(RichPart(UStr(" ")));
			cb.m_part.push_back(RichPart(RICH_ICON, r->icon));
			cb.m_part.push_back(RichPart(UStr(": ")));

			char num[32];
			sprintf(num, "%d ", bt->conmat[ri]);
			cb.m_part.push_back(RichPart(UStr(num)));
			//cb.m_part.push_back(RichPart(UStr(r->unit.c_str())));
		}

		if(ns <= 0)
			cb = cb + RichPart(UStr("\n")) + STRTABLE[STR_NONE];

		int32_t ni = 0;
		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			ni ++;
			Resource* r = &g_resource[ri];

			ib.m_part.push_back(RichPart(UStr("\n")));
			ib.m_part.push_back(RichPart(UStr(r->name.c_str())));
			ib.m_part.push_back(RichPart(UStr(" ")));
			ib.m_part.push_back(RichPart(RICH_ICON, r->icon));
			ib.m_part.push_back(RichPart(UStr(": ")));

			char num[32];
			sprintf(num, "%d ", bt->input[ri]);
			ib.m_part.push_back(RichPart(UStr(num)));
			//ib.m_part.push_back(RichPart(UStr(r->unit.c_str())));
		}

		if(ni <= 0)
			ib = ib + RichPart(UStr("\n")) + STRTABLE[STR_NONE];

		int32_t no = 0;
		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			if(bt->output[ri] <= 0)
				continue;

			no ++;
			Resource* r = &g_resource[ri];

			ob.m_part.push_back(RichPart(UStr("\n")));
			ob.m_part.push_back(RichPart(UStr(r->name.c_str())));
			ob.m_part.push_back(RichPart(UStr(" ")));
			ob.m_part.push_back(RichPart(RICH_ICON, r->icon));
			ob.m_part.push_back(RichPart(UStr(": ")));

			char num[32];
			sprintf(num, "%d ", bt->output[ri]);
			ob.m_part.push_back(RichPart(UStr(num)));
			//ob.m_part.push_back(RichPart(UStr(r->unit.c_str())));
		}

		if(no <= 0)
			ob = ob + RichPart(UStr("\n")) + STRTABLE[STR_NONE];
	}
	else if(bwhat < BL_TYPES + CD_TYPES)
	{
		CdType* ct = &g_cdtype[bwhat - BL_TYPES];
		bname = ct->name;

		db.m_part.push_back(RichPart(UStr(ct->desc.c_str())));

		int32_t ns = 0;
		for(int32_t ri=0; ri<RESOURCES; ri++)
		{
			if(ct->conmat[ri] <= 0)
				continue;

			ns ++;
			Resource* r = &g_resource[ri];

			cb.m_part.push_back(RichPart(UStr("\n")));
			cb.m_part.push_back(RichPart(UStr(r->name.c_str())));
			cb.m_part.push_back(RichPart(UStr(" ")));
			cb.m_part.push_back(RichPart(RICH_ICON, r->icon));
			cb.m_part.push_back(RichPart(UStr(": ")));

			char num[32];
			sprintf(num, "%d ", ct->conmat[ri]);
			cb.m_part.push_back(RichPart(UStr(num)));
			//cb.m_part.push_back(RichPart(UStr(r->unit.c_str())));
		}

		if(ns <= 0)
			cb = cb + RichPart(UStr("\n")) + STRTABLE[STR_NONE];
	}

	tl->m_text = RichText(UStr(bname.c_str()));
	cbw->m_text = cb;
	ibw->m_text = ib;
	obw->m_text = ob;
	dbw->m_text = db;

	g_bpcam.position(TILE_SIZE*3, TILE_SIZE*3, TILE_SIZE*3, 0, 0, 0, 0, 1, 0);
}

void Click_NextBuildButton(int32_t nextpage)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

	for(int32_t i=0; i<9; i++)
		bp->bottomright_button_on[i] = false;

#if 0
	if(nextpage == 1)
		BuildMenu_OpenPage1();
	else if(nextpage == 2)
		BuildMenu_OpenPage2();
	else if(nextpage == 3)
		BuildMenu_OpenPage3();
#else
	//9 - 1 forward button
	int32_t skip = 8 * (nextpage - 0);

	//figure out last page skip?
	if(skip < 0)
	{
		skip = 0;

#if 0
		for(int32_t i=0; i<BL_TYPES; ++i)
		{
			BlType* bt = &g_bltype[i];

			//if(!bt->on)
			//	continue;

			++skip;	
		}

		for(int32_t i=0; i<CD_TYPES; ++i)
		{
			CdType* ct = &g_cdtype[i];

			//if(!ct->on)
			//	continue;

			++skip;
		}

		//divides evenly, i.e., empty after last page
		if(skip / 8 * 8 == skip)
		{
			skip -= 8;
		}
		else
		{
			//restrict to the beginning of page
			//if not enough items to fill last page
			skip = skip / 8 * 8;
		}

		//if there's not enough items to fill 
		//if(skip < 8)
		//	skip = 0;
#endif
	}

	for(int32_t buti=0; buti<8; ++buti)
	{
		int32_t skip2 = skip+buti;
		bool found = false;

		for(int32_t i=0; i<BL_TYPES; ++i)
		{
			BlType* bt = &g_bltype[i];

			//if(!bt->on)
			//	continue;

			if(skip2 <= 0)
			{
				Texture* tex = &g_texture[ bt->icontex ];
				std::string texrel = MakeRelative(tex->fullpath.c_str());

				//InfoMess("b", texrel.c_str());

				bp->bottomright_button[buti] = 
					Button(bp, "name", texrel.c_str(), RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, i, NULL);
	
				bp->bottomright_button_on[buti] = true;

				found = true;

				break;
			}
			else
				--skip2;

			//++skip;	
		}

		if(found)
			continue;

		for(int32_t i=0; i<CD_TYPES; ++i)
		{
			CdType* ct = &g_cdtype[i];

			//if(!ct->on)
			//	continue;
			
			if(skip2 <= 0)
			{
				Texture* tex = &g_texture[ ct->icontex ];
				std::string texrel = MakeRelative(tex->fullpath.c_str());

				bp->bottomright_button[buti] = 
					Button(bp, "name", texrel.c_str(), RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CD_BEG+i, NULL);

				bp->bottomright_button_on[buti] = true;
				
				break;
			}
			else
				--skip2;

			//++skip;
		}

		if(skip2 > 0)
			break;
	}

	int32_t maxskip = 0;

	for(int32_t i=0; i<BL_TYPES; ++i)
	{
		BlType* bt = &g_bltype[i];

		//if(!bt->on)
		//	continue;

		++maxskip;	
	}

	for(int32_t i=0; i<CD_TYPES; ++i)
	{
		CdType* ct = &g_cdtype[i];

		//if(!ct->on)
		//	continue;

		++maxskip;
	}
	
	int32_t nextskip = nextpage*8+8;

	if(nextskip >= maxskip)
		nextskip = 0;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, nextskip/8, NULL);
	bp->bottomright_button_on[8] = true;
#endif

	bp->reframe();
}

#if 0

void BuildMenu_OpenPage1()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

#if 0
	Button(Widget* parent, const char* filepath, const RichText t, int32_t f, int32_t style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int32_t p), void (*overf)(), void (*overf2)(int32_t p), void (*out)(), int32_t parm)
#endif

#if 0	//with gas station
	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/apartment2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HOUSE1);
	bp->bottomright_button_on[0] = true;

	bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/store1.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_STORE);
	bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/farm2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_FARM);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[3] = Button(bp, "name", "gui/brbut/oilwell2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILWELL);
	bp->bottomright_button_on[3] = true;

	bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/refinery2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILREF);
	bp->bottomright_button_on[4] = true;

	bp->bottomright_button[5] = Button(bp, "name", "gui/brbut/gasstation2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_GASST);
	bp->bottomright_button_on[5] = true;

	bp->bottomright_button[6] = Button(bp, "name", "gui/brbut/mine.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_SHMINE);
	bp->bottomright_button_on[6] = true;

	bp->bottomright_button[7] = Button(bp, "name", "gui/brbut/factory3.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_TRFAC);
	bp->bottomright_button_on[7] = true;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 2);
	bp->bottomright_button_on[8] = true;
#else
	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/apartment2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HOUSE1, NULL);
	bp->bottomright_button_on[0] = true;

	bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/store1.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_STORE, NULL);
	bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/farm2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_FARM, NULL);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[3] = Button(bp, "name", "gui/brbut/oilwell2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILWELL, NULL);
	bp->bottomright_button_on[3] = true;

	bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/refinery2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILREF, NULL);
	bp->bottomright_button_on[4] = true;

	bp->bottomright_button[5] = Button(bp, "name", "gui/brbut/mine.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_SHMINE, NULL);
	bp->bottomright_button_on[5] = true;

	bp->bottomright_button[6] = Button(bp, "name", "gui/brbut/factory3.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_TRFAC, NULL);
	bp->bottomright_button_on[6] = true;

	bp->bottomright_button[7] = Button(bp, "name", "gui/brbut/nucpow2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_NUCPOW, NULL);
	bp->bottomright_button_on[7] = true;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 2, NULL);
	bp->bottomright_button_on[8] = true;
#endif

	bp->reframe();
}


void BuildMenu_OpenPage2()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

#if 0 //with gas station , c1
	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/nucpow2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_NUCPOW);
	bp->bottomright_button_on[0] = true;

	//bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/harbour2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HARBOUR);
	//bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/road.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_ROAD);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[3] = Button(bp, "name", "gui/brbut/crudepipeline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CRPIPE);
	bp->bottomright_button_on[3] = true;

	bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/powerline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_POWL);
	bp->bottomright_button_on[4] = true;

	bp->bottomright_button[5] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 1);
	bp->bottomright_button_on[5] = true;
#else
	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/coalpow.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_COALPOW, NULL);
	bp->bottomright_button_on[0] = true;

	bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/chemplant.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CHEMPL, NULL);
	bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/gasstation2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_GASST, NULL);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[3] = Button(bp, "name", "gui/brbut/cemplant.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CEMPL, NULL);
	bp->bottomright_button_on[3] = true;

	//bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/apartment2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HOUSE2, NULL);
	//bp->bottomright_button_on[4] = true;

	//bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/quarry.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_QUARRY, NULL);
	//bp->bottomright_button_on[4] = true;

	bp->bottomright_button[5] = Button(bp, "name", "gui/brbut/smelter.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_IRONSM, NULL);
	bp->bottomright_button_on[5] = true;
	
	bp->bottomright_button[6] = Button(bp, "name", "gui/brbut/barracks.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_BRK, NULL);
	//bp->bottomright_button_on[6] = true;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 3, NULL);
	bp->bottomright_button_on[8] = true;
#endif

	bp->reframe();
}

void BuildMenu_OpenPage3()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/road.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_ROAD, NULL);
	bp->bottomright_button_on[0] = true;

	bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/crudepipeline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CRPIPE, NULL);
	bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/powerline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_POWL, NULL);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 1, NULL);
	bp->bottomright_button_on[8] = true;

	bp->reframe();
}

#endif

void Resize_BlGraphs(Widget* w)
{
	Player* py = &g_player[g_localP];
	
#ifndef PLATFORM_MOBILE
	int32_t centerx = g_width/2;
	int32_t centery = g_height/2;
	
	w->m_pos[0] = centerx-200;
	w->m_pos[1] = centery-200;
	w->m_pos[2] = centerx+200;
	w->m_pos[3] = centery+200;
#elif 1
	w->m_pos[0] = 0;
	w->m_pos[1] = g_height - 300;
	w->m_pos[2] = 400;
	w->m_pos[3] = g_height - 150;
#endif
}

void Resize_BlPreview(Widget* w)
{
	Player* py = &g_player[g_localP];

#ifndef PLATFORM_MOBILE
	int32_t centerx = g_width/2;
	int32_t centery = g_height/2;

	w->m_pos[0] = centerx-350;
	w->m_pos[1] = centery-200;
	w->m_pos[2] = centerx+200;
	w->m_pos[3] = centery+200;
#elif 1
	w->m_pos[0] = 0;
	w->m_pos[1] = g_height - 300;
	w->m_pos[2] = 550;
	w->m_pos[3] = g_height;
#endif
}

void Resize_ConstructionView(Widget* w)
{
	Player* py = &g_player[g_localP];

#ifndef PLATFORM_MOBILE
	w->m_pos[0] = g_width/2 - 200;
	w->m_pos[1] = g_height/2 - 200;
	w->m_pos[2] = g_width/2 + 200;
	w->m_pos[3] = g_height/2 + 200;
#else
	w->m_pos[0] = 0;
	w->m_pos[1] = g_height - 200;
	w->m_pos[2] = 400;
	w->m_pos[3] = g_height;
#endif
}

void Click_MoveConstruction()
{
	int32_t alloced[RESOURCES];
	Zero(alloced);
	int32_t totalloc = 0;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	for(std::list<int32_t>::iterator siter = g_sel.buildings.begin(); siter != g_sel.buildings.end(); siter++)
	{
		int32_t bi = *siter;
		Building* b = &g_building[bi];

		for(int32_t i=0; i<RESOURCES; i++)
		{
			alloced[i] += b->conmat[i];
			totalloc += b->conmat[i];
		}
	}

#if 0
	for(std::list<Widget*>::iterator siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
	{
		Vec2i tpos = *siter;
		RoadTile* r = RoadAt(tpos.x, tpos.y);

		for(int32_t i=0; i<RESOURCES; i++)
		{
			alloced[i] += r->conmat[i];
			totalloc += r->conmat[i];
		}
	}

	for(std::list<Widget*>::iterator siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
	{
		Vec2i tpos = *siter;
		PowlTile* p = PowlAt(tpos.x, tpos.y);

		for(int32_t i=0; i<RESOURCES; i++)
		{
			alloced[i] += p->conmat[i];
			totalloc += p->conmat[i];
		}
	}

	for(std::list<Widget*>::iterator siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
	{
		Vec2i tpos = *siter;
		CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);

		for(int32_t i=0; i<RESOURCES; i++)
		{
			alloced[i] += p->conmat[i];
			totalloc += p->conmat[i];
		}
	}
#endif

	if(totalloc <= 0)
	{
		for(std::list<int32_t>::iterator siter = g_sel.buildings.begin(); siter != g_sel.buildings.end(); siter++)
		{
			int32_t bi = *siter;
			Building* b = &g_building[bi];
			b->on = false;
		}

#if 0
		for(std::list<Widget*>::iterator siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
		{
			Vec2i tpos = *siter;
			RoadTile* r = RoadAt(tpos.x, tpos.y);
			r->on = false;
		}

		for(std::list<Widget*>::iterator siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
		{
			Vec2i tpos = *siter;
			PowlTile* p = PowlAt(tpos.x, tpos.y);
			p->on = false;
		}

		for(std::list<Widget*>::iterator siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
		{
			Vec2i tpos = *siter;
			CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);
			p->on = false;
		}
#endif

		ClearSel(&g_sel);
		gui->hide("cs view");
	}
	else
	{
		ShowMessage(RichText("You've already invested resources in this project."));
	}
}

void Click_CancelConstruction()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	for(std::list<int32_t>::iterator siter = g_sel.buildings.begin(); siter != g_sel.buildings.end(); siter++)
	{
		int32_t bi = *siter;
		Building* b = &g_building[bi];
		b->on = false;
	}

#if 0
	for(std::list<Widget*>::iterator siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
	{
		Vec2i tpos = *siter;
		RoadTile* r = RoadAt(tpos.x, tpos.y);
		r->on = false;
	}

	for(std::list<Widget*>::iterator siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
	{
		Vec2i tpos = *siter;
		PowlTile* p = PowlAt(tpos.x, tpos.y);
		p->on = false;
	}

	for(std::list<Widget*>::iterator siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
	{
		Vec2i tpos = *siter;
		CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);
		p->on = false;
	}
#endif

	ClearSel(&g_sel);
	gui->hide("cs view");
}

void Click_ProceedConstruction()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	ClearSel(&g_sel);
	gui->hide("cs view");
}

void Click_EstimateConstruction()
{
	Player* py = &g_player[g_localP];
	ClearSel(&g_sel);
	GUI* gui = &g_gui;
	gui->hide("cs view");
}

void Resize_Message(Widget* w)
{
	Player* py = &g_player[g_localP];
	w->m_pos[0] = (float)(0);
	w->m_pos[1] = (float)(g_height - 200);
	w->m_pos[2] = (float)(0 + 400);
	w->m_pos[3] = (float)(g_height);
}

void Resize_MessageContinue(Widget* w)
{
	Player* py = &g_player[g_localP];
	w->m_pos[0] = (float)(200 - 80);
	w->m_pos[1] = (float)(g_height - 130);
	w->m_pos[2] = (float)(200 + 80);
	w->m_pos[3] = (float)(g_height - 10);
}

void Click_MessageContinue()
{
#if 0
	std::list<Widget*>::iterator viter = gui->view.begin();
	while(viter != gui->view.end())
	{
		if(stricmp(viter->name.c_str(), "message view") == 0)
		{
			InfoMess("f", "view found");

			viter = gui->view.erase(viter);

			continue;
		}

		viter++;
	}
#endif

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->hide("message view");
}

void ShowMessage(const RichText& msg)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* msgview = (ViewLayer*)gui->get("message view");
	TextBlock* msgblock = (TextBlock*)msgview->get("message");
	msgblock->m_text = msg;
	gui->show("message view");
}

void Resize_Window(Widget* w)
{
	Player* py = &g_player[g_localP];
	w->m_pos[0] = (float)(g_width/2 - 200);
	w->m_pos[1] = (float)(g_height/2 - 200);
	w->m_pos[2] = (float)(g_width/2 + 200);
	w->m_pos[3] = (float)(g_height/2 + 200);
}

void Resize_DebugInfo(Widget* w)
{
	Player* py = &g_player[g_localP];
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->m_pos[2] = (float)(40);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_Transx(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 1;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_Save(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 2;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_QSave(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 3;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_Load(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 4;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}

void Resize_Pause(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 0;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Play(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 1;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Fast(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 2;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_TrMgr(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 3;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Esc(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 4;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Ranks(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 5;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_Graphs(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 6;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*1;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40*2;
}

void Resize_PyGraphs(Widget* w)
{
	Player* py = &g_player[g_localP];
	int32_t i = 10;
	BmpFont* f = &g_font[CHATFONT];
	w->m_pos[0] = (float)(0 + 40*i);
	w->m_pos[1] = CHATTOP + (CHAT_LINES+2) * f->gheight;
	w->m_pos[2] = (float)(40 + 40*i);
	w->m_pos[3] = CHATTOP + (CHAT_LINES+2) * f->gheight + 40;
}


void Click_Pause()
{
	//g_speed = SPEED_PAUSE;
	ClStatePacket csp;
	csp.header.type = PACKET_CLSTATE;
	csp.chtype = CLCH_PAUSE;
	csp.client = g_localC;
	csp.downin = -1;
	if(g_localC >= 0)
	{
		Client* c = &g_client[g_localC];
		csp.downin = c->downin;
	}
	LockCmd((PacketHeader*)&csp);

#if 0
	char cmess[128];
	sprintf(cmess, "pause locking simfr:%u netfr:%u", g_simframe, g_netframe);
	RichText mess = RichText(cmess);
	AddNotif(&mess);
#endif
}

void Click_Play()
{
	if(g_localC >= 0)
	{
		Client* c = &g_client[g_localC];

		if(c->downin != -1)
		{
			RichText r = STRTABLE[STR_STILLDOWN];
			Mess(&r);
			return;
		}
	}

	//g_speed = SPEED_PLAY;
	ClStatePacket csp;
	csp.header.type = PACKET_CLSTATE;
	csp.chtype = CLCH_PLAY;
	csp.client = g_localC;
	csp.downin = -1;
	if(g_localC >= 0)
	{
		Client* c = &g_client[g_localC];
		csp.downin = c->downin;
	}
	LockCmd((PacketHeader*)&csp);
}

void Click_Fast()
{
	if(g_localC >= 0)
	{
		Client* c = &g_client[g_localC];

		if(c->downin != -1)
		{
			RichText r = STRTABLE[STR_STILLDOWN];
			Mess(&r);
			return;
		}
	}

	//g_speed = SPEED_FAST;
	ClStatePacket csp;
	csp.header.type = PACKET_CLSTATE;
	csp.chtype = CLCH_FAST;
	csp.client = g_localC;
	csp.downin = -1;
	if(g_localC >= 0)
	{
		Client* c = &g_client[g_localC];
		csp.downin = c->downin;
	}
	LockCmd((PacketHeader*)&csp);
}

void Click_DebugLines()
{
	g_debuglines = !g_debuglines;
}

void Click_Transx()
{
	g_drawtransx = !g_drawtransx;
}

void Click_Graphs()
{
	RichText ms = RichText("Coming soon.");
	Mess(&ms);
	return;

	GUI *gui = &g_gui;
	gui->show("gen graphs");
}

void Click_PyGraphs()
{
	RichText ms = RichText("Coming soon.");
	Mess(&ms);
	return;

	GUI *gui = &g_gui;
	gui->show("py graphs");
}

void Click_TrMgr()
{
	Selection* sel = &g_sel;
	sel->units.clear();
	GUI *gui = &g_gui;
	TruckMgr *trmgr = (TruckMgr*)gui->get("truck mgr");
	trmgr->regen(&g_sel);
	gui->show("truck mgr");
}

void Click_Ranks()
{
	g_keys[SDL_SCANCODE_TAB] = !g_keys[SDL_SCANCODE_TAB];
}

void Click_Save()
{
#if 0
	if(g_netmode != NETM_SINGLE)
	{
		Mess(&RichText("Coming soon to multiplayer."));
		return;
	}
#endif

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->hide("save");
	gui->hide("load");
	gui->show("save");
	((SaveView*)gui->get("save"))->regen(SAVEMODE_SAVES);
}

void Click_QSave()
{
#if 0
	if(g_netmode != NETM_SINGLE)
	{
		Mess(&RichText("Coming soon to multiplayer."));
		return;
	}
#endif

	if(!g_lastsave[0])
	{
		Click_Save();
		return;
	}

	SaveMap(g_lastsave);

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->hide("save");
	gui->hide("load");
}

void Click_Load()
{
	if(g_netmode != NETM_SINGLE)
	{
		//RichText mess = RichText("Coming soon to multiplayer.");
		RichText mess = STRTABLE[STR_MUSTBESINGLE];
		Mess(&mess);
		return;
	}

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->hide("save");
	gui->hide("load");
	gui->show("load");
	((LoadView*)gui->get("load"))->regen(SAVEMODE_SAVES);
}

void Click_QuitToMenu()
{
	EndSess();
}

void Click_HideBlPrev(int32_t dummy)
{
	GUI* gui = &g_gui;

	gui->hide("bl preview");
	//gui->hide("hide bl preview");
}

void FillPlay()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	g_vptype[VIEWPORT_MINIMAP] = VpType(Vec3f(0, 0, 0), Vec3f(0, 1, 0), "Minimap", true);
	g_vptype[VIEWPORT_ENTVIEW] = VpType(Vec3f(0, MAX_DISTANCE/2, 0), Vec3f(0, 1, 0), "EntView", false);

	//for(int32_t i=0; i<25; i++)
	//Sleep(6000);

	gui->add(new ViewLayer(gui, "play"));
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	//playview->add(new Image(NULL, "gui/backg/white.png", Resize_ResTicker));
	//playview->add(new Text(NULL, "res ticker", RichText(" "), MAINFONT16, Resize_ResTicker, true, 1, 1, 1, 1));
	playview->add(new ResTicker(playview, "res ticker", Resize_ResTicker));
	playview->add(new BotPan(playview, "bottom panel", Resize_BottomPanel));

	playview->add(new Button(playview, "debug lines", "gui/debuglines.png", RichText(), (STRTABLE[STR_SHOWDEBUG]), MAINFONT16, BUST_LEFTIMAGE, Resize_DebugInfo, Click_DebugLines, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "show info", "gui/transx.png", RichText(), (STRTABLE[STR_SHOWTRANSX]), MAINFONT16, BUST_LEFTIMAGE, Resize_Transx, Click_Transx, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "save", "gui/edsave.png", RichText(), (STRTABLE[STR_SAVEGAME2]), MAINFONT16, BUST_LEFTIMAGE, Resize_Save, Click_Save, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "qsave", "gui/qsave.png", RichText(), (STRTABLE[STR_QSAVE]), MAINFONT16, BUST_LEFTIMAGE, Resize_QSave, Click_QSave, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "load", "gui/edload.png", RichText(), (STRTABLE[STR_LOADGAME2]), MAINFONT16, BUST_LEFTIMAGE, Resize_Load, Click_Load, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "pause", "gui/pause.png", RichText(), (STRTABLE[STR_PAUSEUPD]), MAINFONT16, BUST_LEFTIMAGE, Resize_Pause, Click_Pause, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "play", "gui/play.png", RichText(), (STRTABLE[STR_PLAYUNLOCK]), MAINFONT16, BUST_LEFTIMAGE, Resize_Play, Click_Play, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "fast", "gui/fastforward.png", RichText(), (STRTABLE[STR_FASTUNLOCK]), MAINFONT16, BUST_LEFTIMAGE, Resize_Fast, Click_Fast, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "truck mgr", "gui/brbut/truck.png", RichText(), (STRTABLE[STR_TRUCKMGR]), MAINFONT16, BUST_LEFTIMAGE, Resize_TrMgr, Click_TrMgr, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "esc", "gui/esc.png", RichText(), (STRTABLE[STR_ESC]), MAINFONT16, BUST_LEFTIMAGE, Resize_Esc, Esc, NULL, NULL, NULL, NULL, -1, NULL));
	playview->add(new Button(playview, "ranks", "gui/rank.png", RichText(), (STRTABLE[STR_RANKS]), MAINFONT16, BUST_LEFTIMAGE, Resize_Ranks, Click_Ranks, NULL, NULL, NULL, NULL, -1, NULL));
	//playview->add(new Button(playview, "gen graphs", "gui/graphs.png", RichText(), RichText("General graphs"), MAINFONT16, BUST_LEFTIMAGE, Resize_Graphs, Click_Graphs, NULL, NULL, NULL, NULL, -1));
	//playview->add(new Button(playview, "py graphs", "gui/graphs.png", RichText(), RichText("Player graphs"), MAINFONT16, BUST_LEFTIMAGE, Resize_PyGraphs, Click_PyGraphs, NULL, NULL, NULL, NULL, -1));

	//AddNotif(playview);

	//preload all the button images
	//BuildMenu_OpenPage3();
	//BuildMenu_OpenPage2();
	//BuildMenu_OpenPage1();
	Click_NextBuildButton(0);

	gui->add(new ViewLayer(gui, "chat"));
	ViewLayer* chat = (ViewLayer*)gui->get("chat");
	AddNotif(chat);

	//gui->add(new ViewLayer(gui, "cs view"));
	gui->add(new CstrView(gui, "cs view", Resize_ConstructionView, Click_MoveConstruction, Click_CancelConstruction, Click_ProceedConstruction, Click_EstimateConstruction));
	//ViewLayer* constrview = (ViewLayer*)gui->get("cs view");

	//constrview->add(new CstrView(NULL, "cs view", Resize_ConstructionView, Click_MoveConstruction, Click_CancelConstruction, Click_ProceedConstruction, Click_EstimateConstruction));

	//gui->add(new ViewLayer(gui, "bl preview"));
	//gui->add(new ViewLayer(gui, "bl preview"));
	//ViewLayer* blprev = (ViewLayer*)gui->get("bl preview");
	//blprev->add(new TouchListener(blprev, "hide bl preview", Resize_Fullscreen, Click_HideBlPrev, NULL, NULL, -1));
	//blprev->add(new BlPreview(blprev, "bl preview", Resize_BlPreview));
	gui->add(new BlPreview(gui, "bl preview", Resize_BlPreview));
	//ViewLayer* blpreview = (ViewLayer*)gui->get("bl preview");

	//blpreview->add(new TouchListener(NULL, Resize_Fullscreen, NULL, NULL, NULL, -1));
	//blpreview->add(new BlPreview(blpreview, "bl preview", Resize_BlPreview));
	//blpreview->add(new Win(blpreview, "bl preview", Resize_BlPreview));
	
	gui->add(new BlView(gui, "bl view", Resize_BlPreview));
	gui->add(new BlGraphs(gui, "bl graphs", Resize_BlGraphs));
	gui->add(new TruckMgr(gui, "truck mgr", Resize_BlPreview));
	gui->add(new GenGraphs(gui, "gen graphs", Resize_BlPreview));
	gui->add(new PyGraphs(gui, "py graphs", Resize_BlPreview));

	gui->add(new SaveView(gui, "save", Resize_BlPreview));
	gui->add(new LoadView(gui, "load", Resize_BlPreview));

#if 0
	gui->add(new ViewLayer(gui, "construction estimate view"));
	ViewLayer* cev = (ViewLayer*)gui->get("construction estimate view");
#endif

	gui->add(new ViewLayer(gui, "ingame"));
	ViewLayer* ingame = (ViewLayer*)gui->get("ingame");
	ingame->add(new Image(ingame, "", "gui/backg/white.jpg", true, Resize_Fullscreen, 0, 0, 0, 0.5f));
	ingame->add(new TouchListener(ingame, "", Resize_Fullscreen, NULL, NULL, NULL, -1));
	ingame->add(new Link(ingame, "0", (STRTABLE[STR_QUITTOMENU]), MAINFONT16, Resize_MenuItem, Click_QuitToMenu));
	ingame->add(new Link(ingame, "1", (STRTABLE[STR_CANCEL]), MAINFONT16, Resize_MenuItem, Esc));

	gui->add(new ViewLayer(gui, "message view"));
	ViewLayer* msgview = (ViewLayer*)gui->get("message view");

	msgview->add(new Image(msgview, "", "gui/backg/white.jpg", true, Resize_Message));
	msgview->add(new TextBlock(msgview, "message", RichText(""), MAINFONT16, Resize_Message));
	msgview->add(new TouchListener(msgview, "", Resize_Fullscreen, NULL, NULL, NULL, -1));
	msgview->add(new Button(msgview, "continue button", "gui/transp.png", (STRTABLE[STR_CONTINUE]), RichText(""), MAINFONT16, BUST_LEFTIMAGE, Resize_MessageContinue, Click_MessageContinue, NULL, NULL, NULL, NULL, -1, NULL));
}
