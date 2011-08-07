#include "olsr_ack.h"
#include "olsr_message.h"
#include "olsr_send.h"

SET_IMPLEMENT(ack, ACK_SET_MAX_SIZE)

void olsr_ack_tuple_init(olsr_ack_tuple_t* t)
{
  vSemaphoreCreateBinary(t->semphr);
  t->ack = NULL;
}

void
olsr_ack_set_delete(int i)
{
  xSemaphoreGive(ack_set.tuples[i].semphr);
  ack_set.tuples[i].ack = NULL;
  olsr_ack_set_delete_(i);
}

olsr_ack_tuple_t*
olsr_ack_set_get()
{
  const int old = ack_set.first_empty;
  if (ack_set.full)
  {
    ERROR("ack set full! [max_size:%d]", ack_set.max_size);
    SET_ABORT;
    return NULL;
  }

  olsr_ack_set_declare_used_(old);
  ack_set.n_tuples++;

  if (ack_set.n_tuples == ACK_SET_MAX_SIZE)
  {
    ack_set.first_empty = ACK_SET_MAX_SIZE;
    ack_set.full = TRUE;
  }
  else
  {
    for (int i = ack_set.first_empty + 1; i < ACK_SET_MAX_SIZE; i++)
    {
      if (olsr_ack_set_is_empty_(i))
      {
        ack_set.first_empty = i;
        break;
      }
    }
  }

  xSemaphoreTake(ack_set.tuples[old].semphr, portMAX_DELAY);
  ack_set.tuples[old].ack = NULL;

  return ack_set.tuples + old;
}

void
olsr_ack_init(olsr_ack_t* ack)
{
  olsr_ack_tuple_t* tuple = olsr_ack_set_get();

  DEBUG_ACK_SET("initalizing ack (mutex is taken)");

  ack->dest_addr = 0;
  ack->sn = 0;
  ack->time = 0;
  ack->acknowledged = FALSE;
  ack->semphr = tuple->semphr;
  ack->error = olsr_ack_default_callback;
  ack->success = olsr_ack_default_callback;
  ack->tuple = tuple;

  tuple->ack = ack;
}

void
olsr_ack_destroy(olsr_ack_t* ack)
{
  DEBUG_ACK_SET("destroying ack for %d, message %d (giving mutex)",
                ack->dest_addr, ack->sn);

  ack->dest_addr = 0;
  ack->sn = 0;
  ack->time = 0;
  ack->acknowledged = FALSE;
  ack->semphr = NULL;
  ack->error = NULL;
  ack->success = NULL;

  olsr_ack_set_delete(ack->tuple - ack_set.tuples);

  ack->tuple = NULL;
}

void
olsr_ack_default_callback(const olsr_ack_t* t)
{}

bool
olsr_ack_process(packet_byte_t* ack_message, int size)
{
  olsr_message_hdr_t* header = (olsr_message_hdr_t*)ack_message;
  packet_byte_t* content = ack_message + sizeof(olsr_message_hdr_t);

  olsr_ack_content_t* ack_content = (olsr_ack_content_t*)content;

  DEBUG_ACK_SET("processing mutex for %d, message %d",
                header->addr, ack_content->sn);
  DEBUG_ACK_SET("ack set size is %d", ack_set.n_tuples);

  FOREACH_ACK(
    tuple,

    olsr_ack_t* ack = tuple->ack;

    if (olsr_get_current_time() - ack->time
        > olsr_ms_to_time(ACK_VALIDITY_TIME_MS))
    {
      DEBUG_ACK_SET("ack for %d, message %d, expired",
                    ack->dest_addr, ack->sn);
      ack->acknowledged = FALSE;
      ack->error(ack);
      xSemaphoreGive(ack->semphr);
      continue;
    }

    if (ack->dest_addr == header->addr
        && ack->sn == ack_content->sn)

    {
      DEBUG_PRINT("ack for mess %d from %d", WHITE,
                  ack->sn, ack->dest_addr);
      ack->acknowledged = TRUE;
      ack->success(ack);
      xSemaphoreGive(ack->semphr);
      return TRUE;
    });

  WARNING("ack for %d, message %d, not processed",
          header->addr, ack_content->sn);

  return FALSE;
}

void
olsr_ack_send(address_t origin, seq_num_t sn)
{
  olsr_message_t message;
  olsr_ack_content_t* content = (olsr_ack_content_t*)message.content;
  message.content_size = sizeof(olsr_ack_content_t);
  content->sn = sn;
  message.header.type = ACK;
  DEBUG_ACK_SET("send ack to %d:", origin);
  DEBUG_INC;
  if (olsr_send_to_(&message, olsr_ms_to_time(ACK_VALIDITY_TIME_MS), origin))
    DEBUG_ACK_SET("ack to %d was sent", origin);
  DEBUG_DEC;
}

bool
olsr_ack_wait(olsr_ack_t* ack, int timeout_ms)
{
  DEBUG_ACK_SET("waiting for ack from %d, message %d",
                ack->dest_addr, ack->sn);
  if (xSemaphoreTake(ack->semphr, timeout_ms / portTICK_RATE_MS))
    return TRUE;
  return FALSE;
}
