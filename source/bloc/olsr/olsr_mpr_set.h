#ifndef OLSR_MPR_SET_H
# define OLSR_MPR_SET_H

# include <stm32f10x.h>
# include "olsr_set.h"
# include "olsr_types.h"

# define MPR_SET_MAX_SIZE 10
typedef struct
{
  address_t addr;
} olsr_mpr_tuple_t;

SET_DECLARE(mpr, MPR_SET_MAX_SIZE)
SET_DEFAULT_INIT(mpr)
SET_DEFAULT_EMPTY(mpr)
SET_DEFAULT_DELETE(mpr)
SET_DEFAULT_APPLY(mpr)
SET_DEFAULT_FIND(mpr)
SET_DEFAULT_IS_EMPTY(mpr)
SET_DEFAULT_DECLARE_EMPTY(mpr)

# define FOREACH_MPR(Var, Code)                 \
  SET_FOREACH(mpr, Var, Code)

void olsr_mpr_tuple_init(olsr_mpr_tuple_t* tuple);
void olsr_mpr_set_insert(address_t addr);
bool olsr_is_mpr(address_t address);
void olsr_mpr_set_recompute();

# ifdef DEBUG
void olsr_mpr_set_print();
# endif

#endif
