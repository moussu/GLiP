#ifndef OLSR_MPR_SET_H
# define OLSR_MPR_SET_H

# include <stm32f10x.h>
# include "olsr_types.h"

# define MPR_SET_MAX_SIZE 10
typedef struct
{
  int n_tuples;
  address_t tuples[MPR_SET_MAX_SIZE];
} olsr_mpr_set_t;

void olsr_mpr_set_init(olsr_mpr_set_t* set);
bool olsr_is_mpr(olsr_mpr_set_t* set, address_t address);

#endif
