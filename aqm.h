#include "scheduler.h"
#include "packet.h"
#include "queue.h"

typedef struct {
   SCHED* sched;
   QUEUE* queue;
} WRED;


void wred_init(WRED* self, SCHED* sched);
WRED* wred_create(SCHED* sched, int linerate);
void wred_destroy(WRED* obj);
void wred_put(WRED* self, packet* p);