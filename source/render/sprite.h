











#ifndef SPRITE_H
#define SPRITE_H

#include "../texture.h"

class Sprite
{
public:

	Sprite();
	~Sprite();
	void free();

	bool on;
	uint32_t difftexi;
	uint32_t teamtexi;
	uint32_t depthtexi;
	uint32_t elevtexi;
	float offset[4];	//the pixel texture coordinates centered around a certain point, for use for vertex positions when not cropping (old)
	float crop[4];	//the texture coordinates [0..1] of the cropped sprite, used for the texture coords
	float cropoff[4];	//the pixel texture coordinates of the cropped sprite, used for vertex positions
	LoadedTex* pixels;
	std::string fullpath;
};

//#define SPRITES	4096
#define SPRITES	16096
extern Sprite g_sprite[SPRITES];

class SpList
{
public:
	bool on;
	bool inclines;
	bool sides;
	int32_t nsides;
	bool rotations;
	bool frames;
	int32_t nframes;
	int32_t nslices;
	std::string fullpath;

	int nsp;
	uint32_t* sprites;

	void free()
	{
		nsp = 0;
		delete [] sprites;
		sprites = NULL;
		on = false;
		nslices = 1;
	}

	SpList()
	{
		nsp = 0;
		sprites = NULL;
		on = false;
		nslices = 1;
	}

	~SpList()
	{
		free();
	}
};

#define SPRITELISTS	256
extern SpList g_splist[SPRITELISTS];

class SpriteToLoad
{
public:
	std::string relative;
	uint32_t* spindex;
	bool loadteam;
	bool loaddepth;
};

extern std::vector<SpriteToLoad> g_spriteload;

extern int32_t g_lastLSp;

bool Load1Sprite();
void FreeSprites();
bool LoadSprite(const char* relative, uint32_t* spindex, bool loadteam, bool loaddepth);
void QueueSprite(const char* relative, uint32_t* spindex, bool loadteam, bool loaddepth);
void ParseSprite(const char* relative, Sprite* s);
bool PlayAnim(float& frame, int32_t first, int32_t last, bool loop, float rate);
bool PlayAnimB(float& frame, int32_t first, int32_t last, bool loop, float rate);	//Play animation backwards
int32_t SpriteRef(SpList* sl, int32_t frame, int32_t incline, int32_t pitch, int32_t yaw, int32_t roll,
				  int slicex, int slicey);
bool LoadSpriteList(const char* relative, uint32_t* splin, bool loadteam, bool loaddepth, bool queue);
int32_t NewSpriteList();
int32_t NewSprite();

#endif
