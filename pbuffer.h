#include "scheduler.h"

typedef struct {
   int id;
   int create_time;  // time at which packet was created
   int source;
   int dest;
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
    SCHED* sched;
    void (* out)(void* , int);
    void *typex;
    struct pbuffer *st;
    struct pbuffer *en;
} QUEUE;


void packet_init(packet* self, int id, int t, int source, int dest);
packet* packet_create(int id, int t, int source, int dest);
void packet_destroy(packet* obj);
void queue_init(QUEUE* self, SCHED* sched);
QUEUE* queue_create();
void queue_insert(struct pbuffer **st, struct pbuffer **en,  packet* p, int key);
void queue_pop(struct pbuffer **st, struct pbuffer **en,  packet *p, int *key);
void queue_clear(struct pbuffer **st, struct pbuffer **en);
void queue_destroy(QUEUE* obj);
void queue_put(QUEUE* self, packet* p);
void queue_get(QUEUE* self, packet* p);