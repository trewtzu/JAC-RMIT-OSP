/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'conner S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 * * * * * * * * * * * * * * * * * * * * * */

#include "selectionController.h"

void begin(int sockfd)
{
  char selection[PACKET_S+1];
  int sock;
  int choice, valid, result;
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
              list_request(sock);
              break;

        case 2:
              /* Add user selection to queue for playback */
              add_to_queue(sock);      
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

void list_request(int sockfd)
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
    n = read(sockfd, buffer, PACKET_S);
    if (n < 0) 
      error("ERROR read from socket");
    
    song_count = atoi(buffer);
        
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
    n = read(sockfd, buffer, PACKET_S);
    if (n < 0) 
      error("ERROR read from socket");
    
    q_count = atoi(buffer);
        
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

    list_print(allsongs, qsongs);

    /* Free both song lists */
    for (i = 0; allsongs[i] != NULL; i++)
      free (allsongs[i]);
    for (i = 0; qsongs[i] != NULL; i++)
      free (qsongs[i]);
    
    free (allsongs);
    free (qsongs);

    return;
}

void add_to_queue(int sockfd)
{	  
    int n, result;
    int song_choice;
    char buffer[PACKET_S+1];

    printf ("\nEnter number of song to add: ");
    fgets(buffer, PACKET_S, stdin);

    if (strlen(buffer) != 2 || !isdigit(buffer[0]))
    {
      printf("Invalid selection!\n");
      return;
    }
    
    /* Inform server that client will add to queue */
    n = write(sockfd, ADD_TO_QUEUE, PACKET_S);
    if (n < 0) 
      error("ERROR writing to socket"); 
    
    /* Give server the clients selection */
    n = write(sockfd, buffer, PACKET_S);
    if (n < 0)
      error("ERROR writing to socket");

    /* Server returns 0 for FAIL or 1 for SUCCESS */
    n = read(sockfd, buffer, PACKET_S);
    if (n < 0)
      error("ERROR reading from socket");
    
    result = atoi(buffer);

    if (!result)
      printf("Invalid selection!\n");
    else
      printf("Selection added to queue!\n");

    return;
}

void list_print(char** list, char** queue)
{

  int i=0;

  printf("\n|%-40s ||%-40s |\n", "Queue", "Songs");
  printf("|%-40s ||%-40s |\n", "-----", "-----");

  while (list[i] != NULL && queue[i] != NULL)
  {
    printf("|%d: %-38s||", i+1, queue[i]);
    printf("%d: %-38s|\n",i+1,list[i]);
    i++;
  }
  if (list[i] != NULL)
  {
    while (list[i] != NULL)
    {
      printf("|%-41s||"," ");
      printf("%d: %-38s|\n",i+1,list[i]);
      i++;
    }
  }
  else if (queue[i] != NULL)
  {
    while (queue[i] != NULL)
    {
      printf("|%d: %-38s||",i+1,queue[i]);
      printf("%-39s|\n"," ");
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
