#ifndef OLSR_2HOP_NEIGHBOR_SET_H
# define OLSR_2HOP_NEIGHBOR_SET_H

# include "olsr_types.h"

typedef struct
{
  address_t N_neighbor_main_addr;
  address_t N_2hop_addr;
  time_t N_time;
} olsr_2hop_neighbor_tuple_t;

# define TWOHOP_NEIGHBOR_SET_MAX_SIZE 10
typedef struct
{
  int n_tuples;
  olsr_2hop_neighbor_tuple_t tuples[TWOHOP_NEIGHBOR_SET_MAX_SIZE];
} olsr_2hop_neighbor_set_t;


#endif
