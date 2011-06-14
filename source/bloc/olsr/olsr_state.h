#ifndef OLSR_STATE_H
# define OLSR_STATE_H

# include "olsr_types.h"
# include "olsr_message.h"
# include "olsr_2hop_neighbor_set.h"
# include "olsr_constants.h"
# include "olsr_duplicate_set.h"
# include "olsr_link_set.h"
# include "olsr_mpr_selector_set.h"
# include "olsr_mpr_set.h"
# include "olsr_neighbor_set.h"
# include "olsr_topology_set.h"

typedef struct
{
  message_type_t types_implemented[MESSAGE_TYPES];
  address_t address;
  address_t iface_addresses[4];
  olsr_duplicate_set_t duplicate_set;
  olsr_link_set_t link_set;
  olsr_mpr_set_t mpr_set;
  olsr_ms_set_t ms_set;
  olsr_neighbor_set_t neighbor_set;
  olsr_2hop_neighbor_set_t twohop_neighbor_set;
  olsr_topology_set_t topology_set;
  willingness_t willingness;
} olsr_node_state_t;

extern olsr_node_state_t state;

#endif
