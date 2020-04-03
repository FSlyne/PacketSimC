#include <stdlib.h>
#include <stdio.h>
#include "socket.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */

void socket_init(SOCKET* self, SCHED* sched, STORE* store[]){
    self->sched=sched;
    self->store[0]=store[0];
    self->store[1]=store[1];
    self->mask=0;
}

SOCKET* socket_create(SCHED* sched){
    STORE* store[]={store_create(sched), store_create(sched)};
    SOCKET* obj=(SOCKET*) malloc(sizeof(SOCKET));
    socket_init(obj, sched, store);
    return obj;
}

void socket_destroy(SOCKET* self){
    store_destroy(self->store[0]);
    store_destroy(self->store[1]);
    if (self) {
        free(self);
    }
}

unsigned int socket_select(SOCKET* self) {
    unsigned int omask = 0;
    while (0<1) {
        omask = 0;
        if (store_count(self->store[0]) > 0) omask |= 1 << 0;
        if (store_count(self->store[1]) > 0) omask |= 1 << 1;
        if (omask > 0) return omask;
        waitfor(self->sched, 10);
    }
    return omask;
}

void  socket_gen(SOCKET* self) {
    int stackspace[20000] ; stackspace[3]=45;
    packet* p;
    int key;
    unsigned int mask = 0;
    while (self->sched->running > 0) {
         mask = socket_select(self); // block if no I/O 
         if (mask & 1) {
            store_rpop(self->store[0], &p, &key);
            self->out[1](self->typex[1], p);
         }
         if (mask & 2) {
            store_rpop(self->store[1], &p, &key);
            self->out[0](self->typex[0], p);
         }         
    }
}

void socket_put0(SOCKET* self, packet* p){
    store_insert(self->store[0],p, 0);
}

void socket_put1(SOCKET* self, packet* p){
    store_insert(self->store[1],p, 0);
}