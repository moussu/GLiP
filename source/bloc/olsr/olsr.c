#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "utils/lfsr.h"
#include "olsr.h"
#include "olsr_ifaces.h"
#include "olsr_state.h"
#include "olsr_types.h"
#include "olsr_neighbor2_set.h"
#include "olsr_constants.h"
#include "olsr_duplicate_set.h"
#include "olsr_link_set.h"
#include "olsr_ms_set.h"
#include "olsr_mpr_set.h"
#include "olsr_neighbor_set.h"
#include "olsr_topology_set.h"
#include "olsr_receive.h"
#include "olsr_routing_table.h"
#include "olsr_send.h"
#include "olsr_hello.h"

static xSemaphoreHandle olsr_global_mutex;

# define GRAPH_SET_MAX_SIZE 100
typedef struct
{
  address_t dest_addr;
  address_t dest_iface;
  address_t last_addr;
  address_t last_iface;
  bool marqued;
} olsr_graph_tuple_t;
SET_DECLARE(graph, GRAPH_SET_MAX_SIZE)
void olsr_graph_tuple_init(olsr_graph_tuple_t* tuple)
{
  tuple->dest_addr = 0;
  tuple->dest_iface = 0;
  tuple->last_addr = 0;
  tuple->last_iface = 0;
  tuple->marqued = FALSE;
}
SET_DEFAULT_BINDINGS(graph)
SET_IMPLEMENT(graph, GRAPH_SET_MAX_SIZE)
# define FOREACH(Var, Code)                     \
  SET_FOREACH(graph, Var,                       \
              Code)

#define EXPLORE(IfaceAddr, Action)                      \
  {                                                     \
    address_t __iface = IfaceAddr;                      \
                                                        \
    bool __neighbor = TRUE;                             \
    while (__neighbor)                                  \
    {                                                   \
      __neighbor = FALSE;                               \
      address_t __next_neighbor_iface = 0;              \
      {                                                 \
        Action;                                         \
      }                                                 \
      FOREACH(                                          \
        n,                                              \
        if (n->last_iface == __iface)                   \
        {                                               \
          __neighbor = TRUE;                            \
          __next_neighbor_iface = n->dest_iface;        \
          break;                                        \
        });                                             \
      __iface = (__next_neighbor_iface + 2) % 4;        \
    }                                                   \
  }

static int shifts[4][4] =
{
// 0  1  2  3
  {2, 3, 0, 1}, // 0
  {1, 2, 3, 0}, // 1
  {0, 1, 2, 3}, // 2
  {3, 0, 1, 2}, // 3
};

static void
olsr_application_task(void* pvParameters);

void
olsr_init(address_t uid)
{
  olsr_global_mutex = xSemaphoreCreateMutex();

  // Initialize state:
  state.address = olsr_get_interface_address(uid, 0);
  state.willingness = WILL_DEFAULT;

  for (int iface = 0; iface < IFACES_COUNT; iface++)
  {
    state.iface_addresses[iface] =
      olsr_get_interface_address(uid, iface);
    DEBUG_SERVER("iface %s address is 0x%x",
                 olsr_iface_str(iface),
                 state.iface_addresses[iface]);
  }

  // Initialize sets:
  olsr_duplicate_set_init();
  olsr_link_set_init();
  olsr_mpr_set_init();
  olsr_ms_set_init();
  olsr_neighbor_set_init();
  olsr_neighbor2_set_init();
  olsr_topology_set_init();
  olsr_routing_set_init();

  // Initialize services:
  olsr_send_init();
  olsr_receive_init();
  olsr_hello_init();
  olsr_tc_init();

  // Initialize application
  olsr_graph_set_init();
  xTaskCreate(olsr_application_task,
              (signed portCHAR*) "applicationTask",
              configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY, NULL);

}

void
olsr_global_mutex_take()
{
  xSemaphoreTake(olsr_global_mutex, portMAX_DELAY);
}

void
olsr_global_mutex_give()
{
  xSemaphoreGive(olsr_global_mutex);
}

