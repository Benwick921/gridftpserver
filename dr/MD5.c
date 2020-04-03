#include "drutil.h"

char* md5(char* chunk)
{
	//unsinged char and uint8_t is the same thing and shorter to write
	char digest[1<<4]; // 1<<4 = 1 * 2^4 = 16
	MD5((const uint8_t*) chunk, strlen(chunk), (uint8_t*) digest);
	
	char* hash = (char*)calloc(33, sizeof(char));
	for(size_t i = 0; i < 16; i++)
		sprintf(&hash[i<<1], "%02x", (uint8_t)digest[i]);
	return hash;
}
