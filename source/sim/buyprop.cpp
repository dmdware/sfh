
#include "buyprop.h"
#include "selection.h"
#include "player.h"
#include "building.h"
#include "bltype.h"
#include "unit.h"
#include "utype.h"
#include "conduit.h"
#include "../gui/gui.h"
#include "../platform.h"
#include "../gui/layouts/chattext.h"
#include "../net/lockstep.h"
#include "../language.h"

void Click_BuyProp()
{
	Selection* sel = &g_sel;
	Player* py = &g_player[g_localP];
	bool owned = false;	//owned by current player?
	Player* opy;
	int32_t propi = -1;
	int32_t propprice;
	int32_t proptype;
	int8_t tx = -1;
	int8_t ty = -1;

	//TODO units
	//TODO conduits

	if(sel->buildings.size())
	{
		BlType* bt = NULL;
		Building* b = NULL;

		propi = *sel->buildings.begin();
		b = &g_building[propi];
		bt = &g_bltype[b->type];

		if(b->owner == g_localP)
			owned = true;

		if(!b->forsale)
			return;

		proptype = PROP_BL_BEG + b->type;
		propprice = b->propprice;

#if 0
		if(b->type == BL_NUCPOW)
		{
			char msg[1280];
			sprintf(msg, "blview \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d",
				(int32_t)b->transporter[RES_URANIUM],
				(int32_t)g_unit[b->transporter[RES_URANIUM]].mode,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].target,
				bi,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].targtype,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].cargotype);
			InfoMess(msg, msg);
		}
#endif

		opy = &g_player[b->owner];
	}
	else if(sel->units.size())
	{
		Unit* u = NULL;

		propi = *sel->units.begin();
		u = &g_unit[propi];

		if(u->owner == g_localP)
			owned = true;

		if(!u->forsale)
			return;

		proptype = PROP_U_BEG + u->type;
		propprice = u->price;
	}
#if 1
	else if(sel->roads.size() > 0)
	{
		Vec2i t = *sel->roads.begin();
		
		tx = t.x;
		ty = t.y;

		CdType* ct = &g_cdtype[CD_ROAD];

		CdTile* ctile = GetCd(CD_ROAD, tx, ty, false);

		if(ctile->owner == g_localP)
			owned = true;

		if(!ctile->selling)
			return;

		proptype = PROP_CD_BEG + CD_ROAD;
		propprice = 0;
	}
	else if(sel->powls.size() > 0)
	{
		Vec2i t = *sel->powls.begin();
		
		tx = t.x;
		ty = t.y;

		CdType* ct = &g_cdtype[CD_POWL];

		CdTile* ctile = GetCd(CD_POWL, tx, ty, false);

		if(ctile->owner == g_localP)
			owned = true;

		if(!ctile->selling)
			return;

		proptype = PROP_CD_BEG + CD_POWL;
		propprice = 0;
	}
	else if(sel->crpipes.size() > 0)
	{
		Vec2i t = *sel->crpipes.begin();
		
		tx = t.x;
		ty = t.y;

		CdType* ct = &g_cdtype[CD_CRPIPE];

		CdTile* ctile = GetCd(CD_CRPIPE, tx, ty, false);

		if(ctile->owner == g_localP)
			owned = true;

		if(!ctile->selling)
			return;

		proptype = PROP_CD_BEG + CD_CRPIPE;
		propprice = 0;
	}
#endif

	if(py->global[RES_DOLLARS] < propprice)
	{
		RichText textr = STRTABLE[STR_INSUFBUY];
		AddNotif(&textr);
		return;
	}

	BuyPropPacket bpp;
	bpp.header.type = PACKET_BUYPROP;
	bpp.proptype = proptype;
	bpp.propi = propi;
	bpp.pi = g_localP;
	bpp.tx = tx;
	bpp.ty = ty;
	LockCmd((PacketHeader*)&bpp);
}