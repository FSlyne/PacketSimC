#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "scheduler.h"

void sched_insert(struct sbuffer **st, struct sbuffer **en,  void* typex, void (*func_ptr)(), int key, int oneoff)
{
    struct sbuffer *newnode;
    struct sbuffer *idx, *tmp;

    newnode = (struct sbuffer *)malloc(sizeof(struct sbuffer));
    newnode->func_ptr=func_ptr;
    newnode->typex=typex;
    newnode->key=key;
    newnode->oneoff=oneoff;
    
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


void sched_pop(struct sbuffer **st, struct sbuffer **en,  void **typex, int (**func_ptr)(), int *key, int *oneoff)
{
    struct sbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        printf("The stack is empty!\n");
        return ;
        }
           
    top = *en;
    *func_ptr=top->func_ptr;
    *typex=top->typex;
    *key=top->key;
    *oneoff=top->oneoff;
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

void sched_init(SCHED* self, int finish){
    self->now=0;
    self->finish=finish;
    self->st=(struct sbuffer *) NULL;
    self->en=(struct sbuffer *) NULL;
}

SCHED* sched_create(int finish){
    SCHED* obj=(SCHED*) malloc(sizeof(SCHED));
    sched_init(obj, finish);
    return obj;
}

// https://codeforwin.org/2017/12/pass-function-pointer-as-parameter-another-function-c.html
void sched_reg(SCHED* self, void *typex, int (*func_ptr()), int key){
   // key is given in seconds when called externally, so needs to be converted to microseconds
   sched_insert(&self->st,&self->en, typex, func_ptr, key*pow(10,6),0);
}

void sched_reg_oneoff(SCHED* self, void *typex, int (*func_ptr()), int key){
   // key is given in seconds when called externally, so needs to be converted to microseconds
   sched_insert(&self->st,&self->en, typex, func_ptr, key*pow(10,6),1);
}

void sched_run(SCHED* self) {
    int key1, key2, oneoff;
    void *typex;
    int (*func_ptr)();
    while (self->now <= self->finish*pow(10,6)) {
        sched_pop(&self->st, &self->en, &typex, &func_ptr, &key1, &oneoff);
        key2=(*func_ptr)(typex);
        if (oneoff == 0) {
           sched_insert(&self->st,&self->en, typex, func_ptr, key2+key1, 0);
           self->now=key1;
         }
    }
    printf("Finished !!\n");
    return;
}



