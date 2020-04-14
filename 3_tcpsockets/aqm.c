#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include "aqm.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// http://ctips.pbworks.com/w/page/7277591/FNV%20Hash
#define FNV_PRIME_32 16777619
#define FNV_OFFSET_32 2166136261U


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

double rand_0_1(void) // random number betwixt 0 and 1
{
    return rand() / ((double) RAND_MAX);
}

// WRED AQM
void wred_init(WRED* self, SCHED* sched, STORE* store, int linerate, int countlimit, int bytelimit, int latency){
    self->sched=sched;
    self->store=store;
    self->bytesize=0;
    self->countsize=0;
    self->linerate=linerate;
    self->countlimit=countlimit;
    self->bytelimit=bytelimit;
    self->latency=latency;
    self->myclock=0;
}

WRED* wred_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency){
    STORE* store=store_create(sched);
    WRED* obj=(WRED*) malloc(sizeof(WRED));
    wred_init(obj, sched, store, linerate, countlimit, bytelimit, latency);
    return obj;
}


void wred_destroy(WRED* self){
    store_destroy(self->store);
    if (self) {
        free(self);
    }
}


void  wred_gen(int pid, WRED* self) {
    int stackspace[20000] ; stackspace[3]=45;
    packet* p;
    int key;
    while (self->sched->running > 0) {
         store_rpop_block(pid, self->store, &p, &key);
         self->countsize--;
         self->bytesize-=p->size;
         int interval=p->size*8/self->linerate;
         self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
         self->myclock+=interval; // microseconds
         // postprocess
         // Need to replicate self.out.put(p) functionality
         waituntil(self->sched,pid,self->myclock+self->latency);
         self->out(self->typex, p);
    }
}

void wred_put(WRED* self, packet* p){
   // printf("Queue size %d %d\n", self->countsize, self->bytesize);
   if (self->countlimit>0) {
      if (self->countsize >= self->countlimit) {
         //printf("QUEUE Dropping packet - count limit\n");
         packet_destroy(p);
         return;
      }
    }
    if (self->bytelimit>0) {
      if (self->bytesize >= self->bytelimit) {
         //printf("QUEUE Dropping packet - byte limit\n");
         packet_destroy(p);
         return;
      } 
    }
   if (p->flow_id == 1) {
      if (wred_drop(5,20,8,self->countsize) == 1) { // Drop if wred_drop returns 1
         printf("WRED: Packet Drop flow ID %d\n", p->flow_id);
         packet_destroy(p); // Check this !!!!
         return;
      } 
   }
    self->countsize++;
    self->bytesize+=p->size;
    p->enqueue_time=self->sched->now;
    store_insert(self->store,p, 0);
}

 //PIE
 // https://tools.ietf.org/pdf/rfc8033.pdf. Feb 2017
void pie_init(PIE* self, SCHED* sched, STORE* store, int linerate, int countlimit, int bytelimit) {
   self->sched=sched;
   self->store=store;
   self->linerate=linerate;
   self->countlimit = countlimit;
   self->bytelimit = bytelimit;
   self->target=20000*self->sched->usec; // us,  queue delay targets
   self->tupdate=15000*self->sched->usec; // us, queue sampling interval, section 13
   self->alpha= 3; // Hz^2, PI integral gain, section 13 says 0.125
   self->beta= 3; // Hz^2, PI proportional gain, section 13 says 1.25
   self->p=0;
   self->cqdelay=0;
   self->pqdelay=0;
   self->myclock=0;
   self->tupdate_last=0;
}

PIE* pie_create(SCHED* sched, int linerate, int countlimit, int bytelimit){
    srand((unsigned) time(NULL));
    STORE* store=store_create(sched);
    PIE* obj=(PIE*) malloc(sizeof(PIE));
    pie_init(obj, sched, store, linerate, countlimit, bytelimit);
    return obj;
}

void pie_destroy(PIE* self){
    store_destroy(self->store);
    if (self) {
        free(self);
    }
}

void pie_timer(int pid, PIE* self) { // pie update timer
   int stackspace[20000] ; stackspace[3]=45;
   while (0<1) {
      // Section 4.2. Drop probabilty calculations
      float p = (float) (self->alpha*(self->cqdelay - self->target) + self->beta *(self->cqdelay - self->pqdelay))/self->sched->granularity;
      if (self->p < 0.000001) {
         p/=2048;
      } else if (self->p < 0.00001) {
         p/=512;
      } else if (self->p <0.0001) {
         p/=128;
      } else if (self->p < 0.001) {
         p/=32;
      } else if (self->p < 0.01) {
         p/=8;
      } else if (self->p < 0.1) {
         p/=2;
      } else {
         p=p;
      }
      self->p +=p;
      if (self->cqdelay == 0 && self->pqdelay == 0) {
         self->p *= 0.98;
       }
       
       if (self->p < 0) self->p =0;
       if (self->p > 1) self->p =1;
      
       printf("%ld\t%f\t%f\t%f\t%d\t%d\t%d\n", self->sched->now, self->p,self->alpha,self->beta,self->target, self->cqdelay, self->pqdelay);
       self->pqdelay=self->cqdelay;
      waitfor(self->sched, pid, self->tupdate );
   }
}

