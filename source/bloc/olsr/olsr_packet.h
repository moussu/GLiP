#ifndef OLSR_PACKET_H
# define OLSR_PACKET_H

# include <stdint.h>
# include "mtu.h"
# include "olsr_ifaces.h"
# include "olsr_types.h"

/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |         Packet Length         |    Packet Sequence Number     |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Message Type |     Vtime     |         Message Size          |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                      Originator Address                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Time To Live |   Hop Count   |    Message Sequence Number    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      :                            MESSAGE                            :
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Message Type |     Vtime     |         Message Size          |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                      Originator Address                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Time To Live |   Hop Count   |    Message Sequence Number    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      :                            MESSAGE                            :
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      :                                                               :
               (etc.)
 */

typedef struct
{
  uint16_t length;
  /* The length (in bytes) of the packet. */

  seq_num_t sn;
  /* The Packet Sequence Number (PSN) MUST be incremented by one
     each time a new OLSR packet is transmitted.  "Wrap-around" is
     handled as described in section 19.  A separate Packet Sequence
     Number is maintained for each interface such that packets
     transmitted over an interface are sequentially enumerated. */
} olsr_packet_hdr_t;

# define MAX_PACKET_SIZE MTU
# define MAX_PACKET_CONTENT_SIZE                \
  (MAX_PACKET_SIZE - sizeof(olsr_packet_hdr_t))
# define MAX_PACKET_MESSAGES 10

typedef struct
{
  olsr_packet_hdr_t header;
  packet_byte_t content[MAX_PACKET_CONTENT_SIZE];

  // /!\ //
  // Should remain after the content fo olsr_packet_t to be copied directly!
  // /!\ //

  int content_size;
} olsr_packet_t;

void olsr_process_packet(olsr_packet_t* packet, interface_t iface);

#endif
