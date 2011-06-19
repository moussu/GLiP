#include "olsr_message.h"
#include "olsr_neighbor_set.h"
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
                   })
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
                   if (tuple->N_neighbor_main_addr == addr)
                     return TRUE
    )

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
