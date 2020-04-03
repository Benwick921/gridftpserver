#ifndef clientutil_h
#define clientutil_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>     /* for close() */
#include <ctype.h>		/* for isdigit() */
#include <math.h>		/* ceil() */
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h> 
#include <openssl/md5.h>
#include <stdint.h>

//const size_t CHUNK_SIZE = 4096;
#define CHUNK_SIZE ((size_t)4096)
#define BLOCK_SEPARATOR "@"
#define RCVBUFSIZE 64   /* Size of receive buffer */
#define KEY_SIZE 8
// 6 = xxxx@@@  32 = md5
#define BLOCK_SIZE 7+KEY_SIZE+32+CHUNK_SIZE
#define MDSIP  "localhost"
#define MDSPORT 4444

#define DOWNLOAD_FOLDER "download/"


struct blocks
{
	char* singleblock;
	struct blocks* next;
};

int get(char* filename, char* username, int sock);
int put(char* drip, char* drport, struct blocks* blst, char* clntinf);
int createsocket(char* drip, char* drport);
struct blocks* blocklist(struct blocks* blst, int blockindex, char* key, char* chunk, char* md5);
int printlist(struct blocks* head);
int writeblock(char* chunk, char* path);
int printblock(char* block);
int checkmd5(char* block);
char* md5(char* chunk);
int writetosocket(int socket, char* string, int size);
char* readfromsocket(int socket, int size);
void DieWithError(char* errorMessage);

#endif
