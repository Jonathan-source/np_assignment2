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
int loopCount = 0;
int terminate = 0;

/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum)
{
  // As anybody can call the handler, its good coding to check the signal number that called it.
  printf("...waiting for messages...\n");

  if(loopCount > 20)
  {
    printf("Shutting down.\n");
    terminate = 1;
  } 
  return;
}

// Function for handling errors.
void error(char const * msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		error("error, arguments are missing");
	}

	int sockfd;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

	char recvBuff[MAXLINE];

	unsigned int SERVER_PORT = atoi(argv[1]);
	unsigned int n = 0;


	/** socket(int domain, int type,int protocol) :
	* Creates an endpoint for communication and returns a descriptor. 
	* Returns a descriptor referencing the socket, or -1 if an error occurs.
	**/ 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		error("could not create socket.");
	}
	printf("Socket was created, with file descriptor: %d\n", sockfd);
	printf("Setting up server...\n");


	// Fill the server address structure.
	memset(&serverAddr, 0, sizeof(serverAddr));  		

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);			// Convert to network byte order.
	serverAddr.sin_addr.s_addr = INADDR_ANY; 			// "0.0.0.0". 


	/** bind(int s, const struct sockaddr * name, socklen_t namelen ) :
	* When a socket is created with socket(), it exists in a namespace (address family)
	* but has no name assigned to it. The bind() function assigns a name to that unnamed socket. 
	* Return 0 for success, -1 if an error occurs.
	**/
	int rc = bind(sockfd, (const struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (rc < 0)
	{
		error("failed to bind socket.");
	}
	printf("...server setup complete.\nWaiting for message from remote clients ...\n");

	/**
	* Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, 
	* it will be a single alarm 10s after it has been set. 
	**/
	struct itimerval alarmTime;
	alarmTime.it_interval.tv_sec = 10;
	alarmTime.it_interval.tv_usec = 10;
	alarmTime.it_value.tv_sec = 10;
	alarmTime.it_value.tv_usec = 10;

	/**
	* Regiter a callback function, associated with the SIGALRM signal, 
	* which will be raised when the alarm goes of.
	**/
	signal(SIGALRM, checkJobbList);
	setitimer(ITIMER_REAL,&alarmTime,NULL); // Start/register the alarm. 



	// SERVER-LOOP
  	while(terminate == 0)
  	{

	  	memset(recvBuff, 0, sizeof(recvBuff));
		memset(&clientAddr, 0, sizeof(clientAddr));
		socklen_t clientLen = sizeof(clientAddr);


		/** recvfrom(int s,void * buff, size_t len, int flags, (struct sockaddr*)from, (socklen_t *)fromlen) :
	  	* Receive a message from the socket whether or not it's connection-oriented. 
	  	* If "from" is nonzero, and the socket is connectionless, the source address of the message is filled in. 
	  	* If no messages are available at the socket, the receive call waits for a message to arrive.
	  	* Returns the number of bytes received.
	  	*/ 
	  	if((n = recvfrom(sockfd, (char *)recvBuff, sizeof(recvBuff), 
	  		0, (struct sockaddr*) &clientAddr, &clientLen)) < 0)
	  	{
	  		error("error receiving message from the client.");
	  	}
	  	recvBuff[n];


	  	// Client information.
	  	char clientIP[MAXLINE];
	  	memset(clientIP, '\0', sizeof(clientIP));


		/** inet_ntop(int af, const void * src, char * dst, socklen_t size ) :
	  	* Converts a numeric network address pointed to by src into a text string
	  	* in the buffer pointed to by dst. Returns a pointer to the buffer containing
	  	* the text version of the address, or NULL if an error occurs.
	  	*/
	  	if(inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)) == NULL)
	  	{																					
	  		error("inet_ntop failed.");
	  	}
	  	// Print message.
	  	printf("\nReceived a message from client %s:%d\n", clientIP, clientAddr.sin_port);
	  	printf("[%s:%d]: %s\n\n", clientIP, clientAddr.sin_port, recvBuff);




	    sleep(1);
	    loopCount++;
  	}

  	// Close socket
  	close(sockfd);


  	printf("done.\n");
  	return(0);
}
