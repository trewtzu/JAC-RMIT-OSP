#ifndef SCONT_H
#define SCONT_H
#include "../shared/util.h"
#include "../shared/protocol.h"

void simpleDisplay(char** trackList, int trackList_s, char** trackQueue, int trackQueue_s);
void begin(int simple, int sockfd);
#endif
