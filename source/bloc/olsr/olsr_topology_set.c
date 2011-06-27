#include "olsr.h"
#include "olsr_hello.h"
#include "olsr_link_set.h"
#include "olsr_ms_set.h"
#include "olsr_neighbor_set.h"
#include "olsr_send.h"
#include "olsr_state.h"
#include "olsr_topology_set.h"

SET_IMPLEMENT(topology, TOPOLOGY_SET_MAX_SIZE)

static xSemaphoreHandle force_send_mutex;
static void olsr_tc_task(void* pvParameters);
static bool tc_initialized = FALSE;

void
olsr_tc_init()
{
  force_send_mutex = xSemaphoreCreateMutex();
  xTaskCreate(olsr_tc_task,
              (signed portCHAR*) "tcTask",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);
  tc_initialized = TRUE;
}

void
olsr_topology_tuple_init(olsr_topology_tuple_t* tuple)
{
  tuple->T_dest_addr = 0;
  tuple->T_dest_iface = 0;
  tuple->T_last_addr = 0;
  tuple->T_last_iface = 0;
  tuple->T_seq = 0;
  tuple->T_time = 0;
}

int
olsr_generate_tc_message(olsr_message_t* tc_message)
{
  static seq_num_t current_ansn = 0;
  int n = 0;
  olsr_tc_message_hdr_t header;

  tc_message->header.type = TC_MESSAGE;
  tc_message->header.Vtime = olsr_serialize_time(
    olsr_seconds_to_time(TOP_HOLD_TIME_S));
  tc_message->header.ttl = 255;
  tc_message->header.size = sizeof(olsr_message_hdr_t);
  tc_message->content_size = 0;

  header.ansn = current_ansn++;
  DEBUG_TOPOLOGY("current ansn is %d", header.ansn);
  olsr_message_append(tc_message, &header, sizeof(olsr_tc_message_hdr_t));

  olsr_global_mutex_take();

  DEBUG_INC;
  FOREACH_MS_EREW(
    ms,
    DEBUG_TOPOLOGY("appending address %d", ms->MS_main_addr);
    olsr_link_tuple_t* link = NULL;
    FOREACH_LINK_EREW(
      l,
      if (olsr_iface_to_main_address(l->L_neighbor_iface_addr)
          == ms->MS_main_addr)
      {
        link = l;
        break;
      });

    if (!link)
      continue;

    olsr_message_append(tc_message, &(link->L_local_iface_addr),
                        sizeof(address_t));
    olsr_message_append(tc_message, &(link->L_neighbor_iface_addr),
                        sizeof(address_t));
    n++);
  DEBUG_DEC;

  olsr_global_mutex_give();

  return n;
}

