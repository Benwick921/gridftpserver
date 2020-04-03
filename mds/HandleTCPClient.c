#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <sys/wait.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mdsutil.h"

int printlist(struct drinfo* head)
{
	struct drinfo* index = head;
	while(index != NULL)
	{
		printf("#%d | IP: %s | PORT: %d | STORAGE: %d\n",index->number, index->ip, index->port, index->avalaiblestorage);
		index = index->next;
	}
}

struct drinfo* createlst(struct drinfo* head,int number, char* ip, int port, int size)
{
	struct drinfo* elem = malloc(sizeof(struct drinfo));
	elem->number = number;
	elem->ip = ip;
	elem->port = port;
	elem->avalaiblestorage = size;
	elem->next = NULL;
	
	if(head == NULL)
	{
		puts("null\n");
		head = elem;
	}
	else
	{
		struct drinfo* index = head;
		while(index->next != NULL)
			index = index->next;
		index->next = elem;
	}
	return head;
}

// da cambiare il nome, questo metodo viene chiamato quando bisogna dividere i blocchi del file in modo equo in tutti i dr
int update(char* linetoexclude)
{
	printf("linetoexclude: %s\n", linetoexclude);
	FILE* fp = fopen("drlist.conf", "r");
	char line[32];
	int linenumbers = 0;
	
	struct drinfo* head = NULL;
	
	while(fgets(line, 32, fp))
	{
		linenumbers++;
		printf("%s\n",line);
		char* ip = strtok(line, " ");
		char* port = strtok(NULL, " ");
		char* size = strtok(NULL, " ");
		
		head = createlst(head,linenumbers, ip, atoi(port), atoi(size));

		printf(">>%d | %s | %s | %s\n",linenumbers, ip, port, size);
	}
	printlist(head);
	
	// write back the file witout the linetoexplude
	
	
	return 0;
}

