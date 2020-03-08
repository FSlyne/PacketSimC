#include "scheduler.h"

typedef struct {
   int id;
   int create_time;  // time at which packet was created
   int source;
   int dest;
   int size;
} packet;


struct pbuffer
{
    packet *p; 
    int key;
    struct pbuffer *next;
    struct pbuffer *prev;
};

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


void packet_init(packet* self, int id, int t, int source, int dest, int size);
packet* packet_create(int id, int t, int source, int dest, int size);
packet* packet_create_noinit();
void packet_copy(packet* from_p, packet* to_p);
void packet_destroy(packet* obj);
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