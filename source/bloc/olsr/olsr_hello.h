#ifndef OLSR_HELLO_H
# define OLSR_HELLO_H

# include <assert.h>
# include <stdint.h>
# include <stm32f10x.h>
# include "olsr_ifaces.h"
# include "olsr_time.h"
# include "olsr_types.h"
# include "olsr_constants.h"

/*
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |          Reserved             |     Htime     |  Willingness  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   Link Code   |   Reserved    |       Link Message Size       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                  Neighbor Interface Address                   |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                  Neighbor Interface Address                   |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      :                             .  .  .                           :
      :                                                               :
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   Link Code   |   Reserved    |       Link Message Size       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                  Neighbor Interface Address                   |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                  Neighbor Interface Address                   |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      :                                                               :
      :                                       :
   (etc.)
 */

typedef struct
{
  uint16_t reserved;
  time_t Htime;
  willingness_t willingness;
} olsr_hello_message_hdr_t;

typedef struct
{
  uint8_t link_code;
  uint8_t reserved;
  uint16_t size;
} olsr_link_message_hdr_t;

inline uint8_t
olsr_link_code(link_type_t lt, neighbor_type_t nt)
{
  assert(nt < 4);
  assert(lt < 4);
  return (nt << 2) | lt;
}

inline link_type_t
olsr_link_type(uint8_t link_code)
{
  return link_code & 0xf;
}

inline neighbor_type_t
olsr_neighbor_type(uint8_t link_code)
{
  return (link_code >> 2) & 0xf;
}

void olsr_hello_task(void* pvParameters);
void olsr_process_hello_message(packet_byte_t* message, int size,
                                interface_t iface);

#endif