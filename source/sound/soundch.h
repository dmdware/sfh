













#ifndef SOUNDCH_H
#define SOUNDCH_H

#include "../platform.h"
#include "../utils.h"

#define SOUND_CHANNELS	16

class SoundCh
{
public:
	bool done;
	uint64_t last;

	SoundCh(){ last = GetTicks(); done = true; }
};

extern SoundCh g_soundch[SOUND_CHANNELS];

int32_t NewChan();

#endif
