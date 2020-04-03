#ifndef mdsutil_h
#define mdsutil_h

#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <sys/wait.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>

#define RCVBUFSIZE 64   /* Size of receive buffer */
#define KEY_SIZE 8
#define CHUNK_SIZE 4096
#define BLOCK_SIZE 7+KEY_SIZE+32+CHUNK_SIZE

#define SEMNAME "/my_semaphore"
#define PERM 0600

#define TEMPFILE "temp"

struct connection
{
	int clntSock;
	char* ip;
};

struct drinfo
{
	int number;
	char* ip;
	int port;
	int avalaiblestorage;
	struct drinfo* next;
};
int drstatus(char* drip, char* drport);
int notifydr(char* drip, char* drport, char* username, char* key);
int createfolder(char* foldername);
int checknewfile(char* username, char* filename);
char* getfilekey(char* username, char* filename);
int removefile(char* path);
int createsocket(char* drip, char* drport);
int writetofile(char* info, char* path);
int checkdoubledr(char* connectionip, char* portnsize);
int writetosocket(int socket, char* string, int size);
char* readfromsocket(int socket, int size);
void HandleTCPClient(int clntSocket, char* connectionip, sem_t* drdata_semaphore);
struct connection AcceptTCPConnection(int servSock);
int CreateTCPServerSocket(unsigned short port);
void DieWithError(char *errorMessage);
char* generatekey(int keysize);
int checkcredential(char* username, char* passwd);
int checkavalaibleusername(char* username);
int registration(char* username, char* passwd);

#endif
