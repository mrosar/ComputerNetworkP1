#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int equal(char *string1, char *string2)
{
	int i = 0;
	int stop =0;
	while(stop!=1)
	{
		if(string1[i]!='/0' && string2[i]!='/0')
		{
			if(string1[i]!=string2[i])
			{
				stop=1;
			}
			else
			{
				i++;
			}
		}
		else if(string1[i]=='/0' && string2[i]=='/0' && stop==0)
		{
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	int sber,splr,delay;
	char *filename;
	for(int i; i<argc;i++)
	{
		if(
	}
	char *name = argv[1];
	int sock;
	
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	struct addrinfo *listAddr, *res;
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

	int ip = getaddrinfo(NULL,name,&hints,&listAddr);

	for(res = listAddr; res !=NULL; res = res->ai_next)
	{
		if ((sock = socket(res->ai_family, res->ai_socktype,
            res->ai_protocol)) != -1)
            {
            	if (bind(sock, res->ai_addr, res->ai_addrlen) == 0)
            	{
            		// If we get there, bind succeded
            		break;
            	}
    		}
    	else {
    	close(sock);
    	}
	}
	
	if (res == NULL)
	{
		fprintf(stderr,"Binding failed");
	}
	
	freeaddrinfo(listAddr);
	

}



