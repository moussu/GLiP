#ifndef OLSR_SEND_H
# define OLSR_SEND_H

# include "olsr_ack.h"
# include "olsr_message.h"

void olsr_send_init();
void olsr_send_message_content(olsr_message_hdr_t* header,
                               packet_byte_t* content,
                               int content_size,
                               interface_t iface);
void olsr_send_message_content_(olsr_message_hdr_t* header,
                                packet_byte_t* content,
                                int content_size,
                                interface_t iface);

void olsr_send_message(olsr_message_t* message, interface_t iface);
void olsr_send_message_(olsr_message_t* message, interface_t iface);
void olsr_send_message__(olsr_message_t* message, interface_t iface);

bool olsr_send_to_(olsr_message_t* message, olsr_time_t Vtime, address_t addr);
bool olsr_send_to(olsr_message_t* message, olsr_time_t Vtime, address_t addr);
bool olsr_send_to_ack_async(olsr_message_t* message, olsr_time_t Vtime,
                            address_t addr, olsr_ack_t* ack);
bool olsr_send_to_ack(olsr_message_t* message, olsr_time_t Vtime,
                      address_t addr, olsr_ack_t* ack, int timeout_ms,
                      int max_tries);
bool olsr_forward_to(olsr_message_t* message);
void olsr_broadcast(olsr_message_t* message);

#endif
