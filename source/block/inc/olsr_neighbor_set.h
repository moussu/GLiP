#ifndef OLSR_NEIGHBOR_SET_H
# define OLSR_NEIGHBOR_SET_H

# include "olsr_types.h"

typedef enum
{
  NOT_SYM,
  SYM
} link_status_t;

typedef struct
{
  address_t N_neighbor_main_addr;
  link_status_t N_status;
  willingness_t N_willingness;
} olsr_neighbor_tuple_t;

# define NEIGHBOR_SET_MAX_SIZE 10
typedef struct
{
  int n_tuples;
  olsr_link_tuple_t tuples[NEIGHBOR_SET_MAX_SIZE];
} olsr_neighbor_set_t;

#endif
