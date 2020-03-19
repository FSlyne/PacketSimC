#include <stdlib.h>
#include <stdio.h>
#include "dba.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */


void frame_insert(struct fbuffer **st, struct fbuffer **en, struct pbuffer *tlist_st, struct pbuffer *tlist_en, int grant_start, int grant_size)
{
    struct fbuffer *newnode;
    struct fbuffer *idx, *tmp;

    newnode = (struct fbuffer *)malloc(sizeof(struct fbuffer));
    newnode->grant_start=grant_start;
    newnode->grant_size=grant_size;
    newnode->tlist_st=tlist_st;
    newnode->tlist_en=tlist_en;
    
    if (*st == NULL && *en == NULL) { // zero nodes in list, insert new node
        newnode->next=NULL;
        newnode->prev=NULL;
        *st=newnode;
        *en=newnode;
        return;
    }
    idx=*st;
    while (idx) {
        if (grant_start >= idx->grant_start)  // --> 7,6,5,4,3,2 --> 
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

void frame_lpush(struct fbuffer **st, struct fbuffer **en, struct pbuffer **tlist_st, struct pbuffer **tlist_en, int grant_start, int grant_size)
{
    struct fbuffer *newnode;

    newnode = (struct fbuffer *)malloc(sizeof(struct fbuffer));
    newnode->grant_start=grant_start;
    newnode->grant_size=grant_size;
    newnode->tlist_st=*tlist_st;
    newnode->tlist_en=*tlist_en;
    // need to return a NULL pointer for tlist_st and tlist_en
    *tlist_st=(struct pbuffer *) NULL;
    *tlist_en=(struct pbuffer *) NULL;
    
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


void frame_rpop(struct fbuffer **st, struct fbuffer **en, struct pbuffer **tlist_st, struct pbuffer **tlist_en, int *grant_start, int *grant_size)
{
    struct fbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        *tlist_st=(struct pbuffer *) NULL;
        *tlist_en=(struct pbuffer *) NULL;
        return ;
        }
        
    top = *en;
    *tlist_st=top->tlist_st;
    *tlist_en=top->tlist_en;
    *grant_start=top->grant_start;
    *grant_size=top->grant_size;
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


void tcont_init(TCONT* self, SCHED* sched, int linerate, int countlimit, int bytelimit, int latency){
    self->st=(struct pbuffer *) NULL;
    self->en=(struct pbuffer *) NULL;
    self->sched=sched;
    self->bytesize=0;
    self->countsize=0;
    self->linerate=linerate;
    self->countlimit=countlimit;
    self->bytelimit=bytelimit;
    self->latency=latency;
    self->p=(packet*) NULL;
    self->needs=0;
    self->tries=1;
}

TCONT* tcont_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency){
    TCONT* obj=(TCONT*) malloc(sizeof(TCONT));
    tcont_init(obj, sched, linerate, countlimit, bytelimit, latency);
    return obj;
}

void tcont_destroy(TCONT* obj){
    if (obj) {
        free(obj);
    }
}

void tcont_count(struct pbuffer **st, struct pbuffer **en) {
    struct pbuffer *idx;
    int count=0;
    
    idx=*st;
    while (idx) {
        idx = idx->next;
        count++;
    }
    printf("Queue Nodes: %d ", count);
}


int  tcont_gen(TCONT* self) {
    if (self->status == 0) { // first time tcont_gen  is run
        self->status = 1;
    }
    // preprocess
    int key;
//    packet* p=packet_create_noinit();
    packet* p;
//    tcont_count(&(self->st), &(self->en));
    store_rpop(&(self->st), &(self->en), &p, &key);
    self->countsize--;
    self->bytesize-=p->size;
    // postprocess
    // Need to replicate self.out.put(p) functionality
    self->out(self->typex, p);
    return self->sched->now;
}

void tcont_control(TCONT* self, int slot, struct pbuffer **st, struct pbuffer **en) {
    int slotbytes=slot*4; // 32 bit words
    int key;
    while (1>0) {
      if (self->p == (packet*) NULL) {
         store_rpop(&(self->st), &(self->en), &(self->p), &key);
         if (self->p == (packet*) NULL) {
            self->needs=0;
            break;
         }
         self->needs=self->p->size;
      }
      if (self->needs <= slotbytes) {
         store_insert(st, en, self->p,0);
         // sched_reg_oneoff(self->sched, self, tcont_gen, self->sched->now+self->latency*self->tries); // send up stream
         // if (self->tries>1) printf("Fault\n");
         self->tries=1;
         self->p = (packet*) NULL;
         slotbytes-=self->needs;
      } else {
         self->needs-=slotbytes;
         self->tries++;
         break;
      }
    }
    return;
}

void tcont_put(TCONT* self, packet* p){
   //printf("Queue size %d %d\n", self->countsize, self->bytesize);
   if (self->countlimit>0) {
      if (self->countsize >= self->countlimit) {
         //printf("TCONT Dropping packet - count limit\n");
         packet_destroy(p);
         return;
      }
    }
    if (self->bytelimit>0) {
      if (self->bytesize >= self->bytelimit) {
         //printf("TCONT Dropping packet - byte limit\n");
         packet_destroy(p);
         return;
      } 
    }
    self->countsize++;
    self->bytesize+=p->size;
    p->enqueue_time=self->sched->now;
    store_insert(&self->st,&self->en,p, 0);
}


void dba_init(DBA* self, SCHED* sched, TCONT* tcont) {
    self->st_frame=(struct fbuffer *) NULL;
    self->en_frame=(struct fbuffer *) NULL;
    self->status=0;
    self->sched=sched;
    self->tcont=tcont;
}

DBA* dba_create(SCHED* sched, TCONT* tcont) {
    DBA* obj=(DBA*) malloc(sizeof(DBA));
    dba_init(obj, sched, tcont);
    return obj;
}

void dba_gen(DBA* self) {
    int stackspace[20000] ; stackspace[3]=45;
    int grant_start, grant_size, key;
    struct pbuffer *st_w, *st_r;
    struct pbuffer *en_w, *en_r;
    st_w=(struct pbuffer *) NULL;
    st_r=(struct pbuffer *) NULL;
    en_w=(struct pbuffer *) NULL;
    en_r=(struct pbuffer *) NULL;
    jmp_buf flag;
    packet* p;
    // Create the frame
    
    while (self->sched->now <= self->sched->finish*1000000) {
      tcont_control(self->tcont,400, &st_w, &en_w); // a single TCONT, slots (32 bt words)
      frame_lpush(&(self->st_frame), &(self->en_frame), &st_w, &en_w, 1, 100);
      //
      // Read the frame
      frame_rpop(&(self->st_frame), &(self->en_frame), &st_r, &en_r, &grant_start, &grant_size);
      while (!(st_r == NULL)) {
         // printf("grant_start: %d grant_size: %d\n", grant_start, grant_size);
         store_rpop(&st_r, &en_r, &p, &key);
         while(!(p == NULL)) {
            self->tcont->out(self->tcont->typex, p);
            // printf("d: %d %d %d %d\n",self->sched->now, p->id, p->create_time, self->sched->now-p->create_time);
            store_rpop(&st_r, &en_r, &p, &key);
         }
         frame_rpop(&(self->st_frame), &(self->en_frame), &st_r, &en_r, &grant_start, &grant_size);
       }
       
      if (setjmp(flag) == 0) {
         sched_yield(self->sched, flag, self->sched->now+125);
      } 
     }

}