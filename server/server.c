/*
 * Lead Server for the JAC RaspberryPi Jukebox system
 *
 *
 *
 *
 * Init design heavy based on  http://www.linuxhowtos.org/C_C++/socket.htm
 */


/*
 * TODO: need to add code to close socket before all mode of exit as i think it takes the network card a while to do it on its own.
 */


#include "../shared/util.h"
#include "../shared/queue.h"
#include "../shared/protocol.h"
#include "fileio.h"

queue_t *queue; 

void *
clientAction(int sock, int id);
void sendListings(int clisock, int id);

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
	 * Here we will prep our song listings and queue
	 */
	
	
	queue = new_queue (); //NOTE THIS IS THE GLOBEL SONG QUEUE
	read_directory();
	
	
	//NOTE: testing code
	char ** all_songs;
	all_songs = import_list ();
	int i;
	for (i = 0; all_songs[i] != NULL; i++)
		if(i%2==0)
			add_song (queue, all_songs[i]);
	

	/*
	 * Below is the threading loop, this needs to be modded to suit, but its pretty stright forward
	 * */

	
 	int thread_id=0;;
 	while(1)
	{
		printf("Preping thread %d...\n", thread_id);
		
		//i have no idea at all what cli_addr is used for
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen); //this is the core action of connection C to S
		if (newsockfd < 0) 
			error("ERROR on accept");
		

		printf("Detaching Thread...");
		pthread_t thread; //note, if we want to access the thread later we will need to store this somewhere
		pthread_create(&thread,NULL,&clientAction,newsockfd,thread_id);	 //this core action of starting a thread, arg 3 is the func name it will start on, all past that are args to pass to that func
		printf("Thread live\n");
		
		thread_id++;
 	}

	//close the socket
	close(sockfd);
	return 0;
}


/*
 * This is an action to be take by our thread, 
 */
void *clientAction(int clisock, int id){
	int n;
	printf("Starting thread %d\n",id);
	char buffer[PACKET_S+1];
	bzero(buffer,PACKET_S+1);
	
	
	do{
		n = read(clisock,buffer,PACKET_S);
		if (n < 0)
			error("ERROR reading from socket");
		
		switch(atoi(buffer))
		{
			case 101:
				printf("Start sending @ %d\n",id);
				sendListings(clisock, id);
				break;
		
			case 102:
				printf("%d requesting queue change\n",id);
				break;
			default:
				printf("error in the buff, i got %s\n",buffer);
				break;
		}
		
	}while(1);
	

	
	
	
// 	while(1){
// 		bzero(buffer,sizeof(buffer));
// 		
// 		
// 		
// 		n = read(clisock,buffer,255);
// 		
// 		if (n < 0)
// 			error("ERROR reading from socket");
// 		
// 		printf("Client %d:: %s\n",id,buffer);
// 		
// 		
// 
// 	}
	
	printf("%d ending", id);
	close(clisock);
}

void sendListings(int clisock, int id){

	int n;
	char buffer[PACKET_S+1];
	char** listing = import_list();
	char** q_songs = get_list(queue);
	
	
	int i;
	for (i = 0; listing[i] != NULL; i++)
	{
		printf ("Writing %s to %d\n", listing[i], id);
		n = write(clisock,listing[i],PACKET_S);
		if (n < 0)
			error("ERROR writing to socket");
	}	
	
	printf("Sending Track ending\n");
	n = write(clisock,END_OF_TRACKS,PACKET_S);
	if (n < 0)
		error("ERROR writing to socket");
	
	
	for (i = 0; q_songs[i] != NULL; i++)
	{
		printf ("Writing %s to %d\n", q_songs[i], id);
		n = write(clisock,q_songs[i],PACKET_S);
		if (n < 0)
			error("ERROR writing to socket");
	}	
	
	printf("Sending queue ending\n");
	n = write(clisock,END_OF_QUEUE,PACKET_S);
	if (n < 0)
		error("ERROR writing to socket");
	
	
	
	printf("waiting for reply\n");
	n = read(clisock,buffer,PACKET_S);
	if (n < 0)
		error("ERROR reading from socket");
		
	printf("Client %d:: %s\n",id,buffer);
	
}

