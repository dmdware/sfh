











#include "sprite.h"
#include "../utils.h"
#include "../texture.h"
#include "../gui/gui.h"
#include "../debug.h"
#include "tile.h"

std::vector<SpriteToLoad> g_spriteload;
int32_t g_lastLSp = -1;
Sprite g_sprite[SPRITES];
SpList g_splist[SPRITELISTS];

Sprite::Sprite()
{
	on = false;
	difftexi = 0;
	teamtexi = 0;
	pixels = NULL;
}

Sprite::~Sprite()
{
    free();
}

void Sprite::free()
{
    if(pixels)
    {
        delete pixels;
        pixels = NULL;
    }

	//Free textures?

	on = false;
}

void FreeSprites()
{
	for(int32_t i=0; i<SPRITES; i++)
	{
		Sprite* s = &g_sprite[i];

		if(!s->on)
			continue;

		s->free();
	}
	
	for(int32_t i=0; i<SPRITELISTS; i++)
	{
		SpList* sl = &g_splist[i];

		if(!sl->on)
			continue;

		sl->free();
	}
}

bool Load1Sprite()
{
	if(g_lastLSp+1 < g_spriteload.size())
		SetStatus(g_spriteload[g_lastLSp+1].relative.c_str());

	//char m[123];
	//sprintf(m, "l%d", (int)g_spriteload.size()-g_lastLSp);
	//InfoMess("asd",m);

	CHECKGLERROR();

	//g_gui.hideall();
	//g_gui.show("loading");

	if(g_lastLSp >= 0)
	{
		SpriteToLoad* s = &g_spriteload[g_lastLSp];
		if(!LoadSprite(s->relative.c_str(), s->spindex, s->loadteam, s->loaddepth))
		{
			char m[128];
			sprintf(m, "Failed to load sprite %s", s->relative.c_str());
			ErrMess("Error", m);
		}
	}

	g_lastLSp ++;

	if(g_lastLSp >= g_spriteload.size())
	{
		return false;	// Done loading all
	}

	return true;	// Not finished loading
}

void QueueSprite(const char* relative, uint32_t* spindex, bool loadteam, bool loaddepth)
{
	SpriteToLoad stl;
	stl.relative = relative;
	stl.spindex = spindex;
	stl.loadteam = loadteam;
	stl.loaddepth = loaddepth;
	g_spriteload.push_back(stl);
}

int32_t NewSprite()
{
	for(int32_t i=0; i<SPRITES; i++)
	{
		Sprite* s = &g_sprite[i];

		if(!s->on)
			return i;
	}

	return -1;
}

int32_t NewSpriteList()
{
	for(int32_t i=0; i<SPRITELISTS; i++)
	{
		SpList* sl = &g_splist[i];

		if(!sl->on)
			return i;
	}

	return -1;
}

bool FindSprite(uint32_t &spriteidx, const char* relative)
{
	char corrected[WF_MAX_PATH+1];
	strcpy(corrected, relative);
	CorrectSlashes(corrected);
	char fullpath[WF_MAX_PATH+1];
	FullPath(corrected, fullpath);

	for(int32_t i=0; i<SPRITES; i++)
	{
		Sprite* s = &g_sprite[i];

		if(s->on && stricmp(s->fullpath.c_str(), fullpath) == 0)
		{
			//g_texindex = i;
			//texture = t->texname;
			spriteidx = i;
			return true;
		}
	}

	return false;
}

/*
TODO
Convert to C90
All upper case to lower case
CreateTex -> createtex
All class to struct
LoadedTex -> loadedtex_t or loadedtex
*/

bool LoadSprite(const char* relative, uint32_t* spindex, bool loadteam, bool loaddepth)
{
#ifdef ISOTOP
	loaddepth = false;
#endif

	if(FindSprite(*spindex, relative))
		return true;

	int32_t i = NewSprite();

	if(i < 0)
		return false;

	Sprite* s = &g_sprite[i];
	s->on = true;
	*spindex = i;

	char full[WF_MAX_PATH+1];
	FullPath(relative, full);
	CorrectSlashes(full);
	s->fullpath = full;

	char reltxt[WF_MAX_PATH+1];
	char reldiff[WF_MAX_PATH+1];
	char relteam[WF_MAX_PATH+1];
	char reldepth[WF_MAX_PATH+1];
	sprintf(reltxt, "%s.txt", relative);
	sprintf(reldiff, "%s.png", relative);
	sprintf(relteam, "%s_team.png", relative);
	sprintf(reldepth, "%s_depth.png", relative);
	ParseSprite(reltxt, s);

	CreateTex(s->difftexi, reldiff, true, false);
	//if(loadteam)
		CreateTex(s->teamtexi, relteam, true, false);
#ifndef PLATFORM_MOBILE
	if(loaddepth)
		CreateTex(s->depthtexi, reldepth, true, false);
#endif
	
	char pixfull[WF_MAX_PATH+1];
	FullPath(reldiff, pixfull);
	s->pixels = LoadTexture(pixfull);

	if(!s->pixels)
	{
		Log("Failed to load sprite %s\r\n", relative);
		return false;
	}
	else
		Log("%s\r\n", relative);

	return true;
}

