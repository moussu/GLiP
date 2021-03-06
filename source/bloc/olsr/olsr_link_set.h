#ifndef OLSR_LINK_SET_H
# define OLSR_LINK_SET_H

# include <assert.h>
# include <stm32f10x.h>
# include "olsr_ifaces.h"
# include "olsr_neighbor_set.h"
# include "olsr_time.h"
# include "olsr_types.h"
# include "olsr_set.h"

# define LINK_SET_MAX_SIZE 100
typedef struct
{
  address_t L_local_iface_addr;
  address_t L_neighbor_iface_addr;
  olsr_time_t L_SYM_time;
  olsr_time_t L_ASYM_time;
  olsr_time_t L_time;
} olsr_link_tuple_t;

SET_DECLARE(link, LINK_SET_MAX_SIZE)
SET_DEFAULT_INIT(link)
SET_DEFAULT_EMPTY(link)
SET_DEFAULT_IS_EMPTY(link)
SET_DEFAULT_DECLARE_EMPTY(link)
SET_DEFAULT_APPLY(link)
SET_DEFAULT_FIND(link)

# define FOREACH_LINK_CREW(Var, Code)           \
  SET_FOREACH(link, Var, Code)

# define FOREACH_LINK_EREW(Var, Code)                           \
  FOREACH_LINK_CREW(Var,                                        \
    if (Var->L_time < olsr_get_current_time())                  \
    {                                                           \
      DEBUG_LINK("link set tuple expired [n_iface_addr:%d]",    \
                 Var->L_neighbor_iface_addr);                   \
      DEBUG_LINK("current time is %d",                          \
                 olsr_get_current_time());                      \
      olsr_link_set_delete_(__i_link);                          \
      continue;                                                 \
    }                                                           \
    else if (Var->L_SYM_time < olsr_get_current_time())         \
      olsr_link_set_expire(Var);                                \
    Code)

// FIXME: delete/continue too when l->L_SYM_time >= olsr_get_current_time() ?

void olsr_link_set_expire(olsr_link_tuple_t* tuple);
void olsr_link_tuple_init(olsr_link_tuple_t* tuple);
olsr_link_tuple_t* olsr_link_set_has(address_t neighbor_iface_addr);
olsr_link_tuple_t* olsr_link_set_insert(const olsr_link_tuple_t* tuple);
void olsr_link_set_delete(int i);
void olsr_link_set_updated(const olsr_link_tuple_t* tuple);
olsr_neighbor_tuple_t*
olsr_link_set_associated_neighbor(const olsr_link_tuple_t* tuple, int* pos);
bool olsr_is_iface_neighbor(address_t iface_address,
                            address_t neighbor_main_address);

# ifdef DEBUG
void olsr_link_set_print();
# endif

#endif
