#include <stdlib.h>
#include <stdio.h>
#include "message.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */

void channel_init(CHAN* self, SCHED* sched, int id) {
    self->sched=sched;
    self->m = 0;
    self->id = id;
}

CHAN* channel_create(SCHED* sched, int id) {
    CHAN* obj=(CHAN*) malloc(sizeof(CHAN));
    channel_init(obj, sched, id);
    return obj;
}

int channel_write(CHAN* self) {
    self->out(self->typex, self->m);
    return self->sched->now+1;
}

void channel_read(CHAN* self, int m){
    printf("Read:%ld %d %d\n", self->sched->now, self->id, m);
    self->m = m;
    sched_reg_oneoff(self->sched, self, channel_write, self->sched->now+1);
}