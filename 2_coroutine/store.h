#include "pbuffer.h"

#ifndef STORE_H
#define STORE_H

typedef struct {
    SCHED* sched;
    jmp_buf flag;
    int depleted;
    struct pbuffer *st;
    struct pbuffer *en;  
} STORE;

void store_insert(struct pbuffer **st, struct pbuffer **en,  packet* p, int key);
void store_lpush(struct pbuffer **st, struct pbuffer **en,  packet* p, int key);
void store_rpop(struct pbuffer **st, struct pbuffer **en,  packet **p, int *key);
void store_clear(struct pbuffer **st, struct pbuffer **en);
void store_count(int id, struct pbuffer **st, struct pbuffer **en);
STORE* store_create(SCHED* sched);
void store_init(STORE* self, SCHED* sched);
void store_destroy(STORE* obj);
void store_yield(STORE* self, jmp_buf flag);

#endif // STORE_H