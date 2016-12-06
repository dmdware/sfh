










#include "../app/appmain.h"
#include "keymap.h"
#include "../platform.h"
#include "../gui/gui.h"
#include "../math/camera.h"
#include "../math/vec3f.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "../window.h"
#include "../sim/player.h"
#include "../trigger/console.h"

void MouseMidButtonDown()
{
	if(g_appmode == APPMODE_PLAY || g_appmode == APPMODE_EDITOR)
	{
		Player* py = &g_player[g_localP];

		if(g_mousekeys[MOUSE_MIDDLE])
		{
			CenterMouse();
		}
	}
}

void MouseMidButtonUp()
{
}

void MouseWheel(int32_t delta)
{
	if(g_appmode == APPMODE_PLAY ||
		g_appmode == APPMODE_EDITOR)
	{
	}
}

void Esc()
{
	if(g_appmode == APPMODE_PLAY ||
		g_appmode == APPMODE_EDITOR)
	{
		Player* py = &g_player[g_localP];
		GUI* gui = &g_gui;
		Widget* ingame = gui->get("ingame");

		if(!ingame->m_hidden)
			gui->hide("ingame");
		else
			gui->show("ingame");
	}
}

void MapKeys()
{
	for(int32_t i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];
		GUI* gui = &g_gui;
		gui->assignmousewheel(&MouseWheel);
		gui->assignmbutton(MouseMidButtonDown, MouseMidButtonUp);
		//gui->assignkey(SDL_SCANCODE_R, ZoomOut, NULL);
		//gui->assignkey(SDL_SCANCODE_F, ZoomIn, NULL);
		gui->assignkey(SDL_SCANCODE_GRAVE, NULL, ToggleConsole);
		gui->assignkey(SDL_SCANCODE_ESCAPE, NULL, Esc);
	}

	/*
	int32_t key;
	void (*down)();
	void (*up)();
	ifstream f("keymap.ini");
	std::string line;
	char keystr[32];
	char actstr[32];

	while(!f.eof())
	{
		key = -1;
		down = NULL;
		up = NULL;
		strcpy(keystr, "");
		strcpy(actstr, "");

		getline(f, line);
		sscanf(line.c_str(), "%s %s", keystr, actstr);

		if(stricmp(keystr, "SDLK_ESCAPE") == 0)			key = SDLK_ESCAPE;
		else if(stricmp(keystr, "SDLK_SHIFT") == 0)		key = SDLK_SHIFT;
		else if(stricmp(keystr, "SDLK_CONTROL") == 0)		key = SDLK_CONTROL;
		else if(stricmp(keystr, "SDLK_SPACE") == 0)		key = SDLK_SPACE;
		else if(stricmp(keystr, "MouseLButton") == 0)	key = -2;
		else if(stricmp(keystr, "F1") == 0)				key = SDLK_F1;
		else if(stricmp(keystr, "F2") == 0)				key = SDLK_F2;
		else if(stricmp(keystr, "F3") == 0)				key = SDLK_F3;
		else if(stricmp(keystr, "F4") == 0)				key = SDLK_F4;
		else if(stricmp(keystr, "F5") == 0)				key = SDLK_F5;
		else if(stricmp(keystr, "F6") == 0)				key = SDLK_F6;
		else if(stricmp(keystr, "F7") == 0)				key = SDLK_F7;
		else if(stricmp(keystr, "F8") == 0)				key = SDLK_F8;
		else if(stricmp(keystr, "F9") == 0)				key = SDLK_F9;
		else if(stricmp(keystr, "F10") == 0)			key = SDLK_F10;
		else if(stricmp(keystr, "F11") == 0)			key = SDLK_F11;
		else if(stricmp(keystr, "F12") == 0)			key = SDLK_F12;
		else if(stricmp(keystr, "'A'") == 0)			key = 'A';
		else if(stricmp(keystr, "'B'") == 0)			key = 'B';
		else if(stricmp(keystr, "'C'") == 0)			key = 'C';
		else if(stricmp(keystr, "'D'") == 0)			key = 'D';
		else if(stricmp(keystr, "'E'") == 0)			key = 'E';
		else if(stricmp(keystr, "'cost'") == 0)			key = 'cost';
		else if(stricmp(keystr, "'G'") == 0)			key = 'G';
		else if(stricmp(keystr, "'H'") == 0)			key = 'H';
		else if(stricmp(keystr, "'I'") == 0)			key = 'I';
		else if(stricmp(keystr, "'J'") == 0)			key = 'J';
		else if(stricmp(keystr, "'K'") == 0)			key = 'K';
		else if(stricmp(keystr, "'L'") == 0)			key = 'L';
		else if(stricmp(keystr, "'M'") == 0)			key = 'M';
		else if(stricmp(keystr, "'N'") == 0)			key = 'N';
		else if(stricmp(keystr, "'O'") == 0)			key = 'O';
		else if(stricmp(keystr, "'P'") == 0)			key = 'P';
		else if(stricmp(keystr, "'Q'") == 0)			key = 'Q';
		else if(stricmp(keystr, "'R'") == 0)			key = 'R';
		else if(stricmp(keystr, "'S'") == 0)			key = 'S';
		else if(stricmp(keystr, "'T'") == 0)			key = 'T';
		else if(stricmp(keystr, "'U'") == 0)			key = 'U';
		else if(stricmp(keystr, "'V'") == 0)			key = 'V';
		else if(stricmp(keystr, "'W'") == 0)			key = 'W';
		else if(stricmp(keystr, "'X'") == 0)			key = 'X';
		else if(stricmp(keystr, "'Y'") == 0)			key = 'Y';
		else if(stricmp(keystr, "'Z'") == 0)			key = 'Z';
		else if(stricmp(keystr, "'0'") == 0)			key = '0';
		else if(stricmp(keystr, "'1'") == 0)			key = '1';
		else if(stricmp(keystr, "'2'") == 0)			key = '2';
		else if(stricmp(keystr, "'3'") == 0)			key = '3';
		else if(stricmp(keystr, "'4'") == 0)			key = '4';
		else if(stricmp(keystr, "'5'") == 0)			key = '5';
		else if(stricmp(keystr, "'6'") == 0)			key = '6';
		else if(stricmp(keystr, "'7'") == 0)			key = '7';
		else if(stricmp(keystr, "'8'") == 0)			key = '8';
		else if(stricmp(keystr, "'9'") == 0)			key = '9';

		if(key == -1)
		{
			Log("Unknown input: "<<keystr);
			continue;
		}

		if(stricmp(actstr, "Esc();") == 0)				{	down = &Esc;			up = NULL;			}
		else if(stricmp(actstr, "Forward();") == 0)			{	down = &Forward;		up = NULL;			}
		else if(stricmp(actstr, "Left();") == 0)			{	down = &Left;			up = NULL;			}
		else if(stricmp(actstr, "Right();") == 0)			{	down = &Right;			up = NULL;			}
		else if(stricmp(actstr, "Back();") == 0)			{	down = &Back;			up = NULL;			}

		if(down == NULL)		Log("Unknown action: "<<actstr);
		else if(key == -2)		AssignLButton(down, up);
		else					AssignKey(key, down, up);
	}
	*/
}
