/*
 * Example client,  for the JAC RaspberryPi Jukebox system
 *
 *
 *
 *
 * initial design heavy based on  http://www.linuxhowtos.org/C_C++/socket.htm
 */
#include "../shared/util.h"

/*
 * Simple error output, 'msg' should be the conext for the error
 */
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
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
    //NOTE: this is deprecated. ill get rid of it when i put a real header in and replace it with a macro
	bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
	
	//and connect
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
	/*
	 * At this point client behaviour starts
	 */
	
	printf("Starting client payload");
	int i=0;
	while(i<20){
		//printf("Please enter the message: ");
		//bzero(buffer,256);
		//fgets(buffer,255,stdin);
	
		sprintf(buffer, "%d", i);
		n = write(sockfd,buffer,strlen(buffer));
	
		if (n < 0) 
         error("ERROR writing to socket");
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) 
			error("ERROR reading from socket");
    printf("%s\n",buffer);
	
	sleep(3);
	i++;	
	
	}
	
	close(sockfd);
    return 0;
}