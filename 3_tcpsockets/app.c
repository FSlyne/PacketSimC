#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "app.h"
#include "packetgen.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */

void astore_insert(ASTORE* self,  rawdata* rd, int key)
{
    struct abuffer *newnode;
    struct abuffer *idx, *tmp;
    
    //astore_count(1, st,en);

    newnode = (struct abuffer *)malloc(sizeof(struct abuffer));
    newnode->rd=rd;
    newnode->key=key;
    
    if (self->st == NULL && self->en == NULL) { // zero nodes in list, insert new node
        newnode->next=NULL;
        newnode->prev=NULL;
        self->st=newnode;
        self->en=newnode;
        return;
    }
    idx=self->st;
    while (idx) {
        if (key >= idx->key)  // --> 7,6,5,4,3,2 --> 
            break;
        idx = idx->next;
    }
    if (idx == self->st) { // idx pointing to start, insert before this at beginning
        newnode->next=self->st;
        newnode->prev=NULL;
        self->st->prev = newnode;
        self->st = newnode;
    } else if (idx == NULL) { // idx has reached end, insert at end
        newnode->next=NULL;
        newnode->prev=self->en;
        self->en->next=newnode;
        self->en = newnode;
    } else { // idx is in between *st and *en, no change to st nor en
        tmp=idx->prev; // this should be safe
        tmp->next=newnode;
        newnode->prev=tmp;
        newnode->next=idx;
        idx->prev=newnode;
    }
    return;
}

void astore_insert_raw(struct abuffer **st, struct abuffer **en,  rawdata* rd, int key)
{
    struct abuffer *newnode;
    struct abuffer *idx, *tmp;
    
    //astore_count(1, st,en);

    newnode = (struct abuffer *)malloc(sizeof(struct abuffer));
    newnode->rd=rd;
    newnode->key=key;
    
    if (*st == NULL && *en == NULL) { // zero nodes in list, insert new node
        newnode->next=NULL;
        newnode->prev=NULL;
        *st=newnode;
        *en=newnode;
        return;
    }
    idx=*st;
    while (idx) {
        if (key >= idx->key)  // --> 7,6,5,4,3,2 --> 
            break;
        idx = idx->next;
    }
    if (idx == *st) { // idx pointing to start, insert before this at beginning
        newnode->next=*st;
        newnode->prev=NULL;
        (*st)->prev = newnode;
        *st = newnode;
    } else if (idx == NULL) { // idx has reached end, insert at end
        newnode->next=NULL;
        newnode->prev=*en;
        (*en)->next=newnode;
        *en = newnode;
    } else { // idx is in between *st and *en, no change to st nor en
        tmp=idx->prev; // this should be safe
        tmp->next=newnode;
        newnode->prev=tmp;
        newnode->next=idx;
        idx->prev=newnode;
    }
    return;
}

void astore_lpush(ASTORE* self, rawdata* rd, int key)
{
    struct abuffer *newnode;

    newnode = (struct abuffer *)malloc(sizeof(struct abuffer));
    newnode->rd=rd;
    newnode->key=key;
    
    if (self->st== NULL && self->en == NULL) { // zero nodes in list, insert new node
        newnode->next=NULL;
        newnode->prev=NULL;
        self->st=newnode;
        self->en=newnode;
        return;
    }
    
    newnode->next = self->st;
    self->st->prev = newnode;
    self->st = newnode;
    newnode->prev = NULL;
    return ;
}


void astore_rpop(ASTORE* self,  rawdata **rd, int *key)
{
   
    //astore_count(2, st,en);
    struct abuffer *top;

    if (self->st== NULL && self->en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *rd=(rawdata*) NULL;
        return ;
        }
        
    top = self->en;
    //p->id=top->p->id;
    //p->create_time=top->p->create_time;
    //p->source=top->p->source;
    //p->dest=top->p->dest;
    //p->size=top->p->size;
    //tcpseg_copy(top->p, p);
    *rd=top->rd;
    *key=top->key;
    if (self->st == self->en) {
        self->st = NULL;
        self->en = NULL;
        free(top);
        return;
    }

    self->en = self->en->prev;
    self->en->next=NULL;
    free(top);
    return ;
 
}

void astore_rpop_raw(struct abuffer **st, struct abuffer **en,  rawdata **rd, int *key)
{
   
    //astore_count(2, st,en);
    struct abuffer *top;

    if (*st == NULL && *en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *rd=(rawdata*) NULL;
        return ;
        }
        
    top = *en;
    //p->id=top->p->id;
    //p->create_time=top->p->create_time;
    //p->source=top->p->source;
    //p->dest=top->p->dest;
    //p->size=top->p->size;
    //tcpseg_copy(top->p, p);
    *rd=top->rd;
    *key=top->key;
    if (*st == *en) {
        *st = NULL;
        *en = NULL;
        free(top);
        return;
    }

    *en = (*en)->prev;
    (*en)->next=NULL;
    free(top);
    return ;
 
}

void astore_read(ASTORE* self, rawdata **rd, int *key)
{
    struct abuffer *top;

    if (self->en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *rd=(rawdata*) NULL;
        return ;
        }
        
    top = self->en;
    *rd=top->rd;
    *key=top->key;

    self->en = self->en->prev;
    self->en->next=NULL;
    free(top);
    return ;
 
}


