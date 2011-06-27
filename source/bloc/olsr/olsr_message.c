#include <string.h>
#include "olsr_hello.h"
#include "olsr_duplicate_set.h"
#include "olsr_link_set.h"
#include "olsr_message.h"
#include "olsr_neighbor_set.h"
#include "olsr_ms_set.h"
#include "olsr_send.h"
#include "olsr_state.h"
#include "olsr_topology_set.h"

void
olsr_message_append(olsr_message_t* msg, void* data, int size_bytes)
{
  if (MAX_MESSAGE_CONTENT_SIZE - msg->content_size < size_bytes)
  {
    DEBUG_SEND("MAX_MESSAGE_CONTENT_SIZE overflow");
    exit(1);
  }

  memcpy(msg->content + msg->content_size, data, size_bytes);
  msg->header.size += size_bytes;
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
      if (!olsr_already_forwarded(header->addr, header->sn, iface))
        olsr_default_forward(message, size, iface);
      break;
    case MID_MESSAGE:
    case HNA_MESSAGE:
    default:
      DEBUG_RECEIVE("unexpected message forwarding");
      exit(1);
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
      if (!olsr_already_processed(header->addr, header->sn))
        olsr_process_tc_message(message, size, iface);
      break;
    case MID_MESSAGE:
    case HNA_MESSAGE:
    default:
      DEBUG_RECEIVE("unexpected message processing");
      exit(1);
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

/*
     1    If the sender interface address of the message is not detected
          to be in the symmetric 1-hop neighborhood of the node, the
          forwarding algorithm MUST silently stop here (and the message
          MUST NOT be forwarded).
*/

  bool symetric_1hop = FALSE;
  FOREACH_LINK_EREW(
    l,
    if (l->L_neighbor_iface_addr == header->source_addr
        && l->L_SYM_time > olsr_get_current_time())
    {
      symetric_1hop = TRUE;
      break;
    });

  if (!symetric_1hop)
    return;

  /*
          2    If there exists a tuple in the duplicate set where:

               D_addr    == Originator Address

               D_seq_num == Message Sequence Number

          Then the message will be further considered for forwarding if
          and only if:

               D_retransmitted is false, AND

               the (address of the) interface which received the message
               is not included among the addresses in D_iface_list
   */

  if (!olsr_has_to_be_forwarded(header->addr, header->sn, iface))
    return;

  bool must_be_retransmited =
    olsr_is_ms(olsr_iface_to_main_address(header->source_addr))
    && header->ttl > 1;

  bool exists = FALSE;
  FOREACH_DUPLICATE_EREW(tuple,
    if (tuple->addr == header->addr && tuple->sn == header->sn)
    {
      tuple->time = olsr_get_current_time() + DUP_HOLD_TIME_S;
      bool has_iface = FALSE;
      for (int i = 0; i < tuple->n_ifaces; i++)
        if (tuple->ifaces[i] == iface)
          has_iface = TRUE;
      if (!has_iface)
        tuple->ifaces[tuple->n_ifaces++] = iface;
      tuple->retrans = must_be_retransmited;
      exists = TRUE;
      break;
    });

  if (!exists)
  {
    olsr_duplicate_tuple_t tuple =
      {
        .addr = header->addr,
        .sn = header->sn,
        .time = olsr_get_current_time() + olsr_seconds_to_time(DUP_HOLD_TIME_S),
        .ifaces = {0},
        .n_ifaces = 1,
        .retrans = must_be_retransmited,
      };
    tuple.ifaces[0] = iface;
    olsr_duplicate_set_insert(&tuple);
  }

  if (!must_be_retransmited)
  {
    DEBUG_RECEIVE("not retransmiting");
    return;
  }

  new_header.ttl--;
  new_header.hops++;

  for (int iface = 0; iface < IFACES_COUNT; iface++)
  {
    new_header.source_addr = state.iface_addresses[iface];
    olsr_send_message_content_(&new_header,
                               message + sizeof(olsr_message_hdr_t),
                               size - sizeof(olsr_message_hdr_t),
                               iface);
  }
}
