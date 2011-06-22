#include "olsr_message.h"
#include "olsr_mpr_set.h"
#include "olsr_ms_set.h"
#include "olsr_neighbor_set.h"
#include "olsr_neighbor2_set.h"
#include "olsr_hello.h"
#include "olsr_state.h"

SET_IMPLEMENT(neighbor, NEIGHBOR_SET_MAX_SIZE)

void
olsr_neighbor_tuple_init(olsr_neighbor_tuple_t* tuple)
{
  tuple->N_neighbor_main_addr = 0;
  tuple->N_status = NOT_NEIGH;
  tuple->N_willingness = WILL_DEFAULT;
  tuple->advertised = FALSE;
}

static void
olsr_neighbor_set_expire(const olsr_neighbor_tuple_t* tuple)
{
  /*
    -    In case of neighbor loss, all 2-hop tuples with
         N_neighbor_main_addr == Main Address of the neighbor MUST be
         deleted.
   */

  bool deleted = FALSE;
  FOREACH_NEIGHBOR2(n2,
    if (n2->N_neighbor_main_addr == tuple->N_neighbor_main_addr)
    {
      olsr_neighbor2_set_delete_(__i_neighbor2);
      deleted = TRUE;
    });

  FOREACH_MS(ms,
    if (ms->MS_main_addr == tuple->N_neighbor_main_addr)
      olsr_ms_set_delete(__i_ms));

  // Optimisation we use olsr_neighbor2_delete_ and recompute the mpr
  // set only after. See olsr_neighbor2_set_delete.
  if (deleted)
    olsr_mpr_set_recompute();
}

void
olsr_neighbor_set_delete(int i)
{
  olsr_neighbor_set_expire(neighbor_set.tuples + i);
  olsr_neighbor_set_delete_(i);
}

void
olsr_neighbor_reset_advertised()
{
  FOREACH_NEIGHBOR(tuple,
    tuple->advertised = FALSE);
}

bool
olsr_neighbor_set_advertised(address_t addr,
                             neighbor_type_t* neighbor_type)
{
  FOREACH_NEIGHBOR(tuple,
    if (tuple->N_neighbor_main_addr == addr)
    {
      tuple->advertised = TRUE;
      if (neighbor_type)
        *neighbor_type =
          olsr_get_neighbor_type(tuple);
      return TRUE;
    });

  return FALSE;
}

neighbor_type_t
olsr_get_neighbor_type(olsr_neighbor_tuple_t* tuple)
{
  if (tuple->N_status == SYM)
    return SYM_NEIGH;
  else if (tuple->N_status == NOT_SYM)
    return NOT_NEIGH;
  return NOT_NEIGH;
}

bool
olsr_is_symetric_neighbor(address_t addr)
{
  FOREACH_NEIGHBOR(tuple,
    if (tuple->N_neighbor_main_addr == addr
      && tuple->N_status == SYM)
      return TRUE)

  return FALSE;
}

void
olsr_advertise_neighbors(olsr_message_t* hello_message)
{
  link_type_t link_type;
  neighbor_type_t neighbor_type;
  olsr_link_message_hdr_t link_header;
  link_header.reserved = 0;
  link_header.size = sizeof(olsr_link_message_hdr_t) + sizeof(address_t);

  FOREACH_NEIGHBOR(tuple,
    if (tuple->advertised)
      continue;
    neighbor_type = olsr_get_neighbor_type(tuple);
    link_type = UNSPEC_LINK;
    link_header.link_code = olsr_link_code(link_type, neighbor_type);
    olsr_message_append(hello_message, &link_header,
                        sizeof(olsr_link_message_hdr_t));
    olsr_message_append(hello_message, &tuple->N_neighbor_main_addr,
                        sizeof(address_t));
    )
}
