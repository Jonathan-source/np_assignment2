#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
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

int loopCount = 0;
int num_timeouts = 0;
bool isRunning = true;


/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum)
{ 
  if(loopCount == 2){
    isRunning = false;
    return;
  } 
  loopCount++;
  return;   /* An interrupter */
}

void timeout_handler(const char * msg);

int main(int argc, char *argv[])
{
// VARIABLES  
//=====================================================================================================

  int currentState = 1;

  int sockfd;
  int portno = atoi(argv[1]);
  int byteRcvd = 0;
  int byteSent = 0;
  char *buffer[MAXLINE];

  sockaddr_in servAddr;
  socklen_t servLen = sizeof(servAddr);

  // type=22, message=0, protocol=17, major_version=1,minor_version=0).
  calcMessage dataPacket {22,0,17,1,0};

	// Holder for calcMessage.
	calcProtocol * temp = (calcProtocol*)malloc(sizeof(struct calcProtocol));
  memset(&temp, 0, sizeof(temp));


// UDP SOCKET
//=====================================================================================================

  /* Create an UDP socket. */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) 
  {
	  perror("cannot create socket");
  }
  printf("[+] Socket was created.\n");

  /* Fill in the server's address and data. */
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(portno);



// TIME MANAGEMENT.
//=====================================================================================================

  /* 
  Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted,
  it will be a single alarm 10s after it has been set. 
  */
  struct itimerval alarmTime;
  /* Configure the timer to expire after 2 sec... */
  alarmTime.it_interval.tv_sec = 2;
  alarmTime.it_interval.tv_usec = 0;
  alarmTime.it_value.tv_sec = 2;
  alarmTime.it_value.tv_usec = 0;

  setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&alarmTime, sizeof(alarmTime));
  setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&alarmTime, sizeof(alarmTime));

  /* Regiter a callback function, associated with the SIGALRM signal, 
  which will be raised when the alarm goes of */
 // signal(SIGALRM, checkJobbList);
 // setitimer( ITIMER_REAL, &alarmTime, NULL );




// LOOP
//=====================================================================================================
//=====================================================================================================
  while(isRunning)
  {

    // CALCMESSAGE
    //=====================================================================================================

    /* Send calcMessage to the server. */
    byteSent = sendto(sockfd, (const struct calcMessage *) &dataPacket, sizeof(dataPacket), 0, (const struct sockaddr*)&servAddr, sizeof(servAddr));
    if (byteSent < 0) 
    {
      perror("sendto() sent a different number of bytes than expected.");
    } else            
      printf("Packet [%d bytes] was sent to the server.\n", byteSent);
    

    /* Receive from server. */
    byteRcvd = recvfrom(sockfd, temp, sizeof(*temp), 0, (struct sockaddr*) &servAddr, &servLen);
	  if(byteRcvd < 0)
    {
      timeout_handler("Resending segment.\n");
    } 
    else {
      printf("Packet received.\n");

    isRunning = false;
    }
    
    // Send calcMessage.
    //=====================================================================================================














  }

  printf("Shutting down.\n");
  return 0;
}


void timeout_handler(const char * msg)
{
  if(num_timeouts > 2)
  {
    printf("No respons from the server after %d attempts resending the segment.\n", num_timeouts);
    isRunning = false;
  }
  else {
  // Timeout reached
    printf("Timout reached. %s\n", msg);
    num_timeouts++;
  }
}