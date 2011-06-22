#include <string.h>
#include "olsr_hello.h"
#include "olsr_link_set.h"
#include "olsr_message.h"
#include "olsr_neighbor_set.h"
#include "olsr_mpr_set.h"
#include "olsr_send.h"
#include "olsr_state.h"

SET_SYNCHRO_IMPLEMENT(link, LINK_SET_MAX_SIZE)

/*
     -    The L_SYM_time field of a link tuple expires.  This is
          considered as a neighbor loss if the link described by the
          expired tuple was the last link with a neighbor node (on the
          contrary, a link with an interface may break while a link with
          another interface of the neighbor node remains without being
          observed as a neighborhood change).
 */

static void
olsr_link_set_expire(olsr_link_tuple_t* tuple)
{
  int pos = -1;
  olsr_neighbor_tuple_t* neighbor =
    olsr_link_set_associated_neighbor(tuple, &pos);

  if (neighbor && pos != -1)
  {
    bool remove = TRUE;

    FOREACH_LINK(l,
      if (l == tuple)
        continue;
      if (olsr_iface_to_main_address(l->L_neighbor_iface_addr)
          == neighbor->N_neighbor_main_addr)
      {
        remove = FALSE;
        break;
      });

    if (remove)
      olsr_neighbor_set_delete(pos);
  }
}

void
olsr_link_set_task(void* pvParameters)
{
  portTickType xLastWakeTime;
  for (;;)
  {
    vTaskDelayUntil(&xLastWakeTime, SET_REFRESH_TIME_MS);

    if (link_set.n_tuples == 0)
      continue;

    SET_MUTEX_TAKE(link);

    FOREACH_LINK(l,
      if (l->L_time >= olsr_get_current_time())
        olsr_link_set_delete(__i_link);
      else if (l->L_SYM_time >= olsr_get_current_time())
        olsr_link_set_expire(l)); // FIXME: delete too ?

    SET_MUTEX_GIVE(link);
  }
}

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
olsr_link_set_insert(const olsr_link_tuple_t* lt)
{
  olsr_link_tuple_t* tuple = olsr_link_set_insert_(lt);
  olsr_link_set_updated(tuple);
  return tuple;
}

void olsr_link_set_delete(int i)
{
  bool remove_neighbor = TRUE;

  if (olsr_link_set_is_empty(i))
    return;

  olsr_link_tuple_t* tuple = link_set.tuples + i;
  address_t main_address =
    olsr_iface_to_main_address(tuple->L_neighbor_iface_addr);

  FOREACH_LINK(link,
    if (olsr_iface_to_main_address(link->L_neighbor_iface_addr)
        == main_address)
      remove_neighbor = FALSE);

  if (remove_neighbor)
  {
    FOREACH_NEIGHBOR(n,
      if (n->N_neighbor_main_addr == main_address)
      {
        olsr_neighbor_set_delete(__i_neighbor);
        break;
      });
  }

  olsr_link_set_delete_(i);
}

void
olsr_link_set_updated(const olsr_link_tuple_t* lt)
{
  olsr_neighbor_tuple_t tuple;
  olsr_neighbor_tuple_init(&tuple);

  olsr_neighbor_tuple_t* nt =
    olsr_link_set_associated_neighbor(lt, NULL);

  if (!nt)
  {
    tuple.N_neighbor_main_addr =
      olsr_iface_to_main_address(lt->L_neighbor_iface_addr);
    nt = olsr_neighbor_set_insert(&tuple);
  }

  nt->N_status = NOT_SYM;

  FOREACH_LINK(link,
    if (olsr_iface_to_main_address(link->L_neighbor_iface_addr)
        == nt->N_neighbor_main_addr
        && link->L_SYM_time >= olsr_get_current_time())
    {
      nt->N_status = SYM;
    });

  /*
     -    A new link tuple is inserted in the Link Set with a non
          expired L_SYM_time or a tuple with expired L_SYM_time is
          modified so that L_SYM_time becomes non-expired.  This is
          considered as a neighbor appearance if there was previously no
          link tuple describing a link with the corresponding neighbor
          node.
   */

  // FIXME: is that right ?

  if (lt->L_SYM_time > olsr_get_current_time())
  {
    int pos = -1;
    olsr_neighbor_tuple_t* neighbor =
      olsr_link_set_associated_neighbor(lt, &pos);

    if (neighbor && pos != -1)
    {
      olsr_neighbor_tuple_t tuple;
      olsr_neighbor_tuple_init(&tuple);
      neighbor = olsr_neighbor_set_insert(&tuple);
      neighbor->N_neighbor_main_addr =
        olsr_iface_to_main_address(lt->L_neighbor_iface_addr);
    }

    neighbor->N_status = SYM;
  }
}

olsr_neighbor_tuple_t*
olsr_link_set_associated_neighbor(const olsr_link_tuple_t* tuple, int* pos)
{
  FOREACH_NEIGHBOR(t,
    if (t->N_neighbor_main_addr
        == olsr_iface_to_main_address(
          tuple->L_neighbor_iface_addr))
    {
      if (pos)
        *pos = __i_neighbor;
      return t;
    });

  return NULL;
}

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
                        sizeof(address_t)));


  olsr_advertise_neighbors(&hello_message);

  olsr_send_message(&hello_message, iface);
}

bool
olsr_is_iface_neighbor(address_t iface_address,
                       address_t neighbor_main_address)
{
  FOREACH_LINK(link,
    if (link->L_local_iface_addr == iface_address
        && neighbor_main_address
        == olsr_iface_to_main_address(link->L_neighbor_iface_addr))
      return TRUE);

  return FALSE;
}
