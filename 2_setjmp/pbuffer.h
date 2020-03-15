#include "scheduler.h"
#include "packet.h"

typedef struct {   
    int status;
    SCHED* sched;
    void (* out)(void* , packet*);
    void *typex;
} BOX;

typedef struct {
   int pir;
   int pbs;
   int cir;
   int cbs;
   int pbucket;
   int cbucket;
   int last_time;
   SCHED* sched;
   void (* out)(void* , packet*);
   void *typex;
} TRTCM;

void box_init(BOX* self, SCHED* sched);
BOX* box_create(SCHED* sched);
void box_put(BOX* self, packet *p);
void trtcm_init(TRTCM* self, SCHED* sched, int pir, int pbs, int cir, int cbs);
TRTCM* trtcm_create(SCHED* sched, int pir, int pbs, int cir, int cbs);
void trtcm_destroy(TRTCM* obj);
void trtcm_put(TRTCM* self, packet* p);
