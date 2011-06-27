#include "olsr_hello.h"
#include "olsr_ifaces.h"
#include "olsr_link_set.h"
#include "olsr_mpr_set.h"
#include "olsr_neighbor_set.h"
#include "olsr_neighbor2_set.h"
#include "olsr_state.h"

#define MPR_ALGORITHM

SET_IMPLEMENT(mpr, MPR_SET_MAX_SIZE)
SET_IMPLEMENT(N,   MPR_SET_MAX_SIZE)
SET_IMPLEMENT_(N2, neighbor2, MPR_SET_MAX_SIZE)

static bool is_recomputing = FALSE;

bool
olsr_mpr_set_is_recomputing()
{
  return is_recomputing;
}

void
olsr_mpr_set_init()
{
  olsr_N_set_init_();
  olsr_N2_set_init_();
  olsr_mpr_set_init_();
}

void
olsr_mpr_tuple_init(olsr_mpr_tuple_t* tuple)
{
  tuple->addr = 0;
}

void
olsr_mpr_set_insert(address_t addr)
{
  // Avoid duplicates (not in RFC):
  FOREACH_MPR(mpr, if (mpr->addr == addr) return);

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
olsr_N_tuple_init(olsr_N_tuple_t* tuple)
{
  tuple->N_neighbor_main_addr = 0;
  tuple->N_status = NOT_NEIGH;
  tuple->N_willingness = WILL_DEFAULT;
  tuple->D = 0;
  tuple->reachability = 0;
}

void
olsr_mpr_compute_N(address_t iface_address)
{
  DEBUG_MPR("first, make N empty");
  olsr_N_set_empty_();

  DEBUG_MPR("foreach neighbor insert it in N");
  DEBUG_INC;
  FOREACH_NEIGHBOR(neighb,
    const address_t neighb_main_addr =
      neighb->N_neighbor_main_addr;
    DEBUG_MPR("neighbor main address is %d", neighb_main_addr);
    DEBUG_MPR("find out if it is an iface neighbor");
    DEBUG_INC;
    if (olsr_is_iface_neighbor(iface_address, neighb_main_addr))
    {
      olsr_N_tuple_t tuple;
      tuple.N_neighbor_main_addr = neighb->N_neighbor_main_addr;
      tuple.N_status = neighb->N_status;
      tuple.N_willingness = neighb->N_willingness;
      tuple.D = 0;

      DEBUG_MPR("insert it in N");
      olsr_N_set_insert_(&tuple);
    }
    DEBUG_DEC);

  DEBUG_DEC;
}

void
olsr_mpr_compute_N2()
{
  DEBUG_MPR("first, make N2 empty");
  olsr_N2_set_empty_();

  DEBUG_MPR("foreach neighbor2 find the associated neighbor");
  DEBUG_INC;
  FOREACH_NEIGHBOR2_EREW(
    n2,
    DEBUG_MPR("n2 [neighbor_addr:%d, 2hop_addr:%d, time:%d]",
              n2->N_neighbor_main_addr, n2->N_2hop_addr, n2->N_time);

    if (n2->N_2hop_addr == state.address)
      continue;

    bool exists_link = FALSE;
    FOREACH_LINK_EREW(
      l,
      if (olsr_iface_to_main_address(l->L_neighbor_iface_addr)
          == n2->N_2hop_addr)
      {
        exists_link = TRUE;
        break;
      });
    if (exists_link)
      continue;

    DEBUG_INC;
    FOREACH_N(
      n,
      if (n->N_neighbor_main_addr == n2->N_neighbor_main_addr)
      {
        DEBUG_MPR("n found [status:%s, will:%s]",
                  olsr_link_status_str(n->N_status),
                  olsr_willingness_str(n->N_willingness));

        // FIXME: definition is ambiguous there...
        /*
          excluding:

               (i)   the nodes only reachable by members of N with
                     willingness WILL_NEVER
         */
        if (n->N_willingness == WILL_NEVER)
          continue;

        DEBUG_MPR("inserting n2 into N2");
        olsr_N2_set_insert_(n2);

        break;
      })
    DEBUG_DEC);

  DEBUG_DEC;
}

int
olsr_mpr_D(address_t y)
{
  int D = 0;
  FOREACH_NEIGHBOR2_EREW(n2,
    if (n2->N_neighbor_main_addr != y)
      continue;
    if (n2->N_2hop_addr == state.address)
      continue;

    bool is_in_n = FALSE;
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

  DEBUG_MPR("compute N set");
  DEBUG_INC;
  olsr_mpr_compute_N(iface_address);
  DEBUG_DEC;

  DEBUG_MPR("compute N2 set");
  DEBUG_INC;
  olsr_mpr_compute_N2();
  DEBUG_DEC;

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

  while (N2_set.n_tuples > 0)
  {
    // Compute reachability:
    FOREACH_N(
      n,
      n->reachability = 0;
      FOREACH_N2(
        n2,
        bool covered = FALSE;
        FOREACH_MPR(
          mpr,
          if (mpr->addr == n2->N_neighbor_main_addr)
          {
            covered = TRUE;
            break;
          });
        if (!covered)
          n->reachability++));

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
      FOREACH_MPR(mpr,
        if (mpr->addr == n2->N_neighbor_main_addr)
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
  if (is_recomputing)
    return;

  is_recomputing = TRUE;

  DEBUG_MPR("recomputing MPR set, first empty it");
  olsr_mpr_set_empty();

#ifdef MPR_ALGORITHM
  DEBUG_MPR("foreach iface, update it");
  DEBUG_INC;
  for (int iface = 0; iface < IFACES_COUNT; iface++)
  {
    DEBUG_MPR("iface %c:", olsr_iface_print(iface));
    DEBUG_INC;
    olsr_mpr_set_compute_iface(iface);
    DEBUG_DEC;
  }
  DEBUG_DEC;
#else
  FOREACH_NEIGHBOR(
    n,
    olsr_mpr_set_insert(n->N_neighbor_main_addr));
#endif

  is_recomputing = FALSE;

  /*
     -    An additional HELLO message MAY be sent when the MPR set
          changes.
  */
  olsr_hello_force_send();
}



#ifdef DEBUG
void olsr_N_set_print()
{
  DEBUG_MPR("--- N SET ---");
  DEBUG_MPR("");

  DEBUG_INC;

  DEBUG_MPR(".-%s-.-%s-.-%s-.-%s-.-%s-.",
            DASHES(10), DASHES(10), DASHES(12),
            DASHES(4), DASHES(4));

  DEBUG_MPR("| %10s | %10s | %12s | %4s | %4s |",
            "main addr", "status", "will", "D", "r");

  DEBUG_MPR("+-%s-+-%s-+-%s-+-%s-+-%s-+",
            DASHES(10), DASHES(10), DASHES(12),
            DASHES(4), DASHES(4));

  FOREACH_N(
    n,
    DEBUG_MPR("| %10d | %10s | %12s | %4d | %4d |",
              n->N_neighbor_main_addr,
              olsr_link_status_str(n->N_status),
              olsr_willingness_str(n->N_willingness),
              n->D,
              n->reachability
      );
    );

  DEBUG_MPR("'-%s-'-%s-'-%s-'-%s-'-%s-'",
            DASHES(10), DASHES(10), DASHES(12),
            DASHES(4), DASHES(4));

  DEBUG_DEC;

  DEBUG_MPR("");

  DEBUG_MPR("--- END N SET ---");
}

void olsr_N2_set_print()
{
  DEBUG_MPR("--- N2 SET ---");
  DEBUG_MPR("");

  DEBUG_INC;

  DEBUG_MPR("current time is %d", (int)olsr_get_current_time());
  DEBUG_MPR("");

  DEBUG_MPR(".-%s-.-%s-.-%s-.", DASHES(12), DASHES(12), DASHES(10));

  DEBUG_MPR("| %12s | %12s | %10s |", "n main addr", "n2 main addr", "time");

  DEBUG_MPR("+-%s-+-%s-+-%s-+", DASHES(12), DASHES(12), DASHES(10));

  FOREACH_N2(
    n,
    DEBUG_MPR("| %12d | %12d | %10d |",
                   n->N_neighbor_main_addr,
                   n->N_2hop_addr,
                   n->N_time);
    );

  DEBUG_MPR("'-%s-'-%s-'-%s-'", DASHES(12), DASHES(12), DASHES(10));

  DEBUG_DEC;

  DEBUG_MPR("");

  DEBUG_MPR("--- END N2 SET ---");
}

void olsr_mpr_set_print()
{
  DEBUG_MPR("--- MPR SET ---");
  DEBUG_MPR("");

  DEBUG_INC;

  DEBUG_MPR(".-%s-.", DASHES(4));

  DEBUG_MPR("| %4s |", "addr");

  DEBUG_MPR("+-%s-+", DASHES(4));

  FOREACH_MPR(
    m,
    DEBUG_MPR("| %4d |", m->addr));

  DEBUG_MPR("'-%s-'", DASHES(4));

  DEBUG_DEC;

  DEBUG_MPR("");

  DEBUG_MPR("--- END MPR SET ---");
}
#endif
