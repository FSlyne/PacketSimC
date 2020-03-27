#include <stdlib.h>
#include <stdio.h>
#include "pbuffer.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

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

void trtcm_init(TRTCM* self, SCHED* sched, int pir, int pbs, int cir, int cbs) {
   self->sched=sched;
   self->pir=pir;
   self->pbs=pbs;
   self->cir=cir;
   self->cbs=cbs;
   self->last_time=0;
   self->pbucket=pbs;
   self->cbucket=cbs;
   self->tupdate=0;
   self->pbucket_min=0;
   self->cbucket_min=0;
}

TRTCM* trtcm_create(SCHED* sched, int pir, int pbs, int cir, int cbs){
    TRTCM* obj=(TRTCM*) malloc(sizeof(TRTCM));
    trtcm_init(obj, sched, pir, pbs, cir, cbs);
    return obj;
}

void trtcm_destroy(TRTCM* obj){
    if (obj) {
        free(obj);
    }
}

void trtcm_put(TRTCM* self, packet* p){
   int time_inc=self->sched->now - self->last_time;
   float tmp;
   if (time_inc > self->tupdate) { // update pbucket and cbucket at longer intervals
      self->last_time=self->sched->now;
      tmp = (self->pir/8000000)*time_inc;
      self->pbucket+=tmp; // rate in bits, bucket in bytes
      if (self->pbucket > self->pbs) self->pbucket=self->pbs;
      tmp = (self->cir/8000000)*time_inc;
      self->cbucket+=tmp; // rate in bits, bucket in bytes
      if (self->cbucket > self->cbs) self->cbucket=self->cbs;
      //
      if (self->sched->debug > 0)
         printf("%ld %d %d %d\n", self->sched->now, time_inc, self->pbucket, self->cbucket);
   } else {
      self->pbucket_min=min(self->pbucket_min, self->pbucket);
      self->cbucket_min=min(self->cbucket_min, self->cbucket);      
   }
   // printf("%d %d %d %d\n", self->sched->now, time_inc, self->pbucket, self->cbucket);
   if (self->pbucket < p->size ) {
      p->flow_id=2; // Mark as Red   
   } else if (self->cbucket < p->size) {
      p->flow_id=1; // Mark as Yellow
      self->pbucket=self->pbucket - p->size;
   } else {
      p->flow_id=0; // Mark as Green
      self->pbucket=self->pbucket - p->size;
      self->cbucket=self->cbucket - p->size;
   }
   self->out(self->typex, p);
} 


