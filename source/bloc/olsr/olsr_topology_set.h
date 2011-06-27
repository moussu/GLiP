#ifndef OLSR_TOPOLOGY_SET_H
# define OLSR_TOPOLOGY_SET_H

# include "olsr_ifaces.h"
# include "olsr_message.h"
# include "olsr_set.h"
# include "olsr_time.h"
# include "olsr_types.h"

/*
   The proposed format of a TC message is as follows:

       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |              ANSN             |           Reserved            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               Advertised Neighbor Main Address                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |               Advertised Neighbor Main Address                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct
{
/*
     Advertised Neighbor Sequence Number (ANSN)

          A sequence number is associated with the advertised neighbor
          set.  Every time a node detects a change in its advertised
          neighbor set, it increments this sequence number ("Wraparound"
          is handled as described in section 19).  This number is sent
          in this ANSN field of the TC message to keep track of the most
          recent information.  When a node receives a TC message, it can
          decide on the basis of this Advertised Neighbor Sequence
          Number, whether or not the received information about the
          advertised neighbors of the originator node is more recent
          than what it already has.
 */
  seq_num_t ansn;
} olsr_tc_message_hdr_t;

# define TOPOLOGY_SET_MAX_SIZE 100
typedef struct
{
  address_t T_dest_addr;
  address_t T_dest_iface;
  address_t T_last_addr;
  address_t T_last_iface;
  seq_num_t T_seq;
  olsr_time_t T_time;
} olsr_topology_tuple_t;

SET_DECLARE(topology, TOPOLOGY_SET_MAX_SIZE)
SET_DEFAULT_INIT(topology)
SET_DEFAULT_EMPTY(topology)
SET_DEFAULT_DELETE(topology)
SET_DEFAULT_APPLY(topology)
SET_DEFAULT_FIND(topology)
SET_DEFAULT_IS_EMPTY(topology)
SET_DEFAULT_DECLARE_EMPTY(topology)

# define FOREACH_TOPOLOGY_CREW(Var, Code)       \
  SET_FOREACH(topology, Var, Code)

# define FOREACH_TOPOLOGY_EREW(Var, Code)               \
  SET_FOREACH_AUTOREMOVE(topology, Var, T_time, Code)

void olsr_topology_tuple_init(olsr_topology_tuple_t* tuple);
void olsr_tc_init();
void olsr_topology_tuple_init(olsr_topology_tuple_t* tuple);
int olsr_generate_tc_message(olsr_message_t* tc_message);
void olsr_process_tc_message(packet_byte_t* tc_message, int size,
                             interface_t iface);
void olsr_tc_force_send();
olsr_topology_tuple_t*
olsr_topology_set_insert(const olsr_topology_tuple_t* tuple);

# ifdef DEBUG
void olsr_topology_set_print();
# endif

#endif
