#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

struct packet {
uint8_t type : 3;
uint8_t window : 5;
uint8_t seqNum;
uint16_t length;
char payload[512];
uint32_t crc;
} __attribute__((packed));


int equal(char *string1, char *string2)
{
	int i = 0;
	int stop =0;
	while(stop!=1)
	{
		if(string1[i]!='\0' && string2[i]!='\0')
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
		else if(string1[i]=='\0' && string2[i]=='\0' && stop==0)
		{
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	int sber=0,splr=0,delay=100;
	char *filename=NULL;
	char *hostname;
	char *next;
	char *port;
	int i;
	uint16_t seq =1;
	
	for(i=1; i<argc;i++)
	{
		if(equal(argv[i],"--file"))
		{
			i++;
			filename = argv[i];
		}
		else if(equal(argv[i],"--sber"))
		{
			i++;
			sber= strtol(argv[i],&next, 10);
		}
		else if(equal(argv[i],"--splr"))
		{
			i++;
			splr=strtol(argv[i],&next, 10);
		}
		else if(equal(argv[i],"--delay"))
		{
			i++;
			delay=strtol(argv[i],&next, 10);
		}
		else if(i==argc-2)
		{
			hostname = argv[i];
		}
		else if(i==argc-1)
		{
			port =argv[i];
		}
	}
	
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

	int ip = getaddrinfo(NULL,hostname,&hints,&listAddr);

	for(res = listAddr; res !=NULL; res = res->ai_next)
	{
		if ((sock = socket(res->ai_family, res->ai_socktype,
            res->ai_protocol)) != -1)
            {
            	if (connect(sock, res->ai_addr, res->ai_addrlen) == 0)
            	{
            		// If we get there, connect succeded
            		break;
            	}
    		}
    	else {
    	close(sock);
    	}
	}
	
	if (res == NULL)
	{
		fprintf(stderr,"Connection failed");
	}
	
	freeaddrinfo(listAddr);
	
	FILE *fichier = fopen(filename,"r");
	int current = 1;
	int data;
	int j=0;
	
	struct packet *p = malloc(sizeof(struct packet));
	p->type = 1;
	p->window = 0;
	p->seqNum = seq;
	p->length = 0;
	
	while (current>0)
	{
		if(j<512)
		{
			current = fread(p->payload,sizeof(char),1,fichier);
			p->length++;
			j++;
		}
		else
		{

			sendto(sock, p, (8+p->length)*8 , 0, 
       res->ai_addr, sizeof res->ai_addr);
			// créer un nouveau
		}
	}
	
	if(j!=0)
	{
		// si le dernier packet n'est pas vide, envoyer le paquet final
	}
	fclose(fichier);
	
	
}



