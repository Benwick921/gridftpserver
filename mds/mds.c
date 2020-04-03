#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h> 
#include<unistd.h>
#include<fcntl.h> 			/* O_CREAT, O_EXEC          */
#include<errno.h>          /* errno, ECHILD            */
#include<semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include<sys/types.h>      /* key_t, sem_t, pid_t      */

#include"mdsutil.h"

int corruptionchecker()
{
	while(1)
	{
		puts("sleeping\n");
		sleep(5);
		puts("woke up\n");
		// for each DR in dr.conf
		FILE* userfile = fopen("credentials","r");

		char* line = malloc(100);
		while(fgets(line, RCVBUFSIZE, userfile) && userfile) // read existent files uploaded
		{
			printf(">>line1 %s <<\n", line);
			
			char* username = strtok(line, " ");
			char* path = calloc(100, 1);
			
			strcat(path, username);
			strcat(path, "/");
			strcat(path, username);
			printf("%s %s\n", username, path);		
			FILE* files = fopen(path, "r");
			char* line2 = calloc(100, 1);
			
			printf("fd: %d\n", files);
			while(files && fgets(line2, RCVBUFSIZE, files)) // for each uploaded file read the DRs 
			{
				printf(">>line2 %s<<\n", line);
				char* filename = strtok(line2, " ");
				
				char* path2 = calloc(100, 1);
				strcat(path2, username);
				strcat(path2, "/");
				
				//char* c = strtok(filename, ".");
				//printf("strtok: %s\n", c);
				
				strcat(path2, filename);
				
				FILE* filedr = fopen(path2, "r");
				char* line3 = calloc(100, 1);
				while(fgets(line3, RCVBUFSIZE, filedr))
				{			
					printf(">>line3 %s <<\n", line3);
					
					char* drip = strtok(line3, " ");
					
					char* drport = strtok(NULL, " ");
					char* blocknumber = strtok(NULL, " ");
					
					// connect to DR
					int socket = createsocket(drip, drport);
					if(socket == -1)
					{
						printf("%s %s DOWN\n", drip , drport);
						//exit(1); // da levare
						continue;
						
					}
					printf("Path2: %s\n", path2);
					char* command = calloc(RCVBUFSIZE, 1);
					strcat(command, "info ");
					strcat(command, strtok(path2, "."));
					writetosocket(socket, command, RCVBUFSIZE);
					
					char* response = readfromsocket(socket, RCVBUFSIZE);
					
					
					if(atoi(blocknumber) != atoi(response))
					{
						printf("BROKEN! %d %d\n",atoi(blocknumber), atoi(response));
						// delete file
					}
					else puts("INTACT\n");
				}
			}
			puts("confgifile does not exist\n");
		}

	}
}

int main(int argc, char *argv[])
{
	if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }
		
	printf("\n=-=-=-=-=-=  SERVER V3.0  =-=-=-=-=-=\n");
	int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    unsigned short servPort;     	 /* Server port */
    pid_t processID;                 /* Process ID from fork() */
    unsigned int childProcCount = 0; /* Number of child processes */ 
    
    /***** Semaphores *****/
    // semaphore used to write in drlist.conf file and passed to all childs so they can use it.
	sem_t* drdata_semaphore = sem_open(SEMNAME, O_CREAT | O_EXCL, PERM, 1);
	if(drdata_semaphore == SEM_FAILED && errno == EEXIST)
	{
		puts("Named semaohore already exist, destroying and recreating!\n");
		sem_unlink(SEMNAME);
		drdata_semaphore = sem_open(SEMNAME, O_CREAT | O_EXCL, PERM, 1);
	}
	if(drdata_semaphore == SEM_FAILED)
	{
		perror("named sempahore");
		exit(1);
	} 
	puts("Names semaphore created!\n");
    
    servPort = atoi(argv[1]);  /* First arg:  local port */
    servSock = CreateTCPServerSocket(servPort);
	
	
	/*int checker = 0;
	if ((checker = fork()) < 0)
	    DieWithError("fork() failed");
	else if (checker == 0)  // If this is the child process 
		corruptionchecker();
	printf("%d\n",checker);*/
	
	
	int maxconnections = 20;
	while(maxconnections > 0)// diventera un whle true piu o meno con limitazione di un MAX se no mi esplode il server
	{
		struct connection conn = AcceptTCPConnection(servSock);
		clntSock = conn.clntSock;
		printf("> Handling client: %s\n", conn.ip);
		
		/* Fork child process and report any errors */
		if ((processID = fork()) < 0)
		    DieWithError("fork() failed");
		else if (processID == 0)  /* If this is the child process */
		{
		    close(servSock);   /* Child closes parent socket */
		    HandleTCPClient(clntSock, conn.ip, drdata_semaphore);
		    exit(0);           /* Child process terminates */
		}
		maxconnections--;
		close(clntSock);
	}
	sem_unlink(SEMNAME);
	//kill(checker, SIGKILL);
}
