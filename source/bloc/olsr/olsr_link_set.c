#include <string.h>
#include "olsr_hello.h"
#include "olsr_link_set.h"
#include "olsr_message.h"
#include "olsr_neighbor_set.h"
#include "olsr_mpr_set.h"
#include "olsr_send.h"
#include "olsr_state.h"

SET_IMPLEMENT(link, LINK_SET_MAX_SIZE)

void
olsr_link_tuple_init(olsr_link_tuple_t* tuple)
{
  tuple->L_local_iface_addr = 0;
  tuple->L_neighbor_iface_addr = 0;
  tuple->L_SYM_time = 0;
  tuple->L_ASYM_time = 0;
  tuple->L_time = 0;
}

/*
   For each tuple in the Link Set, where L_local_iface_addr is the
   interface where the HELLO is to be transmitted, and where L_time >=
   current time (i.e., not expired), L_neighbor_iface_addr is advertised
   with:

     1    The Link Type set according to the following:

          1.1  if L_SYM_time >= current time (not expired)

                    Link Type = SYM_LINK

          1.2  Otherwise, if L_ASYM_time >= current time (not expired)
               AND

                             L_SYM_time  <  current time (expired)

                    Link Type = ASYM_LINK

          1.3  Otherwise, if L_ASYM_time < current time (expired) AND

                             L_SYM_time  < current time (expired)

                    Link Type = LOST_LINK

     2    The Neighbor Type is set according to the following:

          2.1  If the main address, corresponding to
               L_neighbor_iface_addr, is included in the MPR set:

                    Neighbor Type = MPR_NEIGH

          2.2  Otherwise, if the main address, corresponding to
               L_neighbor_iface_addr, is included in the neighbor set:

               2.2.1
                    if N_status == SYM

                         Neighbor Type = SYM_NEIGH

               2.2.2
                    Otherwise, if N_status == NOT_SYM
                         Neighbor Type = NOT_NEIGH
*/

olsr_link_tuple_t*
olsr_link_set_has(address_t neighbor_iface_addr)
{
  FOREACH_LINK(t,
    if (t->L_neighbor_iface_addr == neighbor_iface_addr)
      return t
    )
  return NULL;
}

void
olsr_send_hello(interface_t iface)
{
  // use the same willingness for everyone!
  const willingness_t willingness = state.willingness;
  address_t iface_address = state.iface_addresses[iface];
  link_type_t link_type;
  neighbor_type_t neighbor_type;

  olsr_message_t hello_message;
  hello_message.header.type = HELLO_MESSAGE;
  hello_message.header.Vtime = olsr_serialize_time(
    olsr_seconds_to_time(NEIGHB_HOLD_TIME_S)
    );
  hello_message.header.ttl = 1;
  hello_message.header.size = sizeof(olsr_message_hdr_t);

  olsr_hello_message_hdr_t hello_header;
  hello_header.reserved = 0;
  hello_header.Htime = olsr_serialize_time(
    olsr_seconds_to_time(HELLO_INTERVAL_S)
    );
  hello_header.willingness = willingness;

  olsr_message_append(&hello_message, &hello_header,
                      sizeof(olsr_hello_message_hdr_t));

  olsr_link_message_hdr_t link_header;
  link_header.reserved = 0;
  link_header.size = sizeof(olsr_link_message_hdr_t) + sizeof(address_t);

  FOREACH_LINK(t,
    if (t->L_time <= olsr_get_current_time())
    {
      olsr_link_set_delete(__i);
      continue;
    }

    if (t->L_local_iface_addr != iface_address)
      continue;

    if (t->L_SYM_time >= olsr_get_current_time())
      link_type = SYM_LINK;
    else if (t->L_ASYM_time >= olsr_get_current_time())
      link_type = ASYM_LINK;
    else
      link_type = LOST_LINK;

    address_t neighbor_main_address =
      olsr_iface_to_main_address(t->L_neighbor_iface_addr);

    // Mark neighbors as advertised, grab neighbor_type:
    olsr_neighbor_set_advertised(neighbor_main_address,
                                 &neighbor_type);

    // If is MPR, alter neighbor_type:
    if (olsr_is_mpr(neighbor_main_address))
      neighbor_type = MPR_NEIGH;

    // Here I'm assuming that if the neighbor_main_address is not in
    // the MPR set it WILL be in the neighbor set...

    link_header.link_code = olsr_link_code(link_type, neighbor_type);
    olsr_message_append(&hello_message, &link_header,
                        sizeof(olsr_link_message_hdr_t));
    olsr_message_append(&hello_message, &t->L_neighbor_iface_addr,
                        sizeof(address_t));
    )


  olsr_advertise_neighbors(&hello_message);

  olsr_send_message(&hello_message, iface);
}