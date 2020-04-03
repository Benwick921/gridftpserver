#include "drutil.h"

int createfolder(char* foldername)
{
	// create folder for user
    struct stat st = {0};
    if(stat(foldername, &st) == -1)
	   	mkdir(foldername, 0700);
}