void
olsr_process_tc_message(packet_byte_t* tc_message, int size, interface_t iface)
{
  olsr_message_hdr_t* header = (olsr_message_hdr_t*)tc_message;
  tc_message += sizeof(olsr_message_hdr_t);
  olsr_time_t Vtime = olsr_deserialize_time(header->Vtime);
  olsr_tc_message_hdr_t* tc_header = (olsr_tc_message_hdr_t*)tc_message;
  tc_message += sizeof(olsr_tc_message_hdr_t);

  olsr_global_mutex_take();

  DEBUG_TOPOLOGY("is there a link tuple such as neighb_iface_addr = %d?",
    header->source_addr);
  bool has = FALSE;
  FOREACH_LINK_EREW(
    l,
    if (l->L_neighbor_iface_addr == header->source_addr)
    {
      DEBUG_TOPOLOGY("yes");
      has = TRUE;
      break;
    });

  if (!has)
  {
    DEBUG_TOPOLOGY("no");
    DEBUG_TOPOLOGY("only processing TC message from ifaces of SYM neighb, aborting");
    olsr_global_mutex_give();
    return;
  }

  DEBUG_TOPOLOGY("is there a topology tuple such as T_last_addr = %d and T_seq > %d ?",
                 header->addr, tc_header->ansn);
  DEBUG_INC;
  bool exists = FALSE;
  FOREACH_TOPOLOGY_EREW(
    t,
    if (t->T_last_addr == header->addr
        && t->T_seq > tc_header->ansn)
    {
      exists = TRUE;
      break;
    });

  if (exists)
  {
    DEBUG_TOPOLOGY("yes, aborting");
    DEBUG_DEC;
    olsr_global_mutex_give();
    return;
  }

  DEBUG_TOPOLOGY("no");
  DEBUG_DEC;

  DEBUG_TOPOLOGY("delete all topology tuples with T_last_addr = %d and T_seq < %d",
                 header->addr, tc_header->ansn);
  DEBUG_INC;
  FOREACH_TOPOLOGY_EREW(
    t,
    if (t->T_last_addr == header->addr
        && t->T_seq < tc_header->ansn)
    {
      DEBUG_TOPOLOGY("this one [dest_addr:%d, last_addr:%d, sn:%d]",
                     t->T_dest_addr, t->T_last_addr, t->T_seq);
      olsr_topology_set_delete(__i_topology);
    });
  DEBUG_DEC;

  const int n_addresses = (header->size
                           - sizeof(olsr_message_hdr_t)
                           - sizeof(olsr_tc_message_hdr_t)) / sizeof(address_t);
  address_t* cursor = (address_t*)tc_message;

  DEBUG_TOPOLOGY("analysing the %d addresses of the TC message [start:%p]:",
                 n_addresses, cursor);
  DEBUG_INC;
  for (int i = 0; i < n_addresses / 2; i++)
  {
    const address_t source_iface = (*cursor) & 0xffff;

    cursor++;

    const address_t iface = (*cursor) & 0xffff;
    const address_t addr = olsr_iface_to_main_address(iface);

    cursor++;

    DEBUG_TOPOLOGY("address is %d", addr);

    DEBUG_TOPOLOGY("searching for an existing topology tuple "
                   "with with T_dest_addr = %d and T_last_addr = %d",
                   addr, header->addr);
    DEBUG_INC;
    bool exists = FALSE;
    FOREACH_TOPOLOGY_EREW(
      t,
      if (t->T_dest_addr == addr
          && t->T_last_addr == header->addr)
      {
        t->T_time = olsr_get_current_time() + Vtime;
        exists = TRUE;
        DEBUG_TOPOLOGY("found one, updating T_time to %d", t->T_time);
        DEBUG_DEC;
        break;
      });

    if (!exists)
    {
      DEBUG_TOPOLOGY("haven't found any, inserting");
      DEBUG_INC;
      olsr_topology_tuple_t tuple =
        {
          .T_dest_addr = addr,
          .T_dest_iface = iface,
          .T_last_addr = header->addr,
          .T_last_iface = source_iface,
          .T_seq = tc_header->ansn,
          .T_time = olsr_get_current_time() + Vtime,
        };

      DEBUG_TOPOLOGY("tuple [dest_addr:%d, last_addr:%d, sn:%d, time:%d]",
                     tuple.T_dest_addr,
                     tuple.T_last_addr,
                     tuple.T_seq,
                     tuple.T_time);
      olsr_topology_set_insert(&tuple);
      DEBUG_DEC;
      DEBUG_DEC;
    }
  }
  DEBUG_DEC;
  olsr_global_mutex_give();
}

olsr_topology_tuple_t*
olsr_topology_set_insert(const olsr_topology_tuple_t* tuple)
{
  if (tuple->T_dest_addr == state.address)
    return NULL;
  return olsr_topology_set_insert_(tuple);
}

