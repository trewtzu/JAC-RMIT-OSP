#include "selectionController.h"


void begin(int simple){
	//make a fake dataset for UI tests
	int total_tracks = 9;
	int queue_size =5;
	char** trackList = malloc(total_tracks*sizeof(char*));
	char** trackQueue = malloc(queue_size*sizeof(char*));
	fakepop(&trackList, &trackQueue);

	// printf("out-%s\n",trackList[1]);
	// printf("out-%s\n",trackQueue[1]);


	if(simple==1){
		while(1){
			simpleDisplay(trackList,total_tracks,trackQueue,queue_size);
			sleep(3);
		}
	}
	else{
		printf("not really for non simple UI");
	}
}

void simpleDisplay(char** trackList, int trackList_s, char** trackQueue, int trackQueue_s){

	int i=0;
	int j=0;

	printf("|%17s || %s\n", "Queue","Songs");
	for(i=0; i<trackList_s; i++){
		if(i<trackQueue_s)
			printf("|%d: %-15s||",i+1,trackQueue[i]);
		else
			printf("|%-18s||"," ");
		printf("%d%15s|\n",i+1,trackList[i]);

	}
}

