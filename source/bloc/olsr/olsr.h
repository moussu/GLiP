#ifndef ROUTING_H
# define ROUTING_H

# include <stdint.h>
# include <stm32f10x.h>

# include "olsr_types.h"

void olsr_init(address_t address);
void olsr_receive_callback(int iSocket, void* pvContext);

#endif
