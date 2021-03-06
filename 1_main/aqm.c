#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "aqm.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

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

double rand_0_1(void)
{
    return rand() / ((double) RAND_MAX);
}

// WRED AQM

void wred_init(WRED* self, SCHED* sched){
   self->sched=sched;
}

WRED* wred_create(SCHED* sched, int linerate){
    srand((unsigned) time(NULL));
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
         p->enqueue_time=self->sched->now;
         queue_put(self->queue, p);
      } else {
         printf("WRED: Packet Drop flow ID %d\n", p->flow_id);
         free(p); // Check this !!!!
      }
   } else {
      p->enqueue_time=self->sched->now;
      queue_put(self->queue, p);
   }
}

// PIE
// https://www.scss.tcd.ie/publications/theses/diss/2017/TCD-SCSS-DISSERTATION-2017-046.pdf

void pie_init(PIE* self, SCHED* sched, int linerate, int countlimit, int bytelimit) {
   self->sched=sched;
   self->linerate=linerate;
   self->countlimit = countlimit;
   self->bytelimit = bytelimit;
   self->target=15900; // us, PI AQM Classic queue delay targets
   self->tupdate=16000; // us, PI Classic queue sampling interval
   self->alpha=10.0; // Hz^2, PI integral gain
   self->beta=100.0; // Hz^2, PI proportional gain
   self->p=0;
   self->cqdelay=0;
   self->pqdelay=0;
   self->myclock=0;
}

PIE* pie_create(SCHED* sched, int linerate, int countlimit, int bytelimit){
    srand((unsigned) time(NULL));
    PIE* obj=(PIE*) malloc(sizeof(PIE));
    pie_init(obj, sched, linerate, countlimit, bytelimit);
    return obj;
}

int pie_update(PIE* self) {
   self->p = (self->alpha*(self->cqdelay - self->target) + self->beta *(self->cqdelay - self->pqdelay))/1000000;
   printf("%f\t%d\t%d\t%d\t%d\t%d\n", self->p,self->alpha,self->beta,self->target, self->cqdelay, self->pqdelay);
   self->pqdelay=self->cqdelay;
   return self->sched->now+self->tupdate; // in milliseconds
}

int pie_gen(PIE* self) {
   int key;
   packet* p;
   queue_rpop(&(self->st), &(self->en), &p, &key);
   self->cqdelay=self->sched->now - p->enqueue_time;
   self->countsize--;
   self->bytesize-=p->size;
   self->out(self->typex, p);
   return self->sched->now;
}

void pie_put(PIE* self, packet* p) {
   if (self->countlimit>0) {
      if (self->countsize >= self->countlimit) {
         printf("PIE Dropping packet - count limit\n");
         packet_destroy(p);
         return;
      }
    }
    if (self->bytelimit>0) {
      if (self->bytesize >= self->bytelimit) {
         printf("PIE Dropping packet - byte limit\n");
         packet_destroy(p);
         return;
      } 
    }
   float n = rand_0_1();
   if (self->p > n) {
      //printf("PIE Dropping packet - AQM limit\n");
      packet_destroy(p);
      return;
   }
   self->countsize++;
   self->bytesize+=p->size;
   int interval=p->size*8/self->linerate;
   self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
   self->myclock+=interval; // microseconds
   sched_reg_oneoff(self->sched, self, pie_gen, self->myclock);
   queue_insert(&self->st,&self->en,p, 0); // 0 => higher priority, on left of queue
   p->enqueue_time=self->sched->now;
}

//  DUALQ AQM

void dualq_init(DUALQ* self, SCHED* sched, int linerate, int countlimit, int bytelimit){
   self->sched=sched;
   self->linerate=linerate*1000000;
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
   //
   self->MTU=1500;
   self->MAX_LINK_RATE=linerate;
   self->MIN_LINK_RATE=linerate/10;
   self->target=15900; // us, PI AQM Classic queue delay targets
   self->tupdate=16000; // us, PI Classic queue sampling interval
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
   self->minTh = 475.0; // us L4S min marking threshold in time units
   self->range = 525.0; // us Range of L4S ramp in time units
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
   self->cqdelay=0;
   self->lqdelay=0;
   self->prevq=0; self->curq=0;
   self->vtime=0;
}

DUALQ* dualq_create(SCHED* sched, int linerate, int countlimit, int bytelimit){
    srand((unsigned) time(NULL));
    DUALQ* obj=(DUALQ*) malloc(sizeof(DUALQ));
    dualq_init(obj, sched, linerate, countlimit, bytelimit);
    return obj;
}

int dualq_laqm(DUALQ* self) {
   if (self->lqdelay >= self->maxTh) {
      return 1;
   } else if (self->lqdelay > self->minTh) {
      return (self->lqdelay-self->minTh) / self->range;
   } else {
      return 0;
   }
}

int dualq_update(DUALQ* self) {
   self->curq=self->cqdelay; // ms
   self->p = self->p + self->alpha_U * (self->curq - self->target) + self->beta_U * (self->curq - self->prevq);
   self->p_CL = self->p * self->k; // Coupled L4S prob = base prob * coupling factor
   self->p_C = self->p^2; // Classic prob = (base prob)^2
   self->prevq=self->curq;
   return self->sched->now+self->tupdate;
}

int dualq_gen(DUALQ* self) {
   int key;
   packet* p;
   queue_rpop(&(self->st), &(self->en), &p, &key);
   if (p->flow_id == 0) {
      self->lqdelay=(self->sched->now-p->create_time);
      int pdash_L=dualq_laqm(self);
      self->p_L=max(pdash_L,self->p_CL);
      self->llpktcount--;
   } else {
      self->cqdelay=(self->sched->now-p->create_time);
      self->clpktcount--;
   }
   self->llpktcount=max(0,self->llpktcount);
   self->clpktcount=max(0,self->clpktcount);
   self->out(self->typex, p);
   return self->sched->now;
}

void dualq_put(DUALQ* self, packet* p) {
   self->vtime+=self->sched->now-self->last_update;
   self->last_update=self->sched->now;
   int c = self->llpktcount+self->clpktcount;
   self->maxqlen=max(c,self->maxqlen);
   if (p->flow_id == 0) {
      self->packets_HPrec++;
      self->llpktcount++;
      int interval=p->size*8/self->linerate;
      sched_reg_oneoff(self->sched, self, dualq_gen, self->sched->now+interval);
      queue_insert(&self->st,&self->en,p, 0); // 0 => higher priority, on left of queue
      p->enqueue_time=self->sched->now;
      return;
   } else {// 1 => lesser priority
      self->packets_LPrec++;
      float n = rand_0_1();
      if (self->p_C > n || self->p_C > self->p_Cmax) {
         self->packets_LPdrop++;
         //printf("Dropping\n");
         return;
      } else {
         //printf("Queuing Classic\n");
         self->packets_LPrec++;
         self->clpktcount++;        
         int interval=p->size*8/self->linerate;
         sched_reg_oneoff(self->sched, self, dualq_gen, self->sched->now+interval);
         queue_insert(&self->st,&self->en,p, 1); // 1 => lesser priority, on left of queue
         p->enqueue_time=self->sched->now;
      }
   }
}