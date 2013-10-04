#ifndef SCONT_H
#define SCONT_H
#include "../shared/util.h"
#include "../shared/protocol.h"

void print_logo();
void simpleDisplay(char** list, char** queue);
void begin(int simple, int sockfd);

#endif