void
olsr_compute_graph()
{
  olsr_graph_tuple_t tuple;
  tuple.marqued = FALSE;

  olsr_graph_set_empty();

  FOREACH_LINK_EREW(
    l,
    tuple.dest_addr = olsr_iface_to_main_address(l->L_neighbor_iface_addr);
    tuple.dest_iface = l->L_neighbor_iface_addr;
    tuple.last_addr = olsr_iface_to_main_address(l->L_local_iface_addr);
    tuple.last_iface = l->L_local_iface_addr;
    olsr_graph_set_insert(&tuple));

  FOREACH_TOPOLOGY_EREW(
    t,
    tuple.dest_addr = t->T_dest_addr;
    tuple.dest_iface = t->T_dest_iface;
    tuple.last_addr = t->T_last_addr;
    tuple.last_iface = t->T_last_iface;
    olsr_graph_set_insert(&tuple));
}

address_t
olsr_mesh_leader()
{
  address_t min = state.address;

  FOREACH(
    n,
    if (n->dest_addr < min)
    {
      min = n->dest_addr;
    });

  return min;
}

void
olsr_mesh_get_lr(interface_t north, address_t addr, int* l, int* r)
{
  *l = 0;
  *r = 0;
  EXPLORE(olsr_get_interface_address(addr, (north + 3) % 4), (*r)++);
  EXPLORE(olsr_get_interface_address(addr, (north + 1) % 4), (*l)++);
}

void
olsr_mesh_get_ud(interface_t north, address_t addr, int* u, int* d)
{
  *u = 0;
  *d = 0;
  EXPLORE(olsr_get_interface_address(addr, (north + 0) % 4), (*u)++);
  EXPLORE(olsr_get_interface_address(addr, (north + 2) % 4), (*d)++);
}

void
olsr_graph_reset_marqued()
{
  FOREACH(n, n->marqued = FALSE);
}

static bool
olsr_mesh_offset_(address_t addr1, address_t addr2, interface_t north,
                  int i, int j, int* x, int* y)
{
  olsr_graph_tuple_t* neighbors[4] = {NULL};

  if (addr1 == addr2)
  {
    DEBUG_APPLI("found addr:%d i:%d j:%d", addr2, i, j);
    *x = j;
    *y = i;
    return TRUE;
  }

  DEBUG_APPLI("addr1:%d addr2:%d i:%d j:%d", addr1, addr2, i, j);

  FOREACH(
    n,
    for (int iface = 0; iface < IFACES_COUNT; iface++)
    {
      if (n->last_iface == olsr_get_interface_address(addr1, iface)
        && !n->marqued)
      {
        neighbors[iface] = n;
      }
    });

  for (int iface = 0; iface < IFACES_COUNT; iface++)
  {
    int _i = i, _j = j;
    if (neighbors[iface])
    {
      const interface_t shifted_iface = (iface - north + 4) % 4;
      neighbors[iface]->marqued = TRUE;
      switch (shifted_iface)
      {
        case NORTH_IFACE:
          DEBUG_APPLI("found N neighbor addr:%d", neighbors[iface]->dest_addr);
          _i = i + 1;
          break;
        case WEST_IFACE:
          DEBUG_APPLI("found W neighbor addr:%d", neighbors[iface]->dest_addr);
          _j = j - 1;
          break;
        case SOUTH_IFACE:
          DEBUG_APPLI("found S neighbor addr:%d", neighbors[iface]->dest_addr);
          _i = i - 1;
          break;
        case EAST_IFACE:
          DEBUG_APPLI("found E neighbor addr:%d", neighbors[iface]->dest_addr);
          _j = j + 1;
          break;
      }

      const interface_t his_iface = olsr_get_interface(neighbors[iface]->dest_iface);
      const interface_t new_north = (north + shifts[iface][his_iface]) % 4;

      DEBUG_INC;
      if (olsr_mesh_offset_(neighbors[iface]->dest_addr, addr2, new_north, _i, _j, x, y))
      {
        DEBUG_DEC;
        return TRUE;
      }
      DEBUG_DEC;
    }
  }

  return FALSE;
}

bool
olsr_mesh_offset(address_t addr1, address_t addr2, interface_t north, int* i, int* j)
{
  olsr_graph_reset_marqued();
  DEBUG_APPLI("COMPUTING OFFSET addr1:%d addr2:%d north:%s",
              addr1, addr2, olsr_iface_str(north));
  DEBUG_INC;
  bool ret = olsr_mesh_offset_(addr1, addr2, north, 0, 0, j, i);
  DEBUG_DEC;
  return ret;
}

