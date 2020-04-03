#include "drutil.h"

char* readfromsocket(int socket, int size)
{
	char* string = (char*)calloc(size, sizeof(char));
    /* Receive message */
	//read(socket, string, size);
    if ((recv(socket, string, size, 0)) < 0)
        DieWithError("recv() failed");
    return string;
}