bool LoadSpriteList(const char* relative, uint32_t* splin, bool loadteam, bool loaddepth, bool queue)
{
	//if(FindSpriteList(*splin, relative))
	//	return;

	//loaddepth = false; 

	int32_t i = NewSpriteList();

	if(i < 0)
		return false;

	SpList* sl = &g_splist[i];
	sl->on = true;
	*splin = i;
	
	char full[WF_MAX_PATH+1];
	FullPath(relative, full);
	CorrectSlashes(full);
	sl->fullpath = full;

	char txtpath[WF_MAX_PATH+1];
	sprintf(txtpath, "%s_list.txt", full);

	int32_t nsides = 1;
	bool dorots = false;	//rotations
	bool dosides = false;	//sides
	bool doincls = false;	//inclines
	bool doframes = false;	//frames
	int32_t nframes = 1;

	FILE* fp = fopen(txtpath, "r");

	if(!fp)
		return false;

	char line[128];

	do
	{
		fgets(line, 100, fp);

		char* tok = strtok(line, " ");

		if(!tok)
			continue;

		if(strcmp(tok, "inclines") == 0)
		{
			tok = strtok(NULL, " ");

			if(!tok)
				continue;

			doincls = (bool)StrToInt(tok);
		}
		else if(strcmp(tok, "frames") == 0)
		{
			tok = strtok(NULL, " ");

			if(!tok)
				continue;

			doframes = (bool)StrToInt(tok);

			tok = strtok(NULL, " ");

			if(!doframes || !tok)
				continue;

			nframes = StrToInt(tok);
		}
		else if(strcmp(tok, "sides") == 0)
		{
			tok = strtok(NULL, " ");

			if(!tok)
				continue;

			dosides = (bool)StrToInt(tok);

			tok = strtok(NULL, " ");

			if(!dosides || !tok)
				continue;

			nsides = StrToInt(tok);
		}
		else if(strcmp(tok, "rotations") == 0)
		{
			tok = strtok(NULL, " ");

			if(!tok)
				continue;

			dorots = (bool)StrToInt(tok);

			tok = strtok(NULL, " ");

			if(!dorots || !tok)
				continue;

			nsides = StrToInt(tok);
		}
	}while(!feof(fp));

	fclose(fp);

	int32_t ncombos = 1;
	int32_t sidechunk = 1;
	int32_t inclchunk = 1;
	int32_t frameschunk = 1;

	if(dorots)
	{
		ncombos *= nsides;
		ncombos *= nsides;
		ncombos *= nsides;
		sidechunk = nsides * nsides * nsides;
	}
	else if(dosides)
	{
		ncombos *= nsides;
		sidechunk = nsides;
	}

	inclchunk = sidechunk;

	if(doincls)
	{
		ncombos *= INCLINES;
		inclchunk = sidechunk * INCLINES;
	}

	frameschunk = inclchunk;

	if(doframes)
	{
		ncombos *= nframes;
		frameschunk = inclchunk * nframes;
	}

	sl->frames = doframes;
	sl->nframes = nframes;
	sl->inclines = doincls;
	sl->nsides = nsides;
	sl->sides = dosides;
	sl->rotations = dorots;
	sl->sprites = new uint32_t [ ncombos ];

	int32_t incli = 0;
	int32_t pitchi = 0;
	int32_t yawi = 0;
	int32_t rolli = 0;
	int32_t sidei = 0;
	int32_t framei = 0;

	for(int32_t ci=0; ci<ncombos; ci++)
	{
		if(dorots)
		{
			yawi = ci % nsides;
			pitchi = (ci / nsides) % nsides;
			rolli = (ci / nsides / nsides) % nsides;
		}
		else if(dosides)
		{
			sidei = ci % nsides;
		}

		if(doincls)
		{
			incli = (ci / sidechunk) % INCLINES;
		}

		if(doframes)
		{
			framei = (ci / inclchunk) % nframes;
		}

		char combo[WF_MAX_PATH+1];

		char frame[32];
		char side[32];
		strcpy(frame, "");
		strcpy(side, "");

		if(doframes)
			sprintf(frame, "_fr%03d", framei);

		if(dosides && !dorots)
			sprintf(side, "_si%d", sidei);
		else if(dorots)
			sprintf(side, "_y%dp%dr%d", yawi, pitchi, rolli);

		//std::string incline = "";
		char incline[32] = "";

		//TODO use INCLINENAME
		if(doincls)
		{
#if 0
			if(incli == IN_0000)	incline = "_inc0000";
			else if(incli == IN_0001)	incline = "_inc0001";
			else if(incli == IN_0010)	incline = "_inc0010";
			else if(incli == IN_0011)	incline = "_inc0011";
			else if(incli == IN_0100)	incline = "_inc0100";
			else if(incli == IN_0101)	incline = "_inc0101";
			else if(incli == IN_0110)	incline = "_inc0110";
			else if(incli == IN_0111)	incline = "_inc0111";
			else if(incli == IN_1000)	incline = "_inc1000";
			else if(incli == IN_1001)	incline = "_inc1001";
			else if(incli == IN_1010)	incline = "_inc1010";
			else if(incli == IN_1011)	incline = "_inc1011";
			else if(incli == IN_1100)	incline = "_inc1100";
			else if(incli == IN_1101)	incline = "_inc1101";
			else if(incli == IN_1110)	incline = "_inc1110";
#else
			sprintf(incline, "_inc%s", INCLINENAME[incli]);
#endif
		}

		std::string stage = "";
#if 0
		if(rendstage == RENDSTAGE_TEAM)
			stage = "_team";
		else if(rendstage == RENDSTAGE_DEPTH)
			stage = "_depth";
#endif

		sprintf(combo, "%s%s%s%s%s", relative, side, frame, incline, stage.c_str());

		if(queue)
			QueueSprite(combo, &sl->sprites[ci], loadteam, loaddepth);
		else
			LoadSprite(combo, &sl->sprites[ci], loadteam, loaddepth);
	}

	return true;
}

