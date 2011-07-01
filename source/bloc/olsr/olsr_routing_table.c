#include "olsr_neighbor_set.h"
#include "olsr_neighbor2_set.h"
#include "olsr_link_set.h"
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
  olsr_routing_set_empty();

  FOREACH_NEIGHBOR(
    n,
    if (n->N_status == SYM)
    {
      olsr_link_tuple_t* link = NULL;
      FOREACH_LINK_EREW(
        l,
        if (olsr_iface_to_main_address(l->L_neighbor_iface_addr)
            == n->N_neighbor_main_addr)
        {
          link = l;
          break;
        });

      if (!link)
        continue;

      olsr_routing_tuple_t new;
      new.R_dest_addr =
        olsr_iface_to_main_address(link->L_neighbor_iface_addr);
      new.R_next_addr = link->L_neighbor_iface_addr;
      new.R_dist = 1;
      new.R_iface_addr = link->L_local_iface_addr;

      olsr_routing_set_insert(&new);
    });

  FOREACH_NEIGHBOR2_EREW(
    n2,
    bool is_n1 = FALSE;
    bool corresponds = FALSE;
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
      if (r->R_dest_addr == n2->N_neighbor_main_addr)
      {
        route = r;
        break;
      });

    if (!route)
      continue;

    olsr_routing_tuple_t new;
    new.R_dest_addr = n2->N_2hop_addr;
    new.R_next_addr = route->R_next_addr;
    new.R_dist = 2;
    new.R_iface_addr = route->R_iface_addr;

    olsr_routing_set_insert(&new));

  /*int h = 2;
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

      if (!exists1)
        continue;

      olsr_routing_tuple_t* route = NULL;
      bool exists2 = FALSE;
      FOREACH_ROUTE(
        r,
        if (r->R_dest_addr == t->T_last_addr
          && r->R_dist == h)
        {
          route= r;
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
    }*/

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
