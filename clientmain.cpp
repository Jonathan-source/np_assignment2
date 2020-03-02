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

int num_timeouts = 0;
bool isRunning = true;

void timeout_handler(const char * msg);

int main(int argc, char *argv[])
{
// VARIABLES  
//=====================================================================================================

  int sockfd;
  int portno = atoi(argv[1]);
  int byteRcvd = 0;
  int byteSent = 0;
  char *buffer[MAXLINE];

  bool job_send_calcMessage = true;
  bool job_calculate_calcProtocol = true;
  bool job_send_calcProtocol = true;

  sockaddr_in servAddr;
  socklen_t servLen = sizeof(servAddr);

  // type = 22, message = 0, protocol = 17, major_version = 1, minor_version = 0).
  calcMessage dataPacket {22,0,17,1,0};

	// Holder for calcMessage.
	calcProtocol * temp = (calcProtocol*)malloc(sizeof(struct calcProtocol));
 


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
  struct timeval alarmTime;
  alarmTime.tv_sec = 2;
  //Configure the timer to expire after 2 sec... 
  /*
  alarmTime.it_interval.tv_sec = 2;
  alarmTime.it_interval.tv_usec = 0;
  alarmTime.it_value.tv_sec = 2;
  alarmTime.it_value.tv_usec = 0;
  */

  setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &alarmTime, sizeof(alarmTime));
  setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, &alarmTime, sizeof(alarmTime));

  /* Regiter a callback function, associated with the SIGALRM signal, 
  which will be raised when the alarm goes of */
 // signal(SIGALRM, checkJobbList);
 // setitimer( ITIMER_REAL, &alarmTime, NULL );




// LOOP
//=====================================================================================================
//=====================================================================================================
  while(isRunning && num_timeouts < 3)
  {
    if(job_send_calcMessage)
    {
      // CALCMESSAGE
      //=====================================================================================================
      /**
        * Send calcMessage to the server. Check what is being received. If it's a calcMessage (16 bits), shutdown program.
        * Otherwise re-send the calcMessage every 2s if server does not reply. 
        * End the program after 3 timeouts.
        **/
      // Send calcMessage: 
      byteSent = sendto(sockfd, (const struct calcMessage *) &dataPacket, sizeof(dataPacket), 0, (const struct sockaddr*)&servAddr, sizeof(servAddr));
      printf("[x] calcMessage [%d bytes] was sent to the server.\n", byteSent);
      // Receive calcProtocol:
      byteRcvd = recvfrom(sockfd, temp, sizeof(*temp), 0, (struct sockaddr*) &servAddr, &servLen);
      if (byteRcvd < 0) 
      {
        if (errno == EWOULDBLOCK) 
        {
          num_timeouts++;
          fprintf(stderr, "socket timeout [%d].\n", num_timeouts);
          continue;
        } 
        else
            perror("recvfrom error");
      }
      else if (byteRcvd == 16){
        printf("[-] Server did not support the protocol.\n");
        return EXIT_FAILURE;
      }           
      printf("[x] calcProtocol [%d bytes] was received from the server:\n\n", byteRcvd);
      
      printf("[calcProtocol]\nArith:%d\nFloat1:%lf\nFloat2:%lf\nInt1:%d\nInt2:%d\n\n", 
        temp->arith, temp->flValue1, temp->flValue2, temp->inValue1, temp->inValue2);

      job_send_calcMessage = false;
    }

    if(job_calculate_calcProtocol)
    {
      switch(temp->arith)
      {
        case 0:
          printf("Error: calcProtocol-->arith = 0.");
          break;
        case 1: 
          temp->inResult =  temp->inValue1 + temp->inValue2;
          break;
        case 2:
          temp->inResult = temp->inValue1 - temp->inValue2;
          break;
        case 3:
          temp->inResult = temp->inValue1 * temp->inValue2;
          break;
        case 4:
          temp->inResult = temp->inValue1 / temp->inValue2;
          break;
        case 5:
          temp->flResult = temp->flValue1 + temp->flValue2;
          break;
        case 6:
          temp->flResult = temp->flValue1 - temp->flValue2;
          break;
        case 7:
          temp->flResult = temp->flValue1 * temp->flValue2;
          break;
        case 8:
          temp->flResult = temp->flValue1 / temp->flValue2;
        break;
      }

      if(temp->arith < 5)
      {
        printf("[x] Calculation complete: %d\n", temp->inResult);
      } else
        printf("[x] Calculation complete: %lf\n", temp->flResult); 

      job_calculate_calcProtocol = false;
    }

    if(job_send_calcProtocol)
    {
    // Send calcProtocol: 
    byteSent = sendto(sockfd, temp, sizeof(*temp), 0, (const struct sockaddr *) &servAddr, sizeof(servAddr));
    printf("[x] calcProtocol [%d bytes] was sent to the server.\n", byteSent);
    // Receive calcMessage:
    byteRcvd = recvfrom(sockfd, &dataPacket, sizeof(dataPacket), 0, (struct sockaddr*) &servAddr, &servLen);
      if (byteRcvd < 0) 
      {
        if (errno == EWOULDBLOCK) 
        {
          num_timeouts++;
          fprintf(stderr, "socket timeout [%d].\n", num_timeouts);
          continue;
        } 
        else
            perror("recvfrom error");
      }           
      printf("[x] calcMessage [%d bytes] was received from the server. calcMessage type = %d, message = %d.\n", byteRcvd, 
          dataPacket.type, dataPacket.message);

      job_send_calcProtocol = false;
    }
    // Everything is done.
    isRunning = false;
  }

  printf("Shutting down.\n");
  return 0;
}
