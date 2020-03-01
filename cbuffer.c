#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cbuffer.h"

void insert(struct cbuffer **st, struct cbuffer **en,  void* typex, void (*func_ptr)(), int key)
{
    struct cbuffer *newnode;
    struct cbuffer *idx, *tmp;

    newnode = (struct cbuffer *)malloc(sizeof(struct cbuffer));
    newnode->func_ptr=func_ptr;
    newnode->typex=typex;
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


void rpop(struct cbuffer **st, struct cbuffer **en,  void **typex, int (**func_ptr)(), int *key)
{
    struct cbuffer *top;

    if (*st == NULL && *en == NULL)
        {
        printf("The stack is empty!\n");
        return ;
        }
           
    top = *en;
    *func_ptr=top->func_ptr;
    *typex=top->typex;
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


void clear(struct cbuffer **st, struct cbuffer **en)
{
    struct cbuffer *top;
    
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
    self->clock=0;
    self->finish=finish;
}

SCHED* sched_create(int finish){
    SCHED* obj=(SCHED*) malloc(sizeof(SCHED));
    sched_init(obj, finish);
    return obj;
}

// https://codeforwin.org/2017/12/pass-function-pointer-as-parameter-another-function-c.html
void sched_reg(SCHED* self, void *typex, int (*func_ptr()), int key){
   insert(&self->st,&self->en, typex, func_ptr, key);
}

void sched_run(SCHED* self) {
    int key1, key2;
    void *typex;
    int (*func_ptr)();
    while (self->clock <= self->finish) {
        rpop(&self->st, &self->en, &typex, &func_ptr, &key1);
        key2=(*func_ptr)(typex);
        insert(&self->st,&self->en, typex, func_ptr, key2+key1);
        self->clock=key1;
    }
    printf("Finished !!\n");
    return;
}



