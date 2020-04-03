#include "clientutil.h"

int createsocket(char* drip, char* drport)
{
	// send to the mds: dr port
	int s,len;
    struct sockaddr_in saddr;
    struct hostent *hst; // a cosa serve? host
    
     // Create the socket
    if ((s=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
	perror("socket");
	return -1;
    }
    puts("> Socket Created");
    
    // Determine host address by its network name
    if ((hst=gethostbyname(drip)) == NULL)  // <--- dr IP
    {
		perror("gethostbyname");
		return -1;
    }
    printf("> Gethostbyname: %u %d\n",*(int*)hst->h_addr,hst->h_length);
    
    // Fill structure
    bcopy(hst->h_addr,&saddr.sin_addr,hst->h_length);
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(atoi(drport)); // MDS port
	
	// Connect to other socket
    if (connect(s,(struct sockaddr *)&saddr,sizeof(saddr))<0)
    {
		perror("connect");
		return -1; // failed to connect
    }
    printf("%s %s DR is UP\n", drip , drport);
    return s;
}
