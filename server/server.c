/*
 * Lead Server for the JAC RaspberryPi Jukebox system
 *
 *
 *
 *
 * initial design heavy based on  http://www.linuxhowtos.org/C_C++/socket.htm
 */


/*
 * NOTES: need to add code to close socket before all mode of exit as i think it takes the network card a while to do it on its own.
 */


#include "../shared/util.h"
void *clientAction(int sock, int j);

/*
 * Simple error output, 'msg' should be the conext for the error
 */
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

/*
 * simple usage disply
 */
void usage()
{
	fprintf(stdout,"Usage: server portNum");	
}


int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portNum;
	socklen_t clilen;

	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		usage();
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");


	//note: bzrror replaced with macro
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portNum = atoi(argv[1]);

	//prep sock settings
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY  is this mechines address
	serv_addr.sin_port = htons(portNum); //note htons ensures the int is in network btye order
	

	//put the socket in to action
	//
	//this needs to be checked over, i dont like that pointer-cast-address
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");

	if(listen(sockfd,5) == -1)
		error("ERROR on listen");


	printf("\nServer is starting on port: %d\n",portNum);
	
	/*
	 * Below is the threading loop, this needs to be modded to suit, but its pretty stright forward
	 * */

	int j =0;
	while(j<10)
	{
	
		j++;
		printf("in loop, @ %di\n", j);
		
		//i have no idea at all what cli_addr is used for
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen); //this is the core action of connection C to S
		if (newsockfd < 0) 
			error("ERROR on accept");
		

		printf("forking\n");
		pthread_t thread; //note, if we want to access the thread later we will need to store this somewhere
		pthread_create(&thread,NULL,&clientAction,newsockfd,j);	 //this core action of starting a thread, arg 3 is the func name it will start on, all past that are args to pass to that func
		printf("forked");
	}

	//close the socket
	close(sockfd);
	return 0;
}


/*
 * This is an example action to be take by our thread, 
 */
void *clientAction(int clisock, int id){
	int n;
	printf("starting action\n");
	char buffer[256];


	while(1){
		bzero(buffer,sizeof(buffer));
		n = read(clisock,buffer,255);
		
		if (n < 0)
			error("ERROR reading from socket");
		
		printf("Client %d:: %s\n",id,buffer);
		
		n = write(clisock,"Ack",18);
		if (n < 0)
			error("ERROR writing to socket");

	}
	
	printf("ending");
	close(clisock);
}
