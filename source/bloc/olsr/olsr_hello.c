#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "utils/max.h"
#include "olsr.h"
#include "olsr_hello.h"
#include "olsr_message.h"
#include "olsr_constants.h"
#include "olsr_ifaces.h"
#include "olsr_link_set.h"
#include "olsr_neighbor2_set.h"
#include "olsr_message.h"
#include "olsr_mpr_set.h"
#include "olsr_ms_set.h"
#include "olsr_state.h"
#include "olsr_send.h"
#include "olsr_time.h"

static xSemaphoreHandle force_send_mutex;
static void olsr_hello_task(void* pvParameters);

void
olsr_hello_init()
{
  force_send_mutex = xSemaphoreCreateRecursiveMutex();
  xTaskCreate(olsr_hello_task,
              (signed portCHAR*) "helloTask",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);
}

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
  message += sizeof(olsr_message_hdr_t);
  olsr_time_t Vtime = olsr_deserialize_time(header->Vtime);
  olsr_link_tuple_t* tuple = NULL;
  olsr_hello_message_hdr_t* hello_header =
    (olsr_hello_message_hdr_t*)message;
  message += sizeof(olsr_hello_message_hdr_t);

  DEBUG_HELLO("hello message [size:%d, sn:%d, Vtime:%d]",
              header->size, header->sn, Vtime);
  DEBUG_INC;

  olsr_global_mutex_take();

  // Link set:

  tuple = olsr_link_set_has(header->source_addr);

  if (tuple == NULL)
  {
    olsr_link_tuple_t t =
      {
        .L_neighbor_iface_addr = header->source_addr,
        .L_local_iface_addr    = state.iface_addresses[iface],
        .L_SYM_time            = olsr_get_current_time() - 1,
        .L_time                = olsr_get_current_time() + Vtime,
      };

    tuple = olsr_link_set_insert_(&t);
  }

  tuple->L_ASYM_time = olsr_get_current_time() + Vtime;

  {
    DEBUG_HELLO("populating link set:");
    DEBUG_INC;

    packet_byte_t* cursor = message;
    packet_byte_t* end = (packet_byte_t*)header + header->size;

    DEBUG_HELLO("parsing link messages starting at %p, stopping at %p",
                cursor, end);

    while (cursor < end)
    {
      olsr_link_message_hdr_t* link_header =
        (olsr_link_message_hdr_t*)cursor;

      link_type_t lt = olsr_link_type(link_header->link_code);

      DEBUG_HELLO("link tuple [size:%d, lt:%s, lc:0x%x]",
                  link_header->size,
                  olsr_link_type_str(lt),
                  link_header->link_code);

      packet_byte_t* link_content =
        (packet_byte_t*)link_header
        + sizeof(olsr_link_message_hdr_t);

      bool breaked = FALSE;

      const int addresses_byte_size = link_header->size
        - sizeof(olsr_link_message_hdr_t);
      for (int i = 0; i < addresses_byte_size; i += sizeof(address_t))
      {
        address_t addr = (*(address_t*)(link_content + i)) & 0xffff;

        DEBUG_HELLO("address %d", addr);
        DEBUG_INC;

        if (addr == state.iface_addresses[iface])
        {
          DEBUG_HELLO("address corresponds to receiving interface address");

          if (lt == LOST_LINK)
          {
            DEBUG_HELLO("LOST_LINK -> make link tuple SYM_time expire");
            tuple->L_SYM_time = olsr_get_current_time() - 1;
          }
          else if(lt == SYM_LINK || lt == ASYM_LINK)
          {
            DEBUG_HELLO("SYM_LINK or ASYM_LINK -> prolongate link tuple SYM_time");
            tuple->L_SYM_time = olsr_get_current_time() + Vtime;
            tuple->L_time = tuple->L_SYM_time
              + olsr_seconds_to_time(NEIGHB_HOLD_TIME_S);
          }
          // UNSPEC LINK should not be processed!

          breaked = TRUE;
          break;
        }
      }

      DEBUG_DEC;

      if (breaked)
        break;

      cursor += link_header->size;
    }

    DEBUG_HELLO("no more link message");

    tuple->L_time = MAX(tuple->L_time, tuple->L_ASYM_time);

    DEBUG_HELLO("call link set update callbacks");
    DEBUG_INC;
    olsr_link_set_updated(tuple);
    DEBUG_DEC;

    DEBUG_DEC;
  }

  // Neighbor set:
  FOREACH_NEIGHBOR(neighbor,
    if (neighbor->N_neighbor_main_addr == header->addr)
      neighbor->N_willingness = hello_header->willingness);

  // 2-hop neighbor set:
  DEBUG_HELLO("populating neighbor2 set:");
  DEBUG_INC;

  if (olsr_is_symetric_neighbor(header->addr))
  {
    DEBUG_HELLO("the neighbor is symetric");

    packet_byte_t* cursor = (packet_byte_t*)message;
    packet_byte_t* end = (packet_byte_t*)header + header->size;

    DEBUG_HELLO("parsing link messages starting at %p, stopping at %p",
                cursor, end);

    while (cursor < end)
    {
      olsr_link_message_hdr_t* link_header =
        (olsr_link_message_hdr_t*)cursor;

      packet_byte_t* link_content =
        (packet_byte_t*)link_header
        + sizeof(olsr_link_message_hdr_t);

      neighbor_type_t nt = olsr_neighbor_type(link_header->link_code);

      DEBUG_HELLO("link message [size:%d, cursor:%p, nt:%s]",
                  link_header->size, cursor,
                  olsr_neighbor_type_str(nt));
      DEBUG_INC;

      const int addresses_byte_size = link_header->size
        - sizeof(olsr_link_message_hdr_t);
      for (int i = 0; i < addresses_byte_size; i += sizeof(address_t))
      {
        address_t addr = (*(address_t*)(link_content + i)) & 0xffff;
        address_t main_addr = olsr_iface_to_main_address(addr);

        DEBUG_HELLO("iface address %d [i:%d]", addr, i);
        DEBUG_INC;

        if (nt == SYM_NEIGH || nt == MPR_NEIGH)
        {
          if(main_addr == state.address)
          {
            DEBUG_HELLO("this neighbor2 is me, continuing");
            DEBUG_DEC;
            continue;
          }

          DEBUG_HELLO("as it is SYM or MPR_NEIGH, inserting in neighbor2 set");

          olsr_neighbor2_tuple_t tuple;
          olsr_neighbor2_tuple_init(&tuple);
          tuple.N_neighbor_main_addr = header->addr;
          tuple.N_2hop_addr = main_addr;
          tuple.N_time = olsr_get_current_time() + Vtime;

          DEBUG_HELLO("[addr:%d, 2hop_addr:%d, time:%d]",
                      tuple.N_neighbor_main_addr,
                      tuple.N_2hop_addr,
                      tuple.N_time);

          olsr_neighbor2_set_insert_or_update(&tuple);
        }
        else // NOT_NEIGH
        {
          DEBUG_HELLO("as it is NOT_NEIGH, we delete all associated n2");

          FOREACH_NEIGHBOR2_EREW(n,
            if (n->N_neighbor_main_addr == header->addr
                && n->N_2hop_addr == addr)
            {
              DEBUG_HELLO("deleting [2hop_addr:%d]", n->N_2hop_addr);
              olsr_neighbor2_set_delete(__i_neighbor2);
            });
        }

        DEBUG_DEC;
      }

      DEBUG_DEC;

      cursor += link_header->size;
    }
  }
  else
    DEBUG_HELLO("the neighbor is NOT symetric, aborting");

  DEBUG_DEC;

  // MS set:
  {
    packet_byte_t* cursor = (packet_byte_t*)message;
    packet_byte_t* end = (packet_byte_t*)header + header->size;

    while (cursor < end)
    {
      olsr_link_message_hdr_t* link_header =
        (olsr_link_message_hdr_t*)cursor;

      packet_byte_t* link_content =
        (packet_byte_t*)link_header
        + sizeof(olsr_link_message_hdr_t);

      neighbor_type_t nt = olsr_neighbor_type(link_header->link_code);

      const int addresses_byte_size = link_header->size
        - sizeof(olsr_link_message_hdr_t);
      for (int i = 0; i < addresses_byte_size; i += sizeof(address_t))
      {
        address_t addr = (*(address_t*)(link_content + i)) & 0xffff;

        if (nt == MPR_NEIGH)
        {
          for (int i = 0; i < IFACES_COUNT; i++)
          {
            if (state.iface_addresses[i] == addr)
            {
              olsr_ms_tuple_t* ms = NULL;
              FOREACH_MS_EREW(m,
                if (m->MS_main_addr == header->addr)
                {
                  ms = m;
                  break;
                });
              if (!ms)
              {
                olsr_ms_tuple_t tuple;
                tuple.MS_main_addr = header->addr;
                ms = olsr_ms_set_insert(&tuple);
              }
              ms->MS_time = olsr_get_current_time() + Vtime;
            }
          }
        }
      }
      cursor += link_header->size;
    }
  }

  olsr_global_mutex_give();

  DEBUG_DEC;
}

