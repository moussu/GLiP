#ifndef OLSR_TIME_H
# define OLSR_TIME_H

# include <FreeRTOS.h>
# include <stdint.h>

typedef portTickType olsr_time_t;
typedef portTickType time_serialized_t;

inline time_serialized_t
olsr_serialize_time(olsr_time_t t)
{
  return (time_serialized_t)t;
}

inline olsr_time_t olsr_deserialize_time(time_serialized_t t)
{
  return (olsr_time_t)t;
}

inline olsr_time_t olsr_seconds_to_time(float t_s)
{
  return (olsr_time_t)(t_s * 1000);
}

olsr_time_t olsr_get_current_time();

#endif
