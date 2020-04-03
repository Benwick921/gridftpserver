#include "clientutil.h"

char* readfromsocket(int socket, int size)
{
	char* string = (char*)calloc(size, sizeof(char));
    /* Receive message */
    if ((recv(socket, string, size, 0)) < 0)
        DieWithError("recv() failed");
    return string;
}
