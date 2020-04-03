#include "mdsutil.h"

int checkdoubledr(char* connectionip, char* portnsize)
{
	puts("Checking double dr.\n");
	char* newportnsize = (char*)malloc(sizeof(char)*strlen(portnsize));
	memcpy(newportnsize, portnsize, strlen(portnsize));
	printf("NEW: %s\n", newportnsize);
	
	char* newport = strtok(newportnsize, " ");
	char* newsize = strtok(NULL, " ");
	
	printf(">> %s %s\n",newport, newsize);
	
	FILE* fp = fopen("drlist.conf","r");
	char line[32];
	while(fgets(line, 32, fp))
	{
		char* ip = strtok(line, " ");
		char* port = strtok(NULL, " ");
		printf(">> ip: %s | port: %s\n", ip ,port);
		if(!strcmp(newport, port) && !strcmp(connectionip, ip))
		{
			puts("Duplicate DR\n");
			return 1;
		}
		puts("New DR\n");
	}
	fclose(fp);
	return 0;
}
