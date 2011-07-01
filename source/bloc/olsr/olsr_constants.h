#ifndef OLSR_CONSTANTS_H
# define OLSR_CONSTANTS_H

# define QUEUES_SIZE 65535

/* C is a scaling factor for the "validity time" calculation ("Vtime"
   and "Htime" fields in message headers, see section 18.3).  The
   "validity time" advertisement is designed such that nodes in a
   network may have different and individually tuneable emission
   intervals, while still interoperate fully.  For protocol functioning
   and interoperability to work:

     -    the advertised holding time MUST always be greater than the
          refresh interval of the advertised information.  Moreover, it
          is recommended that the relation between the interval (from
          section 18.2), and the hold time is kept as specified
          in section 18.3, to allow for reasonable packet loss.

     -    the constant C SHOULD be set to the suggested value.  In order
          to achieve interoperability, C MUST be the same on all nodes.

     -    the emission intervals (section 18.2), along with the
          advertised holding times (subject to the above constraints)
          MAY be selected on a per node basis.

   Note that the timer resolution of a given implementation might not be
   sufficient to wake up the system on precise refresh times or on
   precise expire times: the implementation SHOULD round up the
   'validity time' ("Vtime" and "Htime" of packets) to compensate for
   coarser timer resolution, at least in the case where "validity time"
   could be shorter than the sum of emission interval and maximum
   expected timer error.*/

# define C_S                (1.0f/16.0f)

// 18.2.  Emission Intervals:
# define HELLO_INTERVAL_S   (2)
# define REFRESH_INTERVAL_S (2)
# define TC_INTERVAL_S      (5)
# define MID_INTERVAL_S     TC_INTERVAL_S
# define HNA_INTERVAL_S     TC_INTERVAL_S

// 18.3.  Holding Time:
# define NEIGHB_HOLD_TIME_S (3 * REFRESH_INTERVAL_S)
# define TOP_HOLD_TIME_S    (3 * TC_INTERVAL_S)
# define DUP_HOLD_TIME_S    (30)
# define MID_HOLD_TIME_S    (3 * MID_INTERVAL_S)
# define HNA_HOLD_TIME_S    (3 * HNA_INTERVAL_S)

// 18.9.  Misc. Constants:
# define TC_REDUNDANCY 0
# define MPR_COVERAGE  1
# define MAXJITTER_MS  (1000 * HELLO_INTERVAL_S / 4)

// 18.7.  Link Hysteresis:
# define HYST_THRESHOLD_HIGH 0.8f
# define HYST_THRESHOLD_LOW  0.3f
# define HYST_SCALING        0.5f


// 18.5.  Link Types:
typedef enum
{
  UNSPEC_LINK = 0,
  ASYM_LINK   = 1,
  SYM_LINK    = 2,
  LOST_LINK   = 3,
} link_type_t;

typedef enum
{
  NOT_SYM = 0,
  SYM     = 1,
} link_status_t;

// 18.6.  Neighbor Types:
typedef enum
{
  NOT_NEIGH = 0,
  SYM_NEIGH = 1,
  MPR_NEIGH = 2,
} neighbor_type_t;

// 18.8.  Willingness:
typedef enum
{
  WILL_NEVER   = 0,
  WILL_LOW     = 1,
  WILL_DEFAULT = 3,
  WILL_HIGH    = 6,
  WILL_ALWAYS  = 7,
} willingness_t;

# ifdef DEBUG
# include <stm32f10x.h>
# include "olsr_ifaces.h"
const char* olsr_neighbor_type_str(neighbor_type_t t);
const char* olsr_willingness_str(willingness_t w);
const char* olsr_link_type_str(link_type_t t);
const char* olsr_link_status_str(link_status_t s);
const char* olsr_iface_str(interface_t iface);
inline const char*
olsr_bool_str(bool b)
{
  return b ? "1" : "0";
}
# endif

#endif
