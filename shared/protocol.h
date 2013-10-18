/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'conner S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 * * * * * * * * * * * * * * * * * * * * * */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define BROADCAST_ADDR "127.255.255.255"
#define STREAM_PORT "8909"

#define PACKET_S 255

#define REQUEST_LISTS "101"
#define ADD_TO_QUEUE "102"

typedef struct
{
  int sock;
  int id;
} client_args_t;

#endif
