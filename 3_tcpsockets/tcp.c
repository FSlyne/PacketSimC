
#include <stdlib.h>
#include <stdio.h>
#include "store.h"
#include "socket.h"
#include "tcp.h"
#include "packetgen.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */

//void tstore_insert(TSTORE* self,  tcpseg* s, int key)
//{
//    struct tcpbuffer *newnode;
//    struct tcpbuffer *idx, *tmp;
//    
//    //tstore_count(1, st,en);
//
//    newnode = (struct tcpbuffer *)malloc(sizeof(struct tcpbuffer));
//    newnode->s=s;
//    newnode->key=key;
//    
//    if (self->st == NULL && self->en == NULL) { // zero nodes in list, insert new node
//        newnode->next=NULL;
//        newnode->prev=NULL;
//        self->st=newnode;
//        self->en=newnode;
//        return;
//    }
//    idx=self->st;
//    while (idx) {
//        if (key >= idx->key)  // --> 7,6,5,4,3,2 --> 
//            break;
//        idx = idx->next;
//    }
//    if (idx == self->st) { // idx pointing to start, insert before this at beginning
//        newnode->next=self->st;
//        newnode->prev=NULL;
//        self->st->prev = newnode;
//        self->st = newnode;
//    } else if (idx == NULL) { // idx has reached end, insert at end
//        newnode->next=NULL;
//        newnode->prev=self->en;
//        self->en->next=newnode;
//        self->en = newnode;
//    } else { // idx is in between *st and *en, no change to st nor en
//        tmp=idx->prev; // this should be safe
//        tmp->next=newnode;
//        newnode->prev=tmp;
//        newnode->next=idx;
//        idx->prev=newnode;
//    }
//    return;
//}
//
//void tstore_insert_raw(struct tcpbuffer **st, struct tcpbuffer **en,  tcpseg* s, int key)
//{
//    struct tcpbuffer *newnode;
//    struct tcpbuffer *idx, *tmp;
//    
//    //tstore_count(1, st,en);
//
//    newnode = (struct tcpbuffer *)malloc(sizeof(struct tcpbuffer));
//    newnode->s=s;
//    newnode->key=key;
//    
//    if (*st == NULL && *en == NULL) { // zero nodes in list, insert new node
//        newnode->next=NULL;
//        newnode->prev=NULL;
//        *st=newnode;
//        *en=newnode;
//        return;
//    }
//    idx=*st;
//    while (idx) {
//        if (key >= idx->key)  // --> 7,6,5,4,3,2 --> 
//            break;
//        idx = idx->next;
//    }
//    if (idx == *st) { // idx pointing to start, insert before this at beginning
//        newnode->next=*st;
//        newnode->prev=NULL;
//        (*st)->prev = newnode;
//        *st = newnode;
//    } else if (idx == NULL) { // idx has reached end, insert at end
//        newnode->next=NULL;
//        newnode->prev=*en;
//        (*en)->next=newnode;
//        *en = newnode;
//    } else { // idx is in between *st and *en, no change to st nor en
//        tmp=idx->prev; // this should be safe
//        tmp->next=newnode;
//        newnode->prev=tmp;
//        newnode->next=idx;
//        idx->prev=newnode;
//    }
//    return;
//}
//
//void tstore_lpush(TSTORE* self, tcpseg* s, int key)
//{
//    struct tcpbuffer *newnode;
//
//    newnode = (struct tcpbuffer *)malloc(sizeof(struct tcpbuffer));
//    newnode->s=s;
//    newnode->key=key;
//    
//    if (self->st== NULL && self->en == NULL) { // zero nodes in list, insert new node
//        newnode->next=NULL;
//        newnode->prev=NULL;
//        self->st=newnode;
//        self->en=newnode;
//        return;
//    }
//    
//    newnode->next = self->st;
//    self->st->prev = newnode;
//    self->st = newnode;
//    newnode->prev = NULL;
//    return ;
//}
//
//
//void tstore_rpop(TSTORE* self,  tcpseg **s, int *key)
//{
//   
//    //tstore_count(2, st,en);
//    struct tcpbuffer *top;
//
//    if (self->st== NULL && self->en == NULL)
//        {
//        //printf("The queue stack is empty!\n");
//        *s=(tcpseg*) NULL;
//        return ;
//        }
//        
//    top = self->en;
//    //p->id=top->p->id;
//    //p->create_time=top->p->create_time;
//    //p->source=top->p->source;
//    //p->dest=top->p->dest;
//    //p->size=top->p->size;
//    //tcpseg_copy(top->p, p);
//    *s=top->s;
//    *key=top->key;
//    if (self->st == self->en) {
//        self->st = NULL;
//        self->en = NULL;
//        free(top);
//        return;
//    }
//
//    self->en = self->en->prev;
//    self->en->next=NULL;
//    free(top);
//    return ;
// 
//}
//
//void tstore_rpop_raw(struct tcpbuffer **st, struct tcpbuffer **en,  tcpseg **s, int *key)
//{
//   
//    //tstore_count(2, st,en);
//    struct tcpbuffer *top;
//
//    if (*st == NULL && *en == NULL)
//        {
//        //printf("The queue stack is empty!\n");
//        *s=(tcpseg*) NULL;
//        return ;
//        }
//        
//    top = *en;
//    //p->id=top->p->id;
//    //p->create_time=top->p->create_time;
//    //p->source=top->p->source;
//    //p->dest=top->p->dest;
//    //p->size=top->p->size;
//    //tcpseg_copy(top->p, p);
//    *s=top->s;
//    *key=top->key;
//    if (*st == *en) {
//        *st = NULL;
//        *en = NULL;
//        free(top);
//        return;
//    }
//
//    *en = (*en)->prev;
//    (*en)->next=NULL;
//    free(top);
//    return ;
// 
//}
//
//void tstore_read(TSTORE* self,   tcpseg **s, int *key)
//{
//    struct tcpbuffer *top;
//
//    if (self->en == NULL)
//        {
//        //printf("The queue stack is empty!\n");
//        *s=(tcpseg*) NULL;
//        return ;
//        }
//        
//    top = self->en;
//    *s=top->s;
//    *key=top->key;
//
//    self->en = self->en->prev;
//    self->en->next=NULL;
//    free(top);
//    return ;
// 
//}
//
//
//void tstore_clear(TSTORE* self)
//{
//    struct tcpbuffer *top;
//    
//    while (!(self->st == NULL && self->en == NULL)) {   
//        top = self->st;       
//        if (self->st == self->en) {
//            self->st = NULL;
//            self->en = NULL;
//            free(top);
//        } else {
//            self->st = self->st->next;
//            self->st->prev=NULL;
//            free(top);       
//        }
//    }
//}
//
//int tstore_count(TSTORE* self)
//{
//    struct tcpbuffer *idx;
//    int count=0;
//    
//    idx=self->st;
//    while (idx) {
//        idx = idx->next;
//        count++;
//    }
//    return count;
//}
//
//void tstore_init(TSTORE* self, SCHED* sched){
//    self->st=(struct tcpbuffer *) NULL;
//    self->en=(struct tcpbuffer *) NULL;
//    self->sched=sched;
//}
//
//TSTORE* tstore_create(SCHED* sched){
//    TSTORE* obj=(TSTORE*) malloc(sizeof(TSTORE));
//    tstore_init(obj, sched);
//    return obj;
//}
//
//void tstore_destroy(TSTORE* obj){
//    if (obj) {
//        free(obj);
//    }
//}
//
//void tstore_rpop_block(TSTORE* self, tcpseg **s, int *key)
//{
//   jmp_buf flag;
//   tstore_rpop(self, s, key);
//   while (*s == NULL) { // waiself->tstore,t for a tcpseg, queue is empty
//      self->myclock=(self->myclock>self->sched->now)?self->myclock:self->sched->now;
//      self->myclock+=10;
//      if (setjmp(flag) == 0) {
//         sched_yield(self->sched, flag, self->myclock);
//      } else {
//         tstore_rpop(self, s, key);
//      }
//   }
//}

