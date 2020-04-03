#include "mdsutil.h"

int checknewfile(char* username, char* filename)
{
	char* path = calloc(RCVBUFSIZE, 1);
	strcat(path, username);
	strcat(path, "/");
	strcat(path, filename);
	
	FILE* fp = fopen(path, "r");
	printf("fp: %d\n", fp);
	if(fp)
		return 1; // apertura con successo, file esistente
	return 0; // il file non esiste ancora
}
