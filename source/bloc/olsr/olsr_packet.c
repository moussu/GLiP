#include "olsr_packet.h"

void
olsr_process_packet(olsr_packet_t* packet, interface_t iface)
{
  packet_byte_t* messages = packet->content;

  olsr_message_hdr_t* message_header = (olsr_message_hdr_t*)messages;
  /* 1    If the packet contains no messages (i.e., the Packet Length is
     less than or equal to the size of the packet header), the
     packet MUST silently be discarded. */

  if (packet->header.length <= sizeof(olsr_packet_hdr_t))
    return;

  do
  {
    olsr_dispatch_message((packet_byte_t*)message_header,
                          message_header->size, iface);
    message_header += message_header->size;
  } while ((packet_byte_t*)message_header <
           (packet_byte_t*)&packet->header + packet->header.length);

}
