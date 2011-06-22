#include "olsr_ifaces.h"
#include "olsr_link_set.h"
#include "olsr_mpr_set.h"
#include "olsr_neighbor_set.h"
#include "olsr_neighbor2_set.h"
#include "olsr_state.h"

SET_IMPLEMENT(mpr, MPR_SET_MAX_SIZE)

typedef struct
{
  address_t N_neighbor_main_addr;
  link_status_t N_status;
  willingness_t N_willingness;
  int D;
  int reachability;
} olsr_N_tuple_t;

static void
olsr_N_tuple_init(olsr_N_tuple_t* tuple)
{
  tuple->N_neighbor_main_addr = 0;
  tuple->N_status = NOT_NEIGH;
  tuple->N_willingness = WILL_DEFAULT;
  tuple->D = 0;
  tuple->reachability = 0;
}

SET_DECLARE(N, MPR_SET_MAX_SIZE)
SET_IMPLEMENT(N, MPR_SET_MAX_SIZE)

SET_DECLARE_(N2, neighbor2, MPR_SET_MAX_SIZE)
SET_IMPLEMENT_(N2, neighbor2, MPR_SET_MAX_SIZE)

# define FOREACH_N(Var, Code)                   \
  SET_FOREACH(N, Var, Code)

# define FOREACH_N2(Var, Code)                  \
  SET_FOREACH_(N2, neighbor2, Var, Code)


void
olsr_mpr_tuple_init(olsr_mpr_tuple_t* tuple)
{
  tuple->addr = 0;
}

void
olsr_mpr_set_insert(address_t addr)
{
  olsr_mpr_tuple_t tuple =
    {
      .addr = addr,
    };
  olsr_mpr_set_insert_(&tuple);
}

bool
olsr_is_mpr(address_t address)
{
  FOREACH_MPR(tuple,
    if (tuple->addr == address)
      return TRUE);

  return FALSE;
}

void
olsr_mpr_compute_N(address_t iface_address)
{
  olsr_N_set_empty_();
  FOREACH_NEIGHBOR(neighb,
    const address_t neighb_main_addr =
      neighb->N_neighbor_main_addr;
    olsr_N_tuple_t tuple;
    tuple.N_neighbor_main_addr = neighb->N_neighbor_main_addr;
    tuple.N_status = neighb->N_status;
    tuple.N_willingness = neighb->N_willingness;
    tuple.D = 0;
    if (olsr_is_iface_neighbor(iface_address, neighb_main_addr))
      olsr_N_set_insert_(&tuple));
}

void
olsr_mpr_compute_N2()
{
  olsr_N2_set_empty_();
  FOREACH_NEIGHBOR2(n2,
    FOREACH_N(n,
      if (n->N_neighbor_main_addr == n2->N_neighbor_main_addr)
      {
        olsr_N2_set_insert_(n2);
        break;
      }));
}

int
olsr_mpr_D(address_t y)
{
  int D = 0;
  FOREACH_NEIGHBOR2(n2,
    bool is_in_n = FALSE;
    if (n2->N_neighbor_main_addr != y)
      continue;
    if (n2->N_2hop_addr == state.address) /* FIXME: Main or iface? */
      continue;
    FOREACH_N(n,
      if (n->N_neighbor_main_addr == n2->N_2hop_addr)
      {
        is_in_n = TRUE;
        break;
      });
    if (! is_in_n)
      D++);
  return D;
}

void
olsr_mpr_set_compute_iface(interface_t iface)
{
  const address_t iface_address = state.iface_addresses[iface];
  olsr_mpr_compute_N(iface_address);
  olsr_mpr_compute_N2();

  FOREACH_N(n,
    if (n->N_willingness == WILL_ALWAYS)
      olsr_mpr_set_insert(n->N_neighbor_main_addr));

  FOREACH_N(n,
    n->D = olsr_mpr_D(n->N_neighbor_main_addr));

  FOREACH_N2(n2,
    int reachability = 0;
    FOREACH_N(n,
      if (n->N_neighbor_main_addr == n2->N_neighbor_main_addr)
        reachability++);
    if (reachability > 1)
      continue;
    olsr_mpr_set_insert(n2->N_neighbor_main_addr);
    olsr_N2_set_delete_(__i_N2));

  // Compute reachability:
  FOREACH_N(n,
    FOREACH_N2(n2,
      if (n->N_neighbor_main_addr == n2->N_neighbor_main_addr)
        n->reachability++));

  while (N2_set.n_tuples > 0)
  {
    olsr_N_tuple_t* best_n = NULL;
    FOREACH_N(n, best_n = n; break); // Peak first element.
    FOREACH_N(n,
      if (n == best_n)
        continue;
      if (n->reachability <= 0)
        continue;
      if (n->N_willingness > best_n->N_willingness)
      {
        best_n = n;
        continue;
      }
      else if (n->N_willingness == best_n->N_willingness)
      {
        if (n->reachability > best_n->reachability)
        {
          best_n = n;
          continue;
        }
        else if (n->reachability == best_n->reachability)
        {
          if (n->D > best_n->D)
          {
            best_n = n;
            continue;
          }
        }
      });

    olsr_mpr_set_insert(best_n->N_neighbor_main_addr);

    FOREACH_N2(n2,
      FOREACH_N(n,
        if (n->N_neighbor_main_addr == n2->N_neighbor_main_addr)
        {
          olsr_N2_set_delete_(__i_N2);
          break;
        }));
  }
  // FIXME: As an optimization, process each node, y,
  // in the MPR set in increasing order of N_willingness.  If all
  // nodes in N2 are still covered by at least one node in the MPR
  // set excluding node y, and if N_willingness of node y is
  // smaller than WILL_ALWAYS, then node y MAY be removed from the
  // MPR set.
}

void
olsr_mpr_set_recompute()
{
  olsr_mpr_set_empty();

  for (int iface = 0; iface < IFACES_COUNT; iface++)
    olsr_mpr_set_compute_iface(iface);
}
