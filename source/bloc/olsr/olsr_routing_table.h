#ifndef OLSR_ROUTING_TABLE_H
# define OLSR_ROUTING_TABLE_H

#include "olsr_set.h"
#include "olsr_types.h"

# define ROUTING_TABLE_MAX_SIZE 300
typedef struct
{
  address_t R_dest_addr;
  address_t R_next_addr;
  int R_dist;
  address_t R_iface_addr;
} olsr_routing_tuple_t;

SET_DECLARE(routing, ROUTING_TABLE_MAX_SIZE)
SET_DEFAULT_BINDINGS(routing)

# define FOREACH_ROUTE(Var, Code)               \
  SET_FOREACH(routing, Var, Code)

void olsr_routing_tuple_init(olsr_routing_tuple_t* t);
void olsr_routing_table_compute();

# ifdef DEBUG
void olsr_routing_table_print();
# endif

#endif
