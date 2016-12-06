











#include "../platform.h"
#include "anim.h"

bool PlayAni(float& frame, int32_t first, int32_t last, bool loop, float rate)
{
	if(frame < first || frame > last+1)
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
bool PlayAniB(float& frame, int32_t first, int32_t last, bool loop, float rate)
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