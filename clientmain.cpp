#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h> 		
#include <sys/socket.h>		 
#include <netinet/in.h>	

#include <unistd.h>
/* You will to add includes here */
// Included to get the support library
#include <calcLib.h>

#define MAXLINE 1024


#include "protocol.h"
void error(char const * msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{	
	// Check correct arguments.
	if(argc < 2)
	{
		error("error, arguments are missing.");
	}
	unsigned int SERVER_PORT = atoi(argv[1]);


	// Variables.
	int sockfd;
	struct sockaddr_in serverAddr;
	char * packet = "Greetings, from client.";
	char buffer[MAXLINE];

	// Creating socket file descriptor
  	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  	if(sockfd < 0)
	{
		error("could not create socket.");
	}
	printf("Socket was created, with file descriptor: %d\n", sockfd);


	memset(&serverAddr, '\0', sizeof(serverAddr));

	// Socket address information needed for binding.
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);		// Convert to network standard order.
	serverAddr.sin_addr.s_addr = INADDR_ANY;
  
	
	// Send message.
	sendto(sockfd, (const char *)packet, strlen(packet), 
		0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	
	printf("[%s] sent to server.\n", packet);


	socklen_t serverLen = sizeof(serverAddr);

	// Receive message. n = number of bits received.
	ssize_t n = recvfrom(sockfd, (char *)buffer, MAXLINE,
	 	MSG_WAITALL, (struct sockaddr *) &serverAddr, &serverLen); 
	if(n < 0)
	{
		error("error receiving messages.");
	}
    buffer[n] = '\0'; 
    printf("[Server]: %s\n", buffer); 


    // Close socket.
	close(sockfd);

	return 0;
}
