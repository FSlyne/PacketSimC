#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cbuffer.h"

typedef struct {
   int num;
} packet;

void packet_init(packet* self, int n) {
   self->num=n;
}

packet* packet_create(int n) {
    packet* obj=(packet*) malloc(sizeof(packet));
    packet_init(obj, n);
    return obj;
}

void packet_destroy(packet* obj){
    if (obj) {
        free(obj);
    }
}

typedef struct {
    int pktcnt;
    int status;
    void (* out)(void* , int);
    void *typex;
} PKT;


void pkt_init(PKT* self) {
    self->pktcnt=0;
    self->status=0;
}

PKT* pkt_create() {
    PKT* obj=(PKT*) malloc(sizeof(PKT));
    pkt_init(obj);
    return obj;
}

int  pkt_gen(PKT* self) {
    if (self->status == 0) { // first time pkt_get is run
        self->status = 1;
        self->pktcnt=0;
    }
    // preprocess 
    packet* p=packet_create((self->pktcnt)++);
    // postprocess
    // Need to replicate self.out.put(p) functionality
    self->out(self->typex, p);
    return 1;
}

typedef struct {
    int status;
} SINK;

void sink_init(SINK* self) {
    self->status=0;
}

SINK* sink_create() {
    SINK* obj=(SINK*) malloc(sizeof(SINK));
    sink_init(obj);
    return obj;
}

void sink_put(SINK* self, packet *p) {
    printf("Sinking %d\n", p->num);
    packet_destroy(p);
    return;
}

typedef struct {
    int status;
    void (* out)(void* , int);
    void *typex;
} BOX;

void box_init(BOX* self) {
    self->status=0;
}

BOX* box_create() {
    BOX* obj=(BOX*) malloc(sizeof(BOX));
    box_init(obj);
    return obj;
}

void box_put(BOX* self, packet *p) {
    printf("Transiting %d\n", p->num);
    self->out(self->typex, p);
    return;
}

int main() {
    SCHED* sched=sched_create(1000); 
    PKT* pkt1=pkt_create();
    PKT* pkt2=pkt_create();
    BOX* box=box_create();
    SINK* sink=sink_create();
    
//    sched_reg(sched, pkt, pkt_gen);
    pkt1->out=box_put; pkt1->typex=box;
    pkt2->out=box_put; pkt2->typex=box;
    box->out=sink_put; box->typex=sink;
    
    sched_reg(sched, pkt1, pkt_gen, 3);
    sched_reg(sched, pkt2, pkt_gen, 5);
    
    sched_run(sched);

}
