#ifndef OLSR_ACK_H
# define OLSR_ACK_H

# include "olsr_message.h"
# include "olsr_types.h"
# include "olsr_set.h"
# include "semphr.h"

# define ACK_VALIDITY_TIME_MS 1000
# define ACK_SET_MAX_SIZE 10

struct olsr_ack_tuple_t;
struct olsr_ack_t;

typedef struct olsr_ack_tuple_t olsr_ack_tuple_t;
typedef struct olsr_ack_t olsr_ack_t;

typedef void (*olsr_ack_callback_t)(const struct olsr_ack_t* ack);

struct olsr_ack_t
{
  address_t dest_addr;
  seq_num_t sn;
  time_t time;
  bool acknowledged;
  xSemaphoreHandle semphr;
  olsr_ack_callback_t error;
  olsr_ack_callback_t success;
  olsr_ack_tuple_t* tuple;
};

typedef struct
{
  seq_num_t sn;
} olsr_ack_content_t;

struct olsr_ack_tuple_t
{
  xSemaphoreHandle semphr;
  olsr_ack_t* ack;
};

SET_DECLARE(ack, ACK_SET_MAX_SIZE)
SET_DEFAULT_INIT(ack)
SET_DEFAULT_EMPTY(ack)
SET_DEFAULT_INSERT(ack)
SET_DEFAULT_APPLY(ack)
SET_DEFAULT_FIND(ack)
SET_DEFAULT_IS_EMPTY(ack)
SET_DEFAULT_DECLARE_EMPTY(ack)

# define FOREACH_ACK(Var, Code)                 \
  SET_FOREACH(ack, Var, Code)

void olsr_ack_tuple_init(olsr_ack_tuple_t* t);

void olsr_ack_init(olsr_ack_t* ack);
void olsr_ack_destroy(olsr_ack_t* ack);
bool olsr_ack_wait(olsr_ack_t* ack, int timeout_ms);
bool olsr_ack_process(packet_byte_t* ack_message, int size);
void olsr_ack_send(address_t origin, seq_num_t sn);

void olsr_ack_default_callback(const olsr_ack_t* t);

#endif
