#include "drutil.h"

int checknewfile(char* filename, char* path)
{
	printf("param: %s | %s\n", filename, path);
	FILE* fp = fopen(path, "r");
	if(!fp)
	{
		printf("%s doesent exist yet", filename);
		return 0;
	} 
	char* line = calloc(RCVBUFSIZE, 1);
	while(fgets(line, RCVBUFSIZE, fp))
	{
		printf("line: %s\n",line);
		printf("caomaring: %s %s\n", line, filename);
		printf("cmp: %d | !cmp: %d", strcmp(line, filename), !strcmp(line, filename));
		if(strcmp(line, filename))
			return 1;
	}
	puts("new file\n");
	return 0;	
}
