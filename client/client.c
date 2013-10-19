/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'Connor S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 *
 * Initial design based on:
 * http://www.linuxhowtos.org/C_C++/socket.htm
 * * * * * * * * * * * * * * * * * * * * * */

#include "../shared/util.h"
#include "../shared/protocol.h"
#include "../shared/wav.h"

typedef struct
{
  unsigned char *data;
  uint capacity;
  uint count;
  bool full;
} AudioBuffer;

/* Function to initialize a new AudioBuffer struct */
AudioBuffer *createAudioBuffer(uint size);

/* Thread callbacks */
void *beginPlayback(void *args);
void *receive(void *args);

/*
 * Simple error output, 'msg' should be the conext for the error
 */
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

/* Global audio buffers */
AudioBuffer *buffer1 = NULL;
AudioBuffer *buffer2 = NULL;

/* Thread safety variables */
pthread_mutex_t buffer1Mutex;
pthread_mutex_t buffer2Mutex;
pthread_cond_t bufferCV;

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	/* grab the port */
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	/* grab the ip */
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	/* Prep the connection rules */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);

	/* and connect */
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");

	pthread_t playbackThread;
	pthread_create(&playbackThread, NULL, beginPlayback, NULL);
  
	/* At this point client behaviour starts */
	begin(sockfd);
  
	close(sockfd);
	return 0;
}

AudioBuffer *createAudioBuffer(uint size)
{
  AudioBuffer *buffer = NULL;

  buffer = safe_malloc(sizeof(AudioBuffer));
  buffer->data = safe_malloc(size);
  buffer->capacity = size;
  buffer->count = 0;
  buffer->full = 0;

  return buffer;
}

void *beginPlayback(void *args)
{
  /* Create a format chunk with our supported
   * wav format settings */
  FormatChunk format;
  format.size = 16;
  format.compressionCode = 0;
  format.numChannels = 2;
  format.sampleRate = 44100;
  format.averageBPS = 0;
  format.blockAlign = 0;
  format.significantBPS = 16;

  /* Create storage space for the size of a period
   * (initialize to anything, it will be overwritten anyway */
  snd_pcm_uframes_t periodFrames = 1024;

  /* Prepare the sound device for playback */
  snd_pcm_t *handle = initWavPlayback(&format, &periodFrames);

  /* Get the size of a single frame */
  uint frameSize = getFrameSize(&format);
  
  /* Create a buffer that can hold 150 periods of audio data */
  uint bufferSize = periodFrames * 150 * frameSize;

  /* Initialize the buffers */
  buffer1 = createAudioBuffer(bufferSize);
  buffer2 = createAudioBuffer(bufferSize);
  
  AudioBuffer *leftoverBuffer = createAudioBuffer(frameSize);

  /* Start the buffering thread */
  pthread_t receiveThread;
  pthread_create(&receiveThread, NULL, receive, NULL);

  /* Set the current buffer and mutex */
  AudioBuffer *currentBuffer = buffer1;
  pthread_mutex_t *currentMutex = &buffer1Mutex;

  uint byteCount = 0;
  while (1){
    uint wholeFrames = 0, diff = 0, i = 0;

    /* Lock the current buffer */
    pthread_mutex_lock(currentMutex);

    /* Wait until the buffer has been filled */
    while (!currentBuffer->full)
      pthread_cond_wait(&bufferCV, currentMutex);

    /* Calculate the number of whole frames of data that have been
     * received */
    wholeFrames = currentBuffer->count / frameSize; /* Integer division */

    /* Write those frames to the sound card */
    bufferWav(handle, currentBuffer->data + diff, wholeFrames);
    leftoverBuffer->count = currentBuffer->count - (wholeFrames * frameSize);

    /* Mark the buffer as being empty */
    currentBuffer->count = 0;
    currentBuffer->full = false;

    /* Release the lock on the buffer */
    pthread_mutex_unlock(currentMutex);

    /* Switch to the other buffer */
    if (currentBuffer == buffer1){
      currentBuffer = buffer2;
      currentMutex = &buffer2Mutex;
    }else{
      currentBuffer = buffer1;
      currentMutex = &buffer1Mutex;
    }
  }

  stopWavPlayback(handle);
}

void *receive(void *args)
{
  int sockfd, err, numBytes;
  struct addrinfo hints, *servInfo;

  /* Setup the socket */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  /* Get info about the server address */
  err = getaddrinfo(NULL, STREAM_PORT, &hints, &servInfo);
  if (err != 0)
    error("ERROR getting server address info");

  /* Create the socket */
  sockfd = socket(servInfo->ai_family, servInfo->ai_socktype,
    servInfo->ai_protocol);
  if (sockfd == -1)
    error("ERROR creating socket");

  /* Bind the server info to the socket */
  err = bind(sockfd, servInfo->ai_addr, servInfo->ai_addrlen);
  if (err == -1)
    error("ERROR binding socket");

  /* Server info is no longer needed */
  freeaddrinfo(servInfo);

  uint packetSize = STREAM_PACKET_S;

  /* Set the current buffer/mutex */
  AudioBuffer *currentBuffer = buffer1;
  pthread_mutex_t *currentMutex = &buffer1Mutex;

  /* Begin receiving data from the socket */
  uint byteCount = 0;
  while (1){
    uint wholeFrames = 0, i = 0;

    /* Lock the current buffer */
    pthread_mutex_lock(currentMutex);

    /* Fill the buffer */
    while(currentBuffer->count + packetSize < currentBuffer->capacity){
      int numBytes = 0;
      
      /* Get a packet from the socket */
      numBytes = recvfrom(sockfd, currentBuffer->data + currentBuffer->count, packetSize, 0, NULL, NULL);
      if (numBytes == -1)
        error("ERROR receiving streaming data");
      else if (numBytes == 0)
        error("ERROR connection to server closed");

      /* Update the data count for the buffer */
      currentBuffer->count += numBytes;
    }

    /* Mark the buffer as being full */
    currentBuffer->full = true;

    /* Let the playback thread know the buffer is ready */
    pthread_cond_signal(&bufferCV);

    /* Release the lock on the buffer */
    pthread_mutex_unlock(currentMutex);

    /* Switch to the other buffer */
    if (currentBuffer == buffer1){
      currentBuffer = buffer2;
      currentMutex = &buffer2Mutex;
    }else{
      currentBuffer = buffer1;
      currentMutex = &buffer1Mutex;
    }
  }

  close(sockfd);
}
