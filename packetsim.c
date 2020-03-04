#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "pbuffer.h"
#include <math.h>

typedef struct {
   int mean_pkt_size;
   int linerate;
} DIST;

void dist_init(DIST* self, int linerate, int mean_pkt_size) {
    self->linerate=linerate*pow(10,6);
    self->mean_pkt_size=mean_pkt_size;
}

DIST* dist_create(int linerate, int mean_pkt_size) {
    DIST* obj=(DIST*) malloc(sizeof(DIST));
    dist_init(obj, linerate, mean_pkt_size);
    return obj;
}

int dist_exec(DIST* self) {
   int pps;
   pps=self->linerate/(8*self->mean_pkt_size);
   // https://preshing.com/20111007/how-to-generate-random-timings-for-a-poisson-process/
   // https://www.codecogs.com/library/computing/c/math.h/log.php?alias=logf
   int interval=(logf(1.0f - (float) random() / (RAND_MAX + 1))/pps)*pow(10,6); // Microseconds
   return interval;
}


typedef struct {
    int pktcnt;
    int status;
    SCHED* sched;
    int source;
    int dest;
    void (* out)(void* , int);
    void *typex;
    int (*arrivalfn)();
    void *arrivalfntype;
} PKT;


void pkt_init(PKT* self, SCHED* sched, int source, int dest) {
    self->pktcnt=0;
    self->status=0;
    self->sched=sched;
    self->source=source;
    self->dest=dest;
}

PKT* pkt_create(SCHED* sched, int source, int dest) {
    PKT* obj=(PKT*) malloc(sizeof(PKT));
    pkt_init(obj, sched, source, dest);
    return obj;
}

int  pkt_gen(PKT* self) {
    if (self->status == 0) { // first time pkt_get is run
        self->status = 1;
        self->pktcnt=0;
    }
    // preprocess 
    packet* p=packet_create((self->pktcnt)++, self->sched->now, self->source, self->dest);
    // postprocess
    // Need to replicate self.out.put(p) functionality
    self->out(self->typex, p);
    int delay=self->arrivalfn(self->arrivalfntype);
    return delay;
}

typedef struct {
    int status;
    SCHED* sched;
} SINK;

void sink_init(SINK* self, SCHED* sched) {
    self->status=0;
    self->sched=sched;
}

SINK* sink_create(SCHED* sched) {
    SINK* obj=(SINK*) malloc(sizeof(SINK));
    sink_init(obj, sched);
    return obj;
}

void sink_put(SINK* self, packet *p) {
    printf("s: %d %d %d %d\n", p->create_time, p->id, p->source, p->dest);
    packet_destroy(p);
    return;
}

typedef struct {
    int status;
    SCHED* sched;
    void (* out)(void* , int);
    void *typex;
} BOX;

void box_init(BOX* self, SCHED* sched) {
    self->status=0;
    self->sched=sched;
}

BOX* box_create(SCHED* sched) {
    BOX* obj=(BOX*) malloc(sizeof(BOX));
    box_init(obj, sched);
    return obj;
}

void box_put(BOX* self, packet *p) {
//    printf("Transiting %d\n", p->id);
    self->out(self->typex, p);
    return;
}

int main() {
    SCHED* sched=sched_create(10); // seconds
    PKT* pkt1=pkt_create(sched,1,3);
    PKT* pkt2=pkt_create(sched,2,3);
    DIST* distfunc=dist_create(1,100); // Mbps
    BOX* box=box_create(sched);
    SINK* sink=sink_create(sched);
    
    pkt1->out=box_put; pkt1->typex=box;pkt1->arrivalfn=dist_exec;pkt1->arrivalfntype=distfunc;
    pkt2->out=box_put; pkt2->typex=box;pkt2->arrivalfn=dist_exec;pkt2->arrivalfntype=distfunc;
    box->out=sink_put; box->typex=sink;
    
    sched_reg(sched, pkt1, pkt_gen, 3);
    sched_reg(sched, pkt2, pkt_gen, 5);
    
    sched_run(sched);

}
