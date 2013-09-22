/* * * * * * * * * * * * * * * * *
 * Author:  Cory Mollison
 *          S3369723
 * Date:    September, 2013
 * General listing of genrealist 
 * helper functions used between
 * both server and client.
 * * * * * * * * * * * * * * * * */

#include "util.h"

void *
safe_malloc (size_t size)
{
  void *mem = NULL;

  if ((mem = (void *)malloc(size)) == NULL)
  {
    fprintf (stderr, "*Error during safe_malloc! Aborting..\n");
    exit (EXIT_FAILURE);
  }

  return (mem);
}

void *
safe_realloc (void * old_mem, size_t new_size)
{
  void *new_mem = NULL;

  if ((new_mem = (void *)realloc (old_mem, new_size)) == NULL)
  {
    fprintf (stderr, "*Error during safe_realloc! Aborting..\n");
    exit (EXIT_FAILURE);
  }

  return (new_mem);
}

char *
safe_strdup (const char *str)
{
  char *new_str = NULL;

  new_str = safe_malloc ((strlen(str) + 1) * sizeof (char));
  strcpy (new_str, str);

  return (new_str);
}

int
validate_str (char *str)
{
  if (strlen (str) <= 1)
    return (0);

  if (*(str + (strlen(str) - 1)) == '\n')
    *(str + (strlen(str) - 1)) = '\0';
  else
    return (0);
  
  return 1;
}
