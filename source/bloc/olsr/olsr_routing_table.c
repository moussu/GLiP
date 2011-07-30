#include "olsr_neighbor_set.h"
#include "olsr_neighbor2_set.h"
#include "olsr_link_set.h"
#include "olsr_state.h"
#include "olsr_topology_set.h"
#include "olsr_routing_table.h"

SET_IMPLEMENT(routing, ROUTING_TABLE_MAX_SIZE)

void
olsr_routing_tuple_init(olsr_routing_tuple_t* t)
{
  t->R_dest_addr = 0;
  t->R_next_addr = 0;
  t->R_dist = 0;
  t->R_iface_addr = 0;
}

void
olsr_routing_table_compute()
{
  // 1    All the entries from the routing table are removed.
  olsr_routing_set_empty();

  /*
     2    The new routing entries are added starting with the
          symmetric neighbors (h=1) as the destination nodes. Thus, for
          each neighbor tuple in the neighbor set where:

               N_status   = SYM

          (there is a symmetric link to the neighbor), and for each
          associated link tuple of the neighbor node such that L_time >=
          current time, a new routing entry is recorded in the routing
          table with:

               R_dest_addr  = L_neighbor_iface_addr, of the
                              associated link tuple;

               R_next_addr  = L_neighbor_iface_addr, of the
                              associated link tuple;

               R_dist       = 1;

               R_iface_addr = L_local_iface_addr of the
                              associated link tuple.

          If in the above, no R_dest_addr is equal to the main address
          of the neighbor, then another new routing entry with MUST be
          added, with:

               R_dest_addr  = main address of the neighbor;

               R_next_addr  = L_neighbor_iface_addr of one of the
                              associated link tuple with L_time >=
               current time;

               R_dist       = 1;

               R_iface_addr = L_local_iface_addr of the
                              associated link tuple.
  */
  FOREACH_NEIGHBOR(
    n,
    if (n->N_status == SYM)
    {
      FOREACH_LINK_EREW(
        l,
        if (olsr_iface_to_main_address(l->L_neighbor_iface_addr)
            == n->N_neighbor_main_addr)
        {
          olsr_routing_tuple_t new;
          new.R_dest_addr =
            olsr_iface_to_main_address(l->L_neighbor_iface_addr);
          new.R_next_addr = l->L_neighbor_iface_addr;
          new.R_dist = 1;
          new.R_iface_addr = l->L_local_iface_addr;
          olsr_routing_set_insert(&new);
        });
    });

  /*
     3    for each node in N2, i.e., a 2-hop neighbor which is not a
          neighbor node or the node itself, and such that there exist at
          least one entry in the 2-hop neighbor set where
          N_neighbor_main_addr correspond to a neighbor node with
          willingness different of WILL_NEVER, one selects one 2-hop
          tuple and creates one entry in the routing table with:

               R_dest_addr  =  the main address of the 2-hop neighbor;

               R_next_addr  = the R_next_addr of the entry in the
                              routing table with:

                                  R_dest_addr == N_neighbor_main_addr
                                                 of the 2-hop tuple;

               R_dist       = 2;

               R_iface_addr = the R_iface_addr of the entry in the
                              routing table with:

                                  R_dest_addr == N_neighbor_main_addr
                                                 of the 2-hop tuple;

  */

  FOREACH_NEIGHBOR2_EREW(
    n2,
    bool is_n1 = FALSE;
    bool corresponds = FALSE;

    // FIXME: here we may use N2 computed in olsr_mpr_set.c...
    // Not sure though.

    if (n2->N_2hop_addr == state.address)
      continue;

    FOREACH_NEIGHBOR(
      n1,
      if (n1->N_neighbor_main_addr == n2->N_2hop_addr)
      {
        is_n1 = TRUE;
        break;
      }
      if (n1->N_neighbor_main_addr == n2->N_neighbor_main_addr
          && n1->N_willingness != WILL_NEVER)
        corresponds = TRUE);

    if (is_n1)
      continue;

    if (!corresponds)
      continue;

    olsr_routing_tuple_t* route = NULL;
    FOREACH_ROUTE(
      r,
      // Avoid processing twice the same...
      if (r->R_dest_addr == n2->N_2hop_addr)
      {
        route = NULL;
        break;
      }

      if (r->R_dest_addr == n2->N_neighbor_main_addr)
        route = r);

    if (!route)
      continue;

    olsr_routing_tuple_t new;
    new.R_dest_addr = n2->N_2hop_addr;
    new.R_next_addr = route->R_next_addr;
    new.R_dist = 2;
    new.R_iface_addr = route->R_iface_addr;

    olsr_routing_set_insert(&new));

  /*
     3    The new route entries for the destination nodes h+1 hops away
          are recorded in the routing table.  The following procedure
          MUST be executed for each value of h, starting with h=2 and
          incrementing it by 1 each time.  The execution will stop if no
          new entry is recorded in an iteration.

          3.1  For each topology entry in the topology table, if its
               T_dest_addr does not correspond to R_dest_addr of any
               route entry in the routing table AND its T_last_addr
               corresponds to R_dest_addr of a route entry whose R_dist
               is equal to h, then a new route entry MUST be recorded in
               the routing table (if it does not already exist) where:

                    R_dest_addr  = T_dest_addr;

                    R_next_addr  = R_next_addr of the recorded
                                   route entry where:

                                   R_dest_addr == T_last_addr

                    R_dist       = h+1; and

                    R_iface_addr = R_iface_addr of the recorded
                                   route entry where:

                                      R_dest_addr == T_last_addr.

          3.2  Several topology entries may be used to select a next hop
               R_next_addr for reaching the node R_dest_addr.  When h=1,
               ties should be broken such that nodes with highest
               willingness and MPR selectors are preferred as next hop.
  */

  int h = 2;
  bool something_inserted = TRUE;
  while (something_inserted)
  {
    const int old_size = routing_set.n_tuples;
    DEBUG_TOPOLOGY("H %d", h);

    FOREACH_TOPOLOGY_EREW(
      t,
      bool exists1 = FALSE;
      FOREACH_ROUTE(
        r,
        if (r->R_dest_addr == t->T_dest_addr)
        {
          exists1 = TRUE;
          break;
        });

      if (exists1)
        continue;

      olsr_routing_tuple_t* route = NULL;
      bool exists2 = FALSE;
      FOREACH_ROUTE(
        r,
        if (r->R_dest_addr == t->T_last_addr
          && r->R_dist == h)
        {
          route = r;
          exists2 = TRUE;
          break;
        });

      if (!exists2 || !route)
        continue;

      olsr_routing_tuple_t new;
      new.R_dest_addr = t->T_dest_addr;
      new.R_next_addr = route->R_next_addr;
      new.R_dist = h + 1;
      new.R_iface_addr = route->R_iface_addr;

      olsr_routing_set_insert(&new));

    h++;
    something_inserted = routing_set.n_tuples > old_size;
    }

}

