#define PKT_ID 0XFFFF
#define CLIENT_ID 0XFF
#define DATATYPE 0XFFF1
#define END_PKT_ID 0XFFFF
#define TIMEOUT 3

#define ACK_PKT_CODE 0XFFF2
#define REJ_PKT_CODE 0XFFF3
#define LEN_MISMATCH_PKT_CODE 0XFFF5
#define LAST_PKT_MISSING_CODE 0XFFF6
#define OUT_OF_SEQ_CODE 0XFFF4
#define DUP_PKT_CODE 0XFFF7

#include <stdio.h>
#include<stdint.h>
#include<time.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<string.h>
#include<strings.h>

struct class_data_pkt{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t sequence_number;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct class_ack_pkt {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t sequence_number;
	uint16_t endpacketID;
};
struct class_rej_pkt {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t sequence_number;
	uint16_t endpacketID;
};

struct class_data_pkt new_init_pkt() {
	struct class_data_pkt data;
	data.packetID = PKT_ID;
	data.clientID = CLIENT_ID;
	data.type = DATATYPE;
	data.endpacketID = END_PKT_ID;
	return data;
}
struct class_ack_pkt new_ack_pkt() {
	struct class_ack_pkt data;
	data.packetID = PKT_ID;
	data.clientID = CLIENT_ID;
	data.type = ACK_PKT_CODE;
	data.endpacketID = END_PKT_ID;
	return data;
}
struct class_rej_pkt new_reject_pkt() {
	struct class_rej_pkt data;
	data.packetID = PKT_ID;
	data.clientID = CLIENT_ID;
	data.type = ACK_PKT_CODE;
	data.endpacketID = END_PKT_ID;
	return data;
}

void print(struct class_data_pkt data) {
	printf("Packet Id: %x\n",data.packetID);
	printf("Client id : %hhx\n",data.clientID);
	printf("Data: %x\n",data.type);
	printf("Sequence No : %d \n",data.sequence_number);
	printf("Length %d\n",data.length);
	printf("Payload: %s",data.payload);
	printf("End of Packet Id : %x\n",data.endpacketID);
	printf("\n");
}

void ackprint(struct class_ack_pkt adata) {
	printf("Packet Id: %x\n",adata.packetID);
	printf("Client id : %x\n",adata.clientID);
	printf("Data: %x\n",adata.type);
	printf("Sequence No : %d \n",adata.sequence_number+1);
	printf("End of Packet Id : %x\n",adata.endpacketID);
	printf("\n");
}
void rejprint(struct class_rej_pkt rdata) {
	printf("Packet Id: %x\n",rdata.packetID);
	printf("Client id : %hhx\n",rdata.clientID);
	printf("Data: %x\n",rdata.type);
	printf("Sequence No : %d \n",rdata.sequence_number+1);
	printf("Reject Status : fff3\n");
	printf("Sub Code : %x\n",rdata.subcode);
	printf("End of Packet Id : %x\n",rdata.endpacketID);
	printf("\n");
}
int main(){
	struct class_data_pkt data;
	struct class_rej_pkt receivedPKT;
	struct sockaddr_in cliaddr;
	struct class_ack_pkt ackdata;
	struct class_rej_pkt rej;
	socklen_t addr_size;
	FILE *fp;

	char line[255];
	int sockfd;
	int n = 0;
	int counter = 0;
	int sequenceNo = 1;

	int check = 0;
	
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0) {
		printf("Unable to connect to server on specified port\n");
	}
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port=htons(8081);
	addr_size = sizeof cliaddr ;
	struct timeval tv;
	tv.tv_sec = TIMEOUT;  // 3 Secs Timeout
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
	fp = fopen("input.txt", "rt");
	if(fp == NULL)
	{
		printf("Can not open input.txt at the same location.\n");
		exit(0);
	}
	
	while(1)
	{
	   printf("1. Send packet to server\n");
	   printf("2. Length Mismatch\n");
	   printf("3. End of packet error\n");
	   printf("4. Out of sequence error or Duplicate Packet\n");
	   int i;
	   scanf("%d",&i);
	   check = 0;
	   data = new_init_pkt();
	   ackdata = new_ack_pkt();
	   rej=new_reject_pkt();
	   
	   if(fgets(line, sizeof(line), fp) != NULL) {
		n = 0;
		counter = 0;
		printf("%s",line);
		data.sequence_number = sequenceNo;
		strcpy(data.payload,line);
		data.length = strlen(data.payload);
		data.endpacketID = END_PKT_ID;
	   }
		
	   switch (i)
	   {
		case 1:
			break;		
		case 2:
			data.length++;
			break;
		case 3:
			data.endpacketID= 0;
			break;
		case 4:
			data.sequence_number = 4; 
			break;
		default:
			printf("Invalid option selected");
			check = 1;
		}
		if(check == 0){
			while(n<=0 && counter<3){
				sendto(sockfd,&data,sizeof(struct class_data_pkt),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(sockfd,&receivedPKT,sizeof(struct class_rej_pkt),0,NULL,NULL);
				if(n <= 0 )
				{
					printf("No response from server for three seconds sending the packet again\n");
					counter ++;
				}
				else if(receivedPKT.type == ACK_PKT_CODE  ) {
					print(data);
					printf("Ack packet recieved \n ");
					ackdata.sequence_number = data.sequence_number-1;
					ackprint(ackdata);
				}
				else if(receivedPKT.type == REJ_PKT_CODE ) {
					printf("Reject Packet recieved \n");
						rej.subcode=receivedPKT.subcode;
						rej.sequence_number = data.sequence_number-1;

					if(receivedPKT.subcode == LEN_MISMATCH_PKT_CODE ) {
					printf("Length mismatch error\n");

					}
					if(receivedPKT.subcode == LAST_PKT_MISSING_CODE ) {
						printf("END OF PACKET IDENTIFIER MISSING \n");
					}
						if(receivedPKT.subcode == OUT_OF_SEQ_CODE ) {
						printf("OUT OF SEQUENCE ERROR \n");
					}
					if(receivedPKT.subcode == DUP_PKT_CODE) {
						printf("DUPLICATE PACKET RECIEVED BY THE SERVER \n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) {
				printf("Server does not respond");
				exit(0);
			}
			sequenceNo++;
			printf("========================================================================\n");
		}
	}
}
