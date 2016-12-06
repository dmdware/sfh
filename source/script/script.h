









#ifndef SCRIPT_H
#define SCRIPT_H

#include "../platform.h"
#include "../script/objectscript.h"
#include "../script/os-binder.h"
#include "../math/vec2i.h"
#include "../math/vec3i.h"
#include "../math/vec2s.h"

#define SCRIPT_DATATYPE_ERR			0
#define SCRIPT_DATATYPE_VEC2I		1
#define SCRIPT_DATATYPE_VEC3I		2
#define SCRIPT_DATATYPE_VEC2S		3

//TODO disable floats in OS or do sync checks in multiplayer

extern ObjectScript::OS* g_os;

void ExecScript(const char* screl);
void DefGlobals(ObjectScript::OS* os);


#endif