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



struct node {
	char clientstring[200];	// Lagra client IP och port i en sträng här., sprintf(array, "%s:%d", clientipaddress,clientport))
	int id; 
	char *operation;
	int inVal1, inVal2;
	float flVal1,flVal2;
	struct timeval lastMessage;
	struct node * next;
};





/* Needs to be global, to be rechable by callback and main */
int loopCount=0;
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
	int bitSent = 0;
  	int bitRecv = 0;

  	char CLIENT_IP[MAXLINE];
  	char SERVER_IP[MAXLINE];

	struct sockaddr_in servAddr;
	struct sockaddr_in cliAddr;

	// Holder for calcMessage.
	calcMessage * p_calcMsg = (calcMessage*)malloc(sizeof(calcMessage));

	// calcProtocol.			
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

  	memset(SERVER_IP, '\0', sizeof(SERVER_IP));
  	inet_ntop(AF_INET, &servAddr.sin_addr, SERVER_IP, sizeof(SERVER_IP));

  /* Bind socket to an address. */
	int status = bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if (status < 0)
	{
		perror("failed to bind socket.");
	}
	printf("[+] Socket was successfully bound to %s:%d\n", SERVER_IP, SERVER_PORT);
	printf("Waiting for messages from remote clients...\n\n");



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
  while(isRunning)
  {
	memset(&cliAddr, 0, sizeof(cliAddr));
	socklen_t cliLen = sizeof(cliAddr);


	bitRecv = recvfrom(sockfd, (void*) p_struct, MAXLINE, 0, (struct sockaddr *) &cliAddr, &cliLen);
	if(bitRecv < 0)
	{
	  	perror("error receiving message from the client.");
	}
	else if(bitRecv == 16)
	{
		p_calcMsg = (struct calcMessage*)p_struct;
	}
	else if(bitRecv == 56)
	{
		p_calcProt = (struct calcProtocol*)p_struct;
	}

    // Client IP conversion.
	memset(CLIENT_IP, '\0', sizeof(CLIENT_IP));
    if(inet_ntop(AF_INET, &cliAddr.sin_addr, CLIENT_IP, sizeof(CLIENT_IP)) == NULL)
	{																					
	  	perror("inet_ntop failed.");
	}

    // Print message.
	printf("A message [%d bytes] was received from client %s:%d\n", bitRecv, CLIENT_IP, cliAddr.sin_port);


    // If the received calcMessage is correct, send the calcProtocol.
    // type = 22, message = 0, protocol = 17, major_version = 1, minor_version = 0). 
    if(	p_calcMsg->type == 22 &&
	  	p_calcMsg->message == 0 &&
	  	p_calcMsg->protocol == 17 &&
	  	p_calcMsg->major_version == 1 &&
	  	p_calcMsg->minor_version == 0)
	{
		p_calcProt->arith = randomType();
		if(p_calcProt->arith < 5)
		{
			p_calcProt->inValue1 = randomInt();
			p_calcProt->inValue2 = randomInt();
		}
		else
		{
			p_calcProt->flValue1 = randomFloat();
			p_calcProt->flValue2 = randomFloat();
		}

	  	bitSent = sendto(sockfd, p_calcProt, sizeof(*p_calcProt), 0, (const struct sockaddr *) &cliAddr, sizeof(cliAddr));
	  	if(bitSent < 0)
	  	{
	  		perror("sendto() failed to execute."); 		
	  	}	
	  	printf("calcProtocol [%d bytes] were sent to the target client.\n", bitSent);
	}
/*
	//////////////////////////////
	// HÄR SPARAS CLIENTS.
	/////////////////////////////






	temp->message = 1;
	bitSent = sendto(sockfd, temp, sizeof(*temp), 0, (const struct sockaddr*) &cliAddr, sizeof(cliAddr));	

*/

    printf("This is the main loop, %d time.\n",loopCount);
    sleep(1);
    loopCount++;
  }

  printf("done.\n");
  return(0);


  
}

void push(node * head)
{
	node * current = head;
	while (current->next != NULL) {
        current = current->next;
    }
	/* now we can add a new variable */
    current->next = (node *) malloc(sizeof(node));

	//sprintf(current->next->clientstring, "%s:%d", inet_ntoa(cliAddr.sin_addr), htons(cliAddr.sin_port));
	current->next->id;
	current->next->operation;
	current->next->inVal1;
	current->next->inVal2;
	current->next->flVal1;
	current->next->flVal2;
	gettimeofday(&current->next->lastMessage, NULL);

    current->next->next = NULL;
}

