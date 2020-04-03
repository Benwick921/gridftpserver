#include "clientutil.h"

int checkmd5(char* block)
{
	// spacchetto tutto per controllare che sia tutto ok
	int index = 0;  
	char* key = calloc(KEY_SIZE+1, 1); 
	char* hash = calloc(33, 1);
	char* chunk = calloc(CHUNK_SIZE+1, 1);
	
	bcopy(block, &index, sizeof(int));
	bcopy(&block[sizeof(int)], key, KEY_SIZE);
	bcopy(&block[sizeof(int)+KEY_SIZE], hash, 32);
	bcopy(&block[sizeof(int)+KEY_SIZE+32], chunk, CHUNK_SIZE);

	if(!strcmp(hash, md5(chunk)))
		return index;
	return -1;
}

