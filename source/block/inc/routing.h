#ifndef ROUTING_H
# define ROUTING_H

# include <stdint.h>
# include <stm32f10x.h>

# include "olsr_2hop_neighbor_set.h"
# include "olsr_constants.h"
# include "olsr_duplicate_set.h"
# include "olsr_link_set.h"
# include "olsr_mpr_selector_set.h"
# include "olsr_mpr_set.h"
# include "olsr_neighbor_set.h"
# include "olsr_topology_set.h"
# include "olsr_types.h"
# include "routing.h"

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

typedef struct
{
  message_type_t type;
  /* This field indicates which type of message is to be found in
     the "MESSAGE" part.  Message types in the range of 0-127 are
     reserved for messages in this document and in possible
     extensions. */

  time_t vtime;
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

  time_t ttl;
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
typedef struct
{
  olsr_message_hdr_t header;
  packet_byte_t content[MAX_MESSAGE_CONTENT_SIZE];
  int content_size;
} olsr_message_t;

# define MAX_PACKET_MESSAGES 10
# define MAX_PACKET_CONTENT_SIZE (MAX_PACKET_MESSAGES * 1024)
typedef struct
{
  olsr_packet_hdr_t header;
  packet_byte_t content[MAX_MESSAGE_CONTENT_SIZE];
  int content_size;
} olsr_packet_t;

typedef struct
{
  uint16_t reserved;
  time_t htime;
  willingness_t willingness;
} olsr_hello_packet_hdr_t;

typedef struct
{
  uint8_t link_code;
  uint8_t reserved;
  uint16_t size;
} olsr_link_message_hdr_t;

typedef struct
{
  message_type_t types_implemented[MESSAGE_TYPES];
  address_t address;
  olsr_duplicate_set_t duplicate_set;
  olsr_link_set_t link_set;
  olsr_mpr_set_t mpr_set;
  olsr_mpr_selector_set_t mpr_selector_set;
  olsr_neighbor_set_t neighbor_set;
  olsr_2hop_neighbor_set_t twohop_neighbor_set;
  olsr_topology_set_t topology_set;
} olsr_node_state_t;

void olsr_init(address_t address);
void olsr_process_packet(packet_byte_t* packet, int length, interface_t iface);
void olsr_forward_message(packet_byte_t* message, int size, interface_t iface);
void olsr_process_message(packet_byte_t* message, int size, interface_t iface);
void olsr_dispatch_message(packet_byte_t* message, int size, interface_t iface);
void olsr_default_forward(packet_byte_t* message, int size, interface_t iface);
void send_message(olsr_message_hdr_t* header, packet_byte_t* content,
                  int content_size);
void send_task(void* pvParameters);

bool olsr_process_hello_message(packet_byte_t* message, int size, interface_t iface);
bool olsr_one_hop_symetric_neighbor(address_t addr);
bool olsr_is_mpr_selector(address_t addr);
long get_current_time();

#endif
