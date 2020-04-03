#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <stdint.h>
#include "drutil.h"

void HandleDRTCPClient(int clntSocket, int totalstorage, int usedstorage)
{
	char* buffer = (char*)calloc(RCVBUFSIZE, sizeof(char));
	buffer[RCVBUFSIZE]=0;       /* Buffer for echo string */
    int recvMsgSize;            /* Size of received message */
	
    /* Receive message from client */
    buffer = readfromsocket(clntSocket, RCVBUFSIZE);
	
	char* user = NULL;
	char* file = NULL;
	/* Send received string and receive again until end of transmission */
	while (1)      /* zero indicates end of transmission */
	{
		printf("\n\nbuffer: %s\n",buffer);
		char* command = calloc(RCVBUFSIZE, 1);
		command = strtok(buffer, " ");
		printf("command>> %s\n", command);
		
		if(!strcmp(command, "newclnt"))
		{
			char* username = strtok(NULL, " ");
			char* key = strtok(NULL, " ");
			
			printf("New client: %s | %s\n", username, key);
			
			writetosocket(clntSocket, "OK", RCVBUFSIZE);
			createfolder(username);

			//create auth folder
			createfolder(AUTH_FOLDER);
			
			// create file with name key inside auth folder
			char* path = calloc(RCVBUFSIZE, 1);
			strcat(path, AUTH_FOLDER);
			strcat(path, key);
			writetofile(path, "");

			
			printf(">> %d - %d\n", checknewuser(username), !checknewuser(username));
			if(!checknewuser(username))
			{
				char* tempuser = calloc(RCVBUFSIZE, 1);
				strcpy(tempuser, username);
				strcat(tempuser, "\n");
				// write user to user.conf
				writeblock(tempuser, USER_CONFIG_FILE);
			}
			// free resources
			free(key);
			free(username);
		}		
		else if(!strcmp(command, "clntinf"))
		{
			user = (char*)malloc(sizeof(char)*21);
			file = (char*)malloc(sizeof(char)*21);
			user = strtok(NULL, " ");
			file = strtok(NULL, " ");
			char* key = strtok(NULL, " ");
			
			int wrtmp = 1;
			
			printf("user: %s | file: %s | key: %s\n", user, file, key);
			
			// controllare la key con il file dentro auth
			int x = removeauthfile(key);
			if(!x) puts("auth removed\n");
			else{ puts("cant remove auth\n"); close(clntSocket); exit(1);}
			
			//loop
			char* buf = (char*)malloc(sizeof(char)*RCVBUFSIZE);
			while(strcmp(buf, "DONE"))
			{
				//read from socket
				buf = (char*)malloc(sizeof(char)*BLOCK_SIZE);
				buf = readfromsocket(clntSocket, BLOCK_SIZE);
				
				//check MD5
				int index = checkmd5(buf);
				char* chunknumber = calloc(10, 1);
				sprintf(chunknumber, "%d", index);
				if(index == -1)
				{
					if (write(clntSocket, "ERROR", RCVBUFSIZE) < 0) 
					{
						perror("write");
						exit(1);
					}
					continue;
				}
				
				// conferma al client che il blocco e` corretto
				writetosocket(clntSocket, "OK", RCVBUFSIZE);

				//create folder/filefilder/# path
				char* path = (char*)calloc(100, sizeof(char));
				strcat(path, user);
				strcat(path, "/");
				
				// crea il file contiene tutti i file uploadati dell'utente
				char* temppath = calloc(RCVBUFSIZE, 1);
				strcpy(temppath, path);
				strcat(temppath, user);
				strcat(temppath, ".conf");
				//if(!checknewfile(file, temppath))
				//{
				if(wrtmp){
					char* tempfile = calloc(RCVBUFSIZE, 1);
					
					strcpy(tempfile, file);
					strcat(tempfile, "\n");
					
					writetofile(temppath, tempfile);
					//writeblock(tempfile, temppath);
					wrtmp = 0;
				}
				//}
				
				// continua..
				char* name = strtok(file, ".");
				if(!name) strcat(path, file);
				else strcat(path, name);
				createfolder(path);
				
				
				// contina a scrivie il blocco
				strcat(path, "/");
				strcat(path, chunknumber);
				
				// write file chunk in a file			
				writeblock(buf, path);
				
				// free resources	
				/*free(chunknumber);
				free(key);
				free(chunk);
				free(path);
				free(name);
				free(buf);
				free(bufcpy);*/
			}
		}
		else if(!strcmp(command, "download"))
		{
			user = (char*)calloc(21, sizeof(char));
			file = (char*)calloc(21, sizeof(char));
			user = strtok(NULL, " ");
			file = strtok(NULL, " ");
			
			char* key = strtok(NULL, " ");
			file = strtok(file, ".");
			
			printf("temps: %s %s %s\n", user, file, key);
			
			char* path = (char*)calloc(RCVBUFSIZE, sizeof(char));
			strcat(path, user);
			strcat(path, "/");
			strcat(path, file);
			strcat(path, "/");
			printf(">>path: %s\n", path);
			
			get(path, clntSocket, key);
		}
		else if(!strcmp(command, "info"))
		{
			char* path = strtok(NULL, " ");
			
			// andare a controllare quanti blocchi (files) ci sono nel path
			int files = countfiles(path);
			
			// inviare il numero al mds
			char* buf = calloc(RCVBUFSIZE, 1);
			sprintf(buf, "%d", files);
			writetosocket(clntSocket, buf, RCVBUFSIZE);
		}
		else if(!strcmp(command, "delete"))
		{
			char* username = strtok(NULL, " ");
			char* filename = strtok(NULL, " ");
			
			removefile(TEMPFILE);
			FILE* temp = fopen(TEMPFILE, "a+");
			
			char* path = calloc(RCVBUFSIZE, 1);
			strcat(path, username);
			strcat(path, "/");
			strcat(path, username);
			strcat(path, ".conf");
			
			FILE* userfile = fopen(path, "r");
			char* line = calloc(RCVBUFSIZE, 1);
			strcat(filename, "\n");
			while(fgets(line, RCVBUFSIZE, userfile))
				if(strcmp(line, filename))
					fprintf(temp, "%s", line);
			fclose(temp);
			fclose(userfile);
			
			temp = fopen(TEMPFILE, "r");
			userfile = fopen(path, "w");
					
			line = calloc(RCVBUFSIZE, 1);
					
			while(fgets(line, RCVBUFSIZE, temp))
			{
				fprintf(userfile, "%s", line);
				line = calloc(RCVBUFSIZE, 1);
			}
			fclose(temp);
			fclose(userfile);
			
			removefile(TEMPFILE);
			
			char* foldername = strtok(filename, ".");
			
			printf("> %s %s\n", username, foldername);
			char* shellcmd = calloc(RCVBUFSIZE, 1);
			strcat(shellcmd, "rm -r ");
			strcat(shellcmd, username);
			strcat(shellcmd, "/");
			strcat(shellcmd, foldername);
			
			system(shellcmd);
			
		}
		else puts("Unknown command\n");
		//close(clntSocket);
		free(buffer);
		buffer = (char*)calloc(RCVBUFSIZE, sizeof(char));
		recvMsgSize = 0;
	    /* See if there is more data to receive */
		buffer = readfromsocket(clntSocket, RCVBUFSIZE);
	}
    printf("Closing socket\n");
    free(buffer);
    close(clntSocket);    /* Close client socket */
    exit(0);
}
