#define BUFSIZE 2000

struct sbuffer
{
    void (*func_ptr)();
    void* typex;
    int key;
    int oneoff;
    struct sbuffer *next;
    struct sbuffer *prev;
};

typedef struct {
    int now;
    int finish;
    struct sbuffer *st;
    struct sbuffer *en;
} SCHED;

// http://blog.olkie.com/2013/11/05/online-c-function-prototype-header-generator-tool/
void sched_clear(struct sbuffer **st, struct sbuffer **en);
void sched_count(struct sbuffer **st, struct sbuffer **en);
void sched_rpop(struct sbuffer **st, struct sbuffer **en,  void **typex, int (**func_ptr)(), int *key, int *oneoff);
void sched_insert(struct sbuffer **st, struct sbuffer **en,  void *typex, void (*func_ptr)(), int key, int oneoff);
void sched_rpush(struct sbuffer **st, struct sbuffer **en,  void *typex, void (*func_ptr)(), int key, int oneoff);
void sched_lpush(struct sbuffer **st, struct sbuffer **en,  void *typex, void (*func_ptr)(), int key, int oneoff);
void sched_init(SCHED* self, int finish);
SCHED* sched_create(int finish);
void sched_reg(SCHED* self, void *typex, int (*func_ptr()), int key);
void sched_reg_oneoff(SCHED* self, void *typex, int (*func_ptr()), int key);
void sched_run(SCHED* self);
