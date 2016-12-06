












#ifndef RANDOM_H
#define RANDOM_H

void srand2(uint32_t seed);
uint32_t rand2();

uint32_t rand3();
void srand3(uint32_t seed);

uint32_t rand4();
void srand4(uint32_t s1, uint32_t s2);

#define SRAND	srand3
#define RAND	rand3

class Rand //TODO using rand3
{
public:
	uint32_t counter;

	void seed(uint32_t s);
	uint32_t next();
};

#endif