static bool
olsr_mesh_north_of_(address_t addr1, address_t addr2,
                    interface_t* north)
{
  olsr_graph_tuple_t* neighbors[4] = {NULL};
  interface_t his_north;

  if (addr1 == addr2)
  {
    *north = NORTH_IFACE;
    DEBUG_APPLI("found addr:%d", addr2);
    return TRUE;
  }

  DEBUG_APPLI("addr1:%d addr2:%d", addr1, addr2);

  FOREACH(
    n,
    for (int iface = 0; iface < IFACES_COUNT; iface++)
    {
      if (n->last_iface == olsr_get_interface_address(addr1, iface)
        && !n->marqued)
      {
        neighbors[iface] = n;
      }
    });

  for (int iface = 0; iface < IFACES_COUNT; iface++)
  {
    if (neighbors[iface])
    {
      DEBUG_INC;
      neighbors[iface]->marqued = TRUE;
      if (olsr_mesh_north_of_(neighbors[iface]->dest_addr, addr2, &his_north))
      {
        interface_t his_iface = olsr_get_interface(neighbors[iface]->dest_iface);
        interface_t shift = shifts[his_iface][iface];
        *north = (his_north + shift) % 4;
        DEBUG_APPLI("FOUND shift:%d north:%s", shift, olsr_iface_str(*north));
        DEBUG_DEC;
        return TRUE;
      }

      DEBUG_DEC;
    }
  }

  return FALSE;
}

bool
olsr_mesh_north_of(address_t addr1, address_t addr2, interface_t* north)
{
  olsr_graph_reset_marqued();
  return olsr_mesh_north_of_(addr1, addr2, north);
}


interface_t
olsr_mesh_north(address_t leader)
{
  interface_t north = NORTH_IFACE;

  if (leader == state.address)
    return north;

  olsr_mesh_north_of(state.address, leader, &north);

  return north;
}


void
olsr_mesh_coords(interface_t north, int* w, int* h, int* i, int* j)
{
  int min_i = 0xffff, min_j = 0xffff;
  int max_i = -0xffff, max_j = -0xffff;
  bool iterated = FALSE;

  FOREACH(
    n,
    int x;
    int y;
    if (olsr_mesh_offset(state.address, n->last_addr, north, &x, &y))
    {
      iterated = TRUE;
      if (x > max_j)
        max_j = x;
      if (x < min_j)
        min_j = x;
      if (y > max_i)
        max_i = y;
      if (y < min_i)
        min_i = y;
    });

  if (!iterated)
  {
    *i = 0;
    *j = 0;
    *w = 1;
    *h = 1;
    return;
  }

  *w = max_j - min_j + 1;
  *h = max_i - min_i + 1;
  *i = -min_i;
  *j = -min_j;
}

static void
olsr_application_task(void* pvParameters)
{
  portTickType xLastWakeTime = xTaskGetTickCount();
#ifdef WARNINGS
  portTickType xLastLastWakeTime = xTaskGetTickCount();
#endif

  for (;;)
  {
    olsr_global_mutex_take();
    olsr_compute_graph();
    olsr_application_job();
    olsr_global_mutex_give();

#ifdef WARNINGS
    xLastLastWakeTime = xLastWakeTime;
#endif
    vTaskDelayUntil(&xLastWakeTime, APP_INTERVAL_MS / portTICK_RATE_MS);
#ifdef WARNINGS
    if ((xLastWakeTime - xLastLastWakeTime) * portTICK_RATE_MS
        > APP_INTERVAL_MS)
    {
      WARNING("helloTask delay exceeded: %dms instead of %dms",
              (xLastWakeTime - xLastLastWakeTime) * portTICK_RATE_MS,
              APP_INTERVAL_MS);
    }
#endif
  }
}

void
olsr_application_job()
{
  address_t leader = olsr_mesh_leader();
  interface_t north = olsr_mesh_north(leader);
  int i, j, w, h;
  olsr_mesh_coords(north, &w, &h, &j, &i);

  printf("leader:%d north:%s i:%d j:%d w:%d h:%d\n",
         leader, olsr_iface_str(north), i, j, w, h);
}
