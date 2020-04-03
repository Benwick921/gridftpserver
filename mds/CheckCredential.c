#include <stdio.h>
#include <stdlib.h>
#include "mdsutil.h"
#include <string.h>
int checkcredential(char* username, char* passwd)
{
	//read credential file to authenticate
	FILE* fp;
	fp = fopen("credentials", "r");
	
	if(fp == NULL)
		perror("File problem: ");
	
	char* line = (char*)malloc(sizeof(char)*(RCVBUFSIZE+1));
	while(fgets(line, RCVBUFSIZE, fp) != NULL)
		if(!strcmp(strtok(line, " "), username))
			if(!strcmp(strtok(NULL, "\n"), passwd))
				return 0; // correct credential
	
	free(line);
	fclose(fp);
	return 1; // wrong credential
}
