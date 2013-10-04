/* * * * * * * * * * * * * * * *
 * Author:  Cory Mollison
 *          S3369723
 * Date:    September, 2013
 * * * * * * * * * * * * * * * */

#include "queue.h"

static void
free_data (qnode_t *node)
{
  free (node->song);
  free (node);
}

static void
insert_node (queue_t *queue, qnode_t *node)
{
  assert (queue);
  
  node->next = NULL;

  if (queue->count == 0)
    queue->head = node;
  else
    queue->foot->next = node;

  queue->foot = node;

  /* Increase queue size for new node */
  queue->size += sizeof (*node);
  queue->size += ((strlen(node->song) + 1) * sizeof (char));
  
  /* Update number of items in queue */
  queue->count++;
  
  return;
}

static void
remove_node (queue_t *queue, qnode_t *node)
{
  qnode_t *tmp = NULL;

  assert (queue);

  tmp = queue->head;
  
  if (queue->count > 1)
  {
    queue->head = queue->head->next;
  }
  else
  {
    queue->head = NULL;
    queue->foot = NULL;
  }

  /* Decrease queue size for node being removed */
  queue->size -= sizeof (*tmp);
  queue->size -= ((strlen(tmp->song) + 1) * sizeof (char));

  /* Update number of items in queue */
  queue->count--;

  free_data (tmp);

  return;
}

queue_t *
new_queue ()
{
  queue_t *new_queue = NULL;

  new_queue = safe_malloc (sizeof (queue_t));

  new_queue->head = NULL;
  new_queue->foot = NULL;
  new_queue->count = 0;
  new_queue->size = sizeof (*new_queue);
  pthread_mutex_init (&(new_queue->mutex), NULL);

  return (new_queue);
}

void
delete_queue (queue_t *queue)
{
  qnode_t *node = NULL;
  qnode_t *tmp = NULL;

  node = queue->head;

  while (node != NULL)
  {
    tmp = node;
    node = node->next;
    free_data (tmp);
  }

  free (queue);

 return; 
}

void
add_song (queue_t *queue, char *song)
{
  qnode_t *new_node = NULL;
  char *new_song = NULL;

  assert (queue);
  assert (song);

  new_node = safe_malloc (sizeof (qnode_t));
  new_song = safe_strdup (song);
  
  new_node->song = new_song;
  
  pthread_mutex_lock (&(queue->mutex));
  insert_node (queue, new_node);
  pthread_mutex_unlock (&(queue->mutex));

  return;
}

char *
retrieve_song (queue_t *queue)
{
  char *next_song = NULL;

  assert (queue);

  if (queue->count == 0)
    return (NULL);

  next_song = safe_strdup (queue->head->song);
 
  pthread_mutex_lock (&(queue->mutex));
  remove_node (queue, queue->head);
  pthread_mutex_unlock (&(queue->mutex));

  return (next_song);
}

char**
get_list (queue_t *queue)
{
  char **list = NULL;
  qnode_t *node = NULL;
  int i = 0;

  /* Add 1 extra pointer space to NULL terminate array */
  list = safe_malloc ((queue->count + 1) * sizeof (char *));
  node = queue->head;

  while (node != NULL)
  {
    list[i] = safe_strdup (node->song);
    node = node->next;
    i++;
  }
  /* NULL terminate array */
  list[i] = NULL;

  return (list);
}

void
print_queue (queue_t *queue)
{
  qnode_t *node = NULL;
  int i;

  node = queue->head;

  for (i = 1; node != NULL; i++)
  {
    printf ("Track %d: %s\n", i, node->song);
    node = node->next;
  }
  
  printf ("\n");

  return;
}