static void
olsr_hello_send_ifaces()
{
  olsr_message_t hello_message;

  for (int iface = 0; iface < IFACES_COUNT; iface++)
  {
    DEBUG_HELLO("generate hello message for iface %c",
                olsr_iface_print(iface));
    DEBUG_INC;
    olsr_generate_hello(&hello_message, iface);
    DEBUG_DEC;

    DEBUG_HELLO("send it to iface %c", olsr_iface_print(iface));
    olsr_send_message(&hello_message, iface);
  }

  //force_send = FALSE;
}

void
olsr_generate_hello(olsr_message_t* hello_message, interface_t iface)
{
  // use the same willingness for everyone!
  const willingness_t willingness = state.willingness;
  address_t iface_address = state.iface_addresses[iface];
  link_type_t link_type;
  neighbor_type_t neighbor_type;

  hello_message->header.type = HELLO_MESSAGE;
  hello_message->header.Vtime = olsr_serialize_time(
    olsr_seconds_to_time(NEIGHB_HOLD_TIME_S));
  hello_message->header.ttl = 1;
  hello_message->header.size = sizeof(olsr_message_hdr_t);
  hello_message->content_size = 0;

  olsr_hello_message_hdr_t hello_header;
  hello_header.Htime = olsr_serialize_time(
    olsr_seconds_to_time(HELLO_INTERVAL_S));
  hello_header.willingness = willingness;

  olsr_message_append(hello_message, &hello_header,
                      sizeof(olsr_hello_message_hdr_t));

  olsr_link_message_hdr_t link_header;
  link_header.size = sizeof(olsr_link_message_hdr_t) + sizeof(address_t);
  DEBUG_HELLO("hello message size (headers only) is %d", hello_message->header.size);

  DEBUG_HELLO("packing link tuples");
  DEBUG_HELLO("link set size is %d", link_set.n_tuples);
  //olsr_link_set_print();

  DEBUG_INC;

  olsr_global_mutex_take();

  olsr_neighbor_reset_advertised();

  FOREACH_LINK_EREW(t,
    if (t->L_local_iface_addr != iface_address)
    {
      DEBUG_HELLO("link tuple is not related to iface %c, %d != %d, continuing",
                  iface, iface_address, t->L_local_iface_addr);
      continue;
    }

    if (t->L_SYM_time >= olsr_get_current_time())
    {
      link_type = SYM_LINK;
    }
    else if (t->L_ASYM_time >= olsr_get_current_time())
      link_type = ASYM_LINK;
    else
      link_type = LOST_LINK;

    address_t neighbor_main_address =
      olsr_iface_to_main_address(t->L_neighbor_iface_addr);

    neighbor_type = NOT_NEIGH;

    // Mark neighbors as advertised, grab neighbor_type:
    olsr_neighbor_set_advertised(neighbor_main_address,
                                 &neighbor_type);

    // If is MPR, alter neighbor_type:
    if (olsr_is_mpr(neighbor_main_address))
      neighbor_type = MPR_NEIGH;

    // Here I'm assuming that if the neighbor_main_address is not in
    // the MPR set it WILL be in the neighbor set...

    link_header.link_code = olsr_link_code(link_type, neighbor_type);

    DEBUG_HELLO("appending [size:%d, nt:%s, lt:%s, lc:0x%x, iface_addr:%d]",
                link_header.size,
                olsr_neighbor_type_str(neighbor_type),
                olsr_link_type_str(link_type),
                link_header.link_code,
                t->L_neighbor_iface_addr);

    olsr_message_append(hello_message, &link_header,
                        sizeof(olsr_link_message_hdr_t));
    olsr_message_append(hello_message, &t->L_neighbor_iface_addr,
                        sizeof(address_t)));

  DEBUG_DEC;

  DEBUG_HELLO("neighbor set size is %d,", neighbor_set.n_tuples);
  DEBUG_HELLO("append non advertised neighbors with link type UNSPEC_LINK");
  DEBUG_INC;

  olsr_advertise_neighbors(hello_message);

  DEBUG_DEC;

  DEBUG_HELLO("hello message size (headers + content) is %d", hello_message->header.size);

  olsr_global_mutex_give();
}

void
olsr_hello_force_send()
{
  // FIXME: doesn't work, deadlock...
  /*
  if(!xSemaphoreTake(force_send_mutex, 0))
    return;

  olsr_hello_send_ifaces();

  xSemaphoreGive(force_send_mutex);
  */
}

static void
olsr_hello_task(void* pvParameters)
{
  portTickType xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    xSemaphoreTake(force_send_mutex, portMAX_DELAY);

    olsr_hello_send_ifaces();

    xSemaphoreGive(force_send_mutex);

    vTaskDelayUntil(&xLastWakeTime,
                    HELLO_INTERVAL_S * 1000 - MAXJITTER_MS);
  }
}
