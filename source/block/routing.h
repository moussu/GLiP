#ifndef ROUTING_H
# define ROUTING_H

typedef uint8_t  packet_byte_t;
typedef uint32_t address_t;
typedef uint8_t  time_t;
typedef uint16_t seq_num_t;
typedef void (*message_handler_t)(packet_byte_t* message, int size);

# define QUEUES_SIZE 32

/* C is a scaling factor for the "validity time" calculation ("Vtime"
   and "Htime" fields in message headers, see section 18.3).  The
   "validity time" advertisement is designed such that nodes in a
   network may have different and individually tuneable emission
   intervals, while still interoperate fully.  For protocol functioning
   and interoperability to work:

     -    the advertised holding time MUST always be greater than the
          refresh interval of the advertised information.  Moreover, it
          is recommended that the relation between the interval (from
          section 18.2), and the hold time is kept as specified
          in section 18.3, to allow for reasonable packet loss.

     -    the constant C SHOULD be set to the suggested value.  In order
          to achieve interoperability, C MUST be the same on all nodes.

     -    the emission intervals (section 18.2), along with the
          advertised holding times (subject to the above constraints)
          MAY be selected on a per node basis.

   Note that the timer resolution of a given implementation might not be
   sufficient to wake up the system on precise refresh times or on
   precise expire times: the implementation SHOULD round up the
   'validity time' ("Vtime" and "Htime" of packets) to compensate for
   coarser timer resolution, at least in the case where "validity time"
   could be shorter than the sum of emission interval and maximum
   expected timer error.*/

# define C_S                (1.0f/16.0f)

// 18.2.  Emission Intervals:
# define HELLO_INTERVAL_S   (2)
# define REFRESH_INTERVAL_S (2)
# define TC_INTERVAL_S      (5)
# define MID_INTERVAL_S     TC_INTERVAL_S
# define HNA_INTERVAL_S     TC_INTERVAL_S

// 18.3.  Holding Time:
# define NEIGHB_HOLD_TIME_S (3 * REFRESH_INTERVAL)
# define TOP_HOLD_TIME_S    (3 * TC_INTERVAL)
# define DUP_HOLD_TIME_S    (30)
# define MID_HOLD_TIME_S    (3 * MID_INTERVAL)
# define HNA_HOLD_TIME_S    (3 * HNA_INTERVAL)

// 18.9.  Misc. Constants:
# define TC_REDUNDANCY 0
# define MPR_COVERAGE  1
# define MAXJITTER_MS   (1000 * HELLO_INTERVAL_S / 4)

// 18.7.  Link Hysteresis:
# define HYST_THRESHOLD_HIGH 0.8f
# define HYST_THRESHOLD_LOW  0.3f
# define HYST_SCALING        0.5f

//18.4.  Message Types:
# define MESSAGE_TYPES 4

typedef enum
{
  HELLO_MESSAGE = 1,
  TC_MESSAGE    = 2,
  MID_MESSAGE   = 3,
  HNA_MESSAGE   = 4,
} message_type_t;

// 18.5.  Link Types:
typedef enum
{
  UNSPEC_LINK = 0,
  ASYM_LINK   = 1,
  SYM_LINK    = 2,
  LOST_LINK   = 3,
} link_type_t;

// 18.6.  Neighbor Types:
typedef enum
{
  NOT_NEIGH = 0,
  SYM_NEIGH = 1,
  MPR_NEIGH = 2,
} neighbor_type_t;

// 18.8.  Willingness:
typedef enum
{
  WILL_NEVER   = 0,
  WILL_LOW     = 1,
  WILL_DEFAULT = 3,
  WILL_HIGH    = 6,
  WILL_ALWAYS  = 7,
} willingness_t;

// Interfaces:
typedef enum
{
  NORTH_IFACE = 0,
  WEST_IFACE  = 1,
  SOUTH_IFACE = 2,
  EAST_IFACE  = 3,
} interface_t;

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

  address_t orig;
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

typedef struct
{
  address_t  addr;
  seq_num_t  sn;
  time_t     time;
  enum iface ifaces[4];
  uint8_t    n_ifaces;
  bool       retrans;
} olsr_duplicate_tuple_t;

// FIXME: value set randomly...
# define DUPLICATE_SET_MAX_SIZE 10
typedef struct
{
  olsr_duplicate_tuple_t tuples[DUPLICATE_SET_MAX_SIZE];
  uint8_t n_tuples;
} olsr_duplicate_set_t;

typedef struct
{
  message_type_t types_implemented[MESSAGE_TYPES];
  address_t address;
  olsr_duplicate_set_t duplicate_set;
} olsr_node_state_t;


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

void send_message(olsr_message_hdr_t* header, packet_byte_t* content,
                  int content_size);

#endif
