#include "mdsutil.h"

char* getfilekey(char* username, char* filename)
{
	char* path = calloc(RCVBUFSIZE, 1);
	strcat(path, username);
	strcat(path, "/");
	strcat(path, username);
	
	FILE* fp = fopen(path, "r");
	if(!fp) return NULL;
	
	char* line = calloc(RCVBUFSIZE, 1);
	while(fgets(line, RCVBUFSIZE, fp))
	{
		char* file = strtok(line, " ");
		if(!strcmp(file, filename))
		{
			char* key = strtok(NULL, " ");
			return key;
		}
	}
	return NULL;
}
