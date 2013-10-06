#ifndef SCONT_H
#define SCONT_H
#include "../shared/util.h"
#include "../shared/protocol.h"

void begin(int sockfd);
void list_request(int sockfd);
void list_print(char** list, char** queue);
void print_menu();
void print_logo();

#endif
