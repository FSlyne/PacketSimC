#include <stdlib.h>
#include <stdio.h>
#include "store.h"

void store_insert(struct pbuffer **st, struct pbuffer **en,  packet* p, int key)
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

void store_lpush(struct pbuffer **st, struct pbuffer **en,  packet* p, int key)
{
    struct pbuffer *newnode;

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


void store_rpop(struct pbuffer **st, struct pbuffer **en,  packet **p, int *key)
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

void store_read(struct pbuffer **en,  packet **p, int *key)
{
    struct pbuffer *top;

    if (*en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *p=(packet*) NULL;
        return ;
        }
        
    top = *en;
    *p=top->p;
    *key=top->key;

    *en = (*en)->prev;
    (*en)->next=NULL;
    free(top);
    return ;
 
}


void store_clear(struct pbuffer **st, struct pbuffer **en)
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

void store_count(int id, struct pbuffer **st, struct pbuffer **en)
{
    struct pbuffer *idx;
    int count=0;
    
    idx=*st;
    while (idx) {
        idx = idx->next;
        count++;
    }
    printf("Queue Nodes: %d %d\n", id, count);
}

void store_init(STORE* self, SCHED* sched){
    self->st=(struct pbuffer *) NULL;
    self->en=(struct pbuffer *) NULL;
    self->sched=sched;
    self->depleted=0;
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

void store_yield(STORE* self, jmp_buf flag) {
   if (self->st != NULL) {
      self->depleted=0;
      longjmp(flag,1); // store is not empty, return immediately
   }  
   //
   self->depleted=1;
   memcpy(self->flag, flag, sizeof(jmp_buf)); // save flag for later
   // 
   //int now;
   //jmp_buf flag2;
   //if (self->sched->st != NULL) { // find some event in the time domain to proceed
   //   sched_rpop(&(self->sched->st), &(self->sched->en), &flag2, &now);
   //   self->sched->now=now;
   //   longjmp(flag2,1);
   //}
   //printf("store_yield: exhaustion\n");
   longjmp(flag,1);  // ??????
}

