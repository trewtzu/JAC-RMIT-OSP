/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'Connor S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 *
 * Init design based on:  
 * http://www.linuxhowtos.org/C_C++/socket.htm
 * * * * * * * * * * * * * * * * * * * * * */ 


#include "../shared/util.h"
#include "queue.h"
#include "../shared/protocol.h"
#include "../shared/wav.h"
#include "fileio.h"

queue_t *queue;
queue_t *list;

void *clientAction(void *args);
void *beginStreaming(void *args);
void sendListings(int clisock, int id);

/*
 * Simple error output, 'msg' should be the context for the error
 */
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

/*
 * simple usage display
 */
void usage()
{
	fprintf(stdout,"Usage: server portNum\n");	
}


int main(int argc, char *argv[])
{
	char **all_songs;
	int sockfd, newsockfd, portNum;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n, i;

	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		usage();
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portNum = atoi(argv[1]);

	/* Prep sock settings */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY  is this machines address
	serv_addr.sin_port = htons(portNum); //note htons ensures the int is in network byte order
	
	//binds a socket for us, and starts its up
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");

	if(listen(sockfd,5) == -1)
		error("ERROR on listen");


	printf("\nServer is starting on port: %d\n",portNum);
	
	/* Prepare list for all songs */
	list = new_queue ();  
	/* Prepare list for queued songs */ 
	queue = new_queue ();

	/* Scan directory and gather filenames */
	read_directory ();
	all_songs = import_list ();
	
	/* Populate complete song list */
	for (i = 0; all_songs[i] != NULL; i++)
    add_song (list, all_songs[i]);

  /* Populate queue with one song to start */
  auto_queue (queue, list);
	
 	/* Begin loop to handle connections 
   * between server and client
   */
  int thread_id=0;
  pthread_t streamThread;
	while(1){
		client_args_t *args = NULL;
		printf("Preparing thread %d\n", thread_id);
    		
		args = safe_malloc (sizeof (client_args_t));

		clilen = sizeof(cli_addr);
	
		/* Connect client to server */
		args->sock = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);		
		if (args->sock < 0) 
			error("ERROR on accept");

		args->id = thread_id;

		pthread_t thread; 		
    
		/* Begin the thread.
		* Arg 3 is function name where thread begins execution.
		* Arg 4 is a pointer to a struct of parameters for the function.
		*/
		pthread_create(&thread,NULL,&clientAction,(void *)args);	 	
    
		printf("Thread %d live\n", thread_id);

		/* If this is the first client, begin streaming */
		if (thread_id == 0) 
		{
			pthread_create(&streamThread, NULL, beginStreaming, NULL);
		}

		thread_id++;
	}

	close(sockfd);
	return 0;
}


/*
 * This thread handles core communication from the client
 */
void *clientAction(void *arguments){
	
	client_args_t *args = NULL;
	int n, choice, add_song, result;
	char *message;
	char buffer[PACKET_S+1];
	
	bzero(buffer,PACKET_S+1);
	
	args = (client_args_t *) arguments;

	do{
	/* Wait for client communication */
	n = read(args->sock, buffer, PACKET_S);
	/* Check for error or closed socket */
	if (n < 0)
		error("ERROR reading from socket");
		else if (n == 0)
		break;
    
		choice = atoi(buffer);
    
		if (choice == 101){
			/* Pass queue information to client */
			sendListings(args->sock, args->id);
		}
		else if (choice == 102){
			/* Receive client selection */
			n = read(args->sock, buffer, PACKET_S);
			if (n < 0)
				error("ERROR reading from socket");
      
			add_song = atoi(buffer);

			/* Add client selection to queue */
			result = queue_song(queue, list, add_song);
      
			sprintf(buffer, "%d", result);
			n = write(args->sock, buffer, PACKET_S);
			if (n < 0)
				error("ERROR writing to socket");  
		}
	} while(1);
	
	close(args->sock);
}


/*
 *Thread to control audio streaming to broadcast
 */
void *beginStreaming(void *args)
{
  int sockfd, err, bytesSent;
  struct addrinfo hints, *destInfo;
  int broadcast = 1; /* Tell socket to enable broadcasting */
  WavFile *wav = NULL;

  /* Setup the socket */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; /* Force IPv4 */
  hints.ai_socktype = SOCK_DGRAM;

  /* Get information about the destination address */
  err = getaddrinfo(BROADCAST_ADDR, STREAM_PORT, &hints, &destInfo);
  if (err != 0)
    error("ERROR getting stream address info");

  /* Create the socket */
  sockfd = socket(destInfo->ai_family, destInfo->ai_socktype,
    destInfo->ai_protocol);
  if (sockfd == -1)
    error("ERROR creating socket");

  /* Make the socket a broadcast socket */
  err = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
    sizeof broadcast);
  if (err == -1)
    error("ERROR setting sock opt");

  /* Begin streaming songs from the queue */
  while (1)
  {
    /* Get next song from the queue */
    char *songName = retrieve_song(queue, list);

    if (songName == NULL)
      error("ERROR queue is empty");

    /* Create a new WAV file and load in the data */
    wav = createWav(songName);
    if (!loadWavData(wav))
      error("ERROR loading wave file");

    /* Send data at approximately real-time */
    int packetSize = STREAM_PACKET_SIZE;
    uint frameSize = getFrameSize(wav->format);
    uint bps = wav->format->sampleRate * frameSize;
    uint pps = bps / packetSize;
    
    int totalBytes = 0;
    int bytesLeft = wav->data->size;
    while (bytesLeft > 0)
    {
      int availBytes = packetSize;
      if (bytesLeft < availBytes)
        availBytes = bytesLeft;

      bytesSent = sendto(sockfd, wav->data->data + totalBytes, packetSize, 0,
          destInfo->ai_addr, destInfo->ai_addrlen);
      if (bytesSent == -1)
      {
        error("ERROR sending stream data");
      }

      totalBytes += bytesSent;
      bytesLeft -= bytesSent;

      usleep((1.0 / pps) * 955000);
    }

    update_queue(queue);
    deleteWav(wav);
    free(songName);
  }

  freeaddrinfo(destInfo);
  close(sockfd);
}

/* 
 * Send song listing to client
 */
void sendListings(int clisock, int id){

	int n, i;
	char buffer[PACKET_S+1];
	
	char** all_songs = get_list(list);
	char** q_songs = get_list(queue);
	
	/* Write song count to client */
	sprintf(buffer, "%d", list->count);
  
	n = write(clisock, buffer, PACKET_S);
	if (n < 0)
		error("ERROR writing to socket");
  
	/* Write each song title to client */
	for (i = 0; all_songs[i] != NULL; i++){
		n = write(clisock,all_songs[i],PACKET_S);
		if (n < 0)
			error("ERROR writing to socket");
	  usleep(10000);
	}

	/* Write queue count to client */
	sprintf(buffer, "%d", queue->count);

	n = write(clisock, buffer, PACKET_S);
	if (n < 0)
		error("ERROR writing to socket");

	/* Write each song title to client */
	for (i = 0; q_songs[i] != NULL; i++){
		n = write(clisock,q_songs[i],PACKET_S);
		if (n < 0)
			error("ERROR writing to socket");
	  usleep(10000);
	}
  
	/* Free both song lists */
	for (i = 0; all_songs[i] != NULL; i++)
		free (all_songs[i]);
	for (i = 0; q_songs[i] != NULL; i++)
		free (q_songs[i]);
  
	free (all_songs);
	free (q_songs);

	return;
}
