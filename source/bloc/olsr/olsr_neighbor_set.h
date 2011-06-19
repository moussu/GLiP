#ifndef OLSR_NEIGHBOR_SET_H
# define OLSR_NEIGHBOR_SET_H

# include <stm32f10x.h>
# include "olsr_types.h"
# include "olsr_constants.h"
# include "set.h"

# define NEIGHBOR_SET_MAX_SIZE 10
typedef struct
{
  address_t N_neighbor_main_addr;
  link_status_t N_status;
  willingness_t N_willingness;
  bool advertised;
} olsr_neighbor_tuple_t;
SET_DECLARE(neighbor, NEIGHBOR_SET_MAX_SIZE)
SET_DEFAULT_BINDINGS(neighbor)

void olsr_neighbor_reset_advertised(olsr_neighbor_set_t* set);
bool olsr_neighbor_set_advertised(olsr_neighbor_set_t* set, address_t addr,
                                  neighbor_type_t* neighbor_type);
neighbor_type_t olsr_get_neighbor_type(olsr_neighbor_tuple_t* tuple);
bool olsr_is_symetric_neighbor(olsr_neighbor_set_t* set, address_t addr);
void olsr_advertise_neighbors(olsr_neighbor_set_t* set,
                              olsr_message_t* hello_message);

#endif
