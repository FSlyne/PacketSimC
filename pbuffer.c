#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pbuffer.h"

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
   if (r<=0) return 1; // range is zero or negative
   int v=rand_drop(d*r, c);
   // printf("%d %d %d %d\n", d,r,c,v);
   return v;
}


void queue_insert(struct pbuffer **st, struct pbuffer **en,  packet* p, int key)
{
    struct pbuffer *newnode;
    struct pbuffer *idx, *tmp;

    newnode = (struct pbuffer *)malloc(sizeof(struct pbuffer));
    newnode->p=p;
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

void queue_lpush(struct pbuffer **st, struct pbuffer **en,  packet* p, int key)
{
    struct pbuffer *newnode;
    struct pbuffer *idx, *tmp;

    newnode = (struct pbuffer *)malloc(sizeof(struct pbuffer));
    newnode->p=p;
    newnode->key=key;
    
    if (*st == NULL && *en == NULL) { // zero nodes in list, insert new node
        newnode->next=NULL;
        newnode->prev=NULL;
        *st=newnode;
        *en=newnode;
        return;
    }
    
    newnode->next = *st;
    (*st)->prev = newnode;
    *st = newnode;
    newnode->prev = NULL;
    return ;
}


void queue_rpop(struct pbuffer **st, struct pbuffer **en,  packet **p, int *key)
{
    struct pbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        printf("The stack is empty!\n");
        return ;
        }
        
    top = *en;
    //p->id=top->p->id;
    //p->create_time=top->p->create_time;
    //p->source=top->p->source;
    //p->dest=top->p->dest;
    //p->size=top->p->size;
    //packet_copy(top->p, p);
    *p=top->p;
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


void queue_clear(struct pbuffer **st, struct pbuffer **en)
{
    struct pbuffer *top;
    
    while (!(*st == NULL && *en == NULL)) {   
        top = *st;       
        if (*st == *en) {
            *st = NULL;
            *en = NULL;
            free(top);
        } else {
            *st = (*st)->next;
            (*st)->prev=NULL;
            free(top);       
        }
    }
}

void packet_init(packet* self, int id, int t, int source, int dest, int flow_id, int size) {
   self->id=id;
   self->create_time=t;
   self->source=source;
   self->dest=dest;
   self->flow_id=flow_id;
   self->size=size;
}

packet* packet_create(int id, int t, int source, int dest, int flow_id, int size) {
    packet* obj=(packet*) malloc(sizeof(packet));
    packet_init(obj, id, t, source, dest, flow_id, size);
    return obj;
}

packet* packet_create_noinit() {
    packet* obj=(packet*) malloc(sizeof(packet));
    return obj;
}


void packet_copy(packet* from_p, packet* to_p) {
    to_p->id=from_p->id;
    to_p->create_time=from_p->create_time;
    to_p->source=from_p->source;
    to_p->dest=from_p->dest;
    to_p->size=from_p->size;
    to_p->flow_id=from_p->size;
}

void packet_destroy(packet* obj){
    if (obj) {
        free(obj);
    }
}

void queue_init(QUEUE* self, SCHED* sched, int linerate, int countlimit, int bytelimit, int latency){
    self->st=(struct pbuffer *) NULL;
    self->en=(struct pbuffer *) NULL;
    self->sched=sched;
    self->bytesize=0;
    self->countsize=0;
    self->linerate=linerate;
    self->countlimit=countlimit;
    self->bytelimit=bytelimit;
    self->latency=latency;
    self->myclock=0;
}

QUEUE* queue_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency){
    QUEUE* obj=(QUEUE*) malloc(sizeof(QUEUE));
    queue_init(obj, sched, linerate, countlimit, bytelimit, latency);
    return obj;
}

void queue_destroy(QUEUE* obj){
    if (obj) {
        free(obj);
    }
}

void queue_count(struct pbuffer **st, struct pbuffer **en)
{
    struct pbuffer *idx;
    int count=0;
    
    idx=*st;
    while (idx) {
        idx = idx->next;
        count++;
    }
    printf("Queue Nodes: %d ", count);
}


int  queue_gen(QUEUE* self) {
    if (self->status == 0) { // first time queue_gen  is run
        self->status = 1;
    }
    // preprocess
    int key;
//    packet* p=packet_create_noinit();
    packet* p;
//    queue_count(&(self->st), &(self->en));
    queue_rpop(&(self->st), &(self->en), &p, &key);
    self->countsize--;
    self->bytesize-=p->size;
    // postprocess
    // Need to replicate self.out.put(p) functionality
    self->out(self->typex, p);
    return self->sched->now;
}

void queue_put(QUEUE* self, packet* p){
   //printf("Queue size %d %d\n", self->countsize, self->bytesize);
   if (self->countlimit>0) {
      if (self->countsize >= self->countlimit) {
         printf("Dropping packet - count limit\n");
         packet_destroy(p);
         return;
      }
    }
    if (self->bytelimit>0) {
      if (self->bytesize >= self->bytelimit) {
         printf("Dropping packet - byte limit\n");
         packet_destroy(p);
         return;
      } 
    }
    self->countsize++;
    self->bytesize+=p->size;
    queue_insert(&self->st,&self->en,p, 0);
    int interval=p->size*8/self->linerate;
    self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
    self->myclock+=interval; // microseconds
   //printf("QSched: %d %d %d %d\n", self->sched->now, then, p->size, self->linerate);
    sched_reg_oneoff(self->sched, self, queue_gen, self->myclock+self->latency);
}

//void queue_get(QUEUE* self, packet* p){
//   int key;
//   queue_rpop(&self->st, &self->en, p, &key);
//}


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

void trtcm_init(TRTCM* self, SCHED* sched, int pir, int pbs, int cir, int cbs) {
   self->sched=sched;
   self->pir=pir;
   self->pbs=pbs;
   self->cir=cir;
   self->cbs=cbs;
   self->last_time=0;
   self->pbucket=pbs;
   self->cbucket=cbs;
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
   self->last_time=self->sched->now;
   self->pbucket+=self->pir*time_inc/(8000000); // rate in bits, bucket in bytes
   if (self->pbucket > self->pbs) self->pbucket=self->pbs;
   self->cbucket+=self->cir*time_inc/(8000000); // rate in bits, bucket in bytes
   if (self->cbucket > self->cbs) self->cbucket=self->cbs;
   if (self->pbucket-p->size < 0) {
      p->flow_id=2; // Mark as Red   
   } else if (self->cbucket-p->size < 0) {
      p->flow_id=1; // Mark as Yellow
      self->pbucket-=p->size;
   } else {
      p->flow_id=0; // Mark as Green
      self->pbucket-=p->size;
      self->cbucket-=p->size;
   }
   //printf("%d %d %d %d\n", self->sched->now, time_inc, self->cbucket, self->pbucket);
   self->out(self->typex, p);
} 