void HandleTCPClient(int clntSocket, char* connectionip, sem_t* drdata_semaphore)
{
	printf("INT-%d\n",sizeof(int));
	char number[3] = {0};
	sprintf(number, "%d", 13);
	printf("Sizefo number: %d %s\n",sizeof(number), number);
	
	char* user = NULL; // username
	char* buffer = (char*)calloc(RCVBUFSIZE+1, sizeof(char));
	buffer[RCVBUFSIZE]=0;       /* Buffer for echo string */   		// perche ho fatto sta cosa? in teroia non servirebbe, server per mettere lo 0 all'ultimo posto cosi ha un terminatore
    int recvMsgSize;            /* Size of received message */
    int clientauthenticated = 0;

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

	//while(1) //da levare
	//{
		/* Send received string and receive again until end of transmission */
		//while (recvMsgSize > 0)      /* zero indicates end of transmission */
		while(1)
		{
			printf("\nReceived from client: %s\n",buffer);
			char* command = calloc(RCVBUFSIZE, 1);
			command = strtok(buffer, " ");
			printf("Received from client command: %s\n",command);

			// creare una libreria drfunctionalities.h per tutte le funzioni put,get,del etc etc..
			if(!strcmp(command, "auth"))
			{
				char* username = strtok(NULL, " ");
				printf("Received from client username: %s\n",username);
				
				char* passwd = strtok(NULL, "\n");
				printf("Received from client password: %s\n",passwd);
				
				char response[RCVBUFSIZE] = {0};

				if(!checkcredential(username, passwd))
				{
					printf("Correctly authenticated!\n");
					//manage everithing and errors
					clientauthenticated = 1;
					strcpy(response, "Successfully autenticated.\n");
					user = username; 
					printf(">>>>>> %s\n", user);
				}
				else
				{
					printf("Authentication failed!\n");
					strcpy(response, "Authentication failed.\n");
				}
				 
				puts("> Sending response...\n");
			    if (send(clntSocket, response, RCVBUFSIZE, 0) != RCVBUFSIZE)
			       	DieWithError("send() failed");
			    puts("> Response sent.\n");	
			    buffer = (char*)malloc(sizeof(char)*(RCVBUFSIZE+1));
				//free(buffer);
				
				//free(username);
				//free(passwd);
				
				
			}
			else if(!strcmp(command, "info"))
			{
				puts("illcheck it later...");
				char* path = strtok(NULL, " ");
				
				printf("Opening files: %s\n", path);
				FILE* fp = fopen(path, "r");
				char* line = calloc(RCVBUFSIZE, 1);
				while(fgets(line, RCVBUFSIZE, fp))
				{
					printf("line: %s\n", line);
					char* drip = strtok(line, " ");
					if(!strcmp(connectionip, drip))
					{	
						// ottentere il numero di blocchi
						char* blocks = strtok(NULL, " "); // get port
						blocks = strtok(NULL, " "); // get blocks
						
						puts(blocks);
						
						//send back response
						writetosocket(clntSocket, blocks, RCVBUFSIZE);
						break;
					}
				}
				writetosocket(clntSocket, "-1", RCVBUFSIZE);
			}
			else if(!strcmp(command, "get") && clientauthenticated) // download
			{
				char* filename = strtok(NULL, " ");
				printf("get: %s\n", filename);
				if(filename != NULL) // questo controllo mi serve xk ho un piccolo bug, dopo aver dato il comando il server legge di nuovo il comando ma senza i paramentro
				{
					if(!checknewfile(user, filename))
					{
						puts("Inexistent file\n");
						char* command = calloc(RCVBUFSIZE, 1);
						strcat(command, "inexistent");
						writetosocket(clntSocket, command, RCVBUFSIZE);
						continue;
					}
					
					// ottenere la chiave del file da uploadare
					char* key = calloc(KEY_SIZE, 1);
					key = getfilekey(user, filename);
					char* buf = calloc(RCVBUFSIZE, 1);
					strcat(buf, "KEY ");
					strcat(buf, key);
					printf("key: %s\n", key);
					
					// send key to client
					printf("sending to client: %s\n", key);
					writetosocket(clntSocket, key, RCVBUFSIZE);
					if(strcmp("OK", readfromsocket(clntSocket, RCVBUFSIZE)))
						perror("KEY ERROR");
					
					// leggere il file riga per riga dove ce la lista dei dr (non dr.conf)
					char* fpath = (char*)calloc(RCVBUFSIZE, sizeof(char));
					strcat(fpath, user);
					strcat(fpath, "/");
					strcat(fpath, filename);
					printf("filepath: %s\n", fpath);
					
					char* line = (char*)calloc(RCVBUFSIZE, sizeof(char));
					FILE* fp = fopen(fpath, "r");
					while(fgets(line, RCVBUFSIZE, fp))
					{
						printf("dr line: %s\n", line);

						// notify every dr						
						char* templine = calloc(RCVBUFSIZE, 1);
						strcpy(templine, line);
						char* ip = strtok(templine, " ");
						char* port = strtok(NULL, " ");
						
						notifydr(ip, port, user, key);		
						
						writetosocket(clntSocket, line, RCVBUFSIZE);
						char* ack = calloc(RCVBUFSIZE, 1);
						printf("ACK%s | %d - %d\n",ack, strcmp(ack, "OK") ,!strcmp(ack, "OK"));
						while(strcmp(ack, "OK"))
						{
							puts(".");
							ack = readfromsocket(clntSocket, RCVBUFSIZE);
							printf("Ack: %s\n");
						}
					}
					puts("writing DONE\n");
					writetosocket(clntSocket, "DONE", RCVBUFSIZE);
					fclose(fp);
				}
			}
			else if(!strcmp(command, "put") && clientauthenticated) // upload
			{
				/* se il numero di blocchi per DR asupera 999 deve andare in errore */
				char* filename = strtok(NULL, " ");
				char* filesize = strtok(NULL, " ");
				
				if(filename != NULL) // questo controllo mi serve xk ho un piccolo bug, dopo aver dato il comando il server legge di nuovo il comando ma senza i paramentro
				{
					
					if(checknewfile(user, filename))
					{
						puts("UPLOAD: file exist, cant upload again");
						writetosocket(clntSocket, "exist", RCVBUFSIZE);
						continue;
					}
					
					printf(">>File name: %s\n", filename);
					printf(">>File size: %s\n", filesize);
					
					// calcolo numero di bloccho necessari
					int blocknumbers = ((int)ceil((float)atoi(filesize)/(float)CHUNK_SIZE));
					
					// calcolare se ce posto fra tutti i DR per salvare il file di queste dimenzioni
					// checkstorage();
					
					// Calcolo numeri DR
					FILE* fp = fopen("drlist.conf","r");
					int drcount = 0;
					char line[RCVBUFSIZE] = {0};
					// per ogni linea controllare se il dr e` UP o DOWN
					while(fgets(line, RCVBUFSIZE, fp))
					{
						char* buf = (char*)calloc(RCVBUFSIZE, sizeof(char));
						strcpy(buf, line);
						
						// controllo i DR disponibili e li conto
						char* ip = strtok(buf, " ");
						char* port = strtok(NULL, " ");
						if(!drstatus(ip, port))
							drcount++;
						//else update(buf);
					}
					printf("Total DR: %d\n", drcount);
					fclose(fp);
					
					// Distribuzione blocchi
					int basicdistribution = 0;
					int leftblocks = 0;
					if(drcount < blocknumbers)
					{
						basicdistribution = blocknumbers / drcount;
						printf("Basic distribution: %d\n",basicdistribution);
						
						leftblocks = blocknumbers % drcount;
						printf("Left Blocks: %d\n",leftblocks);
					}
					else
						basicdistribution = blocknumbers;
					
					int blocksval[drcount];
					for(int i = 0; i < drcount; i++)
					{
						if(i < leftblocks && leftblocks != 0)
							blocksval[i] = basicdistribution+1;
						else		
							blocksval[i] = basicdistribution;
						
					//}
					
					
					// Generazione chiave univoca per identificare il file
					char* key = (char*)calloc(RCVBUFSIZE, sizeof(char));
					char* appo = (char*)calloc(KEY_SIZE, sizeof(char));
					appo = generatekey(KEY_SIZE);
					strcat(key, "KEY ");
					strcat(key, appo);
					printf("%s\n", key);
				
					printf("> Sending back the key...\n");
					if (send(clntSocket, key, RCVBUFSIZE, 0) != RCVBUFSIZE)
					   	DieWithError("send() failed");	
					
					
					// creare file associazione nomefile-key
					char* path = (char*)calloc(100, sizeof(char));
					strcat(path, user);
					createfolder(path);
					strcat(path, "/");
					strcat(path,user);
					
					
					
					if(!(fp = fopen(path, "a+")))
						puts("Error: fp = fopen(user, \"a+\")");
					char* linetowrite = (char*)calloc(100, sizeof(char));
					printf("writing to %s: %s\n",path, linetowrite);
					fprintf(fp, "%s %s %s\n", filename, appo, filesize);
					fflush(fp);
					fclose(fp);
					
					// invio informaizoni necessarie al client e salvataggio dei dr in cui sono andati a finire i blocchi
					int index = 0;
					fp = fopen("drlist.conf","r");
					
					path = (char*)calloc(100, sizeof(char));
					strcat(path, user);
					strcat(path, "/");
					strcat(path, filename);
					FILE* fp1 = fopen(path, "a+");
					while(fgets(line, 32, fp))
					{
						char drinfo[RCVBUFSIZE] = {0};
						char* ip = strtok(line, " ");
						char* port = strtok(NULL, " ");
						
						
						// apendo dr e ip riuardanti questo file
						fprintf(fp1, "%s %s %d\n", ip, port, blocksval[index]);
						fflush(fp1);
						fclose(fp1);
						
						// creazione infomazione da inviare al client
						strcat(drinfo, ip);
						strcat(drinfo, " ");
						strcat(drinfo, port);
						strcat(drinfo, " ");
						char intval[3] = {0}; // integer value converted in char
						sprintf(intval, "%d", blocksval[index++]);
						strcat(drinfo, intval);
												
						// notificare qua dato che mi ricavo ip e porta del dr
						printf("Notifying %s %s\n", ip, port);
						notifydr(ip, port, user, appo);						
						
						printf("Sending: %s\n",drinfo);
						writetosocket(clntSocket, drinfo, RCVBUFSIZE);
						//if (send(clntSocket, drinfo, RCVBUFSIZE, 0) != RCVBUFSIZE)
					   	//	DieWithError("send() failed");
						
					}
					fclose(fp);
					
					printf("> DONE\n");
					writetosocket(clntSocket, "DONE", RCVBUFSIZE);
					//if (send(clntSocket, "DONE", RCVBUFSIZE, 0) != RCVBUFSIZE)
					 //  	DieWithError("send() failed");	
					
					
									
					// creare file per salvare in quale dr quale blocco e andato a finire
					
					
					// inviare la chiave ai mds e avvisarli che sta per arrivare una connessione
					// dal client X e i blocchi che inserira in ogni DR, il client aspetta finche
					// i dr non sono pronti e poi puo cominciare a mandare i blocchi 1 alla volta.
					
					//free(filename);
					}
			    }
			}
			else if(!strcmp(command, "register"))
			{
				char* username = strtok(NULL, " ");
				printf("Received from client username: %s\n",username);
				
				char* passwd = strtok(NULL, "\n");
				printf("Received from client password: %s\n",passwd);
				
				char response[RCVBUFSIZE] = {0};
				
				if(!registration(username, passwd))
				{
					printf("registration successfull\n");
					strcpy(response, "Registration succeded.\n");
				}
				else
				{
					printf("User name not avalaible.\n");
					strcpy(response, "Registration failed.\n");
				}
				
				puts("> Sending response...\n");
			    if (send(clntSocket, response, RCVBUFSIZE, 0) != RCVBUFSIZE)
			       	DieWithError("send() failed");
			    puts("> Response sent.\n");	
			    
			    createfolder(user);
			    
			    buffer = (char*)malloc(sizeof(char)*(RCVBUFSIZE+1));
			    free(buffer);
				free(username);
				free(passwd);
			}
			else if(!strcmp(command, "drup"))
			{
				printf("New data repository is up\n");
				/* da completare adesso che so anche l'ip da qui proviene la connessione
				manca da scrivere il numero del DR e mi chiedo se sia un ubformazione necessaria
				il numero dentro al file serve a poco ma ci sta quando viene creata la lista linkata*/
				
				char* portnsize = strtok(NULL, buffer);
				printf("portnsize: %s\n",portnsize);
				if(!checkdoubledr(connectionip, portnsize))
				{
					printf("portnsize: %s\n",portnsize);
					sem_wait(drdata_semaphore);
					FILE* fp = fopen("drlist.conf","a");
					if(fp == NULL)
					{
						sem_post(drdata_semaphore);
						perror("drlist.conf");
						exit(1);
					} 
					printf("wrinfitng to file...\n");
					printf("wrinting: %s %s\n", connectionip, portnsize);
					fprintf(fp, "%s %s\n",connectionip,portnsize);	
					fflush(fp);			
					sem_post(drdata_semaphore);
					
					// da rimuovere e metterlo nel upload file method
					//update();
					
					printf("writing completed.\n");
					fclose(fp);
				}
				else
					puts("DR already esist!");
				
				// inviare un ACK al mittente per avvisare della scrittura completata su file
				printf("> Sending back the ACK.\n");
				char ack[4] = "OK!";
			    if (send(clntSocket, ack, 4, 0) != 4)
			       	DieWithError("send() failed");
			    printf("> ACK sent.\n");
			}
			else if(!strcmp(command, "delete"))
			{
				puts("--------------------------------------------");
				char* filename = strtok(NULL, " ");
				char* username = strtok(NULL, " ");
				// cancellare i blocchi da ogni dr
				char* path = calloc(RCVBUFSIZE, 1);
				strcat(path, username);
				strcat(path, "/");
				strcat(path, filename);
				FILE* fp = fopen(path, "r");
				if(!fp)
				{
					close(clntSocket); 
					puts("file inesistente\n"); 
					break;
				}
				char* line = calloc(RCVBUFSIZE, 1);
				while(fgets(line, RCVBUFSIZE, fp))
				{
					puts(line);
					char* ip = strtok(line, " ");
					char* port = strtok(NULL, " ");
					
					int s = createsocket(ip, port);
					
					char* command = calloc(RCVBUFSIZE, 1);
					strcat(command, "delete ");
					strcat(command, username);
					strcat(command, " ");
					strcat(command, filename);
					
					puts(command);
					writetosocket(s, command, RCVBUFSIZE);
					
					// remove the file user/file 
					removefile(path);					
					
					// remove the file inside user/user
					removefile(TEMPFILE);
					FILE* temp = fopen(TEMPFILE, "a+");
					
					char* userfilepath = calloc(RCVBUFSIZE, 1);
					strcat(userfilepath, username);
					strcat(userfilepath, "/");
					strcat(userfilepath, username);
					FILE* userfile = fopen(userfilepath, "r");
					
					char* line2 = calloc(RCVBUFSIZE, 1);
					while(fgets(line2, RCVBUFSIZE, userfile))
					{
						char* templine = calloc(RCVBUFSIZE, 1);
						strcpy(templine, line2);
						
						//if(!strcmp(templine, "")) break;
						
						if(strcmp(strtok(templine, " "), filename))
							fprintf(temp, "%s", line2);
						else
							printf("deleting: %s\n", line2);
					}
					fclose(temp);
					fclose(userfile);
					
					temp = fopen(TEMPFILE, "r");
					userfile = fopen(userfilepath, "w");
					
					line2 = calloc(RCVBUFSIZE, 1);
					
					while(fgets(line2, RCVBUFSIZE, temp))
					{
						fprintf(userfile, "%s", line2);
						line2 = calloc(RCVBUFSIZE, 1);
					}
					fclose(temp);
					fclose(userfile);
					
					removefile(TEMPFILE);
					
					close(s);
									
				}
			}
			else if(!strcmp(command, "exit"))
			{
				printf("Bye bye\n");
				close(clntSocket);
				break;
			}
			else
			{
				printf("Unknown command or client not authenticated.\n");
			}
			
			free(buffer);
			buffer = (char*)calloc(RCVBUFSIZE+1, sizeof(char));
			
		    /* Wait for the next command to receive */
	   		if ((recvMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
		    	DieWithError("recv() failed");
		}
	//}
    printf("Closing socket\n");
    free(buffer);
    close(clntSocket);    /* Close client socket */
}