void astore_clear(ASTORE* self)
{
    struct abuffer *top;
    
    while (!(self->st == NULL && self->en == NULL)) {   
        top = self->st;       
        if (self->st == self->en) {
            self->st = NULL;
            self->en = NULL;
            free(top);
        } else {
            self->st = self->st->next;
            self->st->prev=NULL;
            free(top);       
        }
    }
}

int astore_count(ASTORE* self)
{
    struct abuffer *idx;
    int count=0;
    
    idx=self->st;
    while (idx) {
        idx = idx->next;
        count++;
    }
    return count;
}

void astore_init(ASTORE* self, SCHED* sched){
    self->st=(struct abuffer *) NULL;
    self->en=(struct abuffer *) NULL;
    self->sched=sched;
}

ASTORE* astore_create(SCHED* sched){
    ASTORE* obj=(ASTORE*) malloc(sizeof(ASTORE));
    astore_init(obj, sched);
    return obj;
}

void astore_destroy(ASTORE* obj){
    if (obj) {
        free(obj);
    }
}

void astore_rpop_block(ASTORE* self, rawdata **rd, int *key)
{
   jmp_buf flag;
   astore_rpop(self, rd, key);
   while (*rd == NULL) { // waiself->store,t for a packet, queue is empty
      self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
      self->myclock+=10;
      if (setjmp(flag) == 0) {
         sched_yield(self->sched, flag, self->myclock);
      } else {
         astore_rpop(self, rd, key);
      }
   }
}

void rawdata_init(rawdata* self, int id, int t, int size, int flow_id) {
   self->id=id;
   self->create_time=t;
   self->size=size;
   self->flow_id=flow_id;
}

rawdata* rawdata_create(int id, int t,  int size, int flow_id) {
    rawdata* obj=(rawdata*) malloc(sizeof(rawdata));
    rawdata_init(obj, id, t, size, flow_id);
    return obj;
}

rawdata* rawdata_create_noinit() {
    rawdata* obj=(rawdata*) malloc(sizeof(rawdata));
    return obj;
}

void rawdata_destroy(rawdata* obj){
    if (obj) {
        free(obj);
    }
}

void appgen_init(APPGEN* self, SCHED* sched, int flow_id) {
    self->pktcnt=0;
    self->sched=sched;
    self->flow_id=flow_id;
}

APPGEN* appgen_create(SCHED* sched, int flow_id) {
    APPGEN* obj=(APPGEN*) malloc(sizeof(APPGEN));
    appgen_init(obj, sched, flow_id);
    return obj;
}

void app_gen(APPGEN* self) {
   int stackspace[200000] ; stackspace[3]=45;
   while (self->sched->running > 0) {
         // preprocess
         //printf("Creating a packet\n");
         rawdata* rd=rawdata_create((self->pktcnt)++, self->sched->now, 
                                 ((DIST*) self->arrivalfntype)->mean_pkt_size, self->flow_id);
         // postprocess
         // Need to replicate self.out.put(p) functionality
         waitfor(self->sched, self->arrivalfn(self->arrivalfntype));
         self->out(self->typex, rd);
    }
}

void appgen_stats(APPGEN* self) {
   printf("Flow %d Packet Count %d Mean Packet Size %d Line Rate %d bps\n",
          self->flow_id, self->pktcnt, ((DIST*) self->arrivalfntype)->mean_pkt_size, ((DIST*) self->arrivalfntype)->linerate);
}

void appsink_init(APPSINK* self, SCHED* sched) {
    self->sched=sched;
    int i;
    for (i=0; i<10; i++ ) {
      self->pkt_rcvd[i]=0;
      self->bytes_rcvd[i]=0;
      self->delay_cumul[i]=0;
    }
}

APPSINK* appsink_create(SCHED* sched) {
    APPSINK* obj=(APPSINK*) malloc(sizeof(APPSINK));
    appsink_init(obj, sched);
    return obj;
}

void appsink_put(APPSINK* self, rawdata *rd) {
    int n = rd->flow_id;
    if (n>=9) n=9;
    // printf("s: %d %d >%d %d %d\n", self->sched->now, (self->sched->now-p->create_time), p->id, p->source, p->dest);
    self->pkt_rcvd[n]++;
    self->bytes_rcvd[n]+=rd->size;
    self->delay_cumul[n]+=(self->sched->now-rd->create_time);
    printf("d: %ld %d %d\n",self->sched->now, rd->create_time, self->sched->now-rd->create_time);
    rawdata_destroy(rd);
    return;
}

void appsink_stats(APPSINK* self) {
   int i;
   printf("\n\nReceived Statistics\n");
   printf("Flows\tPackets\tbits/sec\tlatency(usec)\n");
   for (i=0; i<10; i++) {
      int n = (self->pkt_rcvd[i]>0) ? self->pkt_rcvd[i] : 1;
      printf("%d\t%ld\t%ld\t%ld\n", i, self->pkt_rcvd[i], self->bytes_rcvd[i]*8/(self->sched->finish), self->delay_cumul[i]/n);
   }
}