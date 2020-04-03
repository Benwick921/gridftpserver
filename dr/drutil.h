#ifndef dr_h
#define dr_h

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <stdint.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <ctype.h>		/* for isdigit() */
#include <math.h>		/* ceil() */
#include <fcntl.h>
#include <sys/wait.h> 
#include <netdb.h>
#include <string.h>

#define RCVBUFSIZE 64
#define CHUNK_SIZE ((size_t)4096)
#define KEY_SIZE  8
#define USER_CONFIG_FILE "user.conf"
// 6 = xxxx@@@S  32 = md5
#define BLOCK_SIZE 7+KEY_SIZE+32+CHUNK_SIZE
#define MDSIP "localhost"
#define MDSPORT "4444"
#define AUTH_FOLDER "auth/"
#define TEMPFILE "temp"

int get(char* path, int socket, char* key);
int writeblock(char* block, char* path);
int writetofile(char*path, char* string);
int checknewuser(char* username);
int checknewfile(char* filename, char* path);
int createfolder(char* foldername);
int countfiles(char* path);
int checkmd5(char* block);
int printblock(char* block);
int removeauthfile(char* key);
int removefile(char* path);
char* md5(char* chunk);
char* readfromsocket(int socket, int size);
int writetosocket(int socket, char* string, int size);
void HandleDRTCPClient(int clntSocket, int totalstorage, int usedstorage);
int AcceptDRTCPConnection(int servSock);
int CreateDRTCPServerSocket(unsigned short port);
void DieWithError(char *errorMessage);

#endif
