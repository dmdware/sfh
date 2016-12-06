

#include "demoprop.h"
#include "../gui/layouts/demogui.h"
#include "../sim/player.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/conduit.h"
#include "../sim/map.h"
#include "../net/lockstep.h"
#include "../gui/layouts/messbox.h"
#include "selection.h"

void Click_DemoProp()
{
	//RichText r = RichText("Demolish?");
	//Mess(&r);
	GUI* gui = &g_gui;
	gui->hide("bl view");

	DemolPropPacket dpp;
	dpp.header.type = PACKET_DEMOLPROP;
	dpp.pi = g_localP;

	if(g_sel.buildings.size())
	{
		int32_t bi = *g_sel.buildings.begin();
		Building* b = &g_building[bi];
		dpp.propi = bi;
		dpp.proptype = PROP_BL_BEG + b->type;
		LockCmd((PacketHeader*)&dpp);
		g_sel.buildings.clear();
	}
	else if(g_sel.roads.size())
	{
		dpp.proptype = PROP_CD_BEG + CD_ROAD;

		for(std::list<Vec2i>::iterator ctit=g_sel.roads.begin(); ctit!=g_sel.roads.end(); ctit++)
		{
			dpp.tx = ctit->x;
			dpp.ty = ctit->y;
			dpp.propi = ctit->x + ctit->y * g_mapsz.x;
			LockCmd((PacketHeader*)&dpp);
		}

		g_sel.roads.clear();
	}
	else if(g_sel.powls.size())
	{
		dpp.proptype = PROP_CD_BEG + CD_POWL;

		for(std::list<Vec2i>::iterator ctit=g_sel.powls.begin(); ctit!=g_sel.powls.end(); ctit++)
		{
			dpp.tx = ctit->x;
			dpp.ty = ctit->y;
			dpp.propi = ctit->x + ctit->y * g_mapsz.x;
			LockCmd((PacketHeader*)&dpp);
		}

		g_sel.powls.clear();
	}
	else if(g_sel.crpipes.size())
	{
		dpp.proptype = PROP_CD_BEG + CD_CRPIPE;

		for(std::list<Vec2i>::iterator ctit=g_sel.crpipes.begin(); ctit!=g_sel.crpipes.end(); ctit++)
		{
			dpp.tx = ctit->x;
			dpp.ty = ctit->y;
			dpp.propi = ctit->x + ctit->y * g_mapsz.x;
			LockCmd((PacketHeader*)&dpp);
		}

		g_sel.crpipes.clear();
	}
}

void Demolish(int32_t pyi, int32_t proptype, int32_t propi, int32_t tx, int32_t ty)
{
	if(proptype >= PROP_BL_BEG &&
		proptype < PROP_BL_END)
	{
		Building* b = &g_building[propi];
		b->destroy();
		b->on = false;
	}
	else if(proptype >= PROP_CD_BEG &&
		proptype < PROP_CD_END)
	{
		int32_t ctype = proptype = PROP_CD_BEG;

		CdTile* ctile = GetCd(ctype, tx, ty, false);

		ctile->destroy();
		ctile->on = false;
	}
}