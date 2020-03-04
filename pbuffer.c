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


void queue_pop(struct pbuffer **st, struct pbuffer **en,  packet *p, int *key)
{
    struct pbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        printf("The stack is empty!\n");
        return ;
        }
           
    top = *en;
    *p=*(top->p);
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

void packet_init(packet* self, int id, int t, int source, int dest) {
   self->id=id;
   self->create_time=t;
   self->source=source;
   self->dest=dest;
}

packet* packet_create(int id, int t, int source, int dest) {
    packet* obj=(packet*) malloc(sizeof(packet));
    packet_init(obj, id, t, source, dest);
    return obj;
}

void packet_destroy(packet* obj){
    if (obj) {
        free(obj);
    }
}

void queue_init(QUEUE* self, SCHED* sched){
    self->st=(struct pbuffer *) NULL;
    self->en=(struct pbuffer *) NULL;
    self->sched=sched;
}

QUEUE* queue_create(SCHED* sched){
    QUEUE* obj=(QUEUE*) malloc(sizeof(QUEUE));
    queue_init(obj, sched);
    return obj;
}

void queue_destroy(QUEUE* obj){
    if (obj) {
        free(obj);
    }
}

int  queue_gen(QUEUE* self) {
    if (self->status == 0) { // first time pkt_get is run
        self->status = 1;
    }
    // preprocess
    packet* p; int key;
    queue_pop(&self->st, &self->en, p, &key);
    // postprocess
    // Need to replicate self.out.put(p) functionality
    self->out(self->typex, p);
    return 1;
}

void queue_put(QUEUE* self, packet* p){
   queue_insert(&self->st,&self->en,p, 0);
   sched_reg_oneoff(self->sched, self, queue_gen, 0);
}

void queue_get(QUEUE* self, packet* p){
   int key;
   queue_pop(&self->st, &self->en, p, &key);
}
