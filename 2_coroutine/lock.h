
struct wbuffer
{
    void (*func_ptr)();
    void *typex;
    int key;
    struct wbuffer *next;
    struct wbuffer *prev;
};

typedef struct {
    struct wbuffer *st;
    struct wbuffer *en;
} LOCK;

// http://blog.olkie.com/2013/11/05/online-c-function-prototype-header-generator-tool/
void printstack(struct wbuffer *st);
void wclear(struct wbuffer **st, struct wbuffer **en);
void wcount(struct wbuffer **st, struct wbuffer **en);
int wexist(struct wbuffer **st, struct wbuffer **en, int key);
void wsearch(struct wbuffer **st, struct wbuffer **en,  void** typex, void (**func_ptr)(), int key);
void wrpop(struct wbuffer **st, struct wbuffer **en,  void** typex, void (**func_ptr)(), int *key);
void winsert(struct wbuffer **st, struct wbuffer **en,  void* typex, void (*func_ptr)(), int key);
void wrpush(struct wbuffer **st, struct wbuffer **en,  void* typex, void (*func_ptr)(), int key);
void wlpush(struct wbuffer **st, struct wbuffer **en,  void* typex, void (*func_ptr)(), int key);
LOCK* lock_create();
void lock_init(LOCK* self);
void set_lock(LOCK* lock, void *typex, void (func_ptr()), int key);
void unset_lock(LOCK* lock, int key);


