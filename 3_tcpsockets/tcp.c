
#include <stdlib.h>
#include <stdio.h>
#include "store.h"
#include "socket.h"
#include "tcp.h"
#include "app.h"
#include "packetgen.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */

void tstore_insert_unique(TSTORE* self,  tcpseg* s, int key)
{
    struct tcpbuffer *newnode;
    struct tcpbuffer *idx, *tmp;
    
    //tstore_count(1, st,en);

    newnode = (struct tcpbuffer *)malloc(sizeof(struct tcpbuffer));
    newnode->s=s;
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
        if (key ==  idx->key) { // identical, so duplicate key
            return;
        } else if (key > idx->key) {  // --> 7,6,5,4,3,2 --> 
            break;
        }
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

void tstore_insert_unique_raw(struct tcpbuffer **st, struct tcpbuffer **en,  tcpseg* s, int key)
{
    struct tcpbuffer *newnode;
    struct tcpbuffer *idx, *tmp;
    
    //tstore_count(1, st,en);

    newnode = (struct tcpbuffer *)malloc(sizeof(struct tcpbuffer));
    newnode->s=s;
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
        if (key == idx->key) { // identical, so duplicate key
            return;
        } else if (key > idx->key) { // --> 7,6,5,4,3,2 --> 
            break;
        }
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

void tstore_lpush(TSTORE* self, tcpseg* s, int key)
{
    struct tcpbuffer *newnode;

    newnode = (struct tcpbuffer *)malloc(sizeof(struct tcpbuffer));
    newnode->s=s;
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


void tstore_rpop(TSTORE* self,  tcpseg **s, int *key)
{
    //tstore_count(2, st,en);
    struct tcpbuffer *top;

    if (self->st== NULL && self->en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *s=(tcpseg*) NULL;
        return ;
        }
        
    top = self->en;
    *s=top->s;
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

void tstore_rget(TSTORE* self,  tcpseg **s, int *key)
{
    // rdr points to node that was read
    if (self->st== NULL && self->en == NULL) // null list
        {
        //printf("The queue stack is empty!\n");
        *s=(tcpseg*) NULL;
        self->rdr = NULL;
        return;
        }
    if (self->en == self->st) { // single list
        if (self->rdr == NULL) { // have not read node
            self->rdr = self->en;
        } else { // have read the node
            *s=(tcpseg*) NULL;
            return;
        }
    } else if (self->rdr == self->st) { // multi-nodes, and have read the start of list (left)
        *s=(tcpseg*) NULL;
        return;
    } else if (self->rdr == NULL) {
        self->rdr = self->en;
    }
    
    *s = tcpseg_copy(self->rdr->s);
    *key = self->rdr->key;
    self->rdr = self->rdr->prev;
}

void tstore_rpop_raw(struct tcpbuffer **st, struct tcpbuffer **en,  tcpseg **s, int *key)
{
   
    //tstore_count(2, st,en);
    struct tcpbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *s=(tcpseg*) NULL;
        return ;
        }
        
    top = *en;
    //p->id=top->p->id;
    //p->create_time=top->p->create_time;
    //p->source=top->p->source;
    //p->dest=top->p->dest;
    //p->size=top->p->size;
    //tcpseg_copy(top->p, p);
    *s=top->s;
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

void tstore_read(TSTORE* self,   tcpseg **s, int *key)
{
    struct tcpbuffer *top;

    if (self->en == NULL)
        {
        //printf("The queue stack is empty!\n");
        *s=(tcpseg*) NULL;
        return ;
        }
        
    top = self->en;
    *s=top->s;
    *key=top->key;

    self->en = self->en->prev;
    self->en->next=NULL;
    free(top);
    return ;
 
}


void tstore_clear(TSTORE* self)
{
    struct tcpbuffer *top;
    
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

int tstore_count(TSTORE* self)
{
    struct tcpbuffer *idx;
    int count=0;
    
    idx=self->st;
    while (idx) {
        idx = idx->next;
        count++;
    }
    return count;
}

void tstore_del_upto(TSTORE* self, int max) {
    
    struct tcpbuffer *st, *en, *idx;
    st=NULL; en=NULL;
    
    //printf(" %d: ", max);
    idx=self->st;
    while (idx) {
        if (idx->key > max) { // --> 7,6,5,4,3,2 -->
            //printf("+%d ", idx->s->num);
            tcpseg* s_copy = tcpseg_copy(idx->s);
            tstore_insert_unique_raw(&st, &en, s_copy, idx->key);
            if (self->rdr == idx) {
                //printf("r%d ", idx->s->num);
                self->rdr = st;
            }
        } else {  
            //printf("-%d", idx->s->num);
        }
        idx = idx->next;
    }
    //printf("\n"); 
    tstore_clear(self);
    self->st=st;
    self->en=en;
    if (self->rdr != st) {
        self->rdr = self->en;
    }
}

void tstore_init(TSTORE* self, SCHED* sched){
    self->st=(struct tcpbuffer *) NULL;
    self->en=(struct tcpbuffer *) NULL;
    self->rdr=(struct tcpbuffer *) NULL;
    self->sched=sched;
}

TSTORE* tstore_create(SCHED* sched){
    TSTORE* obj=(TSTORE*) malloc(sizeof(TSTORE));
    tstore_init(obj, sched);
    return obj;
}

void tstore_destroy(TSTORE* obj){
    if (obj) {
        free(obj);
    }
}

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

void tstore_rpop_block(int pid, TSTORE* self, tcpseg **s, int *key)
{
    tstore_rpop(self, s, key);
    while (*s == NULL) { // waiself->tstore,t for a tcpseg, queue is empty
        waitfor(self->sched, pid, 10);
        tstore_rpop(self, s, key);
    }
}

void tstore_rget_block(int pid, TSTORE* self, tcpseg **s, int *key) {
    tstore_rget(self, s, key);
    while (*s == NULL) { // waiself->tstore,t for a tcpseg, queue is empty
        waitfor(self->sched, pid, 10);
        tstore_rget(self, s, key);
    }
}

void tcpseg_init(tcpseg* self, rawdata* rd, int num, int type, long ctime, int txwnd) {
   self->rd=rd;
   self->num=num;
   self->type=type;
   self->ctime=ctime;
   self->wnd=txwnd;
}

tcpseg* tcpseg_create(rawdata* rd, int num, int type, long ctime, int txwnd) {
    tcpseg* obj=(tcpseg*) malloc(sizeof(tcpseg));
    tcpseg_init(obj, rd, num, type, ctime, txwnd);
    return obj;
}


void tcpseg_destroy(tcpseg* obj){
    if (obj) {
        free(obj);
    }
}

tcpseg* tcpseg_copy(tcpseg* target) {
    tcpseg* obj=(tcpseg*) malloc(sizeof(tcpseg));
    obj->rd=rawdata_copy(target->rd);
    obj->num=target->num;
    obj->type=target->type;
    obj->ctime=target->ctime;
    obj->wnd=target->wnd;
    return obj;
}

void tsocket_init(TSOCKET* self, SCHED* sched, STORE* store, ASTORE* astore, TSTORE* txstore,  TSTORE* rxstore){
    self->sched=sched;
    self->store=store;
    self->astore=astore;
    self->txstore=txstore;
    self->rxstore=rxstore;
    self->mask=0;
    self->txwnd=10;
    self->rxwnd=10;
    self->lastackrcvd=0;
    self->RTTm=0; // measured RTT; https://www.geeksforgeeks.org/tcp-timers/
    self->RTTs=0; // smoothed RTT;
    self->RTTd=0; // deviated RTT;
    self->RTO=10; // retransmission Timeout
}

TSOCKET* tsocket_create(SCHED* sched){
    STORE* store=store_create(sched);
    ASTORE* astore=astore_create(sched);
    TSTORE* txstore=tstore_create(sched);
    TSTORE* rxstore=tstore_create(sched);
    TSOCKET* obj=(TSOCKET*) malloc(sizeof(TSOCKET));
    tsocket_init(obj, sched, store, astore, txstore, rxstore);
    return obj;
}

void tsocket_destroy(TSOCKET* self){
    store_destroy(self->store);
    astore_destroy(self->astore);
    tstore_destroy(self->txstore);
    tstore_destroy(self->rxstore);
    if (self) {
        free(self);
    }
}

void tseg_ack(TSOCKET* self, int acknum, long ctime) {
    tcpseg* s;
    packet* p;
    rawdata* rd = (rawdata *) NULL;
    s=tcpseg_create(rd, acknum, 2, ctime,self->txwnd); // 2 = ack
    p=packet_create(4567, self->sched->now, 0,0 ,0, 100);
    p->s=s;
    self->out1(self->typex1, p);    
}

void tseg_transmit(int pid, TSOCKET* self)  {
    int stackspace[20000] ; stackspace[3]=45;
    int pktcnt=0;
    int key=0;
    tcpseg* s;
    packet* p;
    while (self->sched->running > 0) {
        do {
            waitfor(self->sched, pid, 10);
            //printf("%d %d\n", key, self->rwnd+self->lastackrcvd);
        } while (key > self->txwnd+self->lastackrcvd);
        tstore_rget_block(pid, self->txstore, &s, &key);
        //printf("<<%d %d\n", tstore_count(self->txstore), key);
        p=packet_create(pktcnt++, self->sched->now, 0,0 ,0, 100);
        p->s = s;
        self->out1(self->typex1, p);
        //if (pktcnt++ >= self->rwnd) {
        //    tstore_del_upto(self->txstore, key); // dummy delete
        //    pktcnt=0;
        //}
    }
}

void tseg_receive(int pid, TSOCKET* self)  {
    int stackspace[20000] ; stackspace[3]=45;
    int key;
    tcpseg* s;
    int rcount=0;
    int lastacksent=0;
    while (self->sched->running > 0) {
        tstore_rpop_block(pid, self->rxstore, &s, &key);
        self->out0(self->typex0, s->rd); // out to app interface
        // printf(">>%d %d\n", tstore_count(self->rxstore), key);
        if (key >= self->rxwnd + lastacksent) {
            // printf("sending ack %d %d\n", lastacksent, key);
            tseg_ack(self, key,self->sched->now);
            lastacksent=key;
            rcount=0;
        }
        tcpseg_destroy(s);
    }
}

unsigned int tsocket_select(int pid, TSOCKET* self) {
    unsigned int omask = 0;
    while (0<1) {
        omask = 0;
        if (astore_count(self->astore) > 0) omask |= 1 << 0;
        if (store_count(self->store) > 0) omask |= 1 << 1;
        if (omask > 0) return omask;
        waitfor(self->sched, pid, 10);
    }
    return omask;
}

void tcp_timer0(int pid, TSOCKET* self) {
    int stackspace[20000] ; stackspace[3]=45;
    while (self->sched->running > 0) {
        waitfor(self->sched, pid, self->RTO * (1+self->txwnd/2)); // RTO can be reset by tsocket_gen
        // printf("timeout %d\n", self->RTO);
        self->txwnd=self->txwnd/2;
        if (self->txwnd ==0 ) self->txwnd = 1;
        if (self->txwnd > 1) printf("Decreasing TX window to %d\n", self->txwnd);
    }
}

void  tsocket_gen(int pid, TSOCKET* self) {
    int stackspace[20000] ; stackspace[3]=45;
    spawn(self->sched, tseg_transmit, self, 0); // spawn the tseg_transmit subprocess
    spawn(self->sched, tseg_receive, self, 0); // spawn the tseg_receive subprocess
    int pid_timer0=self->sched->pid; // select the next pid
    spawn(self->sched, tcp_timer0, self, 0); // spawn the tseg_receive subprocess
    rawdata* rd;
    packet* p;
    tcpseg* s;
    int tcpsegnum=1;
    int key;
    int state=0;
    unsigned int mask = 0;
    while (self->sched->running > 0) {
         mask = tsocket_select(pid, self); // block if no I/O 
         if (mask & 1) { // app -> line
            astore_rpop(self->astore, &rd, &key);
            s=tcpseg_create(rd, tcpsegnum, 1, self->sched->now, self->txwnd); // 1 =data
            tstore_insert_unique(self->txstore, s, tcpsegnum);
            // printf("** %d %d %d\n", tcpsegnum, s->type, s->rd->id);
            tcpsegnum++;
         }
         if (mask & 2) { // line -> app
            store_rpop(self->store, &p, &key);
            if (p->s->type == 1) { // data
                self->rxwnd=p->s->wnd;
                tstore_insert_unique(self->rxstore, p->s, p->s->num);
            } else { // ack
                if (state == 0) {
                    self->RTTm=(self->sched->now - p->s->ctime)*2;
                    self->RTO = self->RTTm;
                    state = 1;
                } else if (state == 1) {
                    self->RTTm=self->sched->now - p->s->ctime;
                    self->RTTs=self->RTTm;
                    self->RTTd=self->RTTm/2;
                    self->RTO=self->RTTs+4*self->RTTd;
                    state = 2;
                } else {
                    self->RTTm=self->sched->now - p->s->ctime;
                    self->RTTs=0.85*self->RTTm + 0.125*self->RTTs;
                    self->RTTd=(0.75)*self->RTTd + 0.25*(self->RTTm - self->RTTs);
                    self->RTO=self->RTTs+4*self->RTTd;
                }
                self->RTO=200;
                printf("%f %f %f %ld\n", self->RTTm, self->RTTs, self->RTTd, self->RTO);
                //printf("ack received %d\n", p->s->num);
                self->txwnd+=(p->s->num - self->lastackrcvd);
                if (self->txwnd > 1) printf("Increasing TX window to %d\n", self->txwnd);
                self->lastackrcvd=p->s->num;
                sched_reset(self->sched, pid_timer0, self->sched->now+self->RTO*(1+self->txwnd/2));
                tstore_del_upto(self->txstore, self->lastackrcvd);               
                tcpseg_destroy(p->s);
            }
            packet_destroy(p);
         }         
    }
}

void tsocket_put0(TSOCKET* self, rawdata* rd){ // app receive
    astore_insert(self->astore,rd, 0);
}

void tsocket_put1(TSOCKET* self, packet* p){ // line receive
    store_insert(self->store, p, 0);
}
