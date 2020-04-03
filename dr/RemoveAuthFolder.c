#include "drutil.h"

int removeauthfile(char* key)
{
	char* path = calloc(RCVBUFSIZE, 1);
	strcat(path, AUTH_FOLDER);
	strcat(path, "/");
	strcat(path, key);
	puts(path);
	if(!remove(path)) return 0;
	return 1;
}