int32_t SpriteRef(SpList* sl, int32_t frame, int32_t incline, int32_t pitch, int32_t yaw, int32_t roll,
				  int slicex, int slicey)
{
	//int32_t ncombos = 1;
	int32_t sidechunk = 1;
	int32_t inclchunk = 1;
	int32_t frameschunk = 1;
	int32_t sliceschunk = 1;

	//pitch = roll = 0;
	//roll=0;
	//pitch = 1;

	if(sl->rotations)
	{
		//ncombos *= sl->nsides;
		//ncombos *= sl->nsides;
		//ncombos *= sl->nsides;
		sidechunk = sl->nsides * sl->nsides * sl->nsides;
	}
	else if(sl->sides)
	{
		//ncombos *= sl->nsides;
		sidechunk = sl->nsides;
	}
	
	inclchunk = sidechunk;

	if(sl->inclines)
	{
		//ncombos *= INCLINES;
		inclchunk = sidechunk * INCLINES;
	}

	frameschunk = inclchunk;

	if(sl->frames)
	{
		//ncombos *= sl->nframes;
		frameschunk = inclchunk * sl->nframes;

		//if(sl->nframes == 3)
		//	InfoMess("33","33");
	}
	
	sliceschunk = frameschunk;

	if(sl->nslices > 1)
	{
		sliceschunk = frameschunk * sl->nslices * sl->nslices;
	}

	int32_t ci = 0;
	
	if(sl->rotations)
	{
		ci += yaw + pitch * sl->nsides + roll * sl->nsides * sl->nsides;
	}
	else if(sl->sides)
	{
		ci += yaw;
	}

	if(sl->inclines)
	{
		ci += sidechunk * incline;
	}

	if(sl->frames)
	{
		ci += inclchunk * frame;
	}

	if(sl->nslices > 1)
	{
		ci += frameschunk * ( sl->nslices * slicey + slicex );
	}

		//if(strstr(sl->fullpath.c_str(), "lab"))
		//{
		//	char m[123];
		//	sprintf(m, "ci%d y%d f%d", ci, (int)yaw, (int)frame);
		//	InfoMess(m,m);
		//}

	return ci;
}

