#include "olsr_message.h"
#include "olsr_packet.h"

void
olsr_process_packet(olsr_packet_t* packet, interface_t iface)
{
  /* 1    If the packet contains no messages (i.e., the Packet Length is
     less than or equal to the size of the packet header), the
     packet MUST silently be discarded. */

  if (packet->header.length <= sizeof(olsr_packet_hdr_t))
    return;

  packet_byte_t* message = packet->content;

  if (packet->header.length != packet->content_size
      + sizeof(olsr_packet_hdr_t))
  {
    WARNING("packet header length and packet content size "
            "are not consistent %d != %d", packet->header.length,
            (int)(packet->content_size + sizeof(olsr_packet_hdr_t)));
  }

  for (;;)
  {
    olsr_message_hdr_t* header = (olsr_message_hdr_t*)message;

    // Check our pointer is INSIDE the packet:
    if (message >= (packet->content + packet->content_size))
      break;

    // Check header is complete:
    if ((message + sizeof(olsr_message_hdr_t))
        > (packet->content + packet->content_size))
    {
      ERROR("incomplete header in packet");
      break;
    }

    // Check the whole message content is in the packet:
    if ((message + header->size)
        > (packet->content + packet->content_size))
    {
      ERROR("incomplete message in packet");
      break;
    }

    // Avoid empty messages to infinite loop
    if (header->size < 1)
    {
      ERROR("empty message in packet");
      break;
    }

    DEBUG_RECEIVE("message [type:%s, addr:%d, sn:%d]",
                  olsr_message_type_str(header->type),
                  header->addr, header->sn);

    DEBUG_INC;
    olsr_dispatch_message(message, header->size, iface);
    DEBUG_DEC;

    message += header->size;
  }

}
