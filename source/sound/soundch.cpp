













#include "soundch.h"

SoundCh g_soundch[SOUND_CHANNELS];

int32_t NewChan()
{
	int32_t best = 0;
	uint64_t bestd = 0;
	uint64_t now = GetTicks();

	for(unsigned char i=0; i<SOUND_CHANNELS; i++)
	{
		SoundCh* c = &g_soundch[i];
		uint64_t d = now - c->last;

		if(d < bestd)
			continue;

		bestd = d;
		best = i;
	}

	return best;
}