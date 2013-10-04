#include "selectionController.h"
#include "../shared/queue.h"

void begin(int simple, int sockfd){

	int n;
	char buffer[PACKET_S+1];
		
	//NOTE: Simeple interface is blocking!
	if(simple==1){
		while(1){
  
    char **allsongs;
    char **qsongs;
    int song_count;
    int q_count; 
    int i;

				n = write(sockfd, REQUEST_LISTS, PACKET_S);

				if (n < 0) 
					error("ERROR writing to socket");

				bzero(buffer, PACKET_S+1);
        
        /* Get song count from server */
        n = read(sockfd, &song_count, PACKET_S);
			  if (n < 0) 
					error("ERROR read from socket");
        
        allsongs = safe_malloc ((song_count + 1) * sizeof (char *));

				i = 0;
        while(i < song_count){

					n = read(sockfd, buffer, PACKET_S);
					if (n < 0) 
						error("ERROR read from socket");
				  
          allsongs[i] = safe_strdup (buffer);
          i++;
        }
        /* NULL terminate array */ 
        allsongs[i] = NULL;
        
        /* Get queue count from server */
        n = read(sockfd, &q_count, PACKET_S);
			  if (n < 0) 
					error("ERROR read from socket");
        
        qsongs = safe_malloc ((q_count + 1) * sizeof (char *));
        
        i = 0;

				while(i < q_count){

					n = read(sockfd,buffer,PACKET_S);
					if (n < 0) 
						error("ERROR read from socket");
				  
          qsongs[i] = safe_strdup (buffer);
          i++;
        }
        /* NULL terminate array */
        qsongs[i] = NULL;

				n = write(sockfd,"ack",PACKET_S);

				if (n < 0) 
					error("ERROR writing to socket");

				simpleDisplay(allsongs, qsongs);
			  
        /* Manage user interaction / menu here */
        bzero(buffer, PACKET_S+1);
				fgets(buffer, PACKET_S, stdin);
				if(buffer == NULL || buffer[0] =='\n') {
					printf("nul buf");
					//sleep(5);
				}else{
					printf("non nul buf");
					n = write(sockfd,ADD_TO_QUEUE,PACKET_S);
					if (n < 0) 
						error("ERROR writing to socket");
        }
      free (allsongs);
      free (qsongs);
      
      }
		
	  }
	  else
    {
		  printf("not ready for non simple UI");
	  }
  }

	void simpleDisplay(char** list, char** queue){

		int i=0;
    int j=0;

    print_logo ();

		printf("|%32s || %s%26s\n", "Queue", "Songs", "|");
		
    while (list[i] != NULL)
    {
			if(queue[j] != NULL)
      {
				printf("|%d: %-30s||",j+1,queue[j]);
			  j++;
      }
      else
				printf("|%-33s||"," ");

			printf("%d%30s|\n",i+1,list[i]);
      i++;
		}
	}

  void print_logo()
  {
    printf ("                    ___        ___      ___________\n");
    printf ("                    \\  \\      /  /     |______    _|       \n");
    printf ("                     \\  \\    /  /    __ _    |  |    \n");
    printf ("                      \\  \\  /  /     |  |    |  |    \n");
    printf ("                       \\  \\/  /      \\  \\____/  /  \n");
    printf ("                        \\ __ / irtual \\_______ / ukebox    \n");
    printf ("===================================================================\n\n");
  }









