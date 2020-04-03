#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>     /* for close() */
#include <ctype.h>		/* for isdigit() */
#include <math.h>		/* ceil() */
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h> 
#include <openssl/md5.h>
#include <stdint.h>
#include "clientutil.h"

int main()
{
	int s,len;
	int authenticated = 0;
    struct sockaddr_in saddr;
    struct hostent *hst; // a cosa serve? host
    char* buffer = (char*)calloc(RCVBUFSIZE, sizeof(char));
    buffer[RCVBUFSIZE]=0;       /* Buffer for echo string */ //perche ho bisogno de fa sta cosa?
    char* user = NULL;
    
    
    printf("Inizializing connection to MDS.\n");
    // Create the socket
    if ((s=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
	perror("socket");
	exit(1);
    }
    puts("> Socket Created");
    
    // Determine host address by its network name
    if ((hst=gethostbyname(MDSIP))==NULL) {
	perror("gethostbyname");
	exit(1);
    }
    printf("> Gethostbyname: %u %d\n",*(int*)hst->h_addr,hst->h_length);
    
    // Fill structure
    bcopy(hst->h_addr,&saddr.sin_addr,hst->h_length);
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(MDSPORT); // MDS port
	
	// Connect to other socket
    if (connect(s,(struct sockaddr *)&saddr,sizeof(saddr))<0){
	perror("connect");
	exit(1);
    }
    puts("> Connected to MDS!");
   
    while(1)
    {
		// print menu
		printf("\n=-=-=-=-=-=  CLIENT V3.0  =-=-=-=-=-=\n");
		printf("\nMenu\n");
		printf("1 - Registration\n");
		printf("2 - Authentication\n");
		printf("3 - Upload file\n");
		printf("4 - Download file\n");
		printf("5 - Delete file (in progerss...)\n");
		printf("6 - Exit\n");
		
		char commandvalue = -1;
		printf("Select a value: \0");
		commandvalue = getchar(); // getchar imput di stringa
		
		// per qualche strana ragione il buffer deve avere almeno 16 caratteri di lunghezza.
		if(commandvalue == '1') // registration
		{
			int valid = 1;
			char username[20] = {0};
			printf("\nUser name (MAX 20): ");
			
			// input validation ma serve davvero?
			do
			{
				valid = 1;
				scanf(" %s", &username);
				for(int i = 0; i < 20; i++){
					if(username[i] != 0 && (!isalpha(username[i]) && !isdigit(username[i]) && valid))
					{
						valid = 0;
						puts("> Must contain alphanumeric characters!\n");
						break;
					}
				}
				printf(">> valid = %d\n", valid);
			}
			while(!valid);
			valid = 1;
			
			char passwd[20] = {0};
			printf("Password (MAX 20): ");
	   		scanf(" %s", &passwd);
			printf("Username: %s, Password: %s\n",username, passwd);
			printf("> unamelen: %d, pwdlen: %d\n",strlen(username), strlen(passwd));
			
			char command[RCVBUFSIZE] = {0};
			strcat(command, "register ");
			strncat(command, username, strlen(username));
			strcat(command, " ");
			strncat(command, passwd, strlen(passwd));
			
			printf("> cmd: %s\n\n", command);
			 
			// Write (or send) to socket
			writetosocket(s, command, RCVBUFSIZE);
		}
		else if(commandvalue == '2') // Authentication
		{
			if(authenticated)
			{
				puts("Already authenticated!\n");
				continue;
			}
			char username[21] = {0};
			printf("\nUser name (MAX 20): ");
			scanf(" %s", &username);
			
			user = (char*)calloc(21,sizeof(char));
			strcpy(user, username);
			
			char passwd[20] = {0};
			printf("Password (MAX 20): ");
			scanf(" %s", &passwd);
			printf("Username: %s, Password: %s\n",username, passwd);
			
			// +4 for "auth" + 3 for 2 spaces btween auth and usrname and passwd +1 for \0 at the end
			printf("> unamelen: %d, pwdlen: %d\n",strlen(username), strlen(passwd));
			
			char command[RCVBUFSIZE] = {0};
			strcat(command, "auth ");
			strncat(command, username, strlen(username));
			strcat(command, " ");
			strncat(command, passwd, strlen(passwd));
			strcat(command, "\n");
			
			printf("> cmd: %s\n\n", command);
			
			// Write (or send) to socket
			writetosocket(s, command, RCVBUFSIZE);
		}	
		else if(commandvalue == '3') // Upload file
		{
			char filename[20];
			long filesize = 0;
			// Input nome file da Uploadare
			printf("Enter name of file to upload: ");
			scanf(" %s",filename);
			printf("\nYou insert: %s\n", filename);
			
			// Cancolo dimezione file
			FILE* fp = fopen(filename, "r");
			if(fp)
			{
				fseek(fp, 0, SEEK_END);
				filesize = ftell(fp);
				fclose(fp);
			}
			else
			{
				puts("Inexistent file\n");
				continue;
			}
			
			
			// inviare a mds nomefile, dimenzione
			// command: put nomefile dinmenzione
			const long n = snprintf(NULL, 0, "%li", filesize);
			char buff[n+1];
			snprintf(buff, n+1, "%li", filesize);
			
			// dico al serer che cominico un upload
			char command[RCVBUFSIZE] = "put ";	
			strncat(command, filename, strlen(filename));
			strcat(command, " ");
			strncat(command, buff, strlen(buff));
			printf("command to send: %s\n",command);
			// Write (or send) to socket
			writetosocket(s, command, RCVBUFSIZE);
			
			puts("Command sent\n");
			// leggere il file e creare i blocchi
			int fd = open(filename, O_RDONLY);
			//while()
			//{
				int i = 0;
				char* clntinf = (char*)calloc(RCVBUFSIZE, sizeof(char));
				char* key = (char*) calloc(RCVBUFSIZE, sizeof(char));
				while(1)
				{
					struct blocks* lst = NULL;
					buffer = (char*)calloc(RCVBUFSIZE+1, sizeof(char));
					puts("reading socket...");
					
					/* Wait for the operatoin response to receive */
					buffer = readfromsocket(s, RCVBUFSIZE);
						
					// print response
					printf("Buffer>>: %s\n", buffer);
					
					if(!strcmp(buffer, "exist"))
					{
						puts("already exist\n");
						break;
					}
					
					// controllo se il file puo essere uploadatoo e` troppo grande
					if(!strcmp(buffer, "TOOBIG"))
					{
						puts("File too large, cant be uploaded.\n");
						break;
					}
					
					char* bufcpy = (char*)calloc(RCVBUFSIZE, sizeof(char));
					memcpy(bufcpy, buffer, RCVBUFSIZE);
					char* token = strtok(bufcpy, " ");
					if(!strcmp(token, "KEY")) // prima bisogna inviare la chiave
					{
						key = strtok(NULL, " ");
						printf("Token: %s\n", key);
						
						clntinf = (char*)calloc(RCVBUFSIZE, sizeof(char));
						strcat(clntinf, "clntinf ");
						strcat(clntinf, user);
						strcat(clntinf, " ");
						strcat(clntinf, filename);
						strcat(clntinf, " ");
						strcat(clntinf, key);
						printf("clntinf: %s\n", clntinf);
						continue;
					}
					if(!strcmp(token, "DONE"))
					{
						puts("DONE\n");
						break;
					}
					char* drip = strtok(buffer, " ");
					char* drport = strtok(NULL, " ");
					char* blockstosend = strtok(NULL, " ");
					printf(">> drip: %s | drport: %s | blockstosend: %s\n", drip , drport, blockstosend);
					
					while(i < atoi(blockstosend))
					{
						char line[CHUNK_SIZE+1] = {0}; // #Blocksize
						read(fd, line, CHUNK_SIZE);
						printf("chunk read: %s\n", line, md5(line));
						lst = blocklist(lst, i+1, key, line, md5(line));
						i++;
					}
					printlist(lst);
					put(drip, drport, lst, clntinf);
				}
			//}		
			close(fd);
			free(buffer);
			continue;
		}
		else if(commandvalue == '4') // download
		{
			char filename[20] = {0};
			// Input nome file da fare il Download
			printf("Enter name of file to download: ");
			scanf(" %s",filename);
			printf("\nYou insert: %s\n", filename);
			
			get(filename, user, s);
			continue;
		}
		else if(commandvalue == '5') // delete file
		{
			char filename[20] = {0};
			// Input nome file da Uploadare
			printf("Enter name of file to delete: ");
			scanf(" %s",filename);
			puts(filename);
			char* command = calloc(RCVBUFSIZE, 1);
			strcat(command, "delete ");
			strcat(command, filename);
			strcat(command, " ");
			strcat(command, user);
			puts(command);
			// send to MDS
			char port[4] = {0};
			sprintf(port, "%d", MDSPORT);
			puts(port);
			int s = createsocket(MDSIP, port);
			writetosocket(s, command, RCVBUFSIZE);			
			continue;
		}
		else if(commandvalue == '6')
		{
			// send comand exit to the server
			writetosocket(s, "exit", RCVBUFSIZE);
			close(s);
			return 0;
		}
		else{ puts("Unknows command"); continue; }
		puts("reading...\n	");
		// read from socket the response of the operation (success, failed)
		free(buffer);
		buffer = (char*)calloc(RCVBUFSIZE+1, sizeof(char));
		
		/* Wait for the operatoin response to receive */
		buffer = readfromsocket(s, RCVBUFSIZE);
		printf("> %s\n", buffer);
		if(!strcmp(buffer, "Successfully autenticated."))
			authenticated = 1;
    }
	close(s);
}
