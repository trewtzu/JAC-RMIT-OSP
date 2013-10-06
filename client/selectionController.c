#include "selectionController.h"

void begin(int sockfd)
{
  char selection[PACKET_S+1];
	int sock;
  int choice, valid, result;
  int total_songs;
  sock = sockfd;

  print_logo();
  
  choice = 0;
  
  /* Loop for client UI menu options */
  do
  {
    print_menu();
    do
    {
      valid = 1;
      
      printf ("\nOption: ");
      
      bzero(selection, PACKET_S+1);
      fgets(selection, PACKET_S, stdin);
      
      if (strlen(selection) != 2)
        valid = 0;
      else if (!isdigit(selection[0]))
        valid = 0;
    

      choice = atoi(selection);
    
      switch (choice)
      {
        case 1:
              /* Display songs to user and store total count */
              total_songs = list_request(sock);
              break;

        case 2:
              /* Add user selection to queue for playback */
              add_to_queue(sock, total_songs);      
              break;

        case 9:
              /* Quit */
              choice = 9;
              break;

        default:
              valid = 0;
              break;
      }

    }while (!valid);
  }while (choice != 9);    
}

int list_request(int sockfd)
{
    char **allsongs;
    char **qsongs;
	  char buffer[PACKET_S+1];
    int song_count;
    int q_count; 
    int i, n;

    n = write(sockfd, REQUEST_LISTS, PACKET_S);

    if (n < 0) 
      error("ERROR writing to socket");

    bzero(buffer, PACKET_S+1);
        
    /* Get song count from server */
    n = read(sockfd, &song_count, sizeof(int));
    if (n < 0) 
      error("ERROR read from socket");
        
    allsongs = safe_malloc ((song_count + 1) * sizeof (char *));

    i = 0;
    while(i < song_count)
    {
      n = read(sockfd, buffer, PACKET_S);
      if (n < 0) 
        error("ERROR read from socket");

      allsongs[i] = safe_strdup (buffer);
      i++;
    }
    /* NULL terminate array */ 
    allsongs[i] = NULL;
        
    /* Get queue count from server */
    n = read(sockfd, &q_count, sizeof(int));
    if (n < 0) 
      error("ERROR read from socket");
        
    qsongs = safe_malloc ((q_count + 1) * sizeof (char *));
        
    i = 0;
    while(i < q_count)
    {
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

    list_print(allsongs, qsongs);

    free (allsongs);
    free (qsongs);

    return (song_count);
}

void add_to_queue(int sockfd, int song_count)
{	  
    int n, result;
    int song_choice;
    char selection[PACKET_S+1];

    printf ("\nEnter number of song to add: ");
    fgets(selection, PACKET_S, stdin);

    if (strlen(selection) != 2 || !isdigit(selection[0]))
    {
      printf("Invalid selection!\n");
      return;
    }

    song_choice = atoi(selection);
    
    n = write(sockfd, ADD_TO_QUEUE, PACKET_S);
    if (n < 0) 
      error("ERROR writing to socket"); 
    
    n = write(sockfd, &song_choice, sizeof(int));
    if (n < 0)
      error("ERROR writing to socket");

    n = read(sockfd, &result, sizeof(int));
    if (n < 0)
      error("ERROR reading from socket");
    
    if (!result)
      printf("Invalid selection!\n");
    else
      printf("Selection added to queue!\n");

    return;
}

void list_print(char** list, char** queue)
{

  int i=0;

  printf("\n|%32s || %s%26s\n", "Queue", "Songs", "|");

  while (list[i] != NULL && queue[i] != NULL)
  {
    printf("|%d: %-30s||", i+1, queue[i]);
    printf("%d%30s|\n",i+1,list[i]);
    i++;
  }
  if (list[i] != NULL)
  {
    while (list[i] != NULL)
    {
      printf("|%-33s||"," ");
      printf("%d%30s|\n",i+1,list[i]);
      i++;
    }
  }
  else if (queue[i] != NULL)
  {
    while (queue[i] != NULL)
    {
      printf("|%d: %-30s||",i+1,queue[i]);
      printf("%31s|\n"," ");
      i++;
    }
  }
  
  printf("\n");
}

void print_menu()
{
  printf ("\nMENU --------------------\n");
  printf ("1) Display song lists\n");
  printf ("2) Add to queue\n");
  printf ("9) Quit Virtual JukeBox\n");
}

void print_logo()
{
  printf ("                    ___        ___      ___________\n");
  printf ("                    \\  \\      /  /     |______    _|       \n");
  printf ("                     \\  \\    /  /    __ _    |  |    \n");
  printf ("                      \\  \\  /  /     |  |    |  |    \n");
  printf ("                       \\  \\/  /      \\  \\____/  /  \n");
  printf ("                        \\ __ / irtual \\_______ / ukebox    \n");
}
