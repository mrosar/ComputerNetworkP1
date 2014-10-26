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

uint16_t seq =1;

int equal(char *string1, char *string2) // retourne 0 si deux tableaux de char sont égaux ou retourne 1 sinon
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

void producePacket (FILE *fichier, struct packet *p, int *current)
{
	p->type = 1;
	p->window = 0;
	p->seqNum = seq;
	seq++;
	p->length = 0;
	while (*current !=0 && p->length<=512)
	{
		if(p->length<=512) // if packet contains empty slot(s), fill with a byte
		{
			*current = fread(p->payload,sizeof(char),1,fichier);
			p->length++;
		}
	}
	
	// compute CRC
}

int main(int argc, char *argv[])
{
	int sber=0,splr=0,delay=100;
	char *filename=NULL;
	char *hostname;
	char *next;
	char *port;
	int i;
	
	if(argc<3)
	{
		fprintf(stderr,"Number of argument is too short (minimum 2).");
	}
	
	for(i=1; i<argc;i++) // boucle de traitement des arguments
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

	int err = getaddrinfo(NULL,hostname,&hints,&listAddr); // possible ip adresses
	
	if (err!=0)
	{
		fprintf(stderr,"No connection possible");
	}
	int sock; // file descriptor of the socket

	for(res = listAddr; res !=NULL; res = res->ai_next)
	{
		if ((sock = socket(res->ai_family, res->ai_socktype,
            res->ai_protocol)) != -1) // creation of the socket
            {
            	if (connect(sock, res->ai_addr, res->ai_addrlen) == 0)
            	{
            		// If we get there, connect succeded
            		// The destination of the packets is set to res->ai_addr
            		break;
            	}
    		}
    	else {
    	close(sock); // else close the socket fd
    	}
	}
	
	if (res == NULL)
	{
		fprintf(stderr,"Connection failed");
	}
	
	freeaddrinfo(listAddr);
	
	FILE *fichier = fopen(filename,"r");
	
	int *current = (int*) malloc(sizeof(int));
	*current = 1;

	while(current>0)
	{
		struct packet *p = (struct packet*) malloc(sizeof(struct packet));
		producePacket(fichier,p,current);
		sendto(sock, p, (8+p->length)*8 , 0, res->ai_addr, sizeof (res->ai_addr));
		free(p);
	}
	
	// close file opened and socket fd
	fclose(fichier);
	close(sock);
	
	
}



