











#include "script.h"
#include "../utils.h"
#include "../sim/bltype.h"
#include "../mod/mod.h"
#include "../gui/layouts/messbox.h"
#include "../gui/richtext.h"
#include "../math/vec2i.h"
#include "../sim/utype.h"
#include "../sim/conduit.h"
#include "../render/foliage.h"
#include "../language.h"
#include "../gui/icon.h"

ObjectScript::OS* g_os = NULL;

#if 0

void Script_Vec2i_free(ObjectScript::OS* os, void* data, void* user_param)
{
	Vec2i* v = (Vec2i*)data;
	//InfoMess("fr","free");
#if 0
	char m[123];
	sprintf(m, "free %d,%d", v->x, v->y);
	InfoMess("fr",m);
#endif
	v->~Vec2i();
	//delete v;
}
int32_t Script_Vec2i(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	//Vec2i* v = new Vec2i;
	//v->x = os->toInt(-nparams+0);
	//v->y = os->toInt(-nparams+1);
	//ObjectScript::OS_UserdataDtor dtor;

	Vec2i* v = (Vec2i*)os->pushUserdata(SCRIPT_DATATYPE_VEC2I, 0, Script_Vec2i_free, NULL);
	
	if(v)
	{
		v->x = os->toInt(-nparams+0);
		v->y = os->toInt(-nparams+1);
	}
	
#if 0
	char m[123];
	sprintf(m, "alloc %d,%d", v->x, v->y);
	InfoMess("fr",m);
#endif

	return 1;
}

void Script_Vec2s_free(ObjectScript::OS* os, void* data, void* user_param)
{
	Vec2s* v = (Vec2s*)data;
	v->~Vec2s();
}
int32_t Script_Vec2s(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	Vec2s* v = (Vec2s*)os->pushUserdata(SCRIPT_DATATYPE_VEC2S, 0, Script_Vec2s_free, NULL);
	
	if(v)
	{
		v->x = os->toInt(-nparams+0);
		v->y = os->toInt(-nparams+1);
	}

	return 1;
}

void Script_Vec3i_free(ObjectScript::OS* os, void* data, void* user_param)
{
	Vec3i* v = (Vec3i*)data;
	v->~Vec3i();
}
int32_t Script_Vec3i(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	Vec3i* v = (Vec3i*)os->pushUserdata(SCRIPT_DATATYPE_VEC3I, 0, Script_Vec3i_free, NULL);
	
	if(v)
	{
		v->x = os->toInt(-nparams+0);
		v->y = os->toInt(-nparams+1);
		v->z = os->toInt(-nparams+2);
	}

	return 1;
}

#if 0
void DefB(int32_t type,
		  const char* name,
		  const char* iconrel,
		  Vec2i size,
		  bool hugterr,
		  const char* sprel,
		  int32_t nframes,
		  const char* csprel,
		  int32_t cnframes,
		  int32_t foundation,
		  int32_t reqdeposit,
		  int32_t maxhp,
		  int32_t visrange,
		  int32_t opwage,
		  unsigned char flags,
		  int32_t prop,
		  int32_t wprop);
#endif

int32_t Script_DefB(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 17)
	{
		//char m[123];
		//sprintf(m, "
		AbortMod("DefB must have at least 17 parameters.");
		return 0;
	}

	int32_t type = os->toInt(-nparams+0);
	const char* name = os->toString(-nparams+1).toChar();
	const char* iconrel = os->toString(-nparams+2).toChar();
	Vec2i* size = (Vec2i*)os->toUserdata(SCRIPT_DATATYPE_VEC2I, -nparams+3);

	if(!size)
	{
		AbortMod("Third parameter to DefB must be of type Vec2i.");
		return 0;
	}

	bool hugterr = os->toBool(-nparams+4);
	const char* sprel = os->toString(-nparams+5).toChar();
	int32_t nframes = os->toInt(-nparams+6);
	const char* csprel = os->toString(-nparams+7).toChar();
	int32_t cnframes = os->toInt(-nparams+8);

	int32_t foundation = os->toInt(-nparams+9);
	int32_t reqdeposit = os->toInt(-nparams+10);
	int32_t maxhp = os->toInt(-nparams+11);
	int32_t visrange = os->toInt(-nparams+12);
	int32_t opwage = os->toInt(-nparams+13);
	unsigned char flags = (unsigned char)os->toInt(-nparams+14);
	int32_t prop = os->toInt(-nparams+15);
	int32_t wprop = os->toInt(-nparams+16);

	//TODO check values

	DefB(type, name, iconrel, *size, hugterr, sprel, nframes, csprel, cnframes, foundation, reqdeposit, maxhp, visrange, opwage, flags, prop, wprop);


