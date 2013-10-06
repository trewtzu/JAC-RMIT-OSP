#ifndef SCONT_H
#define SCONT_H
#include "../shared/util.h"
#include "../shared/protocol.h"

void begin(int sockfd);
int list_request(int sockfd);
void add_to_queue(int sockfd, int song_count);
void list_print(char** list, char** queue);
void print_menu();
void print_logo();

#endif
