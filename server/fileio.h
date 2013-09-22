/* * * * * * * * * * * * * * * * *
 * Author:  Cory Mollison
 *          S3369723
 * Date:    September, 2013
 * * * * * * * * * * * * * * * * */

#ifndef FILEIO_H_
#define FILEIO_H_

#include "../shared/util.h"

/* Generate a text file containing a list of all valid
 * music files in the root music directory */
void
read_directory ();

/* Read in data from text file containing list of
 * available songs in root music directory */
char **
import_list ();

#endif