#if 0

	//DefB(//16

	char m[123];
	sprintf(m, "debf params %d", nparams);
	InfoMess("os", m);

	//os->getObject(
	//os->pushUserdata(
	Vec2i* v = (Vec2i*)os->toUserdata(-nparams);

	ObjectScript::OS::String str = os->toString(-nparams);
	ObjectScript::OS::String str1 = os->toString(-nparams+1);
	ObjectScript::OS::String str2 = os->toString(-nparams+2);
	int32_t n3 = os->toInt(-nparams+3);
	int32_t n4 = os->toInt(-nparams+4);

	InfoMess("s0", str.toChar());
	InfoMess("s1", str1.toChar());
	InfoMess("s2", str2.toChar());

	sprintf(m, "n3 %d", n3);
	InfoMess("n3", m);
	
	sprintf(m, "n4 %d", n4);
	InfoMess("n4", m);
	
	bool b5 = os->toBool(-nparams+5);
	bool b6 = os->toBool(-nparams+6);
	bool b7 = os->toBool(-nparams+7);
	
	sprintf(m, "b5 %d", (int32_t)b5);
	InfoMess("nb", m);
	
	sprintf(m, "b6 %d", (int32_t)b6);
	InfoMess("nb", m);
	
	sprintf(m, "b7 %d", (int32_t)b7);
	InfoMess("nb", m);

	os->getGlobal("TILE_SIZE");

	int32_t ts = os->popInt();

	sprintf(m, "ts %d", (int32_t)ts);
	InfoMess("nb", m);
#endif

	return 0;
}

//void BMat(int32_t type, int32_t res, int32_t amt);
int32_t Script_BMat(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("BMat must have at least 3 parameters.");
		return 0;
	}
	
	int32_t type = os->toInt(-nparams+0);
	int32_t res = os->toInt(-nparams+1);
	int32_t amt = os->toInt(-nparams+2);

	BMat(type, res, amt);
}

//void BIn(int32_t type, int32_t res, int32_t amt);
int32_t Script_BIn(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("BIn must have at least 3 parameters.");
		return 0;
	}
	
	int32_t type = os->toInt(-nparams+0);
	int32_t res = os->toInt(-nparams+1);
	int32_t amt = os->toInt(-nparams+2);

	BIn(type, res, amt);
}

//void BOut(int32_t type, int32_t res, int32_t amt);
int32_t Script_BOut(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("BOut must have at least 4 parameters.");
		return 0;
	}
	
	int32_t type = os->toInt(-nparams+0);
	int32_t res = os->toInt(-nparams+1);
	int32_t amt = os->toInt(-nparams+2);
	int32_t defl = os->toInt(-nparams+3);

	BOut(type, res, amt, defl);
}

//void BEmit(int32_t type, int32_t emitterindex, int32_t ptype, Vec3f offset);
int32_t Script_BEmit(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("BOut must have at least 4 parameters.");
		return 0;
	}

	int32_t type = os->toInt(-nparams+0);
	int32_t emitterindex = os->toInt(-nparams+1);
	int32_t ptype = os->toInt(-nparams+2);
	Vec3i* offset = (Vec3i*)os->toUserdata(SCRIPT_DATATYPE_VEC3I, -nparams+3);

	if(!offset)
	{
		AbortMod("Fourth parameter to BEmit must be of type Vec3i.");
		return 0;
	}

	BEmit(type, emitterindex, ptype, *offset);
}

