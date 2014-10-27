#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFLEN 512
#define NPACK 10
#define PORT "9930"
#define SRV_IP "127.0.0.1"

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
	
	if(strlen(string1)!=strlen(string2)) return 1;
	while(i<strlen(string1))
	{
		if(string1[i]!=string2[i])
		{
			return 1;
		}
		else
		{
			i++;
		}
	}
	return 0;
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

int main(int argc, char* argv[])
{

    struct sockaddr_in si_other;
    int i, slen=sizeof(si_other);
    char buf[BUFLEN];
    
    int sber=0,splr=0,delay=100;
	char *filename=NULL;
	char *hostname=SRV_IP;
	char *next;
	char *port;
	
/*	if(argc<3)*/
/*	{*/
/*		fprintf(stderr,"Number of argument is too short (minimum 2).");*/
/*	}*/
	
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
			port = argv[i];
		}
	}
	
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	struct addrinfo *listAddr, *res;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int err = getaddrinfo(hostname,PORT,&hints,&listAddr); // possible ip adresses
	
	if (err!=0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
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

    for (i=0; i<NPACK; i++)
    {
    	printf("Sending packet %d\n", i);
    	sprintf(buf, "This is packet %d\n", i);
    	sendto(sock, buf, BUFLEN, 0,res->ai_addr, res->ai_addrlen);
    }
    close(sock);
    return 0;
}
	
/*	FILE *fichier = fopen(filename,"r");*/
/*	*/
/*	int *current = (int*) malloc(sizeof(int));*/
/*	*current = 1;*/
/*	int firstpacket =1;*/
/*	*/
/*	struct packet bufferPackets[31]; // buffer filled with packets sent*/
/*	struct packet *lastAck = (struct packet*) malloc(sizeof(struct packet)); // last ack received*/
/*	*/
/*	int j=0;*/
/*	*/
/*	fd_set readfds,writefds;*/
/*	FD_ZERO(&readfds);*/
/*	FD_ZERO(&writefds);*/
/*	FD_SET(sock,&readfds);*/
/*	FD_SET(sock,&writefds);*/
/*	*/
/*	int ready;*/
/*	struct timeval tv;*/

/*	while(current>0)*/
/*	{*/
/*		if (firstpacket==1) // first packet "probe"*/
/*		{*/
/*			producePacket(fichier,&bufferPackets[j],current);*/
/*			sendto(sock, &bufferPackets[j], (8+bufferPackets[j].length)*8 , 0, res->ai_addr, sizeof (res->ai_addr)); // send first packet*/
/*			recvfrom(sock, lastAck, 4160, 0, res->ai_addr, (socklen_t*) sizeof (res->ai_addr)); // wait for ack*/
/*			firstpacket=0;*/
/*			j++;*/
/*		}*/
/*		else*/
/*		{*/
/*			tv.tv_sec = 0;*/
/*    		tv.tv_usec = delay;*/
/*			ready = select(sock+1,&readfds,&writefds,NULL,&tv);*/
/*			*/
/*			if(ready>0) // if slots available in window*/
/*			{*/
/*				if(FD_ISSET(sock, &readfds) || lastAck->window=0)*/
/*				{*/
/*					recvfrom(sock, lastAck, 4160, 0, res->ai_addr, (socklen_t*) sizeof (res->ai_addr));*/
/*					j=lastAck->seqNum%32;*/
/*				}*/
/*				else if (FD_ISSET(sock, &writefds) && lastAck->window>0)*/
/*				{*/
/*					producePacket(fichier,&bufferPackets[j],current);*/
/*					sendto(sock, &bufferPackets[j], (8+bufferPackets[j].length)*8 , 0, res->ai_addr, sizeof (res->ai_addr));*/
/*					j++;*/
/*				}*/
/*			}*/
/*			else if (ready==0)*/
/*			{*/
/*				j=lastAck->seqNum%32;*/
/*				sendto(sock, &bufferPackets[j], (8+bufferPackets[j].length)*8 , 0, res->ai_addr, sizeof (res->ai_addr));*/
/*				j++;*/
/*			}*/
/*			else*/
/*			{*/
/*				fprintf(stderr,"Select aborted");*/
/*			}*/
/*		}*/
/*	}*/
/*	*/
/*	free(lastAck);*/
/*	*/
/*	// close file opened and socket fd*/
/*	fclose(fichier);*/
/*	close(sock);*/
/*	*/
/*}*/
