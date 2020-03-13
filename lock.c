
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lock.h"

int wexist(struct wbuffer **st, struct wbuffer **en, int key) {
    struct wbuffer *idx;
    idx=*st;
    while (idx) {
        if (key == idx->key)  return 1; // found
        idx = idx->next;
    }
    return 0; // did not find   
}

void wsearch(struct wbuffer **st, struct wbuffer **en,  void** typex, void (**func_ptr)(), int key) {
    struct wbuffer *idx;
    idx=*st;
    while (idx) {
        if (key == idx->key)  {// --> 7,6,5,4,3,2 -->
            *func_ptr=idx->func_ptr;
            *typex=idx->typex;
            if (idx->prev == NULL) { // first
                *st=idx->next;
            } else {
                idx->prev->next = idx->next;
            }
            if (idx->next == NULL) { // last
                *en=idx->prev;
            } else {
                idx->next->prev = idx->prev;
            }
            free(idx);
        }
        idx = idx->next;
    }
}


void winsert(struct wbuffer **st, struct wbuffer **en,  void* typex, void (*func_ptr)(), int key)
{
    struct wbuffer *newnode;
    struct wbuffer *idx, *tmp;

    newnode = (struct wbuffer *)malloc(sizeof(struct wbuffer));
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

void wrpush(struct wbuffer **st, struct wbuffer **en,  void* typex, void (*func_ptr)(), int key)
{
    struct wbuffer *newnode;
    newnode = (struct wbuffer *)malloc(sizeof(struct wbuffer));
    newnode->func_ptr=func_ptr;
    newnode->typex=typex;
    newnode->key=key;
 
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

void wlpush(struct wbuffer **st, struct wbuffer **en,  void* typex, void (*func_ptr)(), int key)
{
    struct wbuffer *newnode;
    newnode = (struct wbuffer *)malloc(sizeof(struct wbuffer));
    newnode->func_ptr=func_ptr;
    newnode->typex=typex;
    newnode->key=key;
 
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


void wrpop(struct wbuffer **st, struct wbuffer **en,  void** typex, void (**func_ptr)(), int *key)
{
    struct wbuffer *top;

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
    return;
 
}


void wclear(struct wbuffer **st, struct wbuffer **en)
{
    struct wbuffer *top;
    
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

void wcount(struct wbuffer **st, struct wbuffer **en)
{
    struct wbuffer *idx;
    int count=0;
    
    idx=*st;
    while (idx) {
        idx = idx->next;
        count++;
    }
    printf("Buffer Nodes: %d ", count);
}

void printstack(struct wbuffer *st)
{
    if (st == NULL)
        {
        printf("The stack is empty!\n");
        return;
        }
 
    do
        {
        printf("%d ", st->key);
        st = st->next;
        }
    while (st);
    printf("\n");
}

void lock_init(LOCK* self){
    self->st=(struct wbuffer *) NULL;
    self->en=(struct wbuffer *) NULL;
}

LOCK* lock_create(int finish){
    LOCK* obj=(LOCK*) malloc(sizeof(LOCK));
    lock_init(obj);
    return obj;
}

void set_lock(LOCK* lock, void *typex, void (func_ptr()), int key) {
    wrpush(&(lock->st),&(lock->en), typex, func_ptr, key);
}

void unset_lock(LOCK* lock, int key) {
    void (*func_ptr)();
    void *typex;
    wsearch(&(lock->st),&(lock->en), &typex, &func_ptr, key);
    while (func_ptr != (void *) NULL) {
        (*func_ptr)(typex);
        wsearch(&(lock->st),&(lock->en), &typex, &func_ptr, key);
    }
}
