
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <setjmp.h>
struct sbuffer
{
    int (*func_ptr)();
    void* typex;
    int key;
    int oneoff;
    jmp_buf flag;
    struct sbuffer *next;
    struct sbuffer *prev;
};

typedef struct {
    int init;
    long now;
    int finish;
    struct sbuffer *st;
    struct sbuffer *en;
    struct tbuffer *st_t;
    struct tbuffer *en_t;    
    jmp_buf token;
} SCHED;

struct tbuffer
{
    int (*func_ptr)();
    void* typex;
    struct tbuffer *next;
    struct tbuffer *prev;
};

// http://blog.olkie.com/2013/11/05/online-c-function-prototype-header-generator-tool/
void sched_clear(struct sbuffer **st, struct sbuffer **en);
void sched_count(struct sbuffer **st, struct sbuffer **en);
void sched_rpop(struct sbuffer **st, struct sbuffer **en,  jmp_buf *flag, int *key);
void sched_insert(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key );
void sched_rpush(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key );
void sched_lpush(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key );
void sched_init(SCHED* self, int finish);
SCHED* sched_create(int finish);
void sched_yield(SCHED* self, jmp_buf flag, int then);
void sched_reg(SCHED* self, void *typex, void (func_ptr()), int then);
//void sched_reg_oneoff(SCHED* self, void *typex, int (func_ptr()), int key);
void sched_run(SCHED* self);
void sched_sweep(SCHED* self);
void task_lpush(struct tbuffer **st, struct tbuffer **en,  void* typex, int (*func_ptr)());
void task_rpop(struct tbuffer **st, struct tbuffer **en,  void **typex, int (**func_ptr)());

#endif // SCHEDULER_H