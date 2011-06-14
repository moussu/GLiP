#ifndef OLSR_SEND_H
# define OLSR_SEND_H

# include "olsr_message.h"

void olsr_send_init();
void olsr_send_message_content(olsr_message_hdr_t* header,
                               packet_byte_t* content,
                               int content_size, interface_t iface);
void olsr_send_message(olsr_message_t* message, interface_t iface);
void olsr_send_task(void* pvParameters);

#endif
