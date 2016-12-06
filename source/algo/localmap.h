

#ifndef LOCALMAP_H
#define LOCALMAP_H

#include "../platform.h"
#include "../texture.h"

//memory localizing hash map with changing hash function cipher key

#define LOWEST0(x)			((~x) & (x + 1))
#define LOWEST1(x)			(x & ((~x) + 1))

#define HASHINT		uint8_t
//#define HASHINT	uint16_t
//#define HASHINT	uint8_t
#define HASHADDR	uint8_t
#define SHASHADDR	int8_t
#define TABINT		uint16_t
//#define HASHADDR	uint32_t
//#define SHASHADDR	int32_t
//#define HASHFIELD	uint32_t
//#define HASHCOUNT	uint8_t
//#define LOG2HASHBITS	2	//(log2(HASHBITS))
#define HASHBITS	2	//don't set odd
//4->0 6->1 1->2 3b 6l
//#define HASHBITS	16
//#define HASHBITS	(1<<LOG2HASHBITS)
#define MAXHASH		((((HASHINT)1)<<HASHBITS)-1)
//#define HASHCROP(x)	(x)
#define HASHCROP(x)	((x)&(MAXHASH))
#define HASHVALS			(MAXHASH+1)	//greater maps => less mask cases shared but more input possibilites eg x10=>1/8.5x,1.25x,a1=b40,a1=b400
//#define HASHVALS			((1<<HASHBITS))
//#define HASHVALS			(HASHBITS+3)
//#define HASHVALS				((1<<(HASHBITS))-1)
//#define HASHVALS			(HASHBITS-1)
#define HASHROTL(x,n)	HASHCROP((x<<n)|(x>>(HASHBITS-n)))
#define HASHROTR(x,n)	HASHCROP((x>>n)|(x<<(HASHBITS-n)))
//#define HASHLEVELS		(HASHVALS*2)	//4bit 4val 11lev
#define HASHLEVELS			(HASHBITS*2)
//#define HASHLEVELS			((HASHBITS*HASHVALS-8))
//#define HASHLEVELS				HASHBITS
//#define HASHLEVELS			(1<<HASHBITS)
//#define HASHLEVELS		(HASHBITS*(1<<(LOG2HASHBITS*LOG2HASHBITS))/(HASHBITS*HASHBITS))
//#define HASHLEVELS		((1<<(LOG2HASHBITS*(LOG2HASHBITS))))
//#define HASHLEVELS	(HASHBITS)	//2 works for 1 bits
//#define HASHLEVELS	(HASHBITS<<2)	//9 works for 3 bits
//#define HASHLEVELS	((HASHBITS<<1))	//9 works for 3 bits
//#define HASHLEVELS		((1<<HASHBITS)+1)
//#define HASHLEVELS	(4)
//#define HASHLEVELS	((HASHBITS<<1)+4)
//#define HASHLEVELS	((HASHBITS*HASHVALS)+HASHVALS)	//works for 3 bits
//#define MAXJUMP			(1<<3)
//#define HASHLEVELS		(7)
//#define HASHLEVELS		(3+HASHBITS)	//log2(MAXJUMP)+HASHBITS
//#define HASHLEVELS	(HASHBITS<<4+1)	//works for 3 bits
//#define HASHLEVELS		(HASHBITS*HASHBITS)
//#define HASHLEVELS		(HASHBITS*HASHBITS*HASHBITS)
//#define HASHLEVELS		((1<<HASHBITS)+1)
//#define HASHLEVELS			((1<<(HASHBITS*HASHBITS))+1)

struct Map
{
	HASHINT i;
	HASHINT o;
};

extern HASHINT m[HASHLEVELS];

void TestHash();
HASHINT Hash(HASHINT* mval, HASHINT oval, HASHADDR level, bool echo=false);

void pb(char* c, HASHINT i);
void pb22(char* c, HASHINT i);
void pb2(char* c, HASHINT t, HASHINT m);

#endif
