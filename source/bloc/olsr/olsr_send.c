#include <stdlib.h>

#include <FreeRTOS.h>
#include <string.h>
#include <queue.h>
#include <task.h>

#include "comm/simulator.h"
#include "utils/lfsr.h"
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

static void
olsr_send_message_copy(olsr_message_t* message,
                       olsr_message_hdr_t* header,
                       packet_byte_t* content,
                       int content_size)
{
  if (content_size > MAX_MESSAGE_CONTENT_SIZE)
  {
    ERROR("MAX_MESSAGE_CONTENT_SIZE overflow");
    // FIXME: here we maybe need to put a special pattern
    // at the end of packets to avoid one to be processed
    // when not complete because of the overflow.
    return;
  }
  if (header->ttl == 0)
  {
    ERROR("sending message with TTL = 0");
    return;
  }
  message->header = *header;
  message->header.size = content_size + sizeof(olsr_message_hdr_t);
  message->content_size = content_size;
  memcpy(message->content, content, content_size);
}

void
olsr_send_message_content(olsr_message_hdr_t* header,
                          packet_byte_t* content,
                          int content_size,
                          interface_t iface)
{
  olsr_message_t message;
  olsr_send_message_copy(&message, header, content, content_size);
  olsr_send_message(&message, iface);
}

void
olsr_send_message_content_(olsr_message_hdr_t* header,
                           packet_byte_t* content,
                           int content_size,
                           interface_t iface)
{
  olsr_message_t message;
  olsr_send_message_copy(&message, header, content, content_size);
  olsr_send_message_(&message, iface);
}

void
olsr_send_message(olsr_message_t* message, interface_t iface)
{
  message->header.addr = state.address;
  message->header.hops = 0;
  olsr_send_message_(message, iface);
}

void
olsr_send_message_(olsr_message_t* message, interface_t iface)
{
  static int message_sn = 0;

  // Just in case it hasn't been already done:
  message->header.size = message->content_size +
    sizeof(olsr_message_hdr_t);
  message->header.source_addr = state.iface_addresses[iface];
  message->header.sn = message_sn++;
  DEBUG_SEND("putting message[sn:%d, size:%d] in sending queue",
             message->header.sn, (int)message->header.size);
  xQueueSend(send_queues[iface], message, portMAX_DELAY);
}

static void
olsr_send_task(void* pvParameters)
{
  //static int current_message_sn = 0;
  olsr_packet_t packet;
  packet_byte_t* write_position;
  packet_byte_t* end __attribute__((unused));
  olsr_message_t message;

#ifdef WARNINGS
  portTickType xLastLastWakeTime = xTaskGetTickCount();
#endif

  portTickType xLastWakeTime = xTaskGetTickCount();

  packet.header.sn = 0;

  for (;;)
  {
    const int wait_time = MAXJITTER_MS * (lfsr(32) / (~(uint32_t)0));
    for (int iface = 0; iface < IFACES_COUNT; iface++)
    {
      vTaskDelayUntil(&xLastWakeTime,
                      (wait_time / IFACES_COUNT) / portTICK_RATE_MS);

      write_position = packet.content;
      end = write_position + MAX_PACKET_CONTENT_SIZE;

#ifdef DEBUG
      int i = 0;
#endif
      int message_count = 0;
      int message_length = 0;
      int content_length = 0;

      while (xQueueReceive(send_queues[iface], &message, 0))
      {
        if (message.header.ttl == 0)
        {
          DEBUG_SEND("TTL expired! ignoring message[sn:%d, size:%d]",
                     message.header.sn, message.header.size);
          continue;
        }

#ifdef DEBUG
        if (i == 0)
        {
          DEBUG_SEND("sending packets -> iface %s",
                     olsr_iface_str(iface));

          DEBUG_INC;

          DEBUG_SEND("packing messages in packet[sn:%d]",
                     packet.header.sn);

          DEBUG_INC;
        }
        i++;
#endif

#ifdef ERRORS
        switch (message.header.type)
        {
          case HELLO_MESSAGE:
          case TC_MESSAGE:
          case MID_MESSAGE:
          case HNA_MESSAGE:
            break;
          default:
            ERROR("sending packet with wrong type %d", message.header.type);
        }
#endif

        //message.header.sn = current_message_sn++;
        memcpy(write_position, &message.header, sizeof(olsr_message_hdr_t));
        write_position += sizeof(olsr_message_hdr_t);

        memcpy(write_position, message.content, message.content_size);
        write_position += message.content_size;

        DEBUG_SEND("packing message[sn:%d, size:%d] in packet",
                    message.header.sn, (int)message.header.size);

        message_length = message.content_size + sizeof(olsr_message_hdr_t);
        content_length += message_length;
        message_count++;

        if (content_length > MAX_PACKET_CONTENT_SIZE - MAX_MESSAGE_SIZE)
        {
          WARNING("packet max size of %d almost reached, packet considered full",
                  (int)MAX_PACKET_CONTENT_SIZE);
          break;
        }
      }

      packet.header.length = content_length;
      packet.content_size = content_length; // Should not be sent!

      if (packet.header.length > 0)
      {
        packet.header.length += sizeof(olsr_packet_hdr_t);
        packet.header.sn++;
        const int sent =
          simulator_send((char*)&packet, packet.header.length, iface);

        if (sent != packet.header.length)
          ERROR("packet was not sent entirely sent %d != size %d",
                sent, packet.header.length);

#ifdef DEBUG
        DEBUG_DEC;

        DEBUG_SEND("send packet[mess:%d, sn:%d, size:%d] -> iface %s",
                   message_count, packet.header.sn,
                   (int)packet.header.length, olsr_iface_str(iface));
        DEBUG_DEC;
#endif
      }

#ifdef WARNINGS
      if ((xTaskGetTickCount() - xLastLastWakeTime) * portTICK_RATE_MS
          > MAXJITTER_MS)
      {
        WARNING("sendTask delay exceeded: %dms when max is %dms",
                (xTaskGetTickCount() - xLastLastWakeTime) * portTICK_RATE_MS,
                MAXJITTER_MS);
      }

      xLastLastWakeTime = xTaskGetTickCount();
#endif
    }
  }
}
