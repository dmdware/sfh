











#include "parity.h"
#include "../math/fixmath.h"
//#include "../algo/bool.h"
#include "../utils.h"

//odd parity
//insz must be at least 1 byte
//1 byte insz, 3 parity bits, total out 2 bytes
//2 byte insz, 4 parity bits, total out 3 bytes
//3 byte insz (24 bits), 5 parity bits, total out 4 bytes
//4 byte insz (32 bits), 5 parity bits, total out 5 bytes
//5 byte insz (40 bits), 6 parity bits, total out 6 bytes
//6 byte insz (48 bits), 6 parity bits, total out 7 bytes
//7 byte insz (56 bits), 6 parity bits, total out 8 bytes
//...
//8 parity bits, 32 bytes insz (256 bits), total out 33 bytes
//9 parity bits, 33-64 bytes (257-512 bits), total out 35-66 bytes
uint32_t Parify(unsigned char *in, unsigned char **out, uint32_t insz)
{
	uint32_t nparbits, outbits, nparbytes, outsz, parbit, parbyte, odd, inbit, inbyte;
	unsigned char *parbits;
	
	nparbits = ilog2ceil(insz * 8);
	//outbits = insz * 8 + nparbits;
	nparbytes = iceil(nparbits, 8);
	
	parbits = (unsigned char*)malloc(nparbytes);
	memset(parbits, 0, nparbytes);
	
	for(parbit=0; parbit<nparbits; ++parbit)
	{
		odd = 0;
		
		for(inbit=1; inbit<=insz*8; inbit<<=1)
		{
			inbyte = (inbit-1) / 8;
			
			if( ( in[inbyte] >> ((inbit-1) % 8) ) & 1 )
				odd = 1 - odd;
		}
		
		parbyte = parbit / 8;
		parbits[parbyte] |= odd << (parbit % 8);
	}

	//TODO replace whole file corpd fix on mac
	
	outsz = iceil(outbits, 8);
	*out = (unsigned char*)malloc(insz+nparbytes);
	memcpy(*out, in, insz);
	memcpy(&(*out)[insz], parbits, nparbytes);
	free(parbits);
	
	return nparbytes + insz;
}

//8 parity bits, 32 bytes insz (256 bits), total out 33 bytes
//9 parity bits, 33-64 bytes (257-512 bits), total out 35-66 bytes

//in 33 bytes, 264 bits, ilog2floor=8
//in 35 bytes, 280 bits, ilog2floor=8

//calculate number of original bytes from parified packet
uint32_t UnparifySz(uint32_t insz)
{
	uint32_t nparbits, outsz, nparbytes;
	
	outsz = 0;
	
	do
	{
		++outsz;
		nparbits = ilog2ceil(outsz * 8);
		//outbits = outsz * 8 + nparbits;
		nparbytes = iceil(nparbits, 8);
	}while(outsz+nparbytes < insz);

	if(outsz+nparbytes > insz)
		--outsz;
	
	return outsz;
}

bool CheckParity(unsigned char *in, unsigned char *parbits, uint32_t insz)
{
	uint32_t nparbits, parbit, parbyte, odd, inbit, inbyte, should;
	
	nparbits = ilog2ceil(insz * 8);
	
	for(parbit=0; parbit<nparbits; ++parbit)
	{
		odd = 0;
		
		for(inbit=1; inbit<=insz*8; inbit<<=1)
		{
			inbyte = (inbit-1) / 8;
			
			if( ( in[inbyte] >> ((inbit-1) % 8) ) & 1 )
				odd = 1 - odd;
		}
		parbyte = parbit / 8;
		//parbits[parbyte] |= odd << (parbit % 8);
		
		//don't attempt correction
		if( ( ( parbits[parbyte] >> (parbit % 8) ) & 1 ) != odd )
			return false;
	}
		   
	return true;
}

//insz must be at least 2 bytes
//if error cannot be corrected, 0 bytes returned and "out" not allocated
uint32_t Unparify(unsigned char *in, unsigned char **out, uint32_t insz)
{
	uint32_t ninbits, nparbits, outbits, nparbytes, outsz, parbit, parbyte, odd, inbit, inbyte;
	
	outsz = UnparifySz(insz);
	*out = NULL;
	
	if(!CheckParity(in, &in[outsz], outsz))
		return 0;
	
	*out = (unsigned char*)malloc(outsz);
	memcpy(*out, in, outsz);
	
	return outsz;
}