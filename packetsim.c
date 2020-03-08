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
    self->linerate=linerate*1000000;
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
   int interval=(logf(1.0f - (float) random() / (RAND_MAX + 1))/pps)*1000000; // Microseconds between end of one packet and start of next
   //printf("interval =%d\n", interval);
   return interval+1000000/pps; // adjust to include transmission time of packet itself
}


typedef struct {
    int pktcnt;
    int status;
    SCHED* sched;
    int source;
    int dest;
    int flow_id;
    void (* out)(void* , int);
    void *typex;
    int (*arrivalfn)();
    void *arrivalfntype;
} PKT;


void pkt_init(PKT* self, SCHED* sched, int source, int dest, int flow_id) {
    self->pktcnt=0;
    self->status=0;
    self->sched=sched;
    self->source=source;
    self->dest=dest;
    self->flow_id=flow_id;
}

PKT* pkt_create(SCHED* sched, int source, int dest, int flow_id) {
    PKT* obj=(PKT*) malloc(sizeof(PKT));
    pkt_init(obj, sched, source, dest, flow_id);
    return obj;
}

int  pkt_gen(PKT* self) {
    if (self->status == 0) { // first time pkt_get is run
        self->status = 1;
        self->pktcnt=0;
    }
    // preprocess 
    packet* p=packet_create((self->pktcnt)++, self->sched->now, self->source, self->dest,self->flow_id,
                            ((DIST*) self->arrivalfntype)->mean_pkt_size);
    // postprocess
    // Need to replicate self.out.put(p) functionality
    self->out(self->typex, p);
    int delay=self->sched->now+self->arrivalfn(self->arrivalfntype);
    return delay;
}

void pkt_stats(PKT* self) {
   printf("TX - packets: %d\n", self->pktcnt);
}

typedef struct {
    int status;
    int pkt_rcvd;
    int bytes_rcvd;
    int delay_cumul;
    SCHED* sched;
} SINK;

void sink_init(SINK* self, SCHED* sched) {
    self->status=0;
    self->pkt_rcvd=0;
    self->bytes_rcvd=0;
    self->delay_cumul=0;
    self->sched=sched;
}

SINK* sink_create(SCHED* sched) {
    SINK* obj=(SINK*) malloc(sizeof(SINK));
    sink_init(obj, sched);
    return obj;
}

void sink_put(SINK* self, packet *p) {
    // printf("s: %d %d %d %d %d\n", self->sched->now, (self->sched->now-p->create_time), p->id, p->source, p->dest);
    self->pkt_rcvd++;
    self->bytes_rcvd+=p->size;
    self->delay_cumul+=(self->sched->now-p->create_time);
    packet_destroy(p);
    return;
}

void sink_stats(SINK* self) {
   printf("RX - Bytes: %d, Packets: %d, Delay: %d\n", self->bytes_rcvd, self->pkt_rcvd, self->delay_cumul);
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
   
    int scenario = 2;
    if (scenario == 1) { // Single queue
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         DIST* distfunc=dist_create(10,1000); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         QUEUE* queue=queue_create(sched,10, 0, 128000, 100); // Mbps, Packet Count limit, Packet Byte limit, latency (usec)
         
         //pkt1->out=queue_put; pkt1->typex=queue1;pkt1->arrivalfn=dist_exec;pkt1->arrivalfntype=distfunc;
         //pkt2->out=queue_put; pkt2->typex=queue1;pkt2->arrivalfn=dist_exec;pkt2->arrivalfntype=distfunc;
         //queue1->out=sink_put;queue1->typex=sink;
         //
         pkt1->out=queue_put; pkt1->typex=queue; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         queue->out=sink_put; queue->typex=sink;
         
         sched_reg(sched, pkt1, pkt_gen, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         sink_stats(sink);
    } else if (scenario == 2) { // WRED queue
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         PKT* pkt2=pkt_create(sched,2,3, 2); // from, to, flow_id
         DIST* distfunc=dist_create(10,1000); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         WRED* wred=wred_create(sched,17); // Mbps
   
         pkt1->out=wred_put; pkt1->typex=wred; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pkt2->out=wred_put; pkt2->typex=wred; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
         wred->queue->out=sink_put; wred->queue->typex=sink;
         
         sched_reg(sched, pkt1, pkt_gen, 0);
         sched_reg(sched, pkt2, pkt_gen, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         pkt_stats(pkt2);
         sink_stats(sink);
    }

}

//    SCHED* sched=sched_create(10); // seconds
//    PKT* pkt1=pkt_create(sched,1,3);
//    PKT* pkt2=pkt_create(sched,2,3);
//    DIST* distfunc=dist_create(1,100); // Transmission (Mbps), Mean Packet size (Bytes)
////    QUEUE* queue1=queue_create(sched);
//    SINK* sink=sink_create(sched);
//    BOX* box=box_create(sched);
//    
//    //pkt1->out=queue_put; pkt1->typex=queue1;pkt1->arrivalfn=dist_exec;pkt1->arrivalfntype=distfunc;
//    //pkt2->out=queue_put; pkt2->typex=queue1;pkt2->arrivalfn=dist_exec;pkt2->arrivalfntype=distfunc;
//    //queue1->out=sink_put;queue1->typex=sink;
//    //
//    pkt1->out=box_put; pkt1->typex=box; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
//    pkt2->out=box_put; pkt2->typex=box; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
//    box->out=sink_put; box->typex=sink;
//    
//    sched_reg(sched, pkt1, pkt_gen, 0);
//    sched_reg(sched, pkt2, pkt_gen, 0);
//    
//    sched_run(sched);

