#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "utils/lfsr.h"
#include "olsr.h"
#include "olsr_state.h"
#include "olsr_types.h"
#include "olsr_neighbor2_set.h"
#include "olsr_constants.h"
#include "olsr_duplicate_set.h"
#include "olsr_link_set.h"
#include "olsr_ms_set.h"
#include "olsr_mpr_set.h"
#include "olsr_neighbor_set.h"
#include "olsr_topology_set.h"
#include "olsr_receive.h"
#include "olsr_routing_table.h"
#include "olsr_send.h"
#include "olsr_hello.h"

static xSemaphoreHandle olsr_global_mutex;

void
olsr_global_mutex_take()
{
  xSemaphoreTake(olsr_global_mutex, portMAX_DELAY);
}

void
olsr_global_mutex_give()
{
  xSemaphoreGive(olsr_global_mutex);
}

void
olsr_init(address_t uid)
{
  olsr_global_mutex = xSemaphoreCreateMutex();

  // Initialize state:
  state.address = olsr_get_interface_address(uid, 0);
  state.willingness = WILL_DEFAULT;

  for (int iface = 0; iface < IFACES_COUNT; iface++)
  {
    state.iface_addresses[iface] =
      olsr_get_interface_address(uid, iface);
    DEBUG_SERVER("iface %c address is 0x%x",
                 olsr_iface_print(iface),
                 state.iface_addresses[iface]);
  }

  // Initialize sets:
  olsr_duplicate_set_init();
  olsr_link_set_init();
  olsr_mpr_set_init();
  olsr_ms_set_init();
  olsr_neighbor_set_init();
  olsr_neighbor2_set_init();
  olsr_topology_set_init();
  olsr_routing_set_init();

  // Initialize services:
  olsr_send_init();
  olsr_receive_init();
  olsr_hello_init();
  olsr_tc_init();
}
