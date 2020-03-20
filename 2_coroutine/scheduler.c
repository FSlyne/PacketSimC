#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "scheduler.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 */


void task_lpush(struct tbuffer **st, struct tbuffer **en,  void* typex, void (*func_ptr)())
{
    struct tbuffer *newnode;
    newnode = (struct tbuffer *)malloc(sizeof(struct tbuffer));
    newnode->func_ptr=func_ptr;
    newnode->typex=typex;
 
    if (*en == NULL)
        {
        newnode->next = NULL;
        newnode->prev = NULL;
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


void task_rpop(struct tbuffer **st, struct tbuffer **en,  void **typex, void (**func_ptr)())
{
    struct tbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        //printf("The Scheduler stack is empty!\n");
        return;
        }
           
    top = *en;
    *func_ptr=top->func_ptr;
    *typex=top->typex;
    if (*st == *en) {
        *st = NULL;
        *en = NULL;
        free(top);
        return;
    }

    *en = (*en)->prev;
    (*en)->next=NULL;
    free(top);
    return;
 
}


void sched_insert(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key, int id )
{
    struct sbuffer *newnode;
    struct sbuffer *idx, *tmp;
    
    //idx=*st;
    //while (idx) {
    //    if (id == idx->id)  // only add unique id's
    //        printf("Warning unique id %d\n", id);
    //        return;
    //    idx = idx->next;
    //}

    newnode = (struct sbuffer *)malloc(sizeof(struct sbuffer));
    newnode->key=key;
    newnode->id=id;
    // https://www.linuxquestions.org/questions/programming-9/objects-and-assignment-in-interpreter-861721/page6.html
    memcpy(newnode->flag, flag, sizeof(jmp_buf));
    
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

void sched_rpush(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key, int id )
{
    struct sbuffer *newnode;
    newnode = (struct sbuffer *)malloc(sizeof(struct sbuffer));
    newnode->key=key;
    newnode->id=id;
    memcpy(newnode->flag, flag, sizeof(jmp_buf));
 
    if (*en == NULL)
        {
        newnode->next = NULL;
        newnode->prev = NULL;
        *st=newnode;
        *en=newnode;
        return;
        }
 
    newnode->prev = *en;
    (*en)->next = newnode;
    *en = newnode;
    newnode->next = NULL;
    return ;
 
}

void sched_lpush(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key, int id )
{
    struct sbuffer *newnode;
    newnode = (struct sbuffer *)malloc(sizeof(struct sbuffer));
    newnode->key=key;
    newnode->id=id;
    memcpy(newnode->flag, flag, sizeof(jmp_buf));
 
    if (*en == NULL)
        {
        newnode->next = NULL;
        newnode->prev = NULL;
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


void sched_rpop(struct sbuffer **st, struct sbuffer **en,  jmp_buf *flag, int *key, int *id)
{
    struct sbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        //printf("The Scheduler stack is empty!\n");
        return;
        }
        
    //if (*st == NULL || *en == NULL)
    //    {
    //    printf("The stack is corrupted!\n");
    //    return;
    //    }
           
    top = *en;
    *key=top->key;
    *id=top->id;
    memcpy(*flag, top->flag, sizeof(jmp_buf));
    if (*st == *en) {
        *st = NULL;
        *en = NULL;
        free(top);
        return;
    }

    *en = (*en)->prev;
    (*en)->next=NULL;
    free(top);
    return;
 
}


void sched_clear(struct sbuffer **st, struct sbuffer **en)
{
    struct sbuffer *top;
    
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

void sched_count(struct sbuffer **st, struct sbuffer **en)
{
    struct sbuffer *idx;
    int count=0;
    
    idx=*st;
    while (idx) {
        idx = idx->next;
        count++;
    }
//    printf("Schedule Nodes: %d ", count);
}

void sched_init(SCHED* self, int finish){
    self->now=0;
    self->finish=finish;
    self->st=(struct sbuffer *) NULL;
    self->en=(struct sbuffer *) NULL;
    self->st_t=(struct tbuffer *) NULL;
    self->en_t=(struct tbuffer *) NULL;
    self->init=0;
    self->ider=0;
}

SCHED* sched_create(int finish){
    SCHED* obj=(SCHED*) malloc(sizeof(SCHED));
    sched_init(obj, finish);
    return obj;
}



// https://codeforwin.org/2017/12/pass-function-pointer-as-parameter-another-function-c.html
void spawn(SCHED* self, void (func_ptr()), void *typex, int then){
   task_lpush(&(self->st_t),&(self->en_t), typex, func_ptr);
   // key is given in seconds when called externally, so needs to be converted to microseconds
   // sched_insert(&(self->st),&(self->en), typex, func_ptr, then ,0);
}

//void sched_reg_oneoff(SCHED* self, void *typex, int (func_ptr()), int then){
//   // key is given in seconds when called externally, so needs to be converted to microseconds
//   sched_insert(&(self->st),&(self->en), typex, func_ptr, then ,1);
//}

void sched_yield(SCHED* self, jmp_buf flag, int then) {
   //printf("Inserting process %d\n", self->ider); 
   sched_insert(&(self->st),&(self->en), flag, then, self->ider++);
   if (self->st_t != NULL) { // starts execution of all processes
      void *typex;
      void (*func_ptr)();
      task_rpop(&(self->st_t), &(self->en_t), &typex, &func_ptr);
      (*func_ptr)(typex);
   }
 
   // handles yields from processes in executions
   int now; int id2;
   jmp_buf flag2;
   if  (self->st != NULL)  {
      sched_rpop(&(self->st), &(self->en), &flag2, &now, &id2);
      //printf(" Popping process %d\n", id2);
      self->now=now;
      longjmp(flag2,1);
   }
   
   if (self->now <= self->finish*1000000) {
      printf("sched_yield: premature exhaustion 1\n");
   }
   // If scheduler gets this far, it has run out of events 
}

void sched_run(SCHED* self) {
   void *typex;
   void (*func_ptr)();
   if (self->st_t != NULL) { // starts execution of first process
      task_rpop(&(self->st_t), &(self->en_t), &typex, &func_ptr);
      (*func_ptr)(typex);
   }
   if (self->now <= self->finish*1000000) {
      printf("sched_yield: premature exhaustion 2\n");
   }
   // If scheduler gets this far, there are either no processes, or the first process has stopped.
}

void waitfor(SCHED* self, int n) {
   jmp_buf flag;
   if (setjmp(flag) == 0) {
      sched_yield(self, flag, self->now+n);
   } 
}

void waituntil(SCHED* self, int n) {
   jmp_buf flag;
   if (setjmp(flag) == 0) {
      sched_yield(self, flag, n);
   }
}


//void sched_run(SCHED* self) {
//    int now;
//    while (self->now <= self->finish*pow(10,6)) {
//        jmp_buf flag;
//        sched_rpop(&(self->st), &(self->en), &flag, &now);
//        if  (flag == NULL) continue;
//        longjmp(flag,1);
//        self->now=now;     
//        if (self->now == 0) {
//            printf("Simulation Clock zero limit\n");
//        }
//    }
//    printf("Finished !!\n");
//    return;
//}



