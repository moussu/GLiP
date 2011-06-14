#include "olsr_message.h"
#include "olsr_neighbor_set.h"
#include "olsr_hello.h"

void
olsr_neighbor_set_init(olsr_neighbor_set_t* set)
{
  set->n_tuples = 0;
}

void
olsr_neighbor_reset_advertised(olsr_neighbor_set_t* set)
{
  for (int i = 0; i < set->n_tuples; i++)
  {
    olsr_neighbor_tuple_t* tuple = set->tuples + i;
    tuple->advertised = FALSE;
  }
}

bool
olsr_neighbor_set_advertised(olsr_neighbor_set_t* set, address_t addr,
                             neighbor_type_t* neighbor_type)
{
  for (int i = 0; i < set->n_tuples; i++)
  {
    olsr_neighbor_tuple_t* tuple = set->tuples + i;
    if (tuple->N_neighbor_main_addr == addr)
    {
      tuple->advertised = TRUE;
      if (neighbor_type)
        *neighbor_type = olsr_get_neighbor_type(tuple);
    }
  }

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

void
olsr_advertise_neighbors(olsr_neighbor_set_t* set, olsr_message_t* hello_message)
{
  link_type_t link_type;
  neighbor_type_t neighbor_type;
  olsr_link_message_hdr_t link_header;
  link_header.reserved = 0;
  link_header.size = sizeof(olsr_link_message_hdr_t) + sizeof(address_t);

  for (int i = 0; i < set->n_tuples; i++)
  {
    olsr_neighbor_tuple_t* tuple = set->tuples + i;
    if (tuple->advertised)
      continue;
    neighbor_type = olsr_get_neighbor_type(tuple);
    link_type = UNSPEC_LINK;
    link_header.link_code = olsr_link_code(link_type, neighbor_type);
    olsr_message_append(hello_message, &link_header,
                        sizeof(olsr_link_message_hdr_t));
    olsr_message_append(hello_message, &tuple->N_neighbor_main_addr,
                        sizeof(address_t));
  }
}
