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
#include <errno.h>

#define NPACK 10
#define PORT "9930"
#define SRV_IP "127.0.0.1"

// fait : gestion des arguments, création et remplissage des ack

// TODO : renvoyer les ack, écrire ou placer les packets reçus dans un buffer s'ils sont hors séquence mais rentrent dans la fenetre

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

// Returns 0 if the "index" in argument fits inside the window ("upperBound" and "lowerBound")
int insideWindow(struct packet *bufferPackets, int nextPack, int seqNum)
{
	if(seqNum>nextPack+31)
	{
		return 1;
	}
	else if(seqNum==bufferPackets[seqNum%32].seqNum)
	{
		return 1;
	}
	return 0;
}

// Returns the number of first packet off sequence (by seqNum), which is the number of the next packet the receiver needs to have
int nextPacketInSeq (int previousPack, struct packet buffer[])
{
	int i=previousPack;
	while ( i<previousPack+32)
	{	
		if(buffer[i%32].seqNum+1 != buffer[(i+1)%32].seqNum)
		return buffer[i%32].seqNum+1;
	}
	return -1;
}

// Fill the ack packets with zeros, window size and number of the next packet required
void producePacket (struct packet *p, int window, int nextPack)
{
	p->type = 0;
	p->window = window;
	p->seqNum = (uint8_t) nextPack;
	printf("SeqNum = %d\n",p->seqNum);
	p->length =(uint16_t) 1;
	p->payload[0]='\0';
	
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
    char *next;
	char *port=PORT;
	char *filename="test2.txt";

	struct addrinfo hints; // Filter of adresses
	memset(&hints, 0, sizeof(struct addrinfo));
	struct addrinfo *listAddr, *res;
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP
	hints.ai_flags = AI_PASSIVE;

	// Connexion to the host "hostname" trough the port "port"

	int err = getaddrinfo(hostname,port,&hints,&listAddr);
	// Place the different possible adresses to connect to "hostname" in the structure "listaddr"

	if (err!=0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
	}
	int sock; // File descriptor of the socket

	for(res = listAddr; res !=NULL; res = res->ai_next)
	// For each adress availaible, try to create and connect
	{
		if ((sock = socket(res->ai_family, res->ai_socktype,
	        res->ai_protocol)) != -1) // Creation of the socket
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
		
		// Connection succeded
	
	FILE *fichier = fopen(filename,"w");

	struct packet *bufferPackets = (struct packet*) malloc(31*sizeof(struct packet)); // buffer filled with packets sent
	struct packet *lastAck = (struct packet*) malloc(sizeof(struct packet)); // last ack sent
	struct packet *lastPacket = (struct packet*) malloc(sizeof(struct packet)); // last packet received
	
	int last=0;
	
	window=31;
	int nextPack = 0;

	int i;
	for(i=0;i<31;i++)
	{
		bufferPackets[i].seqNum=0;
	}
	i=0;

    struct sockaddr_in *address = (struct sockaddr_in*)res->ai_addr;
	
	while(last==0) {
	    err = recvfrom(sock, lastPacket, 520, 0, res->ai_addr,(socklen_t * __restrict__) &res->ai_addrlen);
	    if (err ==-1)
	    {
	    	fprintf(stderr, "Recvfrom failed");
	    }
	    
	    //printf("Received packet from %s:%d\n", inet_ntoa(address->sin_addr), htons(address->sin_port));
	    //fflush(stdout);
	     
	    if(nextPack == lastPacket->seqNum)
		{
			bufferPackets[nextPack%32]=*lastPacket;
			if(bufferPackets[nextPack%32].length!=512) last=1;
			int writeFrom=nextPack;
			nextPack=nextPacketInSeq(nextPack,bufferPackets);
			if (nextPack == -1)
			{
				fprintf(stderr, "NextPack returned -1");
			}
			printf("Next pack (coté receiver) : %d\n",nextPack);
			producePacket(lastAck,window,nextPack);
			printf("Last Ack (coté receiver) : %d\n",lastAck->seqNum);
			err = sendto(sock, (void*)&lastAck, 520 , 0, res->ai_addr, (socklen_t)res->ai_addrlen);
			if (err ==-1)
	     	{
	     		fprintf(stderr, "Sendto failed");
	     	}
			int written = writeData(fichier,bufferPackets,writeFrom,nextPack);
			window = window+nextPack-writeFrom;
		}
	    else if (insideWindow(bufferPackets,nextPack,lastPacket->seqNum))
		{
			sendto(sock, lastAck, 520 , 0, res->ai_addr, sizeof (res->ai_addr));
			bufferPackets[(lastPacket->seqNum)%32]=*lastPacket;
		}
	}
	
	// close file opened and socket fd
	fclose(fichier);
	free(lastAck);
	close(sock);
	free(bufferPackets);
	free(lastPacket);
    freeaddrinfo(listAddr);
    return 0;
 }