void
olsr_tc_force_send()
{
  // FIXME: doesn't work, deadlock...
  /* olsr_message_t tc_message;

  if (!tc_initialized)
    return;

  if(!xSemaphoreTake(force_send_mutex, 0))
    return;

  olsr_generate_tc_message(&tc_message);
  for (int iface = 0; iface < IFACES_COUNT; iface++)
    olsr_send_message(&tc_message, iface);

  xSemaphoreGive(force_send_mutex);*/
}

static void
olsr_tc_task(void* pvParameters)
{
  /*
   When the advertised link set of a node becomes empty, this node
   SHOULD still send (empty) TC-messages during the a duration equal to
   the "validity time" (typically, this will be equal to TOP_HOLD_TIME)
   of its previously emitted TC-messages, in order to invalidate the
   previous TC-messages.  It SHOULD then stop sending TC-messages until
   some node is inserted in its advertised link set.
   */

  olsr_message_t tc_message;
  portTickType xLastWakeTime = xTaskGetTickCount();
  portTickType first_empty_time = xTaskGetTickCount();
  bool sending_empty = FALSE;
  int n;

  for (;;)
  {
    xSemaphoreTake(force_send_mutex, portMAX_DELAY);

    DEBUG_TOPOLOGY("generating TC message");
    DEBUG_INC;
    n = olsr_generate_tc_message(&tc_message);
    DEBUG_DEC;

    if (n == 0)
    {
      if (!sending_empty)
      {
        first_empty_time = xTaskGetTickCount();
        sending_empty = TRUE;
      }

      if ((xTaskGetTickCount() - first_empty_time)
          > (TOP_HOLD_TIME_S * 1000))
        continue;
    }
    else
      sending_empty = FALSE;

    DEBUG_TOPOLOGY("broadcasting TC message");
    DEBUG_INC;
    for (int iface = 0; iface < IFACES_COUNT; iface++)
      olsr_send_message(&tc_message, iface);
    DEBUG_DEC;

    xSemaphoreGive(force_send_mutex);

    vTaskDelayUntil(&xLastWakeTime,
                    TC_INTERVAL_S * 1000 - MAXJITTER_MS);
  }
}

#ifdef DEBUG
void olsr_topology_set_print()
{
  DEBUG_TOPOLOGY("--- TOPOLOGY SET ---");
  DEBUG_TOPOLOGY("");

  DEBUG_INC;

  DEBUG_TOPOLOGY("current time is %d", (int)olsr_get_current_time());
  DEBUG_TOPOLOGY("");

  DEBUG_TOPOLOGY(".-%s-.-%s-.-%s-.-%s-.-%s-.-%s-.",
                  DASHES(10),
                  DASHES(10),
                  DASHES(10),
                  DASHES(10),
                  DASHES(10),
                  DASHES(10));

  DEBUG_TOPOLOGY("| %10s | %10s | %10s | %10s | %10s | %10s |",
                 "dest addr",
                 "dest iface",
                 "last addr",
                 "last iface",
                 "sn",
                 "time");

  DEBUG_TOPOLOGY("+-%s-+-%s-+-%s-+-%s-+-%s-+-%s-+",
                 DASHES(10),
                 DASHES(10),
                 DASHES(10),
                 DASHES(10),
                 DASHES(10),
                 DASHES(10));

  FOREACH_TOPOLOGY_CREW(
    t,
    DEBUG_TOPOLOGY("| %10d | %10d | %10d | %10d | %10d | %10d |",
                   t->T_dest_addr,
                   t->T_dest_iface,
                   t->T_last_addr,
                   t->T_last_iface,
                   t->T_seq,
                   t->T_time);
    );

  DEBUG_TOPOLOGY("'-%s-'-%s-'-%s-'-%s-'-%s-'-%s-'",
                 DASHES(10),
                 DASHES(10),
                 DASHES(10),
                 DASHES(10),
                 DASHES(10),
                 DASHES(10));

  DEBUG_DEC;

  DEBUG_TOPOLOGY("");

  DEBUG_TOPOLOGY("--- END TOPOLOGY SET ---");
}
#endif
