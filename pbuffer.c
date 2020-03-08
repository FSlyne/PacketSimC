#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pbuffer.h"

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

void packet_init(packet* self, int id, int t, int source, int dest, int size) {
   self->id=id;
   self->create_time=t;
   self->source=source;
   self->dest=dest;
   self->size=size;
}

packet* packet_create(int id, int t, int source, int dest, int size) {
    packet* obj=(packet*) malloc(sizeof(packet));
    packet_init(obj, id, t, source, dest,size);
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
}

void packet_destroy(packet* obj){
    if (obj) {
        //free(obj);
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
   queue_lpush(&self->st,&self->en,p, 0);
   int interval=p->size*8/self->linerate;
   if (self->myclock>self->sched->now) {
      self->myclock=self->myclock;
   } else {
      self->myclock=self->sched->now;
   }
   self->myclock+=interval; // microseconds
   //printf("QSched: %d %d %d %d\n", self->sched->now, then, p->size, self->linerate);
   sched_reg_oneoff(self->sched, self, queue_gen, self->myclock);
}

void queue_get(QUEUE* self, packet* p){
   int key;
   queue_rpop(&self->st, &self->en, p, &key);
}
