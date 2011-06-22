#include <FreeRTOS.h>
#include <string.h>
#include <queue.h>
#include <task.h>
#include "comm/simulator.h"
#include "lfsr.h"
#include "olsr_constants.h"
#include "olsr_send.h"
#include "olsr_state.h"
#include "olsr_packet.h"

static xQueueHandle send_queues[IFACES_COUNT];
static void olsr_send_task(void* pvParameters);

void
olsr_send_init()
{
  for (int iface = 0; iface < IFACES_COUNT; iface++)
    send_queues[iface] = xQueueCreate(QUEUES_SIZE,
                                      sizeof(olsr_message_t));
  xTaskCreate(olsr_send_task,
              (signed portCHAR*) "sendTask",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);
}

void
olsr_send_message_content(olsr_message_hdr_t* header,
                          packet_byte_t* content,
                          int content_size, interface_t iface)
{
  olsr_message_t message;
  if (content_size > MAX_MESSAGE_CONTENT_SIZE)
    return;
  if (header->ttl == 0)
    return;
  message.header = *header;
  message.header.size = content_size + sizeof(olsr_message_hdr_t);
  message.content_size = content_size;
  memcpy(message.content, content, content_size);
  olsr_send_message(&message, iface);
}

void
olsr_send_message(olsr_message_t* message, interface_t iface)
{
  // Just in case it hasn't been done:
  message->header.size = message->content_size +
    sizeof(olsr_message_hdr_t);
  message->header.addr = state.iface_addresses[iface];
  message->header.hops = 0;
  xQueueSend(send_queues[iface], &message, portMAX_DELAY);
}

static void
olsr_send_task(void* pvParameters)
{
  static int current_message_sn = 0;
  olsr_packet_t packet;
  packet_byte_t* write_position;
  packet_byte_t* end;
  olsr_message_t message;
  portTickType xLastWakeTime = xTaskGetTickCount();

  packet.header.sn = 0;

  for (;;)
  {
    const int wait_time = MAXJITTER_MS * (lfsr(32) / (~(uint32_t)0));
    for (int iface = 0; iface < IFACES_COUNT; iface++)
    {
      vTaskDelayUntil(&xLastWakeTime, wait_time / IFACES_COUNT);

      write_position = packet.content;
      end = write_position + MAX_PACKET_CONTENT_SIZE;
      while (xQueueReceive(send_queues[iface], &message, 0))
      {
        if (write_position + message.content_size > end)
          break;

        if (message.header.ttl == 0)
          continue;

        message.header.sn = current_message_sn++;
        memcpy(write_position, &message.header,
               sizeof(olsr_message_hdr_t));
        write_position += sizeof(olsr_message_hdr_t);
        memcpy(write_position, message.content, message.content_size);

        write_position += message.content_size;
      }

      packet.header.length = write_position - packet.content;
      if (packet.header.length > 0)
      {
        packet.header.length += sizeof(olsr_packet_hdr_t);
        packet.header.sn++;
        simulator_send((char*)&packet, packet.header.length, iface);
      }
    }
  }
}
