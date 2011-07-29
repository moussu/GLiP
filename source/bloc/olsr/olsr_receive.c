#include <string.h>
#include <errno.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <AsyncIOSocket.h>

#include "olsr.h"
#include "olsr_constants.h"
#include "olsr_message.h"
#include "olsr_packet.h"
#include "olsr_receive.h"
#include "olsr_routing_table.h"
#include "comm/simulator.h"

#if defined DEBUG || defined SIMULATOR_DEBUG
# include "olsr_duplicate_set.h"
# include "olsr_link_set.h"
# include "olsr_mpr_set.h"
# include "olsr_ms_set.h"
# include "olsr_neighbor_set.h"
# include "olsr_neighbor2_set.h"
# include "olsr_topology_set.h"
#endif


static xQueueHandle receive_queue;
static xQueueHandle receive_queues[IFACES_COUNT];
static void olsr_receive_task(void* pvParameters);
static bool queues_created = FALSE;

void
olsr_receive_init()
{
  receive_queue = xQueueCreate(4 * QUEUES_SIZE,
                               sizeof(xUDPPacket));

  for (int iface = 0; iface < IFACES_COUNT; iface++)
    receive_queues[iface] = xQueueCreate(QUEUES_SIZE,
                                         sizeof(olsr_packet_t));

  xTaskCreate(olsr_receive_task,
              (signed portCHAR*) "receiveTask",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);

  queues_created = TRUE;
}

void
olsr_receive_callback(int iSocket, void* pvContext)
{
  portBASE_TYPE xHigherTaskWoken = pdFALSE;
  static xUDPPacket xPacket;
  static olsr_packet_t packet;
  struct sockaddr_in xReceiveAddress;
  int length;
  interface_t iface;

  for (;;)
  {
    length = iSocketUDPReceiveISR(iSocket, &xPacket, &xReceiveAddress);

    if (length > -1)
      break;

    switch (errno)
    {
      case EINTR:
        break;
      default:
        fprintf(stderr, "Error %d in receive_callback: %s\n",
                errno, strerror(errno));
        goto isr;
    }
  }

  if (length < sizeof(olsr_packet_hdr_t))
    goto error;

  if (!queues_created)
    return;

  if (olsr_iface_parse(xPacket.ucPacket[0], &iface) != -1)
  {
    memcpy(&packet.header, xPacket.ucPacket + 1, sizeof(olsr_packet_hdr_t));
    packet.content_size = packet.header.length - sizeof(olsr_packet_hdr_t);
    memcpy(packet.content, xPacket.ucPacket + 1 + sizeof(olsr_packet_hdr_t),
           packet.content_size);

    if (length - 1 < packet.header.length)
    {
      WARNING("corrupted packet received from iface %s, "
              "length in header is %d and content size is %d, sn is %d",
              olsr_iface_str(iface), length - 1, packet.header.length,
              packet.header.sn);
      goto isr;
    }
    else if (length - 1 > packet.header.length)
    {
      WARNING("too long packet received %d bytes for a %d bytes packet",
              length - 1, packet.header.length);
    }

    if (pdPASS != xQueueSendFromISR(receive_queues[iface], &packet,
                                    &xHigherTaskWoken))
      goto error;
    goto isr;
  }

  error:
  WARNING("UDP Rx failed");
  isr:
  portEND_SWITCHING_ISR(xHigherTaskWoken);
}

static void
olsr_receive_task(void* pvParameters)
{
  olsr_packet_t packet;
#ifdef WARNINGS
  portTickType xLastLastWakeTime = xTaskGetTickCount();
#endif


#ifdef DEBUG
  int i __attribute__((unused)) = -1;
#endif

  for (;;)
  {
#ifdef DEBUG
    if ((i = (i + 1) % 100) == 0)
    {
      olsr_duplicate_set_print();
      olsr_link_set_print();
      olsr_mpr_set_print();
      olsr_ms_set_print();
      olsr_neighbor_set_print();
      olsr_neighbor2_set_print();
      olsr_topology_set_print();
      olsr_routing_table_print();
    }
    // FIXME: here it seems that the debug helps the concurrency. When
    // debug is disabled receive tasks seem to starve send tasks. A
    // simple vTaskDelay could solve things...
#endif

    for (int iface = 0; iface < IFACES_COUNT; iface++)
    {
      // Timeout should be 0 here, but if so task starves sending task...
      if(!xQueueReceive(receive_queues[iface], &packet,
                        RECV_INTERVAL_MS / portTICK_RATE_MS))
        continue;

      DEBUG_RECEIVE("received packet[size:%d, sn:%d] <- iface %s",
                    packet.header.length,
                    packet.header.sn,
                    olsr_iface_str(iface));
      DEBUG_INC;

      olsr_process_packet(&packet, iface);

      DEBUG_DEC;
    }

#ifdef WARNINGS
    // Here we set HELLO_INTERVAL_MS as the maximum refresh period
    // of this task as this is the smallest period involved in the
    // protocol. It guarantees that hello messages from all interfaces
    // have been properly processed.

    if ((xTaskGetTickCount() - xLastLastWakeTime) * portTICK_RATE_MS
        > HELLO_INTERVAL_MS)
    {
      WARNING("recvTask delay exceeded: %dms when max is %dms",
              (xTaskGetTickCount() - xLastLastWakeTime) * portTICK_RATE_MS,
              HELLO_INTERVAL_MS);
    }

    xLastLastWakeTime = xTaskGetTickCount();
#endif

    olsr_global_mutex_take();
    olsr_routing_table_compute();
    olsr_global_mutex_give();
  }
}
