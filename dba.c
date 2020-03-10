#include <stdlib.h>
#include <stdio.h>
#include "dba.h"

void tcont_init(TCONT* self, SCHED* sched, int linerate, int countlimit, int bytelimit, int latency){
    self->st_in=(struct pbuffer *) NULL;
    self->en_in=(struct pbuffer *) NULL;
    self->st_out=(struct pbuffer *) NULL;
    self->en_out=(struct pbuffer *) NULL;
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
    queue_rpop(&(self->st_out), &(self->en_out), &p, &key);
    self->countsize--;
    self->bytesize-=p->size;
    // postprocess
    // Need to replicate self.out.put(p) functionality
    self->out(self->typex, p);
    return self->sched->now;
}

void tcont_control(TCONT* self, int slot) {
    int slotbytes=slot*4; // 32 bit words
    int key;
    while (1>0) {
      if (self->p == (packet*) NULL) {
         queue_rpop(&(self->st_in), &(self->en_in), &(self->p), &key);
         if (self->p == (packet*) NULL) {
            self->needs=0;
            return;
         }
         self->needs=self->p->size;
      }
      if (self->needs <= slotbytes) {
         queue_insert(&(self->st_out),&(self->en_out),self->p,0);
         sched_reg_oneoff(self->sched, self, tcont_gen, self->sched->now+self->latency*self->tries); // send up stream
         // if (self->tries>1) printf("Fault\n");
         self->tries=1;
         self->p = (packet*) NULL;
         slotbytes-=self->needs;
      } else {
         self->needs-=slotbytes;
         self->tries++;
         return;
      }
    }
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
    queue_insert(&self->st_in,&self->en_in,p, 0);
}


void dba_init(DBA* self, SCHED* sched, TCONT* tcont) {
    self->status=0;
    self->sched=sched;
    self->tcont=tcont;
}

DBA* dba_create(SCHED* sched, TCONT* tcont) {
    DBA* obj=(DBA*) malloc(sizeof(DBA));
    dba_init(obj, sched, tcont);
    return obj;
}

int  dba_gen(DBA* self) {
    tcont_control(self->tcont,100);
    return (self->sched->now+125);
}