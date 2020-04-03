#include "drutil.h"

int checknewuser(char* username)
{
	FILE* fp = fopen(USER_CONFIG_FILE, "r");
	if(!fp)
	{
		puts("user.conf doesent exist yet");
		return 0;
	} 
	char* line = calloc(RCVBUFSIZE, 1);
	while(fgets(line, RCVBUFSIZE, fp))
		if(strcmp(line, username))
			return 1;
	return 0;	
}