void tcpseg_init(tcpseg* self, rawdata* rd) {
   self->rd=rd;
}

tcpseg* tcpseg_create(rawdata* rd) {
    tcpseg* obj=(tcpseg*) malloc(sizeof(tcpseg));
    tcpseg_init(obj, rd);
    return obj;
}


void tcpseg_destroy(tcpseg* obj){
    if (obj) {
        free(obj);
    }
}

void tsocket_init(TSOCKET* self, SCHED* sched, STORE* store, ASTORE* astore){
    self->sched=sched;
    self->store=store;
    self->astore=astore;
    self->mask=0;
}

TSOCKET* tsocket_create(SCHED* sched){
    STORE* store=store_create(sched);
    ASTORE* astore=astore_create(sched);
    TSOCKET* obj=(TSOCKET*) malloc(sizeof(TSOCKET));
    tsocket_init(obj, sched, store, astore);
    return obj;
}

void tsocket_destroy(TSOCKET* self){
    store_destroy(self->store);
    astore_destroy(self->astore);
    if (self) {
        free(self);
    }
}

unsigned int tsocket_select(TSOCKET* self) {
    unsigned int omask = 0;
    while (0<1) {
        omask = 0;
        if (astore_count(self->astore) > 0) omask |= 1 << 0;
        if (store_count(self->store) > 0) omask |= 1 << 1;
        if (omask > 0) return omask;
        waitfor(self->sched, 10);
    }
    return omask;
}

void  tsocket_gen(TSOCKET* self) {
    int stackspace[20000] ; stackspace[3]=45;
    rawdata* rd;
    packet* p;
    tcpseg* s;
    int key;
    int pktcnt=1;
    unsigned int mask = 0;
    while (self->sched->running > 0) {
         mask = tsocket_select(self); // block if no I/O 
         if (mask & 1) {
            astore_rpop(self->astore, &rd, &key);
            s=tcpseg_create(rd);
            p=packet_create(pktcnt++, self->sched->now, 0,0 ,0, 100);
            p->s = s;
            p->s->rd=rd;
            printf("~~ %d %d\n", p->id, p->s->rd->id);
            self->out1(self->typex1, p);
         }
         if (mask & 2) {
            int x = store_count(self->store);
            store_rpop(self->store, &p, &key);
            rd=p->s->rd;
            printf(">>> %d %d %d\n", p->id, rd->id, x);
            self->out0(self->typex0, rd);
            // tcpseg_destroy(s);
            printf("%d %d\n", key, p->id);
            packet_destroy(p);
         }         
    }
}

void tsocket_put0(TSOCKET* self, rawdata* rd){
    printf("--%d\n", rd->id);
    astore_insert(self->astore,rd, 0);
}

void tsocket_put1(TSOCKET* self, packet* p){
    printf("++ %d %d\n", p->id, p->s->rd->id);
    store_insert(self->store, p, 0);
}
