











#ifndef USTRING_H
#define USTRING_H

#include "platform.h"

//#define USTR_DEBUG

class UStr
{
public:
	uint32_t m_length;	//length doesn't count null-terminator
	uint32_t* m_data;

	UStr();
	~UStr();
	UStr(const UStr& original);
	UStr(const char* cstr);
	UStr(uint32_t k);
	UStr(uint32_t* k);
	UStr& operator=(const UStr &original);
	UStr operator+(const UStr &other);
	UStr substr(int32_t start, int32_t len) const;
	int32_t firstof(UStr find) const;
	std::string rawstr() const;
};

#endif