void pie_gen(int pid, PIE* self) {
    int stackspace[20000] ; stackspace[3]=45;
    packet* p;
    int key;
    spawn(self->sched, pie_timer, self, 0); // spawn the pie timer subprocess
    while (self->sched->running > 0) {
      store_rpop_block(pid, self->store, &p, &key);
      self->cqdelay=self->sched->now - p->enqueue_time; // S4.3 latency calculation refers to Littles Law
      self->countsize--;
      self->bytesize-=p->size;
      int interval=p->size*8/self->linerate;
      self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
      self->myclock+=interval; // microseconds
      waituntil(self->sched,pid, self->myclock);
      //waituntil(self->sched,self->sched->now); // what is the queue handling delay required here
      //printf("%ld returning from scheduler %d\n", self->sched->now, p->flow_id);
      self->out(self->typex, p);
    }
}

void pie_put(PIE* self, packet* p) {
   if (self->countlimit>0) {
      if (self->countsize >= self->countlimit) {
         if (self->sched->debug > 0) printf("PIE Dropping packet - count limit\n");
         packet_destroy(p);
         return;
      }
    }
    if (self->bytelimit>0) {
      if (self->bytesize >= self->bytelimit) {
         if (self->sched->debug > 0) printf("PIE Dropping packet - byte limit\n");
         packet_destroy(p);
         return;
      } 
    }
    // Section 4.1 Random Dropping
   float n = rand_0_1();
   if ((self->pqdelay < self->target && self->p < 0.2) ||
            (self->countsize<10) ) {
      goto ENQUEUE;
   } else if (self->p > n) {
      if (self->sched->debug > 0) printf("PIE Dropping packet - AQM limit\n");
      packet_destroy(p);
      return;
   }
   ENQUEUE:
   self->countsize++;
   self->bytesize+=p->size;
   int interval=p->size*8/self->linerate;
   self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
   self->myclock+=interval; // microseconds
   store_insert(self->store,p, 0); // 0 => higher priority, on left of queue
   p->enqueue_time=self->sched->now;
}

// http://ctips.pbworks.com/w/page/7277591/FNV%20Hash
uint32_t hash32(packet* p) {
   uint32_t hash = FNV_OFFSET_32;
   hash = hash ^ p->source;
   hash=hash*FNV_PRIME_32;
   hash = hash ^ p->dest;
   hash=hash*FNV_PRIME_32;
   return hash;
}

// Qprotect
// https://tools.ietf.org/id/draft-briscoe-docsis-q-protection-00.html
void qprotect_init(QPROT* self, SCHED* sched){
   int linerate=10000000;
   self->sched=sched;
   //
   self->MTU=1500;
   self->MAX_LINK_RATE=linerate;
   self->MIN_LINK_RATE=linerate/10;
    // L4S ramp AQM parameters
   self->minTh = 475.0*self->sched->usec; // us L4S min marking threshold in time units
   self->range = 525.0*self->sched->usec; // us Range of L4S ramp in time units
   self->Th_len = 2.0 * self->MTU; // Min L4S marking threshold in bytes
   // Constants derived from L4S AQM parameters
   self->floor = self->Th_len * 8.0 / self->MIN_LINK_RATE; // MIN_LINK_RATE is in Mb/s
   if (self->minTh < self->floor) {
       // Adjust ramp to exceed serialization time of 2 MTU
       self->range = max(self->range - (self->floor-self->minTh), 1); // 1us avoids /0 error
       self->minTh = self->floor;
   }
   self->maxTh = self->minTh+self->range; // L4S min marking threshold in time units   
    self->criticalql_us = self->maxTh/1000; // C.2.2.7.17.8
    self->criticalql = self->criticalql_us*1000;
    self->lg_aging = 19; // 2^19 bytes/sec. C.2.2.7.17.10
    self->aging = pow(2, self->aging-30);  // Convert lg([B/s]) to [B/ns]
    self->aging_us = self->aging*1000; // B/usec
    self->criticalqlscore_us = 4000; // C.2.2.7.17.9
    self->criticalqlscore=self->criticalqlscore_us*1000;
    self->criticalqlproduct=self->criticalql*self->criticalqlscore;
    self->criticalqlproduct_us=self->criticalql_us * self->criticalqlscore_us;
}

    
QPROT* qprotect_create(SCHED* sched){
    QPROT* obj=(QPROT*) malloc(sizeof(QPROT));
    qprotect_init(obj, sched);
    return obj;
}


