#include <stdlib.h>
#include <stdio.h>
#include "packetgen.h"


void dist_init(DIST* self, int linerate, int mean_pkt_size) {
    self->linerate=linerate*1000000;
    self->mean_pkt_size=mean_pkt_size;
}

DIST* dist_create(int linerate, int mean_pkt_size) {
    signed int t;
    srand((unsigned) time(&t));
    DIST* obj=(DIST*) malloc(sizeof(DIST));
    dist_init(obj, linerate, mean_pkt_size);
    return obj;
}


int dist_exec(DIST* self) {
   int pps;
   pps=self->linerate/(8*self->mean_pkt_size);
   // https://preshing.com/20111007/how-to-generate-random-timings-for-a-poisson-process/
   // https://www.codecogs.com/library/computing/c/math.h/log.php?alias=logf
   // https://stackoverflow.com/questions/34558230/generating-random-numbers-of-exponential-distribution
   // int interval= -(logf(1.0f - (float) rand() / (RAND_MAX + 1))/pps)*1000000; // Microseconds between end of one packet and start of next
   //printf("interval =%d\n", interval);
   double u = rand() / (RAND_MAX + 1.0);
   int interval = -(log(1.0f - (float) u)/pps)*1000000;
   // interval+=self->linerate/(self->mean_pkt_size*8); // adjust to include transmission time of packet itself
   // printf("interval =%d\n", interval);
   return interval; 
}

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

void sink_init(SINK* self, SCHED* sched) {
    self->status=0;
    self->sched=sched;
    int i;
    for (i=0; i<10; i++ ) {
      self->pkt_rcvd[i]=0;
      self->bytes_rcvd[i]=0;
      self->delay_cumul[i]=0;
    }
}

SINK* sink_create(SCHED* sched) {
    SINK* obj=(SINK*) malloc(sizeof(SINK));
    sink_init(obj, sched);
    return obj;
}

void sink_put(SINK* self, packet *p) {
    int n = p->flow_id;
    if (n>=9) n=9;
    // printf("s: %d %d %d %d %d\n", self->sched->now, (self->sched->now-p->create_time), p->id, p->source, p->dest);
    self->pkt_rcvd[n]++;
    self->bytes_rcvd[n]+=p->size;
    self->delay_cumul[n]+=(self->sched->now-p->create_time);
    // printf("d: %d %d %d\n",self->sched->now, p->create_time, self->sched->now-p->create_time);
    packet_destroy(p);
    return;
}

void sink_stats(SINK* self) {
   int i;
   for (i=0; i<10; i++) {
      int n = (self->pkt_rcvd[i]>0) ? self->pkt_rcvd[i] : 1;
      printf("flow: %d\t - %ld\t%ld\t%ld\n", i, self->pkt_rcvd[i], self->bytes_rcvd[i]*8/(self->sched->finish), self->delay_cumul[i]/n);
   }
}