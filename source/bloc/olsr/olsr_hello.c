#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "utils/max.h"
#include "olsr_hello.h"
#include "olsr_message.h"
#include "olsr_constants.h"
#include "olsr_link_set.h"
#include "olsr_neighbor2_set.h"
#include "olsr_mpr_set.h"
#include "olsr_ms_set.h"
#include "olsr_state.h"
#include "olsr_send.h"
#include "olsr_time.h"

static xSemaphoreHandle process_generate_mutex;
/*static xSemaphoreHandle force_send_mutex;
  static bool force_send = FALSE;*/
static void olsr_hello_task1(void* pvParameters);
//static void olsr_hello_task2(void* pvParameters);


void
olsr_hello_init()
{
  //force_send_mutex = xSemaphoreCreateMutex();
  process_generate_mutex = xSemaphoreCreateMutex();
  xTaskCreate(olsr_hello_task1,
              (signed portCHAR*) "helloTask1",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);
  /*xTaskCreate(olsr_hello_task2,
    (signed portCHAR*) "helloTask2",
    configMINIMAL_STACK_SIZE, NULL,
    tskIDLE_PRIORITY, NULL);*/
}

void
olsr_hello_mutex_take()
{
  xSemaphoreTake(process_generate_mutex, portMAX_DELAY);
}

