/*
 * Lead Server for the JAC RaspberryPi Jukebox system
 *
 *
 *
 *
 * Init design heavy based on  http://www.linuxhowtos.org/C_C++/socket.htm
 */

/*
 * TODO: need to add code to close socket before all mode of 
 * exit as i think it takes the network card a while to do it on its own.
 */


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
 * simple usage disply
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
  list = new_queue ();  
  read_directory ();
	all_songs = import_list ();
	
  /* Populate complete song list */
  for (i = 0; all_songs[i] != NULL; i++)
    add_song (list, all_songs[i]);

  /* TEST Populate queue */
	for (i = 0; all_songs[i] != NULL; i++)
		if(i%2==0)
			add_song (queue, all_songs[i]);
	

	/*
	 * Below is the threading loop, this needs to be modded to suit, but its 
   * pretty stright forward
	 */

	
 	int thread_id=0;
  pthread_t streamThread;
 	while(1)
	{
		client_args_t *args = NULL;
    void *arguments; 
    printf("Preping thread %d...\n", thread_id);
    		
    args = safe_malloc (sizeof (client_args_t));

		//i have no idea at all what cli_addr is used for
		clilen = sizeof(cli_addr);
    //this is the core action of connection C to S
		args->sock = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);		
    if (args->sock < 0) 
			error("ERROR on accept");

		args->id = thread_id;

    arguments = args;

		printf("Detaching Thread...");
		// note, if we want to access the thread later 
    // we will need to store this somewhere
    pthread_t thread; 		
    // this core action of starting a thread, arg 3 is the func name 
    // it will start on, after that is the struct of args to pass to that func
    pthread_create(&thread,NULL,&clientAction,arguments);	 	
    
    printf("Thread live\n");

    // If this is the first connection, begin streaming
    if (thread_id == 0) {
      pthread_create(&streamThread, NULL, beginStreaming, NULL);
    }

		thread_id++;
 	}

	//close the socket
	close(sockfd);
	return 0;
}


/*
 * This is an action to be take by our thread, 
 */
void *clientAction(void *arguments){
	
  client_args_t *args = NULL;
  int n;
	char buffer[PACKET_S+1];
	bzero(buffer,PACKET_S+1);
	
  args = (client_args_t *) arguments;

	printf("Starting thread %d\n", args->id);

	do{
		n = read(args->sock,buffer,PACKET_S);
		if (n < 0)
			error("ERROR reading from socket");
		
		switch(atoi(buffer))
		{
			case 101:
				printf("Start sending @ %d\n",args->id);
				sendListings(args->sock, args->id);
				break;
		
			case 102:
				printf("%d requesting queue change\n",args->id);
				break;

			default:
				printf("error in the buff, i got %s\n",buffer);
				break;
		}
		
	}while(1);
	
	printf("%d ending", args->id);
	close(args->sock);
}

void *beginStreaming(void *args)
{
  int sockfd, err, bytesSent;
  struct addrinfo hints, *destInfo;
  int broadcast = 1; /* Tell socket to enable broadcasting */
  WavFile *wav;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; /* Force IPv4 */
  hints.ai_socktype = SOCK_DGRAM;

  err = getaddrinfo(BROADCAST_ADDR, STREAM_PORT, &hints, &destInfo);
  if (err != 0)
    error("ERROR getting stream address info");

  sockfd = socket(destInfo->ai_family, destInfo->ai_socktype,
    destInfo->ai_protocol);
  if (sockfd == -1)
    error("ERROR creating socket");

  err = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
    sizeof broadcast);
  if (err == -1)
    error("ERROR setting sock opt");

  /* Begin streaming songs from the queue */
  while (1)
  {
    char *songName = retrieve_song(queue);
    printf("Loading song: %s\n", songName);

    if (songName == NULL)
      error("ERROR queue is empty");

    wav = createWav(songName);
    if (!loadWavData(wav))
      error("ERROR loading wave file");

    /* Send data at approximately real-time */
    int packetSize = 1470;
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

      usleep((1.0 / pps) * 900000);
    }

    printf("Song finished\n");
    deleteWav(wav);
    free(songName);
  }

  freeaddrinfo(destInfo);
  close(sockfd);
}

void sendListings(int clisock, int id){

	int n, i;
	char buffer[PACKET_S+1];
	
  char** all_songs = get_list(list);
  char** q_songs = get_list(queue);
	
  /* Write song count to client */
  n = write(clisock, &list->count, PACKET_S);

  /* Write each song title to client */
	for (i = 0; all_songs[i] != NULL; i++)
	{
		n = write(clisock,all_songs[i],PACKET_S);
		if (n < 0)
			error("ERROR writing to socket");
	}	
	
	/* Write queue count to client */
  n = write(clisock, &queue->count, PACKET_S);

  /* Write each song title to client */
	for (i = 0; q_songs[i] != NULL; i++)
	{
		n = write(clisock,q_songs[i],PACKET_S);
		if (n < 0)
			error("ERROR writing to socket");
	}	
	
	printf("waiting for reply\n");
	n = read(clisock,buffer,PACKET_S);
	if (n < 0)
		error("ERROR reading from socket");
		
	printf("Client %d:: %s\n",id,buffer);
	
}

