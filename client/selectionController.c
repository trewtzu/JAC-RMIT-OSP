#include "selectionController.h"
#include "../shared/queue.h"

void begin(int simple, int sockfd){

	int n;
		char buffer[PACKET_S+1];

		
	//NOTE: Simeple interface is blocking!
	if(simple==1){
		while(1){
			queue_t *queue;
			queue_t *listing; 
			queue = new_queue ();
			listing = new_queue ();

				n = write(sockfd,REQUEST_LISTS, PACKET_S);

				if (n < 0) 
					error("ERROR writing to socket");


				bzero(buffer,PACKET_S+1);

				printf("Track List:\n");
				while(1){

					n = read(sockfd,buffer,PACKET_S);
					if (n < 0) 
						error("ERROR read from socket");
					//TODO: this will need to be replaced by a const proto code
					if( strcmp(buffer,END_OF_TRACKS) == 0){
						printf("Done\n");
						break;
					}
					add_song(listing, buffer);
					printf("%s\n",buffer);
				}	

				printf("Queue List:\n");
				while(1){

					n = read(sockfd,buffer,PACKET_S);
					if (n < 0) 
						error("ERROR read from socket");
					//TODO: this will need to be replaced by a const proto code
					if( strcmp(buffer,END_OF_QUEUE) == 0){
						printf("Done\n");
						break;
					}
					add_song(queue, buffer);
					printf("%s\n",buffer);
				}




				//TODO: proto code here
				n = write(sockfd,"ack",PACKET_S);

				if (n < 0) 
					error("ERROR writing to socket");

				simpleDisplay( get_list( listing), listing->count, get_list( queue), queue->count);
				
				//TODO remove magic numbers
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
					
					
				
				//TODO this should be less memory shit
				delete_queue(queue);
				delete_queue(listing);
			
			}
		}
		
	}
	else{
		printf("not ready for non simple UI");
	}
	}

	void simpleDisplay(char** trackList, int trackList_s, char** trackQueue, int trackQueue_s){

		int i=0;
		int j=0;

		printf("|%32s || %s\n", "Queue","Songs");
		for(i=0; i<trackList_s; i++){
			if(i<trackQueue_s)
				printf("|%d: %-30s||",i+1,trackQueue[i]);
			else
				printf("|%-33s||"," ");
			printf("%d%30s|\n",i+1,trackList[i]);

		}
	}