//TODO use STRTABLE ref int32_t for desc
//void BDes(int32_t type, const char* desc);
int32_t Script_BDes(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 2)
	{
		AbortMod("BDes must have at least 2 parameters.");
		return 0;
	}
	
	int32_t type = os->toInt(-nparams+0);
	const char* desc = os->toString(-nparams+1).toChar();

	BDes(type, desc);
}

//void BSon(int32_t type, int32_t stype, const char* relative);
int32_t Script_BSon(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("BSon must have at least 3 parameters.");
		return 0;
	}
	
	
	int32_t type = os->toInt(-nparams+0);
	int32_t stype = os->toInt(-nparams+1);
	const char* relative = os->toString(-nparams+2).toChar();
	
	BSon(type, stype, relative);
}

//void BMan(int32_t type, unsigned char utype)
int32_t Script_BMan(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("BMan must have at least 2 parameters.");
		return 0;
	}
	
	int32_t type = os->toInt(-nparams+0);
	unsigned char utype = (unsigned char)os->toInt(-nparams+1);

	BMan(type, utype);
}

//void DefR(int32_t resi, const char* n, const char* depn, int32_t iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a, int32_t conduit, const char* unit);
int32_t Script_DefR(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 13)
	{
		AbortMod("DefR must have at least 13 parameters.");
		return 0;
	}
	
	int32_t resi = os->toInt(-nparams+0);
	const char* n = os->toString(-nparams+1).toChar();
	const char* depn = os->toString(-nparams+2).toChar();
	int32_t iconindex = os->toInt(-nparams+3);
	bool phys = os->toBool(-nparams+4);
	bool cap = os->toBool(-nparams+5);
	bool glob = os->toBool(-nparams+6);
	float r = os->toFloat(-nparams+7);
	float g = os->toFloat(-nparams+8);
	float b = os->toFloat(-nparams+9);
	float a = os->toFloat(-nparams+10);
	int32_t conduit = os->toInt(-nparams+11);
	const char* unit = os->toString(-nparams+12).toChar();

	DefR(resi, n, depn, iconindex, phys, cap, glob, r, g, b, a, conduit, unit);
}

#if 0
void DefU(int32_t type, const char* sprel, int32_t nframes,
	Vec2s size, const char* name,
	int32_t starthp,
	bool landborne, bool walker, bool roaded, bool seaborne, bool airborne,
	int32_t cmspeed, bool military,
	int32_t visrange,
	int32_t prop);
#endif
int32_t Script_DefU(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 15)
	{
		AbortMod("DefU must have at least 15 parameters.");
		return 0;
	}
	
	int32_t type = os->toInt(-nparams+0);
	const char* sprel = os->toString(-nparams+1).toChar();
	int32_t frames = os->toInt(-nparams+2);
	Vec2s* size = (Vec2s*)os->toUserdata(SCRIPT_DATATYPE_VEC2S, -nparams+3);
	
	if(!size)
	{
		AbortMod("Fourth parameter to DefU must be of type Vec2s.");
		return 0;
	}
	
	const char* name = os->toString(-nparams+4).toChar();
	int32_t starthp = os->toInt(-nparams+5);
	bool landborne = os->toInt(-nparams+6);
	bool walker = os->toInt(-nparams+7);
	bool roaded = os->toInt(-nparams+8);
	bool seaborne = os->toInt(-nparams+9);
	bool airborne = os->toInt(-nparams+10);
	int32_t cmspeed = os->toInt(-nparams+11);
	bool military = os->toInt(-nparams+12);
	int32_t visrange = os->toInt(-nparams+13);
	int32_t prop = os->toInt(-nparams+14);

	DefU(type, sprel, frames, *size, name, starthp, landborne, walker, roaded, seaborne, airborne, cmspeed, military, visrange, prop);
}

//void UCost(int32_t type, int32_t res, int32_t amt);
int32_t Script_UCost(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("UCost must have at least 3 parameters.");
		return 0;
	}
	
	int32_t type = os->toInt(-nparams+0);
	int32_t res = os->toInt(-nparams+1);
	int32_t amt = os->toInt(-nparams+2);

	UCost(type, res, amt);
}