void
olsr_hello_mutex_give()
{
  xSemaphoreGive(process_generate_mutex);
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
  bool inserted = FALSE;
  olsr_hello_message_hdr_t* hello_header =
    (olsr_hello_message_hdr_t*)message;
  message += sizeof(olsr_hello_message_hdr_t);

  DEBUG_HELLO("hello message [size:%d, sn:%d]",
              header->size, header->sn);
  DEBUG_INC;

  olsr_hello_mutex_take();

  // Link set:

  tuple = olsr_link_set_has(header->addr);

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
    inserted = TRUE;
  }

  tuple->L_ASYM_time = olsr_get_current_time() + Vtime;

  {
    DEBUG_HELLO("updating link set:");
    DEBUG_INC;

    packet_byte_t* cursor = message;
    packet_byte_t* end = (packet_byte_t*)header + header->size;

    DEBUG_HELLO("parsing link messages starting at %p, stopping at %p",
                cursor, end);

#ifdef DEBUG
    int i = 0;
#endif

    while (cursor < end)
    {
      olsr_link_message_hdr_t* link_header =
        (olsr_link_message_hdr_t*)cursor;

      DEBUG_HELLO("link message [n:%d, size:%d, cursor:%p]",
                  i++, link_header->size, cursor);

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

    DEBUG_HELLO("no more link message");

    tuple->L_time = MAX(tuple->L_time, tuple->L_ASYM_time);

    if (!inserted)
    {
      DEBUG_HELLO("tuple (%p) was updated not inserted", tuple);
      DEBUG_HELLO("call link set update callbacks");
      DEBUG_INC;
      olsr_link_set_updated(tuple);
      DEBUG_DEC;
    }

    DEBUG_DEC;
  }

  // Neighbor set:

  FOREACH_NEIGHBOR(neighbor,
    if (neighbor->N_neighbor_main_addr == header->addr)
      neighbor->N_willingness = hello_header->willingness);

  // 2-hop neighbor set:
  if (olsr_is_symetric_neighbor(header->addr))
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

      for (int i = 0; i < link_header->size; i += sizeof(address_t))
      {
        address_t addr = *(address_t*)(link_content + i);

        if (nt == SYM_NEIGH || nt == MPR_NEIGH)
        {
          if(addr == state.address)
            continue;

          olsr_neighbor2_tuple_t tuple;
          olsr_neighbor2_tuple_init(&tuple);
          tuple.N_neighbor_main_addr = header->addr;
          tuple.N_2hop_addr = addr;
          tuple.N_time = olsr_get_current_time() + Vtime;
          olsr_neighbor2_set_insert(&tuple);
        }
        else // NOT_NEIGH
        {
          FOREACH_NEIGHBOR2(n,
            if (n->N_neighbor_main_addr == header->addr
                && n->N_2hop_addr == addr)
              olsr_neighbor2_set_delete(__i_neighbor2));
        }
      }
      cursor += link_header->size;
    }
  }

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

      for (int i = 0; i < link_header->size; i += sizeof(address_t))
      {
        address_t addr = *(address_t*)(link_content + i);

        if (nt == MPR_NEIGH)
        {
          for (int i = 0; i < IFACES_COUNT; i++)
          {
            if (state.iface_addresses[i] == addr)
            {
              olsr_ms_tuple_t* ms = NULL;
              FOREACH_MS(m,
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

  olsr_hello_mutex_give();

  DEBUG_DEC;
}

static void
olsr_hello_send_ifaces()
{
  for (int iface = 0; iface < IFACES_COUNT; iface++)
  {
    DEBUG_HELLO("generate hello for iface %c",
                olsr_iface_print(iface));
    DEBUG_INC;
    olsr_send_hello(iface);
    DEBUG_DEC;
  }
  //force_send = FALSE;
}

void
olsr_send_hello(interface_t iface)
{
  // use the same willingness for everyone!
  const willingness_t willingness = state.willingness;
  address_t iface_address = state.iface_addresses[iface];
  link_type_t link_type;
  neighbor_type_t neighbor_type;

  olsr_message_t hello_message;

  hello_message.header.type = HELLO_MESSAGE;
  hello_message.header.Vtime = olsr_serialize_time(
    olsr_seconds_to_time(NEIGHB_HOLD_TIME_S)
    );
  hello_message.header.ttl = 1;
  hello_message.header.size = sizeof(olsr_message_hdr_t);
  hello_message.content_size = 0;

  olsr_hello_message_hdr_t hello_header;
  hello_header.Htime = olsr_serialize_time(
    olsr_seconds_to_time(HELLO_INTERVAL_S));
  hello_header.willingness = willingness;

  olsr_message_append(&hello_message, &hello_header,
                      sizeof(olsr_hello_message_hdr_t));

  olsr_link_message_hdr_t link_header;
  link_header.size = sizeof(olsr_link_message_hdr_t) + sizeof(address_t);
  DEBUG_HELLO("hello message size (headers only) is %d", hello_message.header.size);

#ifdef DEBUG
  int i = 0;
#endif
  DEBUG_HELLO("packing link tuples");
  DEBUG_HELLO("link set size is %d", link_set.n_tuples);
  DEBUG_INC;

  olsr_hello_mutex_take();

  FOREACH_LINK(t,
    if (t->L_local_iface_addr != iface_address)
      continue;

    if (t->L_SYM_time >= olsr_get_current_time())
      link_type = SYM_LINK;
    else if (t->L_ASYM_time >= olsr_get_current_time())
      link_type = ASYM_LINK;
    else
      link_type = LOST_LINK;

    address_t neighbor_main_address =
      olsr_iface_to_main_address(t->L_neighbor_iface_addr);

    // Mark neighbors as advertised, grab neighbor_type:
    olsr_neighbor_set_advertised(neighbor_main_address,
                                 &neighbor_type);

    // If is MPR, alter neighbor_type:
    if (olsr_is_mpr(neighbor_main_address))
      neighbor_type = MPR_NEIGH;

    // Here I'm assuming that if the neighbor_main_address is not in
    // the MPR set it WILL be in the neighbor set...

    DEBUG_HELLO("tuple [n:%d]", i++);

    link_header.link_code = olsr_link_code(link_type, neighbor_type);
    olsr_message_append(&hello_message, &link_header,
                        sizeof(olsr_link_message_hdr_t));
    olsr_message_append(&hello_message, &t->L_neighbor_iface_addr,
                        sizeof(address_t)));

  DEBUG_DEC;

  DEBUG_HELLO("neighbor set size is %d,", neighbor_set.n_tuples);
  DEBUG_HELLO("append non advertised neighbors with link type UNSPEC_LINK");
  DEBUG_INC;

  olsr_advertise_neighbors(&hello_message);

  DEBUG_DEC;

  DEBUG_HELLO("hello message size (headers + content) is %d", hello_message.header.size);

  olsr_hello_mutex_give();

  olsr_send_message(&hello_message, iface);
}

void
olsr_hello_force_send()
{
  // FIXME: implement.
  //force_send = TRUE;
}

static void
olsr_hello_task1(void* pvParameters)
{
  portTickType xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //xSemaphoreTake(force_send_mutex, portMAX_DELAY);
    olsr_hello_send_ifaces();

    //xSemaphoreGive(force_send_mutex);

    vTaskDelayUntil(&xLastWakeTime,
                    HELLO_INTERVAL_S * 1000 - MAXJITTER_MS);
  }
}

/*static void
olsr_hello_task2(void* pvParameters)
{
  portTickType xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    xSemaphoreTake(force_send_mutex, portMAX_DELAY);

    if (force_send)
      olsr_hello_send_ifaces();

    xSemaphoreGive(force_send_mutex);

    vTaskDelayUntil(&xLastWakeTime, 100);
  }
  }*/
