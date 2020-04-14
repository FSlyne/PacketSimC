#include "pbuffer.h"

#ifndef STORE_H
#define STORE_H

typedef struct {
    SCHED* sched;
    jmp_buf flag;
    int myclock;
    struct pbuffer *st;
    struct pbuffer *en;  
} STORE;

void store_insert(STORE* self,  packet* p, int key);
void store_lpush(STORE* self,   packet* p, int key);
void store_rpop(STORE* self,   packet **p, int *key);
void store_insert_raw(struct pbuffer **st, struct pbuffer **en,  packet* p, int key);
void store_rpop_raw(struct pbuffer **st, struct pbuffer **en,  packet **p, int *key);
void store_clear(STORE* self);
int store_count(STORE* self);
STORE* store_create(SCHED* sched);
void store_init(STORE* self, SCHED* sched);
void store_destroy(STORE* obj);
void store_yield(STORE* self, jmp_buf flag);
void store_rpop_block(int pid, STORE* self, packet **p, int *key);

#endif // STORE_H