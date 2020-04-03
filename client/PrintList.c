#include "clientutil.h"

int printlist(struct blocks* head)
{
	puts("printing lst\n");
	struct blocks* index = head;
	while(index != NULL)
	{
		// spacchetto tutto per controllare che sia tutto ok
		int i =0;  
		char* k = calloc(KEY_SIZE+1, 1); 
		char* m = calloc(33, 1);
		char* c = calloc(CHUNK_SIZE+1, 1);
		
		bcopy(index->singleblock, &i, sizeof(int));
		bcopy(&index->singleblock[sizeof(int)], k, KEY_SIZE);
		bcopy(&index->singleblock[sizeof(int)+KEY_SIZE], m, 32);
		bcopy(&index->singleblock[sizeof(int)+KEY_SIZE+32], c, CHUNK_SIZE);
		printf("%d | %s | %s | %s\n", i,k, m, c);
		index = index->next;
	}
	puts("end printing\n");
}