#ifdef DEBUG
void
olsr_routing_table_print()
{
  DEBUG_ROUTING_TABLE("--- ROUTING TABLE ---");
  DEBUG_ROUTING_TABLE("");

  DEBUG_INC;

  DEBUG_ROUTING_TABLE(".-%s-.-%s-.-%s-.-%s-.",
                      DASHES(10),
                      DASHES(10),
                      DASHES(4),
                      DASHES(10));

  DEBUG_ROUTING_TABLE("| %10s | %10s | %4s | %10s |",
                      "dest addr",
                      "next addr",
                      "dist",
                      "iface addr");

  DEBUG_ROUTING_TABLE("+-%s-+-%s-+-%s-+-%s-+",
                      DASHES(10),
                      DASHES(10),
                      DASHES(4),
                      DASHES(10));

  FOREACH_ROUTE(
    r,
    DEBUG_ROUTING_TABLE("| %10d | %10d | %4d | %10d |",
                        r->R_dest_addr,
                        r->R_next_addr,
                        r->R_dist,
                        r->R_iface_addr);
    );

  DEBUG_ROUTING_TABLE("'-%s-'-%s-'-%s-'-%s-'",
                      DASHES(10),
                      DASHES(10),
                      DASHES(4),
                      DASHES(10));

  DEBUG_DEC;

  DEBUG_ROUTING_TABLE("");

  DEBUG_ROUTING_TABLE("--- END ROUTING TABLE ---");
}
#endif
