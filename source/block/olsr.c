#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "lfsr.h"
#include "olsr.h"
#include "olsr_state.h"
#include "olsr_types.h"
#include "olsr_2hop_neighbor_set.h"
#include "olsr_constants.h"
#include "olsr_duplicate_set.h"
#include "olsr_link_set.h"
#include "olsr_mpr_selector_set.h"
#include "olsr_mpr_set.h"
#include "olsr_neighbor_set.h"
#include "olsr_topology_set.h"
#include "olsr_send.h"
#include "olsr_hello.h"

void
olsr_init(address_t uid)
{
  state.address = olsr_get_interface_address(uid, 0);
  state.willingness = WILL_DEFAULT;
  olsr_duplicate_set_init(&state.duplicate_set);
  olsr_link_set_init(&state.link_set);
  olsr_mpr_set_init(&state.mpr_set);
  olsr_ms_set_init(&state.ms_set);
  olsr_neighbor_set_init(&state.neighbor_set);
  olsr_2hop_neighbor_set_init(&state.twohop_neighbor_set);
  olsr_topology_set_init(&state.topology_set);
  olsr_send_init();

  for (int iface = 0; iface < IFACES_COUNT; iface++)
    state.iface_addresses[iface] =
      olsr_get_interface_address(uid, iface);

  xTaskCreate(olsr_hello_task, (signed portCHAR*)"HelloTask",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);
  xTaskCreate(olsr_send_task, (signed portCHAR*)"SendTask",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);
}




