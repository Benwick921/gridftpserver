#include "clientutil.h"

int writetosocket(int socket, char* string, int size)
{
	/* wrinte message */
	if (write(socket, string, size) < 0) 
	{
		perror("write");
		exit(1);
	}
}
