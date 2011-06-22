#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "olsr_constants.h"
#include "olsr_receive.h"
#include "olsr_packet.h"
#include "comm/simulator.h"

static xQueueHandle receive_queues[IFACES_COUNT];
static void olsr_receive_task(void* pvParameters);

void
olsr_receive_init()
{
  for (int iface = 0; iface < IFACES_COUNT; iface++)
    receive_queues[iface] = xQueueCreate(QUEUES_SIZE,
                                         sizeof(olsr_packet_t));

  xTaskCreate(olsr_receive_task,
              (signed portCHAR*) "receiveTask",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);

}

static void
olsr_receive_task(void* pvParameters)
{
  packet_byte_t packet[MAX_PACKET_SIZE] = {0};
  interface_t iface;
  int length;

  for (;;)
  {
    length = simulator_receive((char*)packet, MAX_PACKET_SIZE, &iface);
    olsr_process_packet(packet, length, iface);
  }
}
