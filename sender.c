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
      
//Packet structure
struct packet {
uint8_t type : 3;
uint8_t window : 5;
uint8_t seqNum;
uint16_t length;
char payload[512];
uint32_t crc;
} __attribute__((packed));

uint16_t seq =1; // Sequential number for packets sent

int equal(char *string1, char *string2) // Return 0 if two char tabs are equals
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

void producePacket (FILE *fichier, struct packet *p, int *current) // Fill the packet "p" with data contained in the file "fichier" from the pointer "current"
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
			*current = fread((void *)&p->payload[p->length],sizeof(char),1,fichier);
			p->length++;
		}
	}
	
	// compute CRC
}
      
int main(int argc, char* argv[])
	{
      	char *filename="test.txt";
      	char *hostname=SRV_IP;
		char *next;
		char *port=PORT;
        int i;
        int sber=0,splr=0,delay=100;
    
    /*	if(argc<3)*/
/*	{*/
/*		fprintf(stderr,"Number of argument is too short (minimum 2).");*/
/*	}*/
	
	for(i=1; i<argc;i++) // Loop used to get the arguments given at the call
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
		
		// Connection succeded
		
		FILE *fichier = fopen(filename,"r"); // File "filename" opened in reading mode (r)
	
		int *current = (int*) malloc(sizeof(int));
		*current = 1;

    	struct packet *bufferPackets = (struct packet*) malloc(31*sizeof(struct packet));
    	
    	for(i=0;i<NPACK;i++)
    	{
    		producePacket(fichier,&bufferPackets[i],current); // Put data from file inside packets
    	}
    	
/*    	for(i=0;i<NPACK;i++)*/
/*    	{*/
/*    		printf("Buffer packet n°%d :\n %s \n",i,bufferPackets[i].payload);*/
/*    	}*/

        for (i=0; i<NPACK; i++) {
          printf("Sending packet %d\n", i);
          sendto(sock,(void*) &bufferPackets[i],520, 0, res->ai_addr,(socklen_t) res->ai_addrlen); // Send packets
        }
        
        // Close file descriptors, socket and free memory allocated
        close(sock);
        free(bufferPackets);
        free(current);
        fclose(fichier);
        return 0;
      }
