#include "drutil.h"

int get(char* path, int socket, char* key)
{
	// controllare la key con il file dentro auth
	int x = removeauthfile(key);
	if(!x) puts("auth removed\n");
	else
	{ 
		puts("cant remove auth\n"); 
		close(socket); 
		exit(1);
	}
	
	
	struct dirent** namelist;
	int files = scandir(path, &namelist, NULL, alphasort);
	printf("tot files: %d\n", files);
	if(files < 0) perror("scandr");
	else
	{
		for(int i = 0; i < files; i++)
		{
			if (!strcmp (namelist[i]->d_name, "."))
            	continue;
        	if (!strcmp (namelist[i]->d_name, ".."))    
            	continue;
        	printf("reading file: %s\n",namelist[i]->d_name);
        	// leggere ogni file e inviarlo al client
        	char* line = (char*)calloc(BLOCK_SIZE, sizeof(char));
        	
        	char* temppath = (char*)calloc(BLOCK_SIZE, sizeof(char));
        	strcpy(temppath, path);
        	strcat(temppath, namelist[i]->d_name);
			printf("temppath: %s\n", temppath);
			
			int fd = open(temppath, O_RDONLY);
   			read(fd, line, BLOCK_SIZE);
			
			printblock(line);
			
	        char* ack = (char*)calloc(RCVBUFSIZE, sizeof(char));        
			do
			{
				// invio del blocco
				//printblock(line);
			    writetosocket(socket, line, BLOCK_SIZE);
				// rickezione ACK, Error or Ok
				ack = (char*)calloc(RCVBUFSIZE, sizeof(char));
				ack = readfromsocket(socket, RCVBUFSIZE);    
				printf("ACK: %s\n", ack);
			}
        	while(!strcmp(ack, "ERROR"));
        	free(namelist[i]);
		}
		free(namelist);
	}
	puts("done");
	writetosocket(socket, "DONE", RCVBUFSIZE);
}
