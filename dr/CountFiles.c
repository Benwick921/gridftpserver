#include "drutil.h"

int countfiles(char* path)
{
	printf("Counting fles in %s\n", path);
	int filecount = 0;
	DIR* dirp;
	struct dirent* entry;
	
	dirp = opendir(path);
	while((entry = readdir(dirp)) != NULL)
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;
		filecount++;
		puts(entry->d_name);
	}
	return filecount;
}
