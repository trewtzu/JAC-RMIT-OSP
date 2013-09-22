/* * * * * * * * * * * * * * * * *
 * Author:  Cory Mollison
 *          S3369723
 * Date:    September, 2013
 * * * * * * * * * * * * * * * * */

#include "fileio.h"

#define MAX_BUFF 100

/* Generate a text file containing a list of all valid
 * music files in the root music directory */
void
read_directory ()
{
  system ("ls -1 music/*{.mp3,.wav} >music/list.txt 2>>music/log.txt");
  system ("wc -l music/list.txt >music/count.txt 2>>music/log.txt");

  return; 
}

/* Read in data from text file containing list of
 * available songs in root music directory */
char **
import_list ()
{
  FILE *fp_count = NULL;
  FILE *fp_songs = NULL;
  char **song_list = NULL;
  char line[MAX_BUFF];
  char ch;
  int song_count, result; 
  int i = 0;

  if ((fp_count = fopen ("music/count.txt", "r")) == NULL)
    return (NULL);
  if ((fp_songs = fopen ("music/list.txt", "r")) == NULL)
    return (NULL);
  
  if ((result = fscanf (fp_count, "%d", &song_count)) != 1)
    return NULL;

  song_list = safe_malloc((song_count + 1) * sizeof(char *));
  
  while (fgets (line, sizeof (line), fp_songs) != NULL && i < song_count)
  {
    if (validate_str (line))
    {  
      song_list[i] = safe_strdup (line);
      i++;
    }
    else
    {
      while ((ch = fgetc (fp_songs)) != '\n' && (ch != EOF));
    }
  }
  song_list[i] = NULL;

  fclose (fp_count);
  fclose (fp_songs);

  return (song_list);
}
