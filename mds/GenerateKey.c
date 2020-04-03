#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
char* generatekey(int keysize)
{
	struct timeval mytime;
	gettimeofday(&mytime, NULL);
	
	char* key = (char *)malloc(sizeof(char)*(keysize + 1));
	srand(mytime.tv_usec);
	int count = 0;
	while(count < keysize){
		if(rand()%2)
			key[count] = (rand()%26) + 65;
		else
			key[count] = (rand()%10) + 48;
		count++;	
	}
	key[keysize-1]='\0';
	return key;
}
