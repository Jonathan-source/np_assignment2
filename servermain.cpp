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

/* You will to add includes here */


// Included to get the support library
#include <calcLib.h>

#include "protocol.h"


using namespace std;
/* Needs to be global, to be rechable by callback and main */
int loopCount=0;
int terminate=0;


/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum)
{
  // As anybody can call the handler, its good coding to check the signal number that called it.
  printf("Let me be, I want to sleep.\n");

  if(loopCount > 20)
  {
    printf("I had enough.\n");
    terminate = 1;
  } 
  return;
}


void error(char const * msg)
{
	perror(msg);
	exit(0);
}



int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		error("error, arguments are missing");
	}
	unsigned int SERVER_PORT = atoi(argv[1]);

	int sockfd;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

	char sendBuff[256];
	char recvBuff[256];

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		error("could not create socket.");
	}
	printf("Socket created. Setting up server ...\n");


	// Socket address information needed for binding.
	memset(&serverAddr, 0, sizeof(serverAddr));  		// Zero out structure.
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);			// Convert to network byte order.
	serverAddr.sin_addr.s_addr = INADDR_ANY; 			// "0.0.0.0". 

	if(bind(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
	{
		error("could not bind socket to socketaddress.");
	}
	printf("Server setup complete. Waiting for message from remote clients ...\n");

  /* 
     Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  */
  struct itimerval alarmTime;
  alarmTime.it_interval.tv_sec = 10;
  alarmTime.it_interval.tv_usec = 10;
  alarmTime.it_value.tv_sec = 10;
  alarmTime.it_value.tv_usec = 10;

  /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL,&alarmTime,NULL); // Start/register the alarm. 


  // SERVER-LOOP
  while(terminate == 0)
  {

  	memset(recvBuff, 0, sizeof(recvBuff));
	memset(&clientAddr, 0, sizeof(clientAddr));
	unsigned int clientLen = sizeof(clientAddr);


	/** recvfrom(int s,void * buff, size_t len, int flags, (struct sockaddr*)from, (socklen_t *)fromlen) :
  	* Receive a message from the socket whether or not it's connection-oriented. 
  	* If "from" is nonzero, and the socket is connectionless, the source address of the message is filled in. 
  	* If no messages are available at the socket, the receive call waits for a message to arrive.
  	*/ 
  	if(recvfrom(sockfd, recvBuff, sizeof(recvBuff), 0, (struct sockaddr*) &clientAddr, &clientLen) < 0)
  	{
  		error("error receiving message from the client.");
  	}

  	// Client information.
  	char clientIP[256];
  	memset(clientIP, '\0', sizeof(clientIP));

	/** inet_ntop(int af, const void * src, char * dst, socklen_t size ) :
  	* Converts a numeric network address pointed to by src into a text string
  	* in the buffer pointed to by dst.
  	*/
  	if(inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)) == NULL)	// "Returns a pointer to the buffer containing
  	{																					// 	the text version of the address, or NULL if an error occurs"
  		error("inet_ntop failed.");
  	}
  	printf("Received a message from client %s:%d\n", clientIP, clientAddr.sin_port);




    printf("This is the main loop, %d time.\n",loopCount);
    sleep(1);
    loopCount++;
  }

  // Close socket
  close(sockfd);


  printf("done.\n");
  return(0);
}
