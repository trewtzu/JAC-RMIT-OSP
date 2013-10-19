/* * * * * * * * * * * * * * * * * * * * * *
 * Authors: Cory Mollison   S3369723
 *          Andrew O'Connor S3333717
 *          Josh Trew       S3237464
 *
 * Date:    October, 2013
 * * * * * * * * * * * * * * * * * * * * * */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define BROADCAST_ADDR "127.255.255.255"
#define BROADCAST_DELAY 955000
#define STREAM_PORT "8909"
#define STREAM_PACKET_S 1470

#define PACKET_S 255

#define REQUEST_LISTS "101"
#define ADD_TO_QUEUE "102"

typedef struct
{
  int sock;
  int id;
} client_args_t;

#endif
