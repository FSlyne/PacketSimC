#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "packetgen.h"
#include <setjmp.h>

#define PI 3.14159265

void dist_init(DIST* self, SCHED* sched, int linerate, int mean_pkt_size) {
    self->mean_pkt_size=mean_pkt_size;
    self->sched=sched;
    self->linerate=linerate*self->sched->granularity;
}

DIST* dist_create(SCHED* sched, int linerate, int mean_pkt_size) {
    time_t t;
    srand((unsigned) time(&t));
    DIST* obj=(DIST*) malloc(sizeof(DIST));
    dist_init(obj, sched, linerate, mean_pkt_size);
    return obj;
}

long modulated_linerate(DIST* self, int freq) {
   int high, low, modulated_linerate;
   float angle;
   int amp, trans, maxtime;
   
   low=self->linerate/5; high=self->linerate;
   maxtime=self->sched->finish;
   amp=(high-low)/2;
   trans=(high+low)/2;

   float m = (float) self->sched->now/self->sched->granularity;
   angle= m*2*PI*freq;
   modulated_linerate=trans+amp*sin(angle);
   // printf("#%ld, %d, %f %f\n", self->sched->now, modulated_linerate, angle, sin(angle));
   return modulated_linerate;
}


long dist_exec(DIST* self) {
   int pps;
   // pps=modulated_linerate(self,5)/(8*self->mean_pkt_size); // modulated by sine function
   pps=self->linerate/(8*self->mean_pkt_size);
   // https://preshing.com/20111007/how-to-generate-random-timings-for-a-poisson-process/
   // https://www.codecogs.com/library/computing/c/math.h/log.php?alias=logf
   // https://stackoverflow.com/questions/34558230/generating-random-numbers-of-exponential-distribution
   // int interval= -(logf(1.0f - (float) rand() / (RAND_MAX + 1))/pps)*1000000; // Microseconds between end of one packet and start of next
   double u = rand() / (RAND_MAX + 1.0);
   long interval = -(log(1.0f - (float) u)/pps)*self->sched->granularity;
   // interval+=self->linerate/(self->mean_pkt_size*8); // adjust to include transmission time of packet itself
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

void  pkt_gen(PKT* self) {
   int stackspace[200000] ; stackspace[3]=45;
   while (self->sched->running > 0) {
         // preprocess
         //printf("Creating a packet\n");
         packet* p=packet_create((self->pktcnt)++, self->sched->now, self->source, self->dest,self->flow_id,
                                 ((DIST*) self->arrivalfntype)->mean_pkt_size);
         // postprocess
         // Need to replicate self.out.put(p) functionality
         waitfor(self->sched, self->arrivalfn(self->arrivalfntype));
         self->out(self->typex, p);
    }
}

void pkt_stats(PKT* self) {
   printf("Flow %d Packet Count %d Mean Packet Size %d Line Rate %d bps\n",
          self->flow_id, self->pktcnt, ((DIST*) self->arrivalfntype)->mean_pkt_size, ((DIST*) self->arrivalfntype)->linerate);
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
    // printf("s: %d %d >%d %d %d\n", self->sched->now, (self->sched->now-p->create_time), p->id, p->source, p->dest);
    self->pkt_rcvd[n]++;
    self->bytes_rcvd[n]+=p->size;
    self->delay_cumul[n]+=(self->sched->now-p->create_time);
    // printf("d: %ld %d %d\n",self->sched->now, p->create_time, self->sched->now-p->create_time);
    packet_destroy(p);
    return;
}

void sink_stats(SINK* self) {
   int i;
   printf("\n\nReceived Statistics\n");
   printf("Flows\tPackets\tbits/sec\tlatency(usec)\n");
   for (i=0; i<10; i++) {
      int n = (self->pkt_rcvd[i]>0) ? self->pkt_rcvd[i] : 1;
      printf("%d\t%ld\t%ld\t%ld\n", i, self->pkt_rcvd[i], self->bytes_rcvd[i]*8/(self->sched->finish), self->delay_cumul[i]/n);
   }
}