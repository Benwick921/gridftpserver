#include "clientutil.h"

int writeblock(char* chunk, char* path)
{
	printf("write to path: %s\n",path);
	int fd = open(path, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU); //S_IRWXU  00700 user (file owner) has read, write, and execute permission
	write(fd, chunk, CHUNK_SIZE);
}
