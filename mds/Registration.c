#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mdsutil.h"

int registration(char* username, char* passwd)
{
	printf("Registration...\n");
	if(checkavalaibleusername(username))
		return 1;
	// no users with same username but they can have same password (hashed?)
	printf("Username avalaible\n");
	FILE* fp;
	fp = fopen("credentials", "a");
	char buffer[100] = {0};
	
	strcat(buffer, username);
	strcat(buffer, " ");
	strcat(buffer, passwd);
	strcat(buffer, "\n");
	
	int lenght = 0;
	for(int i = 0; buffer[i] != '\0'; i++)
		lenght++;
	
	char newcredential[lenght];	
	for(int i = 0; i<lenght; i++)
		newcredential[i] = buffer[i];
	
	fwrite(newcredential , 1, sizeof(newcredential), fp);
	printf("Successfull\n");
	
	fclose(fp);
	return 0;
}
