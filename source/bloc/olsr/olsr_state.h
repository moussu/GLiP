#ifndef OLSR_STATE_H
# define OLSR_STATE_H

# include "olsr_types.h"
# include "olsr_message.h"
# include "olsr_constants.h"

typedef struct
{
  message_type_t types_implemented[MESSAGE_TYPES];
  address_t address;
  address_t iface_addresses[4];
  willingness_t willingness;
} olsr_node_state_t;

extern olsr_node_state_t state;

#endif
