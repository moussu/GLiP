#ifndef OLSR_MPR_SET_H
# define OLSR_MPR_SET_H

# include <stm32f10x.h>
# include "olsr_constants.h"
# include "olsr_neighbor2_set.h"
# include "olsr_set.h"
# include "olsr_types.h"

# define MPR_SET_MAX_SIZE 100
typedef struct
{
  address_t addr;
} olsr_mpr_tuple_t;

SET_DECLARE(mpr, MPR_SET_MAX_SIZE)
SET_DEFAULT_EMPTY(mpr)
SET_DEFAULT_DELETE(mpr)
SET_DEFAULT_APPLY(mpr)
SET_DEFAULT_FIND(mpr)
SET_DEFAULT_IS_EMPTY(mpr)
SET_DEFAULT_DECLARE_EMPTY(mpr)

# define FOREACH_MPR(Var, Code)                 \
  SET_FOREACH(mpr, Var, Code)

void olsr_mpr_set_init();
bool olsr_mpr_set_is_recomputing();
void olsr_mpr_tuple_init(olsr_mpr_tuple_t* tuple);
void olsr_mpr_set_insert(address_t addr);
bool olsr_is_mpr(address_t address);
void olsr_mpr_set_recompute();

# ifdef DEBUG
void olsr_mpr_set_print();
# endif

typedef struct
{
  address_t N_neighbor_main_addr;
  link_status_t N_status;
  willingness_t N_willingness;
  int D;
  int reachability;
} olsr_N_tuple_t;

SET_DECLARE(N, MPR_SET_MAX_SIZE)
SET_DECLARE_(N2, neighbor2, MPR_SET_MAX_SIZE)

# define FOREACH_N(Var, Code)                   \
  SET_FOREACH(N, Var, Code)

# define FOREACH_N2(Var, Code)                  \
  SET_FOREACH_(N2, neighbor2, Var, Code)

void olsr_N_tuple_init(olsr_N_tuple_t* tuple);

# ifdef DEBUG
void olsr_N_set_print();
void olsr_N2_set_print();
# endif


#endif
