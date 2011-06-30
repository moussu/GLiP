#include <string.h>
#include "olsr_hello.h"
#include "olsr_link_set.h"
#include "olsr_message.h"
#include "olsr_neighbor_set.h"
#include "olsr_mpr_set.h"
#include "olsr_send.h"
#include "olsr_state.h"

SET_IMPLEMENT(link, LINK_SET_MAX_SIZE)

/*
     -    The L_SYM_time field of a link tuple expires.  This is
          considered as a neighbor loss if the link described by the
          expired tuple was the last link with a neighbor node (on the
          contrary, a link with an interface may break while a link with
          another interface of the neighbor node remains without being
          observed as a neighborhood change).
 */

void
olsr_link_set_expire(olsr_link_tuple_t* tuple)
{
  int pos = -1;
  olsr_neighbor_tuple_t* neighbor =
    olsr_link_set_associated_neighbor(tuple, &pos);

  if (neighbor && pos != -1)
  {
    bool remove = TRUE;

    FOREACH_LINK_CREW(l,
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

  FOREACH_LINK_EREW(link,
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
  /*
     -    A new link tuple is inserted in the Link Set with a non
          expired L_SYM_time or a tuple with expired L_SYM_time is
          modified so that L_SYM_time becomes non-expired.  This is
          considered as a neighbor appearance if there was previously no
          link tuple describing a link with the corresponding neighbor
          node.
   */

  bool updated_is_sym = lt->L_SYM_time > olsr_get_current_time();

  olsr_neighbor_tuple_t tuple;
  olsr_neighbor_tuple_init(&tuple);

  DEBUG_LINK("a link tuple was updated or inserted "
             "[iface_addr:%d, n_iface_addr:%d]",
             lt->L_local_iface_addr, lt->L_neighbor_iface_addr);
  DEBUG_INC;

  int pos = -1;
  olsr_neighbor_tuple_t* associated =
    olsr_link_set_associated_neighbor(lt, &pos);
  olsr_neighbor_tuple_t* nt = NULL;

  if (associated)
  {
    nt = associated;
    DEBUG_LINK("an associated tuple has been found in the neighbor set");
    DEBUG_LINK("[status:%s, will:%s]",
               olsr_link_status_str(nt->N_status),
               olsr_willingness_str(nt->N_willingness));
  }
  else
  {
    DEBUG_LINK("no associated tuple exists in the neighbor set, inserting one");
    DEBUG_INC;

    tuple.N_neighbor_main_addr =
      olsr_iface_to_main_address(lt->L_neighbor_iface_addr);
    nt = olsr_neighbor_set_insert(&tuple);

    DEBUG_DEC;
  }

  if (updated_is_sym)
  {
    DEBUG_LINK("setting status to %s as updated link tuple is SYM",
               olsr_link_status_str(SYM));
    nt->N_status = SYM;
    DEBUG_DEC;
    return;
  }

  DEBUG_LINK("setting status to %s", olsr_link_status_str(NOT_SYM));

  nt->N_status = NOT_SYM;

  DEBUG_LINK("seeking for an associated SYM link of our neighbor tuple");
  DEBUG_INC;

  FOREACH_LINK_CREW(link,
    if (olsr_iface_to_main_address(link->L_neighbor_iface_addr)
        == nt->N_neighbor_main_addr
        && link->L_SYM_time >= olsr_get_current_time())
    {
      DEBUG_LINK("found one [iface_addr:%d, n_iface_addr:%d]",
                 lt->L_local_iface_addr, lt->L_neighbor_iface_addr);

      nt->N_status = SYM;
      break;
    });

  DEBUG_DEC;
  DEBUG_LINK("status is now %s", olsr_link_status_str(nt->N_status));

  DEBUG_DEC;
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
  FOREACH_LINK_EREW(t,
    if (t->L_neighbor_iface_addr == neighbor_iface_addr)
      return t
    )
  return NULL;
}

bool
olsr_is_iface_neighbor(address_t iface_address,
                       address_t neighbor_main_address)
{
  DEBUG_LINK("finding out if neighbor [main_addr:%d] "
             "is neighbor of iface [iface_addr:%d]",
             neighbor_main_address, iface_address);

  DEBUG_LINK("foreach link (autoremove)");
  DEBUG_LINK("if a link goes from iface to neighbor (whichever iface)");
  DEBUG_LINK("it is TRUE");

  DEBUG_INC;
  FOREACH_LINK_EREW(
    link,
    address_t main_address =
    olsr_iface_to_main_address(link->L_neighbor_iface_addr);

    if (link->L_local_iface_addr == iface_address
        && neighbor_main_address == main_address)
    {
      DEBUG_LINK("match, return TRUE");
      DEBUG_DEC;
      return TRUE;
    });

  DEBUG_DEC;

  DEBUG_LINK("no match, return FALSE");

  return FALSE;
}

#ifdef DEBUG
void olsr_link_set_print()
{
  DEBUG_LINK("--- LINK SET ---");
  DEBUG_LINK("");

  DEBUG_INC;

  DEBUG_LINK("current time is %d", (int)olsr_get_current_time());
  DEBUG_LINK("");

  DEBUG_LINK(".-%s-.-%s-.-%s-.-%s-.-%s-.",
             DASHES(11),
             DASHES(11),
             DASHES(10),
             DASHES(10),
             DASHES(10));


  DEBUG_LINK("| %11s | %11s | %10s | %10s | %10s |",
             "local iface", "neigh iface",
             "SYM time", "ASYM time", "time");

  DEBUG_LINK("+-%s-+-%s-+-%s-+-%s-+-%s-+",
             DASHES(11),
             DASHES(11),
             DASHES(10),
             DASHES(10),
             DASHES(10));

  FOREACH_LINK_CREW(
    l,
    DEBUG_LINK("| %11d | %11d | %10d | %10d | %10d |",
               l->L_local_iface_addr, l->L_neighbor_iface_addr,
               l->L_SYM_time, l->L_ASYM_time, l->L_time);
    );

  DEBUG_LINK("'-%s-'-%s-'-%s-'-%s-'-%s-'",
             DASHES(11),
             DASHES(11),
             DASHES(10),
             DASHES(10),
             DASHES(10));

  DEBUG_DEC;

  DEBUG_LINK("");

  DEBUG_LINK("--- END LINK SET ---");
}
#endif
