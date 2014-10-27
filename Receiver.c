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

int seq;

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

/*void producePacket (struct packet *p)*/
/*{*/
/*	p->type = 1;*/
/*	p->window = 0;*/
/*	p->seqNum = seq;*/
/*	seq++;*/
/*	p->length = 0;*/
/*	*/
/*	// compute CRC*/
/*}*/

    int main(void)
    {
    	char *hostname=SRV_IP;
		struct sockaddr_in si_other;
		int i, slen=sizeof(si_other);
		char buf[BUFLEN];
		
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
		
		struct sockaddr_in *address = (struct sockaddr_in*)res->ai_addr;
		
		for (i=0; i<NPACK; i++) {
		     recvfrom(sock, buf, BUFLEN, 0, res->ai_addr,(socklen_t * __restrict__) &res->ai_addrlen);
		     printf("Received packet from %s:%d\nData: %s\n\n", inet_ntoa(address->sin_addr), htons(address->sin_port), buf);
		}
		
		close(sock);
		return 0;
}
