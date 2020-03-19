#include <stdlib.h>
#include <stdio.h>
#include "store.h"

void store_insert(STORE* self,  packet* p, int key)
{
    struct pbuffer *newnode;
    struct pbuffer *idx, *tmp;
    
    //store_count(1, st,en);

    newnode = (struct pbuffer *)malloc(sizeof(struct pbuffer));
    newnode->p=p;
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

void store_insert_raw(struct pbuffer **st, struct pbuffer **en,  packet* p, int key)
{
    struct pbuffer *newnode;
    struct pbuffer *idx, *tmp;
    
    //store_count(1, st,en);

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

void store_lpush(STORE* self, packet* p, int key)
{
    struct pbuffer *newnode;

    newnode = (struct pbuffer *)malloc(sizeof(struct pbuffer));
    newnode->p=p;
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


void store_rpop(STORE* self,  packet **p, int *key)
{
   
    //store_count(2, st,en);
    struct pbuffer *top;

    if (self->st== NULL && self->en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *p=(packet*) NULL;
        return ;
        }
        
    top = self->en;
    //p->id=top->p->id;
    //p->create_time=top->p->create_time;
    //p->source=top->p->source;
    //p->dest=top->p->dest;
    //p->size=top->p->size;
    //packet_copy(top->p, p);
    *p=top->p;
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

void store_rpop_raw(struct pbuffer **st, struct pbuffer **en,  packet **p, int *key)
{
   
    //store_count(2, st,en);
    struct pbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *p=(packet*) NULL;
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

void store_read(STORE* self,   packet **p, int *key)
{
    struct pbuffer *top;

    if (self->en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *p=(packet*) NULL;
        return ;
        }
        
    top = self->en;
    *p=top->p;
    *key=top->key;

    self->en = self->en->prev;
    self->en->next=NULL;
    free(top);
    return ;
 
}


void store_clear(STORE* self)
{
    struct pbuffer *top;
    
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

int store_count(STORE* self)
{
    struct pbuffer *idx;
    int count=0;
    
    idx=self->st;
    while (idx) {
        idx = idx->next;
        count++;
    }
    return count;
}

void store_init(STORE* self, SCHED* sched){
    self->st=(struct pbuffer *) NULL;
    self->en=(struct pbuffer *) NULL;
    self->sched=sched;
}

STORE* store_create(SCHED* sched){
    STORE* obj=(STORE*) malloc(sizeof(STORE));
    store_init(obj, sched);
    return obj;
}

void store_destroy(STORE* obj){
    if (obj) {
        free(obj);
    }
}

void store_rpop_block(STORE* self, packet **p, int *key)
{
   jmp_buf flag;
   store_rpop(self, p, key);
   while (*p == NULL) { // waiself->store,t for a packet, queue is empty
      self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
      self->myclock+=10;
      if (setjmp(flag) == 0) {
         sched_yield(self->sched, flag, self->myclock);
      } else {
         store_rpop(self, p, key);
      }
   }
}