//TODO size up to power of 2 for mobile etc
void ParseSprite(const char* relative, Sprite* s)
{
	char fullpath[WF_MAX_PATH+1];
	FullPath(relative, fullpath);

	FILE* fp = fopen(fullpath, "r");
	if(!fp) return;

	float centerx;
	float centery;
	float width;
	float height;
	float clipszx, clipszy;
	float clipminx, clipminy, clipmaxx, clipmaxy;

	fscanf(fp, "%f %f", &centerx, &centery);
	fscanf(fp, "%f %f", &width, &height);
	fscanf(fp, "%f %f", &clipszx, &clipszy);
	fscanf(fp, "%f %f %f %f", &clipminx, &clipminy, &clipmaxx, &clipmaxy);
	
	s->offset[0] = -centerx;
	s->offset[1] = -centery;
	s->offset[2] = s->offset[0] + width;
	s->offset[3] = s->offset[1] + height;

	//s->crop[0] = clipminx / width;
	//s->crop[1] = clipminy / height;
	//s->crop[2] = clipmaxx / width;
	//s->crop[3] = clipmaxy / height;
	//s->crop[2] = (clipminx + clipszx) / width;
	//s->crop[3] = (clipminy + clipszy) / height;
	s->crop[0] = 0;
	s->crop[1] = 0;
	s->crop[2] = clipszx / width;
	s->crop[3] = clipszy / height;

	//s->cropoff[0] = clipminx - centerx;
	//s->cropoff[1] = clipminy - centery;
	//s->cropoff[2] = clipmaxx - centerx;
	//s->cropoff[3] = clipmaxy - centery;
	//s->cropoff[2] = clipminx + clipszx - centerx;
	//s->cropoff[3] = clipminy + clipszy - centery;
	s->cropoff[0] = -centerx;
	s->cropoff[1] = -centery;
	s->cropoff[2] = clipszx - centerx;
	s->cropoff[3] = clipszy - centery;

	fclose(fp);

#if 0
	char fullpath[WF_MAX_PATH+1];

	char frame[32];
	char side[32];
	strcpy(frame, "");
	strcpy(side, "");

	if(g_rendertype == RENDER_UNIT || g_rendertype == RENDER_BUILDING)
		sprintf(frame, "_fr%03d", g_renderframe);

	if(g_rendertype == RENDER_UNIT)
		sprintf(side, "_si%d", g_rendside);

	std::string incline = "";

	if(g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD)
	{
		if(g_currincline == INC_0000)	incline = "_inc0000";
		else if(g_currincline == INC_0001)	incline = "_inc0001";
		else if(g_currincline == INC_0010)	incline = "_inc0010";
		else if(g_currincline == INC_0011)	incline = "_inc0011";
		else if(g_currincline == INC_0100)	incline = "_inc0100";
		else if(g_currincline == INC_0101)	incline = "_inc0101";
		else if(g_currincline == INC_0110)	incline = "_inc0110";
		else if(g_currincline == INC_0111)	incline = "_inc0111";
		else if(g_currincline == INC_1000)	incline = "_inc1000";
		else if(g_currincline == INC_1001)	incline = "_inc1001";
		else if(g_currincline == INC_1010)	incline = "_inc1010";
		else if(g_currincline == INC_1011)	incline = "_inc1011";
		else if(g_currincline == INC_1100)	incline = "_inc1100";
		else if(g_currincline == INC_1101)	incline = "_inc1101";
		else if(g_currincline == INC_1110)	incline = "_inc1110";
	}

	std::string stage = "";

	if(rendstage == RENDSTAGE_TEAM)
		stage = "_team";

	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG(fullpath, &finalsprite);
	//sprite.channels = 3;
	//sprintf(fullpath, "%s_si%d_fr%03d-rgb.png", g_renderbasename, g_rendside, g_renderframe);
	//SavePNG(fullpath, &sprite);

	sprintf(fullpath, "%s%s%s%s.txt", g_renderbasename, side, frame, incline.c_str());
	std::ofstream ofs(fullpath, std::ios_base::out);
	ofs<<finalcenter.x<<" "<<finalcenter.y);
	ofs<<finalimagew<<" "<<finalimageh);
	ofs<<finalclipsz.x<<" "<<finalclipsz.y);
	ofs<<finalclipmin.x<<" "<<finalclipmin.y<<" "<<finalclipmax.x<<" "<<finalclipmax.y;
#endif

#if 0
	char infopath[WF_MAX_PATH+1];
	strcpy(infopath, texpath);
	StripExt(infopath);
	strcat(infopath, ".txt");

	std::ifstream infos(infopath);

	if(!infos)
		return;

	int32_t centeroff[2];
	int32_t imagesz[2];
	int32_t clipsz[2];

	infos>>centeroff[0]>>centeroff[1];
	infos>>imagesz[0]>>imagesz[1];
	infos>>clipsz[0]>>clipsz[1];

	t->sprite.offset[0] = -centeroff[0];
	t->sprite.offset[1] = -centeroff[1];
	t->sprite.offset[2] = t->sprite.offset[0] + imagesz[0];
	t->sprite.offset[3] = t->sprite.offset[1] + imagesz[1];
#endif
}

bool PlayAnim(float& frame, int32_t first, int32_t last, bool loop, float rate)
{
    if(frame < first || frame >= last)
    {
        frame = first;
        return false;
    }

    frame += rate;

    if(frame > last)
    {
        if(loop)
            frame = first;
		else
			frame = last;

        return true;
    }

    return false;
}

//Play animation backwards
bool PlayAnimB(float& frame, int32_t first, int32_t last, bool loop, float rate)
{
    if(frame < first-1 || frame > last)
    {
        frame = last;
        return false;
    }

    frame -= rate;

    if(frame < first)
    {
        if(loop)
            frame = last;
		else
			frame = first;

        return true;
    }

    return false;
}
