#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>     /* for close() */
#include "drutil.h"

int avalaiblestorage = 100; // in mega byte

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

int countfile(char* path)
{
	//strcat(path, "/");
	int filecount = 0;
	DIR* dirp;
	struct dirent* entry;
	
	dirp = opendir(path);
	while((entry = readdir(dirp)) != NULL)
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;
		filecount++;
	}
	return filecount;
}

int notifymds(char* mdsip, char* mdsport, char* drport)
{
	// send to the mds: dr port
	int s,len;
    struct sockaddr_in saddr;
    struct hostent *hst; // a cosa serve? host
    
     // Create the socket
    if ((s=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
	perror("socket");
	exit(1);
    }
    puts("> Socket Created");
    
    // Determine host address by its network name
    if ((hst=gethostbyname(mdsip)) == NULL)  // <--- MDS IP
    {
		perror("gethostbyname");
		exit(1);
    }
    printf("> Gethostbyname: %u %d\n",*(int*)hst->h_addr,hst->h_length);
    
    // Fill structure
    bcopy(hst->h_addr,&saddr.sin_addr,hst->h_length);
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(atoi(mdsport)); // MDS port
	
	// Connect to other socket
    if (connect(s,(struct sockaddr *)&saddr,sizeof(saddr))<0)
    {
		perror("connect");
		exit(1);
    }
    puts("> Connected to MDS!");
    
    // Write (or send) to socket
    char command[RCVBUFSIZE] = {0};
    strcat(command, "drup ");
    strcat(command, drport);
    char *charstorage = (char*)malloc(sizeof(char)*3);
    sprintf(charstorage, "%d", avalaiblestorage);
    strcat(command, " ");
    strcat(command, charstorage);
    
    printf("> command: %s\n",command);
    
    if (write(s, command, RCVBUFSIZE) < 0) 
    {
		perror("write");
		exit(1);
    }
    
    // ricezione ACK
    char* buffer = (char*)malloc(sizeof(char)*(RCVBUFSIZE+1));
	buffer[16] = 0;       /* Buffer for echo string */
	int recvMsgSize;  
    /* Wait for the ACK to receive */
	if ((recvMsgSize = recv(s, buffer, RCVBUFSIZE, 0)) < 0)
	   	DieWithError("recv() failed");
	   	
	printf("> buffer: %s\n", buffer);
	free(buffer);
}

int corruptionchecker()
{
	while(1){
	puts("sleeping...\n");
	sleep(5);
	puts("weakup\n");
	// read user.conf
	FILE* user = fopen(USER_CONFIG_FILE, "r");
	char* line = calloc(RCVBUFSIZE, 1);
	while(user && fgets(line, RCVBUFSIZE, user))
	{
		printf(">>line %s <<\n", line);
		char* path = calloc(RCVBUFSIZE, 1);
		char* username = strtok(line, "\n"); // in questo modo non mi prende l`accapo
		strcat(path, username);
		strcat(path, "/");
		strcat(path, username);
		strcat(path, ".conf");
		
		char* line2 = calloc(RCVBUFSIZE, 1);
		FILE* file = fopen(path, "r");
		printf("fd: %d\n", file);
		while(file && fgets(line2, RCVBUFSIZE, file))
		{
			printf(">>line2 %s <<\n", line2);
			// crare il socket con mds
			int socket = createsocket(MDSIP, MDSPORT);
			if(socket == -1)
			{
				printf("MDS not ready yet\n");
				continue;
			}
			
			
			char* filename = strtok(line2, "\n");
			// inviare al dr il nome file da controllare
			puts(line2);
			// inviare (info username/filename)
			char* command = calloc(RCVBUFSIZE, 2);
			strcat(command, "info ");
			strcat(command, username);
			strcat(command, "/");
			strcat(command, filename);
			
			writetosocket(socket, command, RCVBUFSIZE);
			
			char* response = readfromsocket(socket, RCVBUFSIZE);
			// count files in username/filename
			
			char* path2 = calloc(RCVBUFSIZE, 1);
			strcat(path2, username);
			strcat(path2, "/");
			strcat(path2, filename);
			
			if(atoi(response) == countfile(strtok(path2, "."))) puts("INTACT\n");
			else puts("CORRUPTED\n");
			
		}
			puts("Config file does not exist\n");
	}

	}
}

int main(int argc, char** argv)// dr port mds ip msd port
{
	if (argc != 4)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port> <MDS IP> <MDS Port>\n", argv[0]);
        exit(1);
    }
	printf("\n=-=-=-=-=-=  DATA REPOSITORY V3.0  =-=-=-=-=-=\n");
	/*printf("> %d\n", argc);
	printf("> %s\n", argv[0]);
	printf("> %d\n", atoi(argv[1])); // DR port
	printf("> %s\n", argv[2]); //MDS IP
	printf("> %d\n", atoi(argv[3]));// MDS PORT*/
	
	int totalstorage = 20;
	int usedstorage = 0;
	
	int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    unsigned short echoServPort;     /* Server port */
    pid_t processID;                 /* Process ID from fork() */
    unsigned int childProcCount = 0; /* Number of child processes */ 

    
    if((atoi(argv[1]) == atoi(argv[3])) && !strcmp(argv[1],"localhost"))
    {
    	fprintf(stderr, "DR and MDS can`t have same port in the same machine\n");
    	exit(1);
    }
    
    // notify msd
    
    printf("Creating socket\n");
    echoServPort = atoi(argv[1]);  /* First arg:  local port */
    servSock = CreateDRTCPServerSocket(echoServPort);
    
    // notify MDS then start to accept requests.
	notifymds(argv[2], argv[3], argv[1]);
    
    
    /*int checker = 0;
	if ((checker = fork()) < 0)
	    DieWithError("fork() failed");
	else if (checker == 0)  // If this is the child process 
		corruptionchecker();
	printf("%d\n",checker);*/
    
    
    int maxconnections = 30;
	while(maxconnections > 0)// diventera un whle true piu o meno con limitazione di un MAX se no mi esplode il server
	{
		clntSock = AcceptDRTCPConnection(servSock);

		/* Fork child process and report any errors */
		if ((processID = fork()) < 0)
		    DieWithError("fork() failed");
		else if (processID == 0)  /* If this is the child process */
		{
		    close(servSock);   /* Child closes parent socket */
		    HandleDRTCPClient(clntSock, totalstorage, usedstorage);
		    exit(0);           /* Child process terminates */
		}
		maxconnections--;
		close(clntSock);
	}
	//kill(checker, SIGKILL);
}
