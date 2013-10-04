/*
 * Example client,  for the JAC RaspberryPi Jukebox system
 *
 *
 *
 *
 * initial design heavy based on  http://www.linuxhowtos.org/C_C++/socket.htm
 */
#include "../shared/util.h"
#include "../shared/queue.h"
#include "../shared/protocol.h"
#include "../shared/wav.h"

void *beginPlayback(void *args);

/*
 * Simple error output, 'msg' should be the conext for the error
 */
void error(const char *msg)
{
	perror(msg);
	exit(0);
}


/*
 * note, this is a total fake population system for testing
 */
void fakepop(char*** trackList_ptr, char*** trackQueue_ptr){

	int total_tracks = 9;
	int queue_size =5;

	int MAX_TITLE_LEN = 64;

	char** trackList = *trackList_ptr ;
	char** trackQueue = *trackQueue_ptr;

	int i;
	for(i=0; i <total_tracks; i++){
		trackList[i] = malloc(MAX_TITLE_LEN * sizeof(char));
		bzero(trackList[i],sizeof(trackList[i]));
		sprintf(trackList[i],"song%d",i);
	}

	printf("%s\n",trackList[1]);



	for(i=0; i <queue_size; i++){
		trackQueue[i] = malloc(MAX_TITLE_LEN * sizeof(char));
		bzero(trackQueue[i],sizeof(trackQueue[i]));
		sprintf(trackQueue[i],"song%d",i);
	}

	printf("%s\n",trackQueue[1]);

	//    trackList_ptr = trackList;
	//    trackQueue_ptr = trackQueue;

}


int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	//begin(1);
	//	char* sL[] = {"song 1","song 2","song 3","song 4"};
	//	char* sQ[] = {"Qsong 1","Qsong 2","Qsong 3","Qsong 4"};
	//	simpleDisplay(sL,4,sQ,4);



	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	//grab the port
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	//grab the ip
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	//Prep the connection rules
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	//NOTE: this is deprecated. it will need to be replaced with a macro
	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);

	//and connect
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");

  pthread_t playbackThread;
  pthread_create(&playbackThread, NULL, beginPlayback, NULL); 

	/*
	 * At this point client behaviour starts
	 */
	printf("Starting client payload\n\n");

	begin(1, sockfd);



	sleep(10);

	close(sockfd);
	return 0;
}

void *beginPlayback(void *args)
{
  int sockfd, err, numBytes;
  struct addrinfo hints, *servInfo;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  err = getaddrinfo(NULL, STREAM_PORT, &hints, &servInfo);
  if (err != 0)
    error("ERROR getting server address info");

  sockfd = socket(servInfo->ai_family, servInfo->ai_socktype,
    servInfo->ai_protocol);
  if (sockfd == -1)
    error("ERROR creating socket");

  err = bind(sockfd, servInfo->ai_addr, servInfo->ai_addrlen);
  if (err == -1)
    error("ERROR binding socket");

  freeaddrinfo(servInfo);

  FormatChunk format;
  format.size = 16;
  format.compressionCode = 0;
  format.numChannels = 2;
  format.sampleRate = 44100;
  format.averageBPS = 0;
  format.blockAlign = 0;
  format.significantBPS = 16;

  snd_pcm_uframes_t periodFrames = 1024;
  snd_pcm_t *handle = initWavPlayback(&format, &periodFrames);

  uint frameSize = getFrameSize(&format);
  
  /* Initally buffer 10 periods of audio */
  int bufferSize = periodFrames * 10 * frameSize;
  char *buffer = safe_malloc(sizeof(char) * bufferSize);
  int packetSize = 1470;

  int byteCount = 0;
  while (byteCount + packetSize < bufferSize)
  {
    numBytes = recvfrom(sockfd, buffer + byteCount, packetSize, 0, NULL, NULL);
    if (numBytes == -1)
      error("ERROR receiving streaming data");
    else if (numBytes == 0)
      error("ERROR connection to server closed");

    byteCount += numBytes;
  }

  /* Buffer full, so write all the frames we have to sound device */
  int wholeFrames = byteCount / frameSize; /* Integer division */
  bufferWav(handle, buffer, wholeFrames);
  int leftoverBytes = byteCount - (wholeFrames * frameSize);
  
  /* Shift leftover bytes to start of buffer */
  int i;
  for (i = 0; i < leftoverBytes; ++i)
  {
    buffer[i] = buffer[wholeFrames * frameSize + i];
  }

  byteCount = leftoverBytes;
  while (1)
  {
    /* Buffer at least a period worth */
    while(byteCount < periodFrames * frameSize * 5)
    {
    numBytes = recvfrom(sockfd, buffer + byteCount, packetSize, 0, NULL, NULL);
    if (numBytes == -1)
      error("ERROR receiving streaming data");
    else if (numBytes == 0)
      error("ERROR connection to server closed");

    byteCount += numBytes;
    }

    wholeFrames = byteCount / frameSize;
    bufferWav(handle, buffer, wholeFrames);
    leftoverBytes = byteCount - (wholeFrames * frameSize);
    
    for (i = 0; i < leftoverBytes; ++i)
    {
      buffer[i] = buffer[wholeFrames * frameSize + i];
    }

    byteCount = leftoverBytes;
  }

  stopWavPlayback(handle);

  close(sockfd);
}
