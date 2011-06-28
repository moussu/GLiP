#ifndef ROUTING_H
# define ROUTING_H

# include <stdint.h>
# include <stm32f10x.h>

# include "olsr_ifaces.h"
# include "olsr_types.h"

void olsr_init(address_t address);
void olsr_receive_callback(int iSocket, void* pvContext);

void olsr_global_mutex_take();
void olsr_global_mutex_give();

void olsr_compute_graph();
void olsr_graph_reset_marqued();

address_t olsr_mesh_leader();
void olsr_mesh_get_lr(interface_t north, address_t addr, int* l, int* r);
void olsr_mesh_get_ud(interface_t north, address_t addr, int* u, int* d);
bool olsr_mesh_offset(address_t addr1, address_t addr2, interface_t north, int* i, int* j);
bool olsr_mesh_north_of(address_t addr1, address_t addr2, interface_t* north);
interface_t olsr_mesh_north(address_t leader);
void olsr_mesh_coords(interface_t north, int* w, int* h, int* i, int* j);
void olsr_application_job();

#endif
