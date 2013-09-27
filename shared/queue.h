/* * * * * * * * * * * * * * * * *
 * Author:  Cory Mollison
 *          S3369723
 * Date:    September, 2013
 * * * * * * * * * * * * * * * * */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "../shared/util.h"

typedef struct qnode qnode_t;

struct qnode
{
  char *song;     /* Filename of song (inc. extension) */
  qnode_t *next;  /* Pointer to next node in queue */
};

typedef struct
{
  qnode_t *head;          /* Pointer to the head of the queue */
  qnode_t *foot;          /* Pointer to the foot of the queue */
  int count;              /* Number of nodes in the queue */
  int size;               /* Size of the queue in bytes */
  pthread_mutex_t mutex;  /* Mutex for thread safety */
} queue_t;

/* Initialise an empty queue */
queue_t *
new_queue (void);

/* Free all memory related to a queue */
void
delete_queue (queue_t *queue);

/* Push new song to the end of the queue */
void
add_song (queue_t *queue, char *song);

/* Retrieve song at head of queue for playback */
char *
retrieve_song (queue_t *queue);

/* Retrieve a list of all songs in queue */
char **
get_list (queue_t *queue);

/* Print list of all songs in queue */
void
print_queue (queue_t *queue);

#endif
