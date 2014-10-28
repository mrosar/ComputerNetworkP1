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

#define NPACK 10
#define PORT "9930"
#define SRV_IP "127.0.0.1"

// Packet structure
struct packet { 
uint8_t type : 3;
uint8_t window : 5;
uint8_t seqNum;
uint16_t length;
char payload[512];
uint32_t crc;
} __attribute__((packed));

int insideWindow(int lowerBound, int upperBound, int index) // Returns 0 if the "index" in argument fits inside the window ("upperBound" and "lowerBound")
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

// Returns the number of first packet off sequence (by seqNum), which is the number of the next packet the receiver needs to have 
int nextPacketInSeq (int lowerBound, int upperBound, struct packet buffer[]) 
{
	int i=lowerBound;
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

void producePacket (struct packet *p, int window, int nextPack) // Fill the ack packets with zeros, window size and number of the next packet required
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

// Write data inside the file "filename" from the "writeFrom"-th packet till the "end"-th packet from the structure "buffer"
int writeData (FILE *fileName, struct packet *buffer, int writeFrom, int end)
{
    
    int i, j;
    for (i = 0; i<end; i++) {
        if (i >= writeFrom) {
            j = 0;
            for (j = 0; j<buffer->length; j++) {
                fputc(buffer->payload[j], fileName);
            }
        }
        buffer++;
    }

    return 0;
}
    
  int main(int argc, char* argv[])
  {
    char *hostname=SRV_IP;
	char *port=PORT;
	char *filename="test2.txt";

	struct addrinfo hints; // Filter of adresses
	memset(&hints, 0, sizeof(struct addrinfo));
	struct addrinfo *listAddr, *res;
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_DGRAM;	// UDP
	hints.ai_flags = AI_PASSIVE;
	
	// Connexion to the host "hostname" trough the port "port"
	
		int err = getaddrinfo(hostname,port,&hints,&listAddr); // Place the different possible adresses to connect to "hostname" in the structure "listaddr"
	
		if (err!=0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		}
		
		int sock; // File descriptor of the socket

		for(res = listAddr; res !=NULL; res = res->ai_next) // For each adress availaible, try to create and connect a socket
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
	
	// connexion succeded
	
	FILE *fichier = fopen(filename,"w"); // File "filename" opened in writing mode (w)

	struct packet *bufferPackets = (struct packet*) malloc(31*sizeof(struct packet)); // buffer filled with packets sent
	struct packet *lastAck = (struct packet*) malloc(sizeof(struct packet)); // last ack sent
	struct packet *lastPacket = (struct packet*) malloc(sizeof(struct packet)); // last packet received


    struct sockaddr_in *address = (struct sockaddr_in*)res->ai_addr;
	
	int i;
	for (i=0; i<NPACK; i++) {
	     recvfrom(sock, (void*) &bufferPackets[i], 520, 0, res->ai_addr,(socklen_t * __restrict__) &res->ai_addrlen);
	     printf("Received packet from %s:%d\n", inet_ntoa(address->sin_addr), htons(address->sin_port));
	     //printf("Buffer packet n°%d \n %s",i,bufferPackets[i].payload);
	}
	int r = writeData(fichier,bufferPackets,0,9);
	
	// close file opened and socket fd
	fclose(fichier);	
	free(lastAck);
	close(sock);
	free(bufferPackets);
	free(lastPacket);
    freeaddrinfo(listAddr);
    return 0;
 }
