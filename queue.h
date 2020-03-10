#include "packet.h"
#include "scheduler.h"

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
    void (* out)(void* , int);
    void *typex;
    struct pbuffer *st;
    struct pbuffer *en;  
} QUEUE;


void queue_init(QUEUE* self, SCHED* sched, int linerate, int countlimit, int bytelimit, int latency);
QUEUE* queue_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency);
void queue_count(struct pbuffer **st, struct pbuffer **en);
void queue_insert(struct pbuffer **st, struct pbuffer **en,  packet* p, int key);
void queue_lpush(struct pbuffer **st, struct pbuffer **en,  packet* p, int key);
void queue_rpop(struct pbuffer **st, struct pbuffer **en,  packet **p, int *key);
void queue_clear(struct pbuffer **st, struct pbuffer **en);
void queue_destroy(QUEUE* obj);
void queue_put(QUEUE* self, packet* p);
void queue_get(QUEUE* self, packet* p);