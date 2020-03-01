#define BUFSIZE 2000

struct cbuffer
{
    void (*func_ptr)();
    void* typex;
    int key;
    struct cbuffer *next;
    struct cbuffer *prev;
};

typedef struct {
    int clock;
    int finish;
    struct cbuffer *st;
    struct cbuffer *en;
} SCHED;

// http://blog.olkie.com/2013/11/05/online-c-function-prototype-header-generator-tool/
void clear(struct cbuffer **st, struct cbuffer **en);
void rpop(struct cbuffer **st, struct cbuffer **en,  void **typex, int (**func_ptr)(), int *key);
void insert(struct cbuffer **st, struct cbuffer **en,  void *typex, void (*func_ptr)(), int key);
void sched_init(SCHED* self, int finish);
SCHED* sched_create(int finish);
void sched_reg(SCHED* self, void *typex, int (*func_ptr()), int key);
void sched_run(SCHED* self);
