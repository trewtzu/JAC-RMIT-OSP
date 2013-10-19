/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'Connor S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 * * * * * * * * * * * * * * * * * * * * * */

#ifndef SCONT_H
#define SCONT_H
#include "../shared/util.h"
#include "../shared/protocol.h"

void begin(int sockfd);
void list_request(int sockfd);
void add_to_queue(int sockfd);
void list_print(char** list, char** queue);
void print_menu();
void print_logo();

#endif
