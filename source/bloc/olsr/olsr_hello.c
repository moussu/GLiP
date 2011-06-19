#include <FreeRTOS.h>
#include <task.h>
#include "max.h"
#include "olsr_hello.h"
#include "olsr_message.h"
#include "olsr_constants.h"
#include "olsr_link_set.h"
#include "olsr_state.h"
#include "olsr_time.h"

/*
  Upon receiving a HELLO message, the "validity time" MUST be computed
  from the Vtime field of the message header (see section 3.3.2).
  Then, the Link Set SHOULD be updated as follows:

     1    Upon receiving a HELLO message, if there exists no link tuple
          with

               L_neighbor_iface_addr == Source Address

          a new tuple is created with

               L_neighbor_iface_addr = Source Address

               L_local_iface_addr    = Address of the interface
                                       which received the
                                       HELLO message

               L_SYM_time            = current time - 1 (expired)

               L_time                = current time + validity time

     2    The tuple (existing or new) with:

               L_neighbor_iface_addr == Source Address

          is then modified as follows:

          2.1  L_ASYM_time = current time + validity time;

          2.2  if the node finds the address of the interface which
               received the HELLO message among the addresses listed in
               the link message then the tuple is modified as follows:

               2.2.1
                    if Link Type is equal to LOST_LINK then

                         L_SYM_time = current time - 1 (i.e., expired)

               2.2.2
                    else if Link Type is equal to SYM_LINK or ASYM_LINK
                    then

                         L_SYM_time = current time + validity time,

                         L_time     = L_SYM_time + NEIGHB_HOLD_TIME

          2.3  L_time = max(L_time, L_ASYM_time)

   The above rule for setting L_time is the following: a link losing its
   symmetry SHOULD still be advertised during at least the duration of
   the "validity time" advertised in the generated HELLO.  This allows
   neighbors to detect the link breakage.
 */

void
olsr_process_hello_message(packet_byte_t* message, int size,
                           interface_t iface)
{
  olsr_message_hdr_t* header = (olsr_message_hdr_t*)message;
  time_t Vtime = olsr_deserialize_time(header->Vtime);
  olsr_link_tuple_t* tuple = olsr_link_set_has(header->addr);

  if (tuple == NULL)
  {
    olsr_link_tuple_t t =
      {
        .L_neighbor_iface_addr = header->addr,
        .L_local_iface_addr    = state.iface_addresses[iface],
        .L_SYM_time            = olsr_get_current_time() - 1,
        .L_time                = olsr_get_current_time() + Vtime,
      };
    tuple = olsr_link_set_insert(&t);
  }

  if (tuple == NULL)
    return;

  tuple->L_ASYM_time = olsr_get_current_time() + Vtime;

  packet_byte_t* cursor = (packet_byte_t*)message
    + sizeof(olsr_message_hdr_t);

  packet_byte_t* end = (packet_byte_t*)message +  header->size;

  while (cursor < end)
  {
    olsr_link_message_hdr_t* link_header =
      (olsr_link_message_hdr_t*)cursor;

    packet_byte_t* link_content =
      (packet_byte_t*)link_header
      + sizeof(olsr_link_message_hdr_t);

    bool breaked = FALSE;

    for (int i = 0; i < link_header->size; i += sizeof(address_t))
    {
      address_t addr = *(address_t*)(link_content + i);
      if (addr == state.iface_addresses[iface])
      {
        link_type_t lt = olsr_link_type(link_header->link_code);

        if (lt == LOST_LINK)
          tuple->L_SYM_time = olsr_get_current_time() - 1;
        else // SYM_LINK or ASYM_LINK
        {
          tuple->L_SYM_time = olsr_get_current_time() + Vtime;
          tuple->L_time = tuple->L_SYM_time
            + olsr_seconds_to_time(NEIGHB_HOLD_TIME_S);
        }

        breaked = TRUE;
        break;
      }
    }

    if (breaked)
      break;

    cursor += link_header->size;
  }

  tuple->L_time = MAX(tuple->L_time, tuple->L_ASYM_time);
}

void
olsr_hello_task(void* pvParameters)
{
  portTickType xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    for (int iface = 0; iface < IFACES_COUNT; iface++)
      olsr_send_hello(iface);

    vTaskDelayUntil(&xLastWakeTime, HELLO_INTERVAL_S * 1000);
  }
}
