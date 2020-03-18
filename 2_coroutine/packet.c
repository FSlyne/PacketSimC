#include <stdlib.h>
#include <stdio.h>
#include "packet.h"

void packet_init(packet* self, int id, int t, int source, int dest, int flow_id, int size) {
   self->id=id;
   self->create_time=t;
   self->source=source;
   self->dest=dest;
   self->flow_id=flow_id;
   self->size=size;
}

packet* packet_create(int id, int t, int source, int dest, int flow_id, int size) {
    packet* obj=(packet*) malloc(sizeof(packet));
    packet_init(obj, id, t, source, dest, flow_id, size);
    return obj;
}

packet* packet_create_noinit() {
    packet* obj=(packet*) malloc(sizeof(packet));
    return obj;
}


void packet_copy(packet* from_p, packet* to_p) {
    to_p->id=from_p->id;
    to_p->create_time=from_p->create_time;
    to_p->source=from_p->source;
    to_p->dest=from_p->dest;
    to_p->size=from_p->size;
    to_p->flow_id=from_p->size;
}

void packet_destroy(packet* obj){
    if (obj) {
        free(obj);
    }
}