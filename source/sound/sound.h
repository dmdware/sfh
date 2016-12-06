











#ifndef SOUND_H
#define SOUND_H

#include "../platform.h"

class Sound
{
public:
	Sound() 
	{ 
		on = false;
		sample = NULL;
	}
	Sound(const char* fp);
	~Sound();

	bool on;
	Mix_Chunk *sample;
	char filepath[WF_MAX_PATH+1];
	void play();
};

#define SOUNDS	1024

extern Sound g_sound[SOUNDS];

class SoundLoad
{
	char fullpath[WF_MAX_PATH+1];
	int32_t* retindex;
};

//TO DO queue sound

extern int32_t g_volume;

extern int16_t g_beep;

void SoundPath(const char* from, char* to);
bool QueueSound(const char* relative, int16_t* index);
bool LoadSound(const char* relative, int16_t* index);
void FreeSounds();
void ReloadSounds();
void PlayClip(int16_t si);
void SetVol(int32_t percent);
void ChannelDone(int channel);

void Sound_Order();

#endif
