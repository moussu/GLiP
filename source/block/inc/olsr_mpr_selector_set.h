#ifndef OLSR_MPR_SELECTOR_SET_H
# define OLSR_MPR_SELECTOR_SET_H

# include "olsr_types.h"

typedef struct
{
  address_t MS_main_addr;
  time_t MS_time;
} olsr_mpr_selector_tuple_t;

# define MPR_SELECTOR_SET_MAX_SIZE 10
typedef struct
{
  int n_tuples;
  olsr_mpr_selector_tuple_t tuples[MPR_SELECTOR_SET_MAX_SIZE];
} olsr_mpr_selector_set_t;

#endif
