#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h> 		
#include <sys/socket.h>		 
#include <netinet/in.h>	
/* You will to add includes here */


// Included to get the support library
#include <calcLib.h>


#include "protocol.h"
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

	char sendBuff[256];
	char recvBuff[256];

	// Clear buffers.
	memset(sendBuff, '\0', sizeof(sendBuff));
	memset(recvBuff, '\0', sizeof(recvBuff));

	unsigned int serverLen = sizeof(serverAddr);

	// Socket address information needed for binding.
	memset(&serverAddr, '\0', serverLen);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);		// Convert to network standard order.

	inet_pton(AF_INET, "127.0.0.1", &serverAddr);
  
  	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		error("could not create socket.");
	}
	printf("Socket created.\n");

	struct sData{
		int x = 5;
	}; sData myData;

	unsigned int dataLen = sizeof(myData);

	if(sendto(sockfd, &myData, dataLen, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		error("error with sendto()");
	}

	return 0;
}
