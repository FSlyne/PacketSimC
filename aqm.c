#include <stdlib.h>
#include <stdio.h>
#include "aqm.h"

int rand_drop(int n, int m) {
// drop m in n
   if (m>n) return 1;
   if (m==0) return 0;
   int v=rand()%n+1;
   if (v<m) return 1;
   return 0;
}

int wred_drop(int b, int e, int d, int c) {
// https://networklessons.com/cisco/ccie-routing-switching-written/wred-weighted-random-early-detection
// begin, end, denominator, count
   if (b<0 || e<0 || d<0 || c<0) return 0;
   if (e<b) return 0;
   if (e==b) return 1; // always drop
   if (c>e) return 1; // always drop
   c=c-b; // normalise count
   if (c<=0) return 0; // don't drop
   int r=e-b;
   if (r<=0) return 1; // drop if range is zero or negative
   int v=rand_drop(d*r, c);
   // printf("%d %d %d %d\n", d,r,c,v);
   return v;
}


void wred_init(WRED* self, SCHED* sched){
   self->sched=sched;
}

WRED* wred_create(SCHED* sched, int linerate){
    signed int t;
    srand((unsigned) time(&t));
    WRED* obj=(WRED*) malloc(sizeof(WRED));
    wred_init(obj, sched);
    obj->queue=(QUEUE*)queue_create(sched,linerate, 0, 0, 0); // Mbps, Packet Count limit, Packet Byte limit, latency (usec)
    return obj;
}

void wred_destroy(WRED* obj){
    queue_destroy(obj->queue);
    if (obj) {
        free(obj);
    }
}

void wred_put(WRED* self, packet* p) {
   if (p->flow_id == 1) {
      if (wred_drop(5,20,8,self->queue->countsize) == 0) { // Drop if wred_drop returns 1
         queue_put(self->queue, p);
      } else {
         printf("WRED: Packet Drop flow ID %d\n", p->flow_id);
      }
   } else {
      queue_put(self->queue, p);
   }
}