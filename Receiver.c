#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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

int window;

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

int insideWindow(int lowerBound, int upperBound, int index)
{
	if(lowerBound > upperBound)
	{
		if((index >=lowerBound && index <= 31) || (index >= 0 && index <= upperBound))
		{
			return 0;
		}
		else return 1;
	}
	else
	{
		if(index >= lowerBound && index <=upperBound)
		{
		return 0;
		}
		else return 1;
	}
}

int nextPacketInSeq (int lowerBound, int upperBound, struct packet buffer[])
{
	int i=lowerBound;;
	if( lowerBound < upperBound)
	{
		while ( i<=upperBound)
		{	
			if(buffer[i].seqNum == buffer[(i+1)%32].seqNum+1)
			i=(i+1)%32;
			else
			return (i+1)%32;
		}
	}
	else
	{
		while ( (i<=31 && i>=upperBound) || (i>=0 && i<=lowerBound ))
		{	
			if(buffer[i].seqNum == buffer[(i+1)%32].seqNum+1)
			i=(i+1)%32;
			else
			return (i+1)%32;
		}
	}
	return -1;
}

void producePacket (struct packet *p, int window, int nextPack)
{
	p->type = 0;
	p->window = window;
	p->seqNum = nextPack;
	p->length = 512;
	int i=0;
	while(i<512)
	{
		// if packet contains empty slot(s), fill with a byte
		p->payload[i]='\0';
		i++;
	}
	
	// compute CRC
}

int writeData (FILE *fileName, struct packet *buffer, int writeFrom, int end)
{
	// this function returns the number of packets written in sequence
	int count=0;
	return count;
}

int main(int argc, char* argv[])
{
	char *hostname=SRV_IP;
	int i;
	//char buf[BUFLEN];
	char *filename="Receiver2.c";
	
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
	        	if (bind(sock, res->ai_addr, res->ai_addrlen) == 0)
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
	
/*	struct sockaddr_in *address = (struct sockaddr_in*)res->ai_addr;*/
/*	*/
/*	for (i=0; i<NPACK; i++) {*/
/*	     recvfrom(sock, buf, BUFLEN, 0, res->ai_addr,(socklen_t * __restrict__) &res->ai_addrlen);*/
/*	     printf("Received packet from %s:%d\nData: %s\n\n", inet_ntoa(address->sin_addr), htons(address->sin_port), buf);*/
/*	}*/
		
	FILE *fichier = fopen(filename,"w");
	
	window=31;
	int nextPack = 0;
	
	struct packet *bufferPackets = (struct packet*) malloc(31*sizeof(struct packet)); // buffer filled with packets sent
	struct packet *lastAck = (struct packet*) malloc(sizeof(struct packet)); // last ack sent
	producePacket(lastAck,window,nextPack);
	struct packet *lastPacket = (struct packet*) malloc(sizeof(struct packet)); // last packet received
	int upperBound=30,lowerBound=0;
	int last =0;
	for(i=0;i<31;i++)
	{
		bufferPackets[i].seqNum=0;
	}
	

	while(last!=1)
	{
		recvfrom(sock, lastPacket, 4160, 0, res->ai_addr, (socklen_t*) sizeof (res->ai_addr));
		if( nextPack == (lastPacket->seqNum)%32 && insideWindow(lowerBound,upperBound,nextPack))
		{
			bufferPackets[nextPack]=*lastPacket;
			window--;
			int writeFrom=nextPack;
			nextPack=nextPacketInSeq(lowerBound,upperBound,bufferPackets);
			producePacket(lastAck,window,nextPack);
			sendto(sock, lastAck, 4160 , 0, res->ai_addr, sizeof (res->ai_addr));
			int written = writeData(fichier,bufferPackets,writeFrom,nextPack);
			window = window+written;
			upperBound = (lowerBound+written)%32;
		}
		else if (insideWindow(lowerBound,upperBound,(lastPacket->seqNum)%32))
		{
			sendto(sock, lastAck, 4160 , 0, res->ai_addr, sizeof (res->ai_addr));
			bufferPackets[(lastPacket->seqNum)%32]=*lastPacket;
			lowerBound = (lowerBound+1)%32;
			window--;
		}
	}
	
	free(lastAck);
	
	// close file opened and socket fd
	fclose(fichier);
	close(sock);
	return 0;
}
