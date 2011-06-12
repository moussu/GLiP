#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "routing.h"
#include "lfsr.h"

static olsr_node_state_t state;
static xQueueHandle send_queue;
static xQueueHandle receive_queue;

void
olsr_init(address_t address)
{
  send_queue    = xQueueCreate(QUEUES_SIZE, sizeof(olsr_message_t));
  receive_queue = xQueueCreate(QUEUES_SIZE, sizeof(olsr_message_t));
  state.address = address;
  olsr_duplicate_set_init(&state.duplicate_set);
}

void
olsr_process_packet(packet_byte_t* packet, int length, interface_t iface)
{

  olsr_packet_hdr_t* packet_header = (olsr_packet_hdr_t*)packet;
  packet_byte_t* messages = (packet_byte_t*)(packet + sizeof(olsr_packet_hdr_t));
  olsr_message_hdr_t* message_header = (olsr_message_hdr_t*)messages;
  /* 1    If the packet contains no messages (i.e., the Packet Length is
     less than or equal to the size of the packet header), the
     packet MUST silently be discarded. */

  if (packet_header->length <= sizeof(olsr_packet_hdr_t))
    return;

  // Avoid pathologic case [not in RFC]
  if (packet_header->length != length)
    return;

  do
  {
    olsr_dispatch_message((packet_byte_t*)message_header,
                        message_header->size, iface);
    message_header += message_header->size;
  } while ((packet_byte_t*)message_header <
           (packet_byte_t*)packet_header + packet_header->length);

}

void
olsr_forward_message(packet_byte_t* message, int size, interface_t iface)
{
  const olsr_message_hdr_t* header = (olsr_message_hdr_t*)message;
  switch (header->type)
  {
    case HELLO_MESSAGE:
      return;
    case TC_MESSAGE:
    case MID_MESSAGE:
    case HNA_MESSAGE:
    default:
      olsr_default_forward(message, size, iface);
  }
}

void
olsr_process_message(packet_byte_t* message, int size, interface_t iface)
{
  const olsr_message_hdr_t* header = (olsr_message_hdr_t*)message;
  switch (header->type)
  {
    case HELLO_MESSAGE:
      olsr_process_hello_message(message, size, iface);
      break;
    case TC_MESSAGE:
    case MID_MESSAGE:
    case HNA_MESSAGE:
    default:
      return;
  }
}

void
olsr_dispatch_message(packet_byte_t* message, int size, interface_t iface)
{
  const olsr_message_hdr_t* header = (olsr_message_hdr_t*)message;

  /* If the time to live of the message is less than or equal to
     '0' (zero), or if the message was sent by the receiving node
     (i.e., the Originator Address of the message is the main
     address of the receiving node): the message MUST silently be
     dropped. */

  if (header->ttl <= 0 || header->addr == state.address)
    return;

  if (!olsr_already_processed(header->addr, header->sn))
    olsr_process_message(message, size, iface);

  if (!olsr_already_forwarded(header->addr, header->sn, iface))
    olsr_forward_message(message, size, iface);
}

void
olsr_default_forward(packet_byte_t* message, int size, interface_t iface)
{
  const olsr_message_hdr_t* header = (olsr_message_hdr_t*)message;
  olsr_message_hdr_t new_header = *header;

  int position = -1;

  if (!olsr_one_hop_symetric_neighbor(header->addr))
    return;

  if (!olsr_has_to_be_forwarded(header->addr, header->sn, iface, &position))
    return;

  bool will_be_retransmited =
    olsr_is_mpr_selector(header->addr) && header->ttl > 1;

  if (position > -1)
  {
    olsr_duplicate_tuple_t* tuple =
      state.duplicate_set.tuples + position;

    tuple->time = get_current_time() + DUP_HOLD_TIME_S;

    // Interface has already proven not to belong to ifaces array.
    // It is not possible to have more than 4 interfaces, so no test
    // on value of n_ifaces...
    tuple->ifaces[tuple->n_ifaces++] = iface;

    if (will_be_retransmited)
      tuple->retrans = TRUE;
  }
  else if (state.duplicate_set.n_tuples < DUPLICATE_SET_MAX_SIZE)
  {
    olsr_duplicate_tuple_t* tuple =
      state.duplicate_set.tuples + state.duplicate_set.n_tuples;

    tuple->addr = header->addr;
    tuple->sn = header->sn;
    tuple->time = get_current_time() + DUP_HOLD_TIME_S;
    tuple->ifaces[0] = iface;
    tuple->n_ifaces = 1;
    tuple->retrans = will_be_retransmited;

    state.duplicate_set.n_tuples++;
  }

  if (!will_be_retransmited)
    return;

  new_header.ttl--;
  new_header.hops++;

  for (int i = 0; i < 4; i++)
    send_message(&new_header, message + sizeof(olsr_message_hdr_t),
                 size - sizeof(olsr_message_hdr_t));
}

void
send_message(olsr_message_hdr_t* header, packet_byte_t* content,
             int content_size)
{
  olsr_message_t message;
  if (content_size > MAX_MESSAGE_CONTENT_SIZE)
    return;
  message.header = *header;
  message.content_size = content_size;
  memcpy(message.content, content, content_size);
  xQueueSend(send_queue, &message, portMAX_DELAY);
}

void
send_task(void* pvParameters)
{
  olsr_packet_t packet;
  packet_byte_t* write_position;
  olsr_message_t message;

  packet.header.sn = 0;

  for (;;)
  {
    vTaskDelay(MAXJITTER_MS * lfsr(32) / (~(uint32_t)0));

    write_position = packet.content;
    while (xQueueReceive(send_queue, &message, 0))
    {
      memcpy(write_position, &message.header, sizeof(olsr_message_hdr_t));
      write_position += sizeof(olsr_message_hdr_t);
      memcpy(write_position, message.content, message.content_size);
      write_position += message.content_size;
    }

    packet.header.length = write_position - packet.content;
    if (packet.header.length > 0)
    {
      packet.header.length += sizeof(olsr_packet_hdr_t);
      packet.header.sn++;
      //FIXME: SEND PACKET USING DATA LINK LAYER
    }
  }
}