//void DefF(int32_t type, const char* sprel, Vec2s size);
int32_t Script_DefF(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("DefF must have at least 3 parameters.");
		return 0;
	}

	int32_t type = os->toInt(-nparams+0);
	const char* sprel = os->toString(-nparams+1).toChar();
	Vec2s* size = (Vec2s*)os->toUserdata(SCRIPT_DATATYPE_VEC2S, -nparams+2);

	if(!size)
	{
		AbortMod("Third parameter to DefF must be of type Vec2s.");
		return 0;
	}

	DefF(type, sprel, *size);
}

#if 0
void DefCd(unsigned char ctype,
			const char* name,
			const char* iconrel,
           bool blconduct,
           bool cornerpl,
           Vec2i physoff,
           Vec3i drawoff,
		   const char* lacktex,
		   uint16_t maxsideincl,
		   uint16_t maxforwincl,
		   bool reqsource,
		  unsigned char flags);
#endif
int32_t Script_DefCd(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 11)
	{
		AbortMod("DefCd must have at least 11 parameters.");
		return 0;
	}
	
	unsigned char ctype = os->toInt(-nparams+0);
	const char* name = os->toString(-nparams+1).toChar();
	const char* iconrel = os->toString(-nparams+2).toChar();
	bool blconduct = os->toBool(-nparams+3);
	bool cornerpl = os->toBool(-nparams+4);
	Vec2i* physoff = (Vec2i*)os->toUserdata(SCRIPT_DATATYPE_VEC2I, -nparams+5);
	Vec3i* drawoff = (Vec3i*)os->toUserdata(SCRIPT_DATATYPE_VEC3I, -nparams+6);
	const char* lacktex = os->toString(-nparams+7).toChar();
	uint16_t maxsideincl = os->toInt(-nparams+8);
	uint16_t maxforwincl = os->toInt(-nparams+9);
	bool reqsource = os->toBool(-nparams+10);
	unsigned char flags = (unsigned char)os->toInt(-nparams+11);

	if(!physoff)
	{
		AbortMod("Fifth parameter to DefCd must be of type Vec2i.");
		return 0;
	}
	
	if(!drawoff)
	{
		AbortMod("Sixth parameter to DefCd must be of type Vec3i.");
		return 0;
	}

	DefCd(ctype, name, iconrel, blconduct, cornerpl, *physoff, *drawoff, lacktex, maxsideincl, maxforwincl, reqsource, flags);
}

//void CdMat(unsigned char ctype, unsigned char rtype, int16_t ramt);
int32_t Script_CdMat(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 3)
	{
		AbortMod("CdMat must have at least 3 parameters.");
		return 0;
	}
	
	unsigned char ctype = os->toInt(-nparams+0);
	unsigned char rtype = os->toInt(-nparams+1);
	int16_t ramt = os->toInt(-nparams+2);

	CdMat(ctype, rtype, ramt);
}

//void CdDes(unsigned char ctype, const char* desc);
int32_t Script_CdDes(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 2)
	{
		AbortMod("CdDes must have at least 2 parameters.");
		return 0;
	}

	unsigned char ctype = os->toInt(-nparams+0);
	const char* desc = os->toString(-nparams+1).toChar();

	CdDes(ctype, desc);
}

#if 0
void DefConn(unsigned char conduittype,
	unsigned char connectiontype,
	bool finished,
	const char* sprel);
#endif
int32_t Script_DefConn(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 4)
	{
		AbortMod("DefConn must have at least 4 parameters.");
		return 0;
	}
	
	unsigned char conduittype = os->toInt(-nparams+0);
	unsigned char connectiontype = os->toInt(-nparams+1);
	bool finished = os->toBool(-nparams+2);
	const char* sprel = os->toString(-nparams+3).toChar();

	DefConn(conduittype, connectiontype, finished, sprel);
}

//void SwitchLang(const char* dir);
int32_t Script_SwitchLang(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
	if(nparams < 1)
	{
		AbortMod("SwitchLang must have at least 1 parameter.");
		return 0;
	}

	const char* dir = os->toString(-nparams+0).toChar();

	SwitchLang(dir);
}

