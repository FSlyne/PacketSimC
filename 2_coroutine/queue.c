#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */

void queue_init(QUEUE* self, SCHED* sched, STORE* store, int linerate, int countlimit, int bytelimit, int latency){
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

QUEUE* queue_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency){
    STORE* store=store_create(sched);
    QUEUE* obj=(QUEUE*) malloc(sizeof(QUEUE));
    queue_init(obj, sched, store, linerate, countlimit, bytelimit, latency);
    return obj;
}

void queue_destroy(QUEUE* self){
    store_destroy(self->store);
    if (self) {
        free(self);
    }
}

void timer1000(QUEUE* self) { // simple timer example
   int stackspace[20000] ; stackspace[3]=45;
   while (0<1) {
      printf("%ld tick\n", self->sched->now);
      waitfor(self->sched, 1000);
   }
}

void  queue_gen(QUEUE* self) {
   int stackspace[20000] ; stackspace[3]=45;
    if (self->status == 0) { // first time queue_gen  is run
        self->status = 1;
    }
    packet* p;
    int key;
    sched_reg(self->sched, self, timer1000, 0); // example of spawning a sub process
    while (self->sched->now <= self->sched->finish*1000000) {
         store_rpop_block(self->store, &p, &key);
         self->countsize--;
         self->bytesize-=p->size;
         int interval=p->size*8/self->linerate;
         self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
         self->myclock+=interval; // microseconds
         // postprocess
         // Need to replicate self.out.put(p) functionality
         waituntil(self->sched,self->myclock+self->latency);
         // printf("%ld returning from scheduler %d\n", self->sched->now, p->flow_id);
         self->out(self->typex, p);
    }
}

void queue_put(QUEUE* self, packet* p){
   //printf("Queue size %d %d\n", self->countsize, self->bytesize);
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
    self->countsize++;
    self->bytesize+=p->size;
    p->enqueue_time=self->sched->now;
    store_insert(self->store,p, 0);
}