













#ifndef HINT_H
#define HINT_H

#include "../platform.h"

class Hint
{
public:
	std::string message;
	std::string graphic;
	int32_t gwidth;
	int32_t gheight;
    
	Hint();
	void reset();
};

extern Hint g_lasthint;

#endif