#ifndef OLSR_NEIGHBOR_SET_H
# define OLSR_NEIGHBOR_SET_H

# include <stm32f10x.h>
# include "olsr_types.h"
# include "olsr_constants.h"
# include "olsr_message.h"
# include "olsr_set.h"

# define NEIGHBOR_SET_MAX_SIZE 100
typedef struct
{
  address_t N_neighbor_main_addr;
  link_status_t N_status;
  willingness_t N_willingness;
  bool advertised;
} olsr_neighbor_tuple_t;

SET_DECLARE(neighbor, NEIGHBOR_SET_MAX_SIZE)
SET_DEFAULT_INIT(neighbor)
SET_DEFAULT_EMPTY(neighbor)
SET_DEFAULT_APPLY(neighbor)
SET_DEFAULT_FIND(neighbor)
SET_DEFAULT_IS_EMPTY(neighbor)
SET_DEFAULT_DECLARE_EMPTY(neighbor)

# define FOREACH_NEIGHBOR(Var, Code)            \
  SET_FOREACH(neighbor, Var, Code)

void olsr_neighbor_tuple_init(olsr_neighbor_tuple_t* tuple);
olsr_neighbor_tuple_t*
olsr_neighbor_set_insert(const olsr_neighbor_tuple_t* t);
void olsr_neighbor_set_delete(int i);
void olsr_neighbor_reset_advertised();
bool olsr_neighbor_set_advertised(address_t addr,
                                  neighbor_type_t* neighbor_type);
neighbor_type_t olsr_get_neighbor_type(olsr_neighbor_tuple_t* tuple);
bool olsr_is_symetric_neighbor(address_t addr);
void olsr_advertise_neighbors(olsr_message_t* hello_message);

# ifdef DEBUG
void olsr_neighbor_set_print();
# endif

#endif
