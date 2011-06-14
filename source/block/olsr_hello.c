#include <FreeRTOS.h>
#include <task.h>
#include "olsr_hello.h"
#include "olsr_constants.h"
#include "olsr_link_set.h"

void
olsr_hello_task(void* pvParameters)
{
  portTickType xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    for (int iface = 0; iface < IFACES_COUNT; iface++)
      olsr_send_hello(iface);

    vTaskDelayUntil(&xLastWakeTime, HELLO_INTERVAL_S * 1000);
  }
}
