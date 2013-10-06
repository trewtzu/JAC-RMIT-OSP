#include "selectionController.h"

void begin(int sockfd)
{
  char selection[PACKET_S+1];
	int sock;
  int choice, valid;
  sock = sockfd;

  print_logo();
  
  while(1)
  {
    
    print_menu();
    
    do
    {
      valid = 1;
      
      printf ("Option: ");
      
      bzero(selection, PACKET_S+1);
      fgets(selection, PACKET_S, stdin);
      
      if (strlen(selection) != 2)
        valid = 0;
      else if (!isdigit(selection[0]))
        valid = 0;
    
    }while (!valid);

    choice = atoi(selection);
    
    switch (choice)
    {
      case 1:
            list_request(sock);
            break;
      case 2:
            /* Add to queue */
            break;
      default:
            printf("\nInvalid option..\n\n");
            break;
    }
  }    
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
    n = read(sockfd, &song_count, PACKET_S);
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
    n = read(sockfd, &q_count, PACKET_S);
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
}

void list_print(char** list, char** queue)
{

  int i=0;
  int j=0;

  printf("\n|%32s || %s%26s\n", "Queue", "Songs", "|");

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

  printf("\n");
}

void print_menu()
{
  printf ("\nMENU --------------------\n");
  printf ("1) Display song lists\n");
  printf ("2) Add to queue\n\n");
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
