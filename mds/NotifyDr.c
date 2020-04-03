#include "mdsutil.h"

int notifydr(char* drip, char* drport, char* username, char* key)
{
	// send to the mds: dr port
	int s = createsocket(drip, drport);
    
    // Write (or send) to socket
    char command[RCVBUFSIZE] = {0};
    strcat(command, "newclnt ");
    strcat(command, username);
    strcat(command, " ");
    strcat(command, key);
    
    printf("> command: %s\n",command);
    
    if (write(s,command, RCVBUFSIZE)<0) 
    {
		perror("write");
		exit(1);
    }
    
    // ricezione ACK
    char* buffer = (char*)malloc(sizeof(char)*(RCVBUFSIZE+1));
	buffer[16] = 0;       // Buffer for echo string 
	int recvMsgSize;  
    // Wait for the ACK to receive
	if ((recvMsgSize = recv(s, buffer, RCVBUFSIZE, 0)) < 0)
	   	DieWithError("recv() failed");
	   	
	printf("> buffer: %s\n", buffer);
	free(buffer);
	close(s);
}
