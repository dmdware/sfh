











#ifndef UTILS_H
#define UTILS_H

#include "platform.h"

#define CORRECT_SLASH '/'

#define ARRSZ(a)	(sizeof(a)/sizeof(a[0]))

extern FILE* g_applog;

const std::string DateTime();
const std::string DateTime(uint64_t milli);
const std::string Time();
const std::string FileDateTime();
void FullPath(const char* filename, char* full);
void FullWritePath(const char* filename, char* full);
std::string MakeRelative(const char* full);
void ExePath(char* exepath);
std::string StripFile(std::string filepath);
void StripPathExt(const char* n, char* o);
void StripExt(char* filepath);
void StripPath(char* filepath);
void OpenLog(const char* filename, uint64_t version);
void Log(const char* format, ...);
void MakeDir(const char* fulldir);
float StrToFloat(const char *s);
int32_t HexToInt(const char* s);
int32_t StrToInt(const char *s);
void CorrectSlashes(char* corrected);
void BackSlashes(char* corrected);
void ErrMess(const char* title, const char* message);
void InfoMess(const char* title, const char* message);
void WarnMess(const char* title, const char* message);
void OutOfMem(const char* file, int32_t line);
uint64_t Add64(uint64_t a, uint64_t b);



bool TryRelative(const char* relfile);
bool SmartPath(char* o, std::string* mod, const char* dir);


#define OUTOFMEM()		OutOfMem(__FILE__,__LINE__)

inline float fmax(const float a, const float b)
{
	return (((a)>(b))?(a):(b));
}

inline float fmin(const float a, const float b)
{
	return (((a)<(b))?(a):(b));
}

inline int32_t imax(const int32_t x, const int32_t y)
{
#if 0
	return (((x)>(y))?(x):(y));
#else	//probably wrong on mac
	// https://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
	//int32_t x;  // we want to find the minimum of x and y
	//int32_t y;   
	//int32_t r;  // the result goes here 

	return x ^ ((x ^ y) & -(x < y)); // max(x, y)
#endif
}

inline int32_t imin(const int32_t x, const int32_t y)
{
#if 0
	return (((x)<(y))?(x):(y));
#else	//probably wrong on mac
	// https://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
	//int32_t x;  // we want to find the minimum of x and y
	//int32_t y;   
	//int32_t r;  // the result goes here 

	return y ^ ((x ^ y) & -(x < y)); // min(x, y)
#endif
}

#define enmax(a,b) (((a)>(b))?(a):(b))
#define enmin(a,b) (((a)<(b))?(a):(b))

inline int32_t iabs(int32_t v)
{
#if 1	//gives wrong result on mac
	//return x & INT_MAX;
	//return x & 0x7FFFFFFF;
	//return v < 0 ? -v : v;

	// https://graphics.stanford.edu/~seander/bithacks.html
	//int32_t v;           // we want to find the absolute value of v
	//uint32_t r;  // the result goes here 
	int32_t const mask = v >> ( (sizeof(int32_t) * CHAR_BIT) - 1 );
	return (v + mask) ^ mask;
#else
	return (v < 0) ? -v : v;
#endif
}

inline int32_t isign(int32_t x)
{
	//return x < 0 ? -1 : 1;

	//https://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign

	return +1 | (x >> (sizeof(int32_t) * CHAR_BIT - 1));
}

inline float fsign(float x)
{
	return x / fabs(x);
}

//deterministic ceil
inline int32_t iceil(const int32_t num, const int32_t denom)
{
	if(denom  == 0)
		return 0;

	int32_t div = num / denom;
	const int32_t mul = div * denom;
	const int32_t rem = num - mul;

	if(rem > 0)
		div += 1;

	return div;
}

//https://github.com/preshing/CompareIntegerMaps/blob/master/util.h

//typedef unsigned int uint32_t;
//typedef unsigned long long uint64_t;

inline uint32_t upper_power_of_two(uint32_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

inline uint64_t upper_power_of_two(uint64_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
}

// from code.google.com/p/smhasher/wiki/MurmurHash3
inline uint32_t integerHash(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

// from code.google.com/p/smhasher/wiki/MurmurHash3
inline uint64_t integerHash64(uint64_t k)
{
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccd;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53;
	k ^= k >> 33;
	return k;
}

std::string iform(int32_t n);
std::string ullform(uint64_t n);
uint64_t GetTicks();

std::string LoadTextFile(char* strFile);
void ListFiles(const char* fullpath, std::list<std::string>& files);
void ListDirs(const char* fullpath, std::list<std::string>& dirs);

#ifdef PLATFORM_WIN
void MiniDumpFunction( uint32_t nExceptionCode, EXCEPTION_POINTERS *pException );
#endif

#endif	//UTILS_H
