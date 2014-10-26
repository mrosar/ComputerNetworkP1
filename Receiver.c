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

void producePacket (struct packet *p)
{
	p->type = 1;
	p->window = 0;
	p->seqNum = seq;
	seq++;
	p->length = 0;
	
	// compute CRC
}

int main(int argc, char *argv[])
{
	char *filename=NULL;
	char *hostname;
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
	
}
