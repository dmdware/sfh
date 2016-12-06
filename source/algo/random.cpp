












#include "../platform.h"
#include "random.h"

// http://stackoverflow.com/questions/4768180/rand-implementation

static uint64_t next = 1;

uint32_t rand2() // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (uint32_t)(next/65536) % 32768;
}

void srand2(uint32_t seed)
{
    next = seed;
}

// own implementation

// http://en.wikipedia.org/wiki/Circular_shift
/*
 * Shift operations in C are only defined for shift values which are
 * not negative and smaller than sizeof(value) * CHAR_BIT.
 */
 
uint32_t rotl(uint32_t value, int32_t shift) {
    return (value << shift) | (value >> (sizeof(value) * CHAR_BIT - shift));
}
 
uint32_t rotr(uint32_t value, int32_t shift) {
    return (value >> shift) | (value << (sizeof(value) * CHAR_BIT - shift));
}

static uint32_t counter = 0;

void srand3(uint32_t seed)
{
	//srand(seed);
	counter = seed;
	next = seed;
	//settable(12345,65435,34221,12345,9983651,95746118);
	//settable(12345,65435,seed,12345,9983651,95746118);
	//settable(12345,65435,34221,seed,9983651,95746118);
}

uint32_t rand3()
{
    //next = next * 1103515245 + 12345;
	//for(unsigned char i=0; i<=next%5; i++)
	//	next = next ^ (next << (next % 16) + next >> (next % 16)) + 1;
	//next = (next << (next % 16) + next >> (next % 16));
	
	//return next;

	//const uint32_t mask = 0xffffffff;

	//little endian?
	//const uint32_t leftmask = 0xffff0000;
	//const uint32_t rightmask = 0x0000ffff;
	
	//big endian?
	//const uint32_t leftmask = 0x0000ffff;
	//const uint32_t rightmask = 0xffff0000;

	//need masks to get over undefined behaviour on certain systems of shift overflow

	//next = next ^ 3;
	//uint32_t shiftamount = ((uint32_t)32 - next) % 32;
	//uint32_t leftshift = (next & rightmask) << 16;
	//uint32_t rightshift = (next & leftmask) >> 16;
	//next = next ^ ( leftshift | rightshift );

	//uint32_t original = next;
	//next = next ^ 0x0f0f0f0f;
	//next = next + 1;
    //uint32_t temp = next * 1103515245 + 12345;
    //temp = (uint32_t)(temp/65536) % 32768;
	//next = next ^ ~temp;
	//next = rotr(next, next % 32);

	//uint32_t temp = next % 32;
	//next = next * 2 + 1;
	//next = next ^ 0x4a3b2c1d;
	//next = rotl(next, 1) ^ counter;
	//counter++;
	//return next;
	
	//just keep a counter and rearrange its bits to give a non-linear result
	//basically a sequence with a period of 2^32
	//to get a different sequence, change which bits are switched

#if 0

	const uint32_t bit00mask = 1 << 0;
	const uint32_t bit01mask = 1 * 2;
	const uint32_t bit02mask = 1 << 2;
	const uint32_t bit03mask = 1 << 3;
	const uint32_t bit04mask = 1 << 4;
	const uint32_t bit05mask = 1 << 5;
	const uint32_t bit06mask = 1 << 6;
	const uint32_t bit07mask = 1 << 7;
	const uint32_t bit08mask = 1 << 8;
	const uint32_t bit09mask = 1 << 9;
	const uint32_t bit10mask = 1 << 10;
	const uint32_t bit11mask = 1 << 11;
	const uint32_t bit12mask = 1 << 12;
	const uint32_t bit13mask = 1 << 13;
	const uint32_t bit14mask = 1 << 14;
	const uint32_t bit15mask = 1 << 15;
	const uint32_t bit16mask = 1 << 16;
	const uint32_t bit17mask = 1 << 17;
	const uint32_t bit18mask = 1 << 18;
	const uint32_t bit19mask = 1 << 19;
	const uint32_t bit20mask = 1 << 20;
	const uint32_t bit21mask = 1 << 21;
	const uint32_t bit22mask = 1 << 22;
	const uint32_t bit23mask = 1 << 23;
	const uint32_t bit24mask = 1 << 24;
	const uint32_t bit25mask = 1 << 25;
	const uint32_t bit26mask = 1 << 26;
	const uint32_t bit27mask = 1 << 27;
	const uint32_t bit28mask = 1 << 28;
	const uint32_t bit29mask = 1 << 29;
	const uint32_t bit30mask = 1 << 30;
	const uint32_t bit31mask = 1 << 31;
	
	uint32_t oldbit00 = ( counter & bit00mask ) >> 0;
	uint32_t oldbit01 = ( counter & bit01mask ) >> 0;
	uint32_t oldbit02 = ( counter & bit02mask ) >> 0;
	uint32_t oldbit03 = ( counter & bit03mask ) >> 0;
	uint32_t oldbit04 = ( counter & bit04mask ) >> 0;
	uint32_t oldbit05 = ( counter & bit05mask ) >> 0;
	uint32_t oldbit06 = ( counter & bit06mask ) >> 0;
	uint32_t oldbit07 = ( counter & bit07mask ) >> 0;
	uint32_t oldbit08 = ( counter & bit08mask ) >> 0;
	uint32_t oldbit09 = ( counter & bit09mask ) >> 0;
	uint32_t oldbit10 = ( counter & bit10mask ) >> 0;
	uint32_t oldbit11 = ( counter & bit11mask ) >> 0;
	uint32_t oldbit12 = ( counter & bit12mask ) >> 0;
	uint32_t oldbit13 = ( counter & bit13mask ) >> 0;
	uint32_t oldbit14 = ( counter & bit14mask ) >> 0;
	uint32_t oldbit15 = ( counter & bit15mask ) >> 0;
	uint32_t oldbit16 = ( counter & bit16mask ) >> 0;
	uint32_t oldbit17 = ( counter & bit17mask ) >> 0;
	uint32_t oldbit18 = ( counter & bit18mask ) >> 0;
	uint32_t oldbit19 = ( counter & bit19mask ) >> 0;
	uint32_t oldbit20 = ( counter & bit20mask ) >> 0;
	uint32_t oldbit21 = ( counter & bit21mask ) >> 0;
	uint32_t oldbit22 = ( counter & bit22mask ) >> 0;
	uint32_t oldbit23 = ( counter & bit23mask ) >> 0;
	uint32_t oldbit24 = ( counter & bit24mask ) >> 0;
	uint32_t oldbit25 = ( counter & bit25mask ) >> 0;
	uint32_t oldbit26 = ( counter & bit26mask ) >> 0;
	uint32_t oldbit27 = ( counter & bit27mask ) >> 0;
	uint32_t oldbit28 = ( counter & bit28mask ) >> 0;
	uint32_t oldbit29 = ( counter & bit29mask ) >> 0;
	uint32_t oldbit30 = ( counter & bit30mask ) >> 0;
	uint32_t oldbit31 = ( counter & bit31mask ) >> 0;

	counter++;
	
	uint32_t newbit00 = oldbit00 << 0;
	uint32_t newbit01 = oldbit00 * 2;
	uint32_t newbit02 = oldbit00 << 2;
	uint32_t newbit03 = oldbit00 << 3;
	uint32_t newbit04 = oldbit00 << 4;
	uint32_t newbit05 = oldbit00 << 5;
	uint32_t newbit06 = oldbit00 << 6;
	uint32_t newbit07 = oldbit00 << 7;
	uint32_t newbit08 = oldbit00 << 8;
	uint32_t newbit09 = oldbit00 << 9;
	uint32_t newbit10 = oldbit00 << 10;
	uint32_t newbit11 = oldbit00 << 11;
	uint32_t newbit12 = oldbit00 << 12;
	uint32_t newbit13 = oldbit00 << 13;
	uint32_t newbit14 = oldbit00 << 14;
	uint32_t newbit15 = oldbit00 << 15;
	uint32_t newbit16 = oldbit00 << 16;
	uint32_t newbit17 = oldbit00 << 17;
	uint32_t newbit18 = oldbit00 << 18;
	uint32_t newbit19 = oldbit00 << 19;
	uint32_t newbit20 = oldbit00 << 20;
	uint32_t newbit21 = oldbit00 << 21;
	uint32_t newbit22 = oldbit00 << 22;
	uint32_t newbit23 = oldbit00 << 23;
	uint32_t newbit24 = oldbit00 << 24;
	uint32_t newbit25 = oldbit00 << 25;
	uint32_t newbit26 = oldbit00 << 26;
	uint32_t newbit27 = oldbit00 << 27;
	uint32_t newbit28 = oldbit00 << 28;
	uint32_t newbit29 = oldbit00 << 29;
	uint32_t newbit30 = oldbit00 << 30;
	uint32_t newbit31 = oldbit00 << 31;

	uint32_t r = 0;

	r = r | newbit00;

#elif 0

	//using loops

	bool old[32];
	
	for(unsigned char i=0; i<32; i++)
		old[i] = (bool)(counter & (1 << i));

	uint32_t r = 0;

	for(unsigned char b1=0; b1<32; b1++)
	{
		unsigned char b2;
		//rotate left by 16+1 bits
		b2 = (16+1 + b1) % 32;
		//switch every two bits
		unsigned char twopair = b1 / 2;
		unsigned char twomember = b1 % 2;
		b2 = twopair * 2 + (1 - twomember);
		//switch every four bits (outer bits of the four pair)
		unsigned char fourpair = b1 / 4;
		unsigned char fourmember = b1 % 4;
		b2 = fourpair * 4 + (3 - fourmember);

		//don't shuffle bits 0 and 15
		//so that we get odd/even counter
		if(b1 == 0 || b2 == 0)
			b2 = b1;

		uint32_t bit = (uint32_t)old[b1] << b2;
		r = r | bit;
	}

	counter++;

	return r;

#elif 1

	uint32_t r = counter ^ 0x4a3b2c1d;
	//counter++;
	r = rotl(r, counter % 32);
	//counter++;
	r = r ^ counter;
	counter++;
	//r = r + (0xffff0000 - counter);
	return r;
#elif 0
	//uint32_t next = counter;
	//counter++;
    //next = next * 1103515245 + 12345;
    //next = (uint32_t)(next/65536) % 32768;
	//next = rotl(next, counter % 32);
	//return next;

    next = next * 1103515245 + 12345;
    return (uint32_t)(next/65536) % 32768;
#endif

#if 0
	unsigned char times = next % 33 + 1;

	for(unsigned char i=1; i<=times; i++)
	{
		uint32_t rot = rotl(next, (next+i) % 32);
		next = next + 2;
		next = next ^ ~rot;
	}
#elif 0
	uint32_t rot = rotl(next, (next+1) % 32);
	//next = next + 2;
	uint32_t temp = original ^ ~rot;
	next = next ^ (temp ^ rand2());
	
#if 0
	rot = rotl(next, (original+1) % 32);
	next = temp / 65536;
	next = next ^ ~rot;
	
	rot = rotl(next, (temp+1) % 32);
	temp = next + 1;
	next = temp ^ ~rot;
#endif
#endif


	//if you find a repeat, add here
	//const uint32_t repeat1 = 4294967233;
	//next = original + repeat1;

	//return next;
	//counter++;
	//return CONG + counter;
	//return rand();
}

// http://en.wikipedia.org/wiki/Random_number_generation

static uint32_t m_w = 1;    /* must not be zero, nor 0x464fffff */
static uint32_t m_z = 1;    /* must not be zero, nor 0x9068ffff */

void srand4(uint32_t s1, uint32_t s2)
{
	m_w = s1;
	m_z = s2;
}
 
uint32_t rand4()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}
