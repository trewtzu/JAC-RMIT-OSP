/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'Connor S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 * * * * * * * * * * * * * * * * * * * * * */

#ifndef UTIL_H_
#define UTIL_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#include <netdb.h> 

//a short macro to replace the depreciated bzerro
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define MAX_BUFF 100

/* Create a new block of memory with error checking.
 * Return value should be free'd by calling function.
 */
void *safe_malloc (size_t size);

/* Reallocate existing block of memory with error checking.
 * Return value should be free'd by calling function.
 */
void *safe_realloc (void *old_mem, size_t new_size);

/* Create a copy of an existing string with error checking.
 * Return value should be free'd by calling function.
 */
char *
safe_strdup (const char *str);

/* Validate and strip newline character from string. 
 * Returns '1' upon success, and '0' upon failure.
 */
int
validate_str (char *str);


#endif
