#include "packet.h"
#include "scheduler.h"
#include "store.h"

#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
    int status;
    int linerate;
    int latency;
    int bytesize;
    int countsize;
    int countlimit;
    int bytelimit;
    int myclock;
    SCHED* sched;
    STORE* store;
    void (* out)(void* , packet*);
    void *typex;
    struct pbuffer *st;
    struct pbuffer *en;  
} QUEUE;


void queue_init(QUEUE* self, SCHED* sched, STORE* store, int linerate, int countlimit, int bytelimit, int latency);
QUEUE* queue_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency);
void queue_destroy(QUEUE* obj);
void queue_put(QUEUE* self, packet* p);
void  queue_gen(int pid, QUEUE* self) ;

#endif // QUEUE_H