void qprotect_destroy(QPROT* self){
    if (self) {
        free(self);
    }
}

int pick_bucket(QPROT* self, packet* p) {
   int j;
   uint32_t h32;
   int h;
   h32=hash32(p);
   int hsav = NBUCKETS;
   for (j=0; j<ATTEMPTS; j++) {
      h =  (int) h32 & (int) MASK;
      if (self->buckets[h].id == h32) {
         if (self->buckets[h].t_exp < self->sched->now)
            self->buckets[h].t_exp = self->sched->now;
         return h;
      } else if ((hsav == NBUCKETS) && (self->buckets[h].t_exp <= self->sched->now) ) {
         hsav = h;
      }
      h32 >>= BI_SIZE;
   }
   if (hsav != NBUCKETS) {
      self->buckets[hsav].t_exp = self->sched->now;
   } else {
      if (self->buckets[hsav].t_exp <= self->sched->now) {
         self->buckets[hsav].t_exp = self->sched->now;
      }
   }
   self->buckets[hsav].id=h32;
   return hsav;
}

long fill_bucket(QPROT* self, int bckt_id, packet* p, float probNative) {
   self->buckets[bckt_id].t_exp += (probNative * p->size / self->aging);
   return (self->buckets[bckt_id].t_exp - self->sched->now);
}

int qprotect(QPROT* self, DUALQ* dualq, packet* p) {
   int bckt_id;
   float qLscore;
   bckt_id=pick_bucket(self, p);
   float probNative = dualq_laqm(dualq);
   qLscore=fill_bucket(self, bckt_id, p, probNative);
   printf("%d %f %f %d %d %d\n", bckt_id, qLscore, probNative, dualq->lqdelay, self->criticalql, self->criticalqlproduct);
   if ((dualq->lqdelay > self->criticalql) && (dualq->lqdelay * qLscore > self->criticalqlproduct))
      return 1;
   else
      return 0;
}

  // DUALQ AQM
  // https://specification-search.cablelabs.com/CM-SP-MULPIv3.1
  // https://tools.ietf.org/pdf/draft-ietf-tsvwg-aqm-dualq-coupled-08.pdf
void dualq_init(DUALQ* self, SCHED* sched, STORE* store, int linerate, int countlimit, int bytelimit){
   self->sched=sched;
   self->store=store;
   self->linerate=linerate*self->sched->granularity;
   self->packets_rec=0;
   self->packets_drop=0;
   self->countlimit = countlimit;
   self->bytelimit = bytelimit;
   self->byte_size = 0;  //Current size of the queue in bytes
   self->last_update=0.0;
   self->llpktcount=0;
   self->clpktcount=0;
   self->packets_HPrec = 0;
   self->packets_HPdrop = 0;
   self->packets_LPrec = 0;
   self->packets_LPdrop = 0;
   self->maxqlen=0;
   self->myclock=0;
   //
   self->MTU=1500;
   self->MAX_LINK_RATE=linerate;
   self->MIN_LINK_RATE=linerate/10;
   self->target=15900*self->sched->usec; // us, PI AQM Classic queue delay targets
   self->tupdate=16000*self->sched->usec; // us, PI Classic queue sampling interval
   self->alpha=10.0; // Hz^2, PI integral gain
   self->beta=100.0; // Hz^2, PI proportional gain
   self->p_Cmax=0.25;
   // Constants derived from PI2 AQM parameters
   self->alpha_U = self->alpha *self->tupdate; // PI integral gain per update interval
   self->beta_U = self->beta * self->tupdate;  // PI prop.nal gain per update interval
   // DualQ Coupled framework parameters
   self->k = 2; // Coupling factor
   // scheduler weight or equival.t parameter (scheduler-dependent)
   self->limit = self->MAX_LINK_RATE * 250000; // us Dual buffer size
   // L4S ramp AQM parameters
   self->minTh = 475.0*self->sched->usec; // us L4S min marking threshold in time units
   self->range = 525.0*self->sched->usec; // us Range of L4S ramp in time units
   self->Th_len = 2.0 * self->MTU; // Min L4S marking threshold in bytes
   // Constants derived from L4S AQM parameters
   self->p_Lmax = min(self->k*sqrt(self->p_Cmax), 1); // Max L4S marking prob
   self->floor = self->Th_len * 8.0 / self->MIN_LINK_RATE; // MIN_LINK_RATE is in Mb/s
   if (self->minTh < self->floor) {
       // Adjust ramp to exceed serialization time of 2 MTU
       self->range = max(self->range - (self->floor-self->minTh), 1); // 1us avoids /0 error
       self->minTh = self->floor;
   }
   self->maxTh = self->minTh+self->range; // L4S min marking threshold in time units
   self->p=0;
   self->cqdelay=0.0;
   self->lqdelay=0.0;
   self->vtime=0;
   self->tupdate_last=0;
   self->qprot=qprotect_create(sched);
}

