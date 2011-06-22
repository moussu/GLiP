#ifndef OLSR_MESSAGE_H
# define OLSR_MESSAGE_H

# include "olsr_ifaces.h"
# include "olsr_types.h"
# include "olsr_time.h"

//18.4.  Message Types:
# define MESSAGE_TYPES 4
typedef enum
{
  HELLO_MESSAGE = 1,
  TC_MESSAGE    = 2,
  MID_MESSAGE   = 3,
  HNA_MESSAGE   = 4,
} message_type_t;

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
  message_type_t type;
  /* This field indicates which type of message is to be found in
     the "MESSAGE" part.  Message types in the range of 0-127 are
     reserved for messages in this document and in possible
     extensions. */

  time_serialized_t Vtime;
  /* This field indicates for how long time after reception a node
     MUST consider the information contained in the message as
     valid, unless a more recent update to the information is
     received.  The validity time is represented by its mantissa
     (four highest bits of Vtime field) and by its exponent (four
     lowest bits of Vtime field).  In other words:

     validity time = C*(1+a/16)* 2^b  [in seconds]

     where a is the integer represented by the four highest bits of
     Vtime field and b the integer represented by the four lowest
     bits of Vtime field.  The proposed value of the scaling factor
     C is specified in section 18.*/

  uint16_t size;
  /* This gives the size of this message, counted in bytes and
     measured from the beginning of the "Message Type" field and
     until the beginning of the next "Message Type" field (or - if
     there are no following messages - until the end of the packet). */

  address_t addr;
  /* This field contains the main address of the node, which has
     originally generated this message.  This field SHOULD NOT be
     confused with the source address from the IP header, which is
     changed each time to the address of the intermediate interface
     which is re-transmitting this message.  The Originator Address
     field MUST *NEVER* be changed in retransmissions. */

  ttl_t ttl;
  /* This field contains the maximum number of hops a message will
     be transmitted.  Before a message is retransmitted, the Time To
     Live MUST be decremented by 1.  When a node receives a message
     with a Time To Live equal to 0 or 1, the message MUST NOT be
     retransmitted under any circumstances.  Normally, a node would
     not receive a message with a TTL of zero.

     Thus, by setting this field, the originator of a message can
     limit the flooding radius. */

  uint8_t hops;
  /* This field contains the number of hops a message has attained.
     Before a message is retransmitted, the Hop Count MUST be
     incremented by 1.

     Initially, this is set to '0' by the originator of the message. */

  seq_num_t sn;
  /* While generating a message, the "originator" node will assign a
     unique identification number to each message.  This number is
     inserted into the Sequence Number field of the message.  The
     sequence number is increased by 1 (one) for each message
     originating from the node.  "Wrap-around" is handled as
     described in section 19.  Message sequence numbers are used to
     ensure that a given message is not retransmitted more than once
     by any node. */
} olsr_message_hdr_t;


# define MAX_MESSAGE_CONTENT_SIZE 1024
# define MAX_MESSAGE_SIZE                                       \
  (MAX_MESSAGE_CONTENT_SIZE + sizeof(olsr_message_hdr_t))

typedef struct
{
  olsr_message_hdr_t header;
  packet_byte_t content[MAX_MESSAGE_CONTENT_SIZE];
  int content_size;
} olsr_message_t;

void olsr_message_append(olsr_message_t* msg, void* data, int size_bytes);
void olsr_forward_message(packet_byte_t* message, int size, interface_t iface);
void olsr_process_message(packet_byte_t* message, int size, interface_t iface);
void olsr_dispatch_message(packet_byte_t* message, int size, interface_t iface);
void olsr_default_forward(packet_byte_t* message, int size, interface_t iface);

#endif
