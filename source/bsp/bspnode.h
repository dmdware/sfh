

#ifndef BSPNODE_H
#define BSPNODE_H

#include "../platform.h"

class BSPNode
{
public:
	int plane;
	int child[2];

	std::vector<Brush*> brushes;
};

#endif