//void DefI(int32_t type, const char* relative, const UStr tag);
int32_t Script_DefI(ObjectScript::OS* os, int32_t nparams, int32_t closure_values, int32_t need_ret_values, void * param)
{
#ifndef MATCHMAKER
#ifndef DEDICATED
	if(nparams < 3)
	{
		//char m[123];
		//sprintf(m, "
		AbortMod("DefI must have at least 3 parameters.");
		return 0;
	}

	int32_t type = os->toInt(-nparams+0);
	const char* relative = os->toString(-nparams+1).toChar();
	const char* tag = os->toString(-nparams+2).toChar();

	DefI(type, relative, tag);

#endif
#endif
}


void DefGlobals(ObjectScript::OS* os)
{	
	os->pushCFunction(Script_DefB);
	os->setGlobal("DefB");
	
	os->pushCFunction(Script_BMat);
	os->setGlobal("BMat");
	
	os->pushCFunction(Script_BIn);
	os->setGlobal("BIn");
	
	os->pushCFunction(Script_BOut);
	os->setGlobal("BOut");

	os->pushCFunction(Script_BEmit);
	os->setGlobal("BEmit");
	
	os->pushCFunction(Script_BDes);
	os->setGlobal("BDes");

	os->pushCFunction(Script_BSon);
	os->setGlobal("BSon");

	os->pushCFunction(Script_BMan);
	os->setGlobal("BMan");
	
	os->pushCFunction(Script_DefR);
	os->setGlobal("DefR");

	os->pushCFunction(Script_DefU);
	os->setGlobal("DefU");

	os->pushCFunction(Script_UCost);
	os->setGlobal("UCost");
	
	os->pushCFunction(Script_DefF);
	os->setGlobal("DefF");
	
	os->pushCFunction(Script_DefCd);
	os->setGlobal("DefCd");
	
	os->pushCFunction(Script_CdMat);
	os->setGlobal("CdMat");
	
	os->pushCFunction(Script_CdDes);
	os->setGlobal("CdDes");
	
	os->pushCFunction(Script_DefConn);
	os->setGlobal("DefConn");
	
	os->pushCFunction(Script_SwitchLang);
	os->setGlobal("SwitchLang");
	
	os->pushCFunction(Script_DefI);
	os->setGlobal("DefI");

	os->pushCFunction(Script_Vec2i);
	os->setGlobal("Vec2i");

	os->pushCFunction(Script_Vec2s);
	os->setGlobal("Vec2s");
	
	os->pushCFunction(Script_Vec3i);
	os->setGlobal("Vec3i");
}

//void LoadMod(const char* modrel)
//{
	//g_os = ObjectScript::OS::create();
	//g_os->pushCFunction(testfunc);
	//g_os->setGlobal("testfunc");
	//os->eval("testfunc();");
	//os->eval("function require(){ /* if(relative == \"called.os\") */ { testfunc(); } }");
	//char autoexecpath[WF_MAX_PATH+1];
	//FullPath("scripts/autoexec.os", autoexecpath);
	//g_os->require(autoexecpath);
	//g_os->release();
//}

void ExecScript(const char* screl)
{
	//InfoMess(screl,screl);
	char scfull[WF_MAX_PATH+1];
	FullPath(screl, scfull);
	//InfoMess(scfull,scfull);

	//for(int32_t i=0; i<900000; i++)
	//	1+1;

	ObjectScript::OS* os = ObjectScript::OS::create();

	os->pushCFunction(Script_DefB);
	os->setGlobal("DefB");
	
	os->pushCFunction(Script_Vec2i);
	os->setGlobal("Vec2i");
	
	//os->setGlobal(ObjectScript::def("Vec2i", Script_Vec2i));   // use binder

	//os->eval("testfunc();");
	//os->eval("function require(){ /* if(relative == \"called.os\") */ { testfunc(); } }");
	os->require(scfull);
	//os->setGlobal(def("test", test));   // use binder
	//os->eval("print(test(2, 3))");      // outputs: 5
	os->release();
}

#endif