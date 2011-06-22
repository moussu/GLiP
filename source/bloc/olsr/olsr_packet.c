#include "olsr_packet.h"

void
olsr_process_packet(packet_byte_t* packet, int length, interface_t iface)
{

  olsr_packet_hdr_t* packet_header = (olsr_packet_hdr_t*)packet;
  packet_byte_t* messages =
    (packet_byte_t*)(packet + sizeof(olsr_packet_hdr_t));
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
