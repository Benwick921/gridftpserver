#include "clientutil.h"

int get(char* filename, char* username, int sock)
{
	// chiedere al mds quali sono i DR da contattare
	char* buf = (char*)calloc(RCVBUFSIZE, sizeof(char));
	strcat(buf, "get ");
	strcat(buf, filename);
	writetosocket(sock, buf, RCVBUFSIZE);
	
	char* key = calloc(KEY_SIZE, 1);
	key = strtok(readfromsocket(sock, RCVBUFSIZE), " ");
	printf("KEY: %s\n", key);
	writetosocket(sock, "OK", RCVBUFSIZE);
	
	// ricevo 1 alla volta tutti i DR e per ognuna di loro li contatto, quando ir DR ha fnito di inviare inviera DONE
	while(strcmp(buf, "DONE"))
	{
		buf = (char*)calloc(RCVBUFSIZE, sizeof(char));
		buf = readfromsocket(sock, RCVBUFSIZE);
		printf("BUF: %s\n");
		
		if(!strcmp(buf, "inexistent"))
		{
			puts("Inexistent file, cant be downloaded\n");
			return 1;
		}
		if(!strcmp(buf, "DONE")) break;
		// ricavo ip e porta del DR
		char* ip = strtok(buf, " ");
		char* port = strtok(NULL, " ");
		
		printf("IP: %s | PORT: %s\n", ip, port);
		
		int s = createsocket(ip, port);
		if(s == -1) return 1;
		
		puts(buf);
		// send username and filename to download
		buf = (char*)calloc(RCVBUFSIZE, sizeof(char));
		strcat(buf, "download ");
		strcat(buf, username);
		strcat(buf, " ");
		strcat(buf, filename);
		strcat(buf, " ");
		strcat(buf, key);
		
		writetosocket(s, buf, RCVBUFSIZE);
		
		char* buf1 = calloc(BLOCK_SIZE, 1);
		while(strcmp(buf, "DONE"))
		//while(1)
		{
			// ricezione blocchi
			buf1 = (char*)calloc(BLOCK_SIZE, sizeof(char));
			buf1 = readfromsocket(s, BLOCK_SIZE);
			printf("INF BUF: %s\n", buf1);
			if(!strcmp(buf1, "inexistent"))
			{
				puts("Inexisetnt file\n");
				break;
			}
			
			if(!strcmp(buf1, "DONE"))
				break;
			
			//printblock(buf1);
			// spacchetto tutto per controllare che sia tutto ok
			int index = 0;  
			char* key = calloc(KEY_SIZE, 1); 
			char* hash = calloc(32, 1);
			char* chunk = calloc(CHUNK_SIZE, 1);
			
			bcopy(buf1, &index, sizeof(int));
			bcopy(&buf1[sizeof(int)], key, KEY_SIZE);
			bcopy(&buf1[sizeof(int)+KEY_SIZE], hash, 32);
			bcopy(&buf1[sizeof(int)+KEY_SIZE+32], chunk, CHUNK_SIZE);
			//printf("received: %d | %s | %s | %s\n", index,key, hash, chunk);
				
			if(index == 0) continue;
			
			if(checkmd5(buf1) == -1)
			{
				writetosocket(s, "ERROR", RCVBUFSIZE);
				continue;
			}
			char* ack = calloc(RCVBUFSIZE, 1);
			strcpy(ack, "OK");
			printf("sending ack: %s\n", ack);
			writetosocket(s, ack, RCVBUFSIZE);
			char* appo = malloc(21);
			strcpy(appo, filename);
			strcat(appo, "1");
			writeblock(chunk, appo);
			
			//free(tempbuf);
			free(buf1);
		}
		puts(buf);
		writetosocket(sock, "OK", RCVBUFSIZE);
		puts("closing...\n");
		getchar();
		close(s);
	}
}
