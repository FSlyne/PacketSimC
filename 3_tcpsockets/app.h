#include "scheduler.h"
#include "packet.h"

#ifndef APP_H
#define APP_H

struct abuffer
{
    rawdata *rd;
    int key;
    struct abuffer *next;
    struct abuffer *prev;
};

typedef struct {
    SCHED* sched;
    int myclock;
    struct abuffer *st;
    struct abuffer *en;  
} ASTORE;

typedef struct {
    int pktcnt;
    SCHED* sched;
    int flow_id;
    void (* out )(void* , rawdata*);
    void *typex;
    long (*arrivalfn)();
    void *arrivalfntype;
    //jmp_buf flag;
} APPGEN;

typedef struct {
    int status;
    long pkt_rcvd[10];
    long bytes_rcvd[10];
    long delay_cumul[10];
    SCHED* sched;
} APPSINK;

void app_gen(APPGEN* self);
APPGEN* appgen_create(SCHED* sched, int flow_id);
void appgen_init(APPGEN* self, SCHED* sched, int flow_id);
void appgen_stats(APPGEN* self);
APPSINK* appsink_create(SCHED* sched);
void appsink_init(APPSINK* self, SCHED* sched);
void appsink_put(APPSINK* self, rawdata *rd);
void appsink_stats(APPSINK* self);
void astore_clear(ASTORE* self);
int astore_count(ASTORE* self);
ASTORE* astore_create(SCHED* sched);
void astore_destroy(ASTORE* obj);
void astore_init(ASTORE* self, SCHED* sched);
void astore_insert_raw(struct abuffer **st, struct abuffer **en, rawdata* rd, int key);
void astore_insert(ASTORE* self, rawdata* rd, int key);
void astore_lpush(ASTORE* self, rawdata* rd, int key);
void astore_read(ASTORE* self, rawdata **rd, int *key);
void astore_rpop_block(ASTORE* self, rawdata **rd, int *key);
void astore_rpop_raw(struct abuffer **st, struct abuffer **en, rawdata **rd, int *key);
void astore_rpop(ASTORE* self, rawdata **rd, int *key);
rawdata* rawdata_create_noinit();
rawdata* rawdata_create(int id, int t, int size, int flow_id);
void rawdata_destroy(rawdata* obj);
rawdata* rawdata_copy(rawdata* target);
void rawdata_init(rawdata* self, int id, int t, int size, int flow_id);


#endif // APP_H