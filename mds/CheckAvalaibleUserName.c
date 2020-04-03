#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mdsutil.h"

int checkavalaibleusername(char* username)
{
	//read credential file to register
	FILE* fp;
	fp = fopen("credentials", "r");
	
	if(fp == NULL)
		perror("File problem: ");

	char* line = (char*)malloc(sizeof(char)*(RCVBUFSIZE+1));
	while(fgets(line, RCVBUFSIZE, fp) != NULL){
		if(!strcmp(strtok(line, " "), username))
		{
			fclose(fp);
			free(line);
			return 1; // username already exist ERROR
		}
	}
	fclose(fp);
	free(line);
	return 0;
}

