#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h> 		
#include <sys/socket.h>		 
#include <netinet/in.h>	
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <calcLib.h>
#include "protocol.h"

#define MAXLINE 1024

using namespace std;


/* Needs to be global, to be rechable by callback and main */
int ID = 1;
int loopCount = 0;
bool isRunning = true;

/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum){
  // As anybody can call the handler, its good coding to check the signal number that called it.

  printf("Let me be, I want to sleep.\n");

  if(loopCount > 20) {
    printf("I had enough.\n");
    isRunning = false;
  }
  
  return;
}

struct node {
	char clientstring[200];	// Lagra client IP och port i en sträng här., sprintf(array, "%s:%d", clientipaddress,clientport))
	unsigned int id; 
	int arith;
	int inVal1, inVal2;
	int iResult;
	float flVal1,flVal2;
	float fResult;
	struct timeval lastMessage;
	struct node * next;
};

// Linked List Functions
void add_node(node * head, calcProtocol &calcProt, struct sockaddr_in &cliAddr);
void checkJob(node * head, const char *Iaddress, calcProtocol * p_calcProt, calcMessage &calcMsg);

void printLinkedList(node * head);


int main(int argc, char *argv[])
{
  
  if(argc != 2)
	{
		perror("error, arguments are missing.");
	}
	unsigned short int SERVER_PORT = atoi(argv[1]);

  // Initialize random values.
	initCalcLib();

// VARIABLES  
//=====================================================================================================
	int sockfd;
	int byteSent = 0;
  	int byteRcvd = 0;

	char comprString[200];
	char *packet;

	struct sockaddr_in servAddr;
	struct sockaddr_in cliAddr;

	// Structs
	node head;
	head.id = 0;
	head.next = NULL;
	calcProtocol calcProt;
	calcMessage calcMsg;

	// Holder for calcMessage.
	calcMessage * p_calcMsg = (calcMessage*)malloc(sizeof(calcMessage));

	// Holder for calcProtocol.			
	calcProtocol * p_calcProt = (calcProtocol*)malloc(sizeof(calcProtocol));

	// Flexible pointer
	void* p_struct = malloc(sizeof(calcProtocol));
	



// UDP SOCKET.
//=====================================================================================================
	/* Create an UDP socket. */
  	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("could not create socket.");
	}
	printf("\n[+] Socket was created. File descriptor: %d\n", sockfd);

	/* Fill in the server's address and data. */
	memset(&servAddr, 0, sizeof(servAddr));  		
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);				
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  	/* Bind socket to an address. */
	int status = bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if (status < 0)
	{
		perror("failed to bind socket.");
	}
	printf("[+] Socket was successfully bound to %s:%d\n", inet_ntoa(servAddr.sin_addr), SERVER_PORT);
	printf("=== Waiting for messages from remote clients ===\n");



// TIME MANAGEMENT.
//=====================================================================================================
	/**
	* Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, 
	* it will be a single alarm 10s after it has been set. 
	*/
	itimerval alarmTime;
	/* Configure the timer to expire after 10 sec... */
	alarmTime.it_interval.tv_sec = 10;
	alarmTime.it_interval.tv_usec = 0;
	/* ... and every 10 sec after that. */
	alarmTime.it_value.tv_sec = 10;
	alarmTime.it_value.tv_usec = 0;

	signal(SIGALRM, checkJobbList);
	setitimer(ITIMER_REAL,&alarmTime,NULL); // Start/register the alarm. 



// LOOP.
//=====================================================================================================
//=====================================================================================================
  while(isRunning)
  {
	memset(&packet, 0, sizeof(packet));
	memset(&cliAddr, 0, sizeof(cliAddr));
	memset(&calcMsg, 0, sizeof(calcMsg));
	socklen_t cliLen = sizeof(cliAddr);

	// Receive & identify packet.
	byteRcvd = recvfrom(sockfd, (void*) p_struct, MAXLINE, 0, (struct sockaddr *) &cliAddr, &cliLen);
	if(byteRcvd < 0) {
	  	perror("error receiving message from the client.");
	}
	else if(byteRcvd == 16) {
		packet = "calcMessage";
	}
	else if(byteRcvd == 56) {
		packet = "calcProtocol";
	}
	printf("[x] %s [%d bytes] was received from client %s:%d\n", packet, byteRcvd, inet_ntoa(cliAddr.sin_addr), htons(cliAddr.sin_port));


	// Handle packets of 16 bytes.
	if(byteRcvd == 16)
	{	
		// If the received calcMessage is correct, send the calcProtocol.
		// type = 22, message = 0, protocol = 17, major_version = 1, minor_version = 0). 
		p_calcMsg = (struct calcMessage*)p_struct;

		if(	p_calcMsg->type == 22 &&
			p_calcMsg->message == 0 &&
			p_calcMsg->protocol == 17 &&
			p_calcMsg->major_version == 1 &&
			p_calcMsg->minor_version == 0)
		{
			memset(&calcProt, 0, sizeof(calcProt));
			calcProt.arith = randomType();
			if(calcProt.arith < 5)
			{
				calcProt.id = ID++;
				calcProt.inValue1 = randomInt();
				calcProt.inValue2 = randomInt();
			}
			else
			{
				calcProt.id = ID++;
				calcProt.flValue1 = randomFloat();
				calcProt.flValue2 = randomFloat();
			}

			byteSent = sendto(sockfd, &calcProt, sizeof(calcProt), 0, (const struct sockaddr *) &cliAddr, sizeof(cliAddr));
			if(byteSent < 0)
			{
				perror("sendto() failed to execute."); 		
			}	
			printf("[x] calcProtocol [%d bytes] were sent to %s:%d\n", byteSent, inet_ntoa(cliAddr.sin_addr), htons(cliAddr.sin_port));

		// SAVE CLIENT TO LINKED-LIST.
		add_node(&head, calcProt, cliAddr);

		} 

		// If the received calcMessage is incorrect. Reply with the supported calcMessage protocol.
		else {
			// calcMessage, type = 2, message = 2, major_version = 1, minor_version = 0
			p_calcMsg->type = 2;
			p_calcMsg->message = 2;
			p_calcMsg->major_version = 1;
			p_calcMsg->minor_version = 0;

			byteSent = sendto(sockfd, p_calcMsg, sizeof(*p_calcMsg), 0, (const struct sockaddr *) &cliAddr, sizeof(cliAddr));
			printf("[x] Protocol not supported. A calcMessage [%d bytes] were sent back to %s:%d\n", byteSent, inet_ntoa(cliAddr.sin_addr), htons(cliAddr.sin_port));	
		}
	}

	// Handle packets of 56 bytes.
	if(byteRcvd == 56)
	{
		p_calcProt = (struct calcProtocol*)p_struct;
		sprintf(comprString, "%s:%d", inet_ntoa(cliAddr.sin_addr), htons(cliAddr.sin_port));

		// Search joblist, compare and check if everything is correct.
		checkJob(&head, comprString, p_calcProt, calcMsg);
		byteSent = sendto(sockfd, (const struct calcMessage *) &calcMsg, sizeof(calcMsg), 0, (const struct sockaddr*)&cliAddr, sizeof(cliAddr));
   		printf("[x] calcMessage [%d bytes] was sent to the server.\n", byteSent);

	}

    sleep(1);
    loopCount++;
  }



  printf("Shutdown.\n");
  return(0);
}




