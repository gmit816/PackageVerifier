#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<strings.h>
#include<string.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>

#define PORT 8081
#define PACKETID 0XFFFF
#define CLIENTID 0XFF
#define ENDPACKETID 0XFFFF
#define TIMEOUT 1
#define DATATYPE 0XFFF1
#define ACKPACKET 0XFFF2
#define REJECTPACKETCODE 0XFFF3
#define OUTOFSEQUENCECODE 0XFFF4
#define LENGTHMISMATCHCODE 0XFFF5
#define ENDPACKETIDMISSINGCODE 0XFFF6
#define DUPLICATECODE 0XFFF7

struct datapacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t sequence_No;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct ackpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t sequence_No;
	uint16_t endpacketID;
};
struct rejectpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t sequence_No;
	uint16_t endpacketID;
};
// function to printout the packet information
void show(struct datapacket data) {
	printf("Received packet details:\n");
	printf(" packetID: %hx\n",data.packetID);
	printf("Client id : %hhx\n",data.clientID);
	printf("data: %x\n",data.type);
	printf("sequence no : %d\n",data.sequence_No);
	printf("length %d\n",data.length);
	printf("payload: %s\n",data.payload);
	printf("end of packet id : %x\n",data.endpacketID);
}
// function to generate the reject packet 
struct rejectpacket generaterejectpacket(struct datapacket data) {
	struct rejectpacket reject;
	reject.packetID = data.packetID;
	reject.clientID = data.clientID;
	reject.sequence_No = data.sequence_No;
	reject.type = REJECTPACKETCODE;
	reject.endpacketID = data.endpacketID;
	return reject;
}
// function to generate the ack packet
struct ackpacket generateackpacket(struct datapacket data) {
	struct ackpacket ack;
	ack.packetID = data.packetID;
	ack.clientID = data.clientID;
	ack.sequence_No = data.sequence_No;
	ack.type = ACKPACKET ;
	ack.endpacketID = data.endpacketID;
	return ack;
}

int main(int argc, char**argv)
{
	int sockfd,n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	struct datapacket data;
	struct ackpacket  ack;
	struct rejectpacket reject;

	// to store what all packets recieved.
	int buffer[20];
	int j;	
	for(j = 0; j < 20;j++) {
		buffer[j] = 0;
	}
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	int expectedPacket = 1;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("Server started \n");
	
	for(; ;) {
		n = recvfrom(sockfd,&data,sizeof(struct datapacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		printf("New : \n");
		show(data);
		buffer[data.sequence_No]++;
		if(data.sequence_No == 10 || data.sequence_No == 11) {
			buffer[data.sequence_No] = 1;
		}
		int length = strlen(data.payload);

		if(buffer[data.sequence_No] != 1) {
			reject = generaterejectpacket(data);
			reject.subcode = DUPLICATECODE;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("DUPLICATE PACKET RECIEVED! \n\n");
		}

		else if(length != data.length) {
			reject = generaterejectpacket(data);

			reject.subcode = LENGTHMISMATCHCODE ;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("LENGTH MISMATCH ERROR! \n\n");
		}
		else if(data.endpacketID != ENDPACKETID ) {
			reject = generaterejectpacket(data);

			reject.subcode = ENDPACKETIDMISSINGCODE ;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("END OF PACKET IDENTIFIER MISSING\n\n");

		}
		else if(data.sequence_No != expectedPacket && data.sequence_No != 10 && data.sequence_No != 11) {
			reject = generaterejectpacket(data);

			reject.subcode = OUTOFSEQUENCECODE;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("OUT OF SEQUENCE ERROR \n\n");
		}
		else {
			if(data.sequence_No == 10) {
				sleep(15);
			}
			ack = generateackpacket(data);
			sendto(sockfd,&ack,sizeof(struct ackpacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		expectedPacket++;
		printf("============================================================================================\n");
	}
}
