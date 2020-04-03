#include "clientutil.h"

struct blocks* blocklist(struct blocks* blst, int blockindex, char* key, char* chunk, char* md5)
{
	// create block
	char* block = (char*)calloc(BLOCK_SIZE, sizeof(char));
	char index[3] = {0};
	
	char* b = calloc(100, sizeof(char));
	// concateno con i bcopy e non strcat
	bcopy(&blockindex, block, sizeof(int)); // sizeof(int) = 4
	bcopy(key, &block[sizeof(int)], KEY_SIZE);
	bcopy(md5, &block[12], 32);
	bcopy(chunk, &block[44], CHUNK_SIZE);
	
	// spacchetto tutto per controllare che sia tutto ok
	int i =0;  
	char* k = calloc(KEY_SIZE, 1); 
	char* m = calloc(32, 1);
	char* c = calloc(CHUNK_SIZE, 1);
	
	bcopy(block, &i, sizeof(int));
	bcopy(&block[sizeof(int)], k, KEY_SIZE);
	bcopy(&block[sizeof(int)+KEY_SIZE], m, 32);
	bcopy(&block[sizeof(int)+KEY_SIZE+32], c, CHUNK_SIZE);
	printf("%d | %s | %s | %s\n", i,k, m, c);
	
	struct blocks* elem = malloc(sizeof(struct blocks));
	elem->singleblock = block;
	elem->next = NULL;
	
	if(blst == NULL)
		blst = elem;
	else
	{
		struct blocks* index = blst;
		while(index->next != NULL)
			index = index->next;
		index->next = elem;
	
	}	
	return blst;
}
