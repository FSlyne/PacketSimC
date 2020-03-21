
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <setjmp.h>
struct sbuffer
{
    void (*func_ptr)();
    void* typex;
    int key;
    int oneoff;
    jmp_buf flag;
    int id;
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
    int ider;
    int debug;
} SCHED;

struct tbuffer
{
    void (*func_ptr)();
    void* typex;
    struct tbuffer *next;
    struct tbuffer *prev;
};

// http://blog.olkie.com/2013/11/05/online-c-function-prototype-header-generator-tool/
void sched_clear(struct sbuffer **st, struct sbuffer **en);
void sched_count(struct sbuffer **st, struct sbuffer **en);
void sched_rpop(struct sbuffer **st, struct sbuffer **en,  jmp_buf *flag, int *key, int *id);
void sched_insert(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key, int id );
void sched_rpush(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key, int id );
void sched_lpush(struct sbuffer **st, struct sbuffer **en,  jmp_buf flag, int key, int id );
void sched_init(SCHED* self, int finish);
SCHED* sched_create(int finish);
void sched_yield(SCHED* self, jmp_buf flag, int then);
void spawn(SCHED* self, void (func_ptr()), void *typex, int then);
void sched_run(SCHED* self);
void sched_sweep(SCHED* self);
void task_lpush(struct tbuffer **st, struct tbuffer **en,  void* typex, void (*func_ptr)());
void task_rpop(struct tbuffer **st, struct tbuffer **en,  void **typex, void (**func_ptr)());
void waitfor(SCHED* self, int n);
void waituntil(SCHED* self, int n);

#endif // SCHEDULER_H