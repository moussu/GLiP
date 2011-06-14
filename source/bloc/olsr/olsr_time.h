#ifndef OLSR_TIME_H
# define OLSR_TIME_H

# include <stdint.h>

typedef long time_t;
typedef uint8_t time_serialized_t;

/*
   Given one of the above holding times, a way of computing the
   mantissa/exponent representation of a number T (of seconds) is the
   following:

     -    find the largest integer 'b' such that: T/C >= 2^b

     -    compute the expression 16*(T/(C*(2^b))-1), which may not be a
          integer, and round it up.  This results in the value for 'a'

     -    if 'a' is equal to 16: increment 'b' by one, and set 'a' to 0

     -    now, 'a' and 'b' should be integers between 0 and 15, and the
          field will be a byte holding the value a*16+b

   For instance, for values of 2 seconds, 6 seconds, 15 seconds, and 30
   seconds respectively, a and b would be: (a=0,b=5), (a=8,b=6),
   (a=14,b=7) and (a=14,b=8) respectively.
 */

inline time_serialized_t
olsr_serialize_time(time_t t)
{
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  return (time_serialized_t)t;
}

inline time_t
olsr_deserialize_time(time_serialized_t t)
{
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  return t;
}

inline time_t
olsr_seconds_to_time(float t_s)
{
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  //FIXME: implement!!!
  return (time_t)t_s;
}

inline time_t olsr_get_current_time(){ return 0; } // FIXME: implement
// FIXME: comparisons between time_t and get_current_time should be
// fixed.

#endif