DUALQ* dualq_create(SCHED* sched, int linerate, int countlimit, int bytelimit){
    srand((unsigned) time(NULL));
    DUALQ* obj=(DUALQ*) malloc(sizeof(DUALQ));
    STORE* store=store_create(sched);
    dualq_init(obj, sched, store, linerate, countlimit, bytelimit);
    return obj;
}

int dualq_laqm(DUALQ* self) { // is this ProbNative?
   if (self->lqdelay >= self->maxTh) {
      return 1;
   } else if (self->lqdelay > self->minTh) {
      return (self->lqdelay-self->minTh) / self->range;
   } else {
      return 0;
   }
}

//int dualq_update(DUALQ* self) {
//   self->curq=self->cqdelay; // ms
//   self->p = self->p + self->alpha_U * (self->curq - self->target) + self->beta_U * (self->curq - self->prevq);
//   self->p_CL = self->p * self->k; // Coupled L4S prob = base prob * coupling factor
//   self->p_C = self->p^2; // Classic prob = (base prob)^2
//   self->prevq=self->curq;
//   return self->sched->now+self->tupdate;
//}

void dualq_timer(int pid, DUALQ* self) { // dualq update timer
   int stackspace[20000] ; stackspace[3]=45;
   while (0<1) {
      self->p = self->p + (float) (self->alpha_U * (self->cqdelay - self->target) +
                                          self->beta_U * (self->cqdelay - self->pqdelay))/self->sched->granularity;
      if (self->p < 0) self->p =0;
      if (self->p > 1) self->p =1;
      self->p_CL = self->p * self->k; // Coupled L4S prob = base prob * coupling factor
      self->p_C = (self->p)*(self->p); // Classic prob = (base prob)^2
      printf("%ld\t%f\t%f\t%f\t%d\t%d\t%d\n",
             self->sched->now, self->p,self->alpha_U,self->beta_U,self->target, self->cqdelay, self->pqdelay);
      self->pqdelay=self->cqdelay;
      waitfor(self->sched, pid, self->tupdate );
   }
}

void dualq_gen(int pid, DUALQ* self) {
    int stackspace[20000] ; stackspace[3]=45;
    packet* p;
    int key;
    spawn(self->sched, dualq_timer, self, 0); // spawn the dualq timer subprocess
    while (self->sched->running > 0) {
      // self->pqdelay=self->cqdelay;
         store_rpop_block(pid, self->store, &p, &key);
         if (p->flow_id == 0) {
            int redirect = qprotect(self->qprot, self, p);
            if (redirect > 0) {
               p->flow_id = 1;  
            }
         }
         if (p->flow_id == 0) {
            self->lqdelay=(self->sched->now-p->create_time);
            int pdash_L=dualq_laqm(self);
            self->p_L=max(pdash_L,self->p_CL);
            self->llpktcount--;
         } else {
            self->cqdelay= self->sched->now-p->create_time;
            self->clpktcount--;
         }
         self->llpktcount=max(0,self->llpktcount);
         self->clpktcount=max(0,self->clpktcount);
      int interval=p->size*8*self->sched->granularity/self->linerate;
      //printf("interval: %d\n", interval);
      self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
      self->myclock+=interval; // microseconds
      waituntil(self->sched,pid, self->myclock); // what is the queue handline delay here
      //printf("%ld returning from scheduler %d\n", self->sched->now, p->flow_id);
      self->out(self->typex, p);
    }
}

void dualq_put(DUALQ* self, packet* p) {
   self->vtime+=self->sched->now-self->last_update;
   self->last_update=self->sched->now;
   int c = self->llpktcount+self->clpktcount;
   self->maxqlen=max(c,self->maxqlen);
   if (p->flow_id == 0) {
      self->packets_HPrec++;
      self->llpktcount++;
      //int interval=p->size*8/self->linerate;
      store_insert(self->store,p, 0); // 0 => higher priority, on left of queue
      p->enqueue_time=self->sched->now;
      return;
   } else {// 1 => lesser priority
      self->packets_LPrec++;
      float n = rand_0_1();
      // printf("%.3f %.3f %.3f\n", n, self->p_C, self->p_Cmax);
      if (self->p_C > n || self->p_C > self->p_Cmax) {
         self->packets_LPdrop++;
         // printf("Dropping\n");
         return;
      } else {
         // printf("Queuing Classic\n");
         self->packets_LPrec++;
         self->clpktcount++;        
         //int interval=p->size*8/self->linerate;
         store_insert(self->store,p, 1); // 1 => lesser priority, on left of queue
         p->enqueue_time=self->sched->now;
      }
   }
}