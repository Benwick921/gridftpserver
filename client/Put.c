#include "clientutil.h"

int put(char* drip, char* drport, struct blocks* blst, char* clntinf)
{
    int s = createsocket(drip, drport);
    
    // send information about the clinent such as username nad filename to send also indicates the start of blocks transfer
    writetosocket(s, clntinf, RCVBUFSIZE);
    
    struct blocks* index = blst;
    while(index != NULL)
    {
		//strcat(command, index->singleblock);
		printblock(index->singleblock);
    	writetosocket(s, index->singleblock, BLOCK_SIZE);
		
		// ricezione ACK
	    char* buffer = (char*)calloc(RCVBUFSIZE, sizeof(char));
		buffer[RCVBUFSIZE] = 0;       // Buffer for echo string 
	    // Wait for the ACK to receive
		buffer = readfromsocket(s, RCVBUFSIZE);
		
		if(!strcmp(buffer, "ERROR"))
		{
			puts("Resending block\n");
			continue;
		}
	
		index = index->next;
    }
   	if(write(s, "DONE", RCVBUFSIZE) < 0) // avviso il DR che ho finito di inviare i blocchi
	{
			perror("write");
			exit(1);
	}
	//free(buffer);
	close(s);
}
