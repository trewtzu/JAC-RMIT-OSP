#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define BROADCAST_ADDR "127.255.255.255"
#define STREAM_PORT "8909"

#define PACKET_S 255
#define END_OF_TRACKS "98"
#define END_OF_QUEUE "99"

#define REQUEST_LISTS "101"
#define ADD_TO_QUEUE "102"

typedef struct
{
  int sock;
  int id;
} client_args_t;

#endif
