#include <string.h>
#include "olsr_hello.h"
#include "olsr_message.h"
#include "olsr_send.h"
#include "olsr_state.h"

void
olsr_message_append(olsr_message_t* msg, void* data, int size_bytes)
{
  if (MAX_MESSAGE_CONTENT_SIZE - msg->content_size < size_bytes)
    return;

  memcpy(msg->content + msg->content_size, data, size_bytes);
  msg->content_size += size_bytes;
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
      if (!olsr_already_forwarded(header->addr, header->sn, iface))
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
      if (!olsr_already_processed(header->addr, header->sn))
        ;
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

  olsr_process_message(message, size, iface);
  olsr_forward_message(message, size, iface);
}

void
olsr_default_forward(packet_byte_t* message, int size, interface_t iface)
{
  const olsr_message_hdr_t* header = (olsr_message_hdr_t*)message;
  olsr_message_hdr_t new_header = *header;

  int position = -1;

  if (!olsr_is_symetric_neighbor(&state.neighbor_set, header->addr))
    return;

  if (!olsr_has_to_be_forwarded(header->addr, header->sn, iface, &position))
    return;

  bool will_be_retransmited =
    olsr_is_ms(&state.ms_set, header->addr) && header->ttl > 1;

  if (position > -1)
  {
    olsr_duplicate_tuple_t* tuple =
      state.duplicate_set.tuples + position;

    tuple->time = olsr_get_current_time() + DUP_HOLD_TIME_S;

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
    tuple->time = olsr_get_current_time() + DUP_HOLD_TIME_S;
    tuple->ifaces[0] = iface;
    tuple->n_ifaces = 1;
    tuple->retrans = will_be_retransmited;

    state.duplicate_set.n_tuples++;
  }

  if (!will_be_retransmited)
    return;

  new_header.ttl--;
  new_header.hops++;

  for (int iface = 0; iface < IFACES_COUNT; iface++)
    olsr_send_message_content(&new_header, message + sizeof(olsr_message_hdr_t),
                              size - sizeof(olsr_message_hdr_t), iface);
}
