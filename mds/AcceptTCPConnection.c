#include <stdio.h>      /* for printf() */
#include <sys/socket.h> /* for accept() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include "mdsutil.h"

void DieWithError(char *errorMessage);  /* Error handling function */

struct connection AcceptTCPConnection(int servSock)
{
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in clntAddr;	 /* Client address */
    unsigned int clntLen;            /* Length of client address data structure */

    /* Set the size of the in-out parameter */
    clntLen = sizeof(clntAddr);
    
    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, 
           &clntLen)) < 0)
        DieWithError("accept() failed");
    
    /* clntSock is connected to a client! */

	struct connection conn;
	conn.clntSock = clntSock;
	conn.ip = inet_ntoa(clntAddr.sin_addr);
	
    return conn;
}
