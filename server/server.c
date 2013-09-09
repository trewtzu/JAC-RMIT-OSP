/*
 * Lead Server for the JAC RaspberryPi Jukebox system
 *
 *
 *
 *
 * http://www.linuxhowtos.org/C_C++/socket.htm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void usage()
{
	fprintf(stdout,"Usage: server portNum");	
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portNum;
	socklen_t clilen;
	char buffer[256];

	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		usage()
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");


	//note: bzrror replaced with macro
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portNum = atoi(argv[1]);


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY  is this mechines address
	serv_addr.sin_port = htons(portNum); //note htons ensures the int is in network btye order
	//i dont trustthat cast
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
			&clilen);
	if (newsockfd < 0) 
		error("ERROR on accept");
	bzero(buffer,256);
	n = read(newsockfd,buffer,255);
	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: %s\n",buffer);
	n = write(newsockfd,"I got your message",18);
	if (n < 0) error("ERROR writing to socket");
	close(newsockfd);
	close(sockfd);
	return 0;
} 
