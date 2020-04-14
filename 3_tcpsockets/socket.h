#include "packet.h"
#include "scheduler.h"
#include "store.h"

#ifndef SOCKET_H
#define SOCKET_H

typedef struct {
    SCHED* sched;
    STORE* store[2];
    void (* out[2])(void* , packet*);
    void *typex[2];
    unsigned mask;
    struct pbuffer *st;
    struct pbuffer *en;  
} SOCKET;


void socket_init(SOCKET* self, SCHED* sched, STORE* store[]);
SOCKET* socket_create(SCHED* sched);
void socket_destroy(SOCKET* self);
unsigned int socket_select(int pid, SOCKET* self);
void  socket_gen(int pid, SOCKET* self);
void socket_put0(SOCKET* self, packet* p);
void socket_put1(SOCKET* self, packet* p);

#endif // SOCKET_H