void add_node(node * head, calcProtocol &calcProt, struct sockaddr_in &cliAddr)
{
	node * current = head;
	while (current->next != NULL) {
        current = current->next;
    }
	/* now we can add a new variable */
    current->next = (node *) malloc(sizeof(node));

	sprintf(current->next->clientstring, "%s:%d", inet_ntoa(cliAddr.sin_addr), htons(cliAddr.sin_port));

	current->next->id = calcProt.id;
	current->next->arith = calcProt.arith;
	current->next->inVal1 = calcProt.inValue1;
	current->next->inVal2 = calcProt.inValue2;
	current->next->flVal1 = calcProt.flValue1;
	current->next->flVal2 = calcProt.flValue2;

	// Calculate and store result.
		switch(current->next->arith)
    	{
			case 0:
				printf("Error: calcProtocol-->arith = 0.");
				break;
			case 1: 
				current->next->iResult = current->next->inVal1 + current->next->inVal2;
				break;
			case 2:
				current->next->iResult = current->next->inVal1 - current->next->inVal2;
				break;
			case 3:
				current->next->iResult = current->next->inVal1 * current->next->inVal2;
				break;
			case 4:
				current->next->iResult = current->next->inVal1 / current->next->inVal2;
				break;
			case 5:
				current->next->fResult = current->next->flVal1 + current->next->flVal2;
				break;
			case 6:
				current->next->fResult = current->next->flVal1 - current->next->flVal2;
				break;
			case 7:
				current->next->fResult = current->next->flVal1 * current->next->flVal2;
				break;
			case 8:
				current->next->fResult = current->next->flVal1 / current->next->flVal2;
			break;
    	}

	gettimeofday(&current->next->lastMessage, NULL);
    current->next->next = NULL;

	printf("[x] %s successfully added to joblist. ID: %d\n", current->next->clientstring, current->next->id);
}



void checkJob(node * head, const char *Iaddress, calcProtocol * p_calcProt, calcMessage &calcMsg)
{
	node * current = head;

	bool isSearching = true;
	bool hasFound = false;

	int dDelta = 0;
	double dEpsilon = 0.0001;

	// Search ID
	while (isSearching) 
	{	
		if(current->id == p_calcProt->id){
			isSearching = false;
			hasFound = true;
		}
		else if(current->next == NULL){
			isSearching = false;
		}
		else {
        current = current->next;
		}
    }

	// ID found
	if(hasFound)
	{	
		// Check client data match
		if(strcmp(current->clientstring, Iaddress) == 0)
		{
			// Compare result
			if(current->arith < 5) {
				if(current->iResult == p_calcProt->inResult)
				{
					calcMsg.message = 1;
				} 
				else
					calcMsg.message = 2;
			}
			else {
				dDelta = (current->fResult - p_calcProt->flResult);
				if(dDelta < 0) {
					dDelta *= -1.0;
				} 
				if (dDelta <= dEpsilon)
				{
					calcMsg.message = 1;
				}
				else
					calcMsg.message = 2;
			}
			calcMsg.type = 2;
		} 
		else {
			printf("Warning: %s might be a hacker!\n", Iaddress);
			calcMsg.type = 3;
			calcMsg.message = 2; 
		}
	}
}

/* Debugging */
void printLinkedList(node * head)
{
	printf("==========================================================\n");
	node * current = head;
	while (current->next != NULL) {
		current = current->next;
		printf("%d %d %d %lf %lf %s\n", current->id, current->inVal1, current->inVal2, current->flVal1, current->flVal2, current->clientstring);
    }	
	printf("==========================================================\n");
}