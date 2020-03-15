#include "packet.h"
#include "scheduler.h"
#include "queue.h"

typedef struct {
    int status;
    int linerate;
    int latency;
    int bytesize;
    int countsize;
    int countlimit;
    int bytelimit;
    SCHED* sched;
    void (* out)(void* , packet*);
    void *typex;
    struct pbuffer *st;
    struct pbuffer *en;
    packet *p;
    int needs;
    int tries;
} TCONT;

struct fbuffer { // Frame Buffer
    int grant_start; // slot
    int grant_size; // slot = 1 word, 32 bits, 4 bytes
    struct pbuffer *tlist_st; // pointer to tcont linked list
    struct pbuffer *tlist_en;
    struct fbuffer *next;
    struct fbuffer *prev;
};


typedef struct {
    int status;
    SCHED* sched;
    TCONT* tcont;
    struct fbuffer * st_frame;
    struct fbuffer * en_frame;
} DBA;

void frame_insert(struct fbuffer **st, struct fbuffer **en, struct pbuffer *tlist_st, struct pbuffer *tlist_en, int grant_start, int grant_size);
void frame_lpush(struct fbuffer **st, struct fbuffer **en, struct pbuffer *tlist_st, struct pbuffer *tlist_en, int grant_start, int grant_size);
void frame_rpop(struct fbuffer **st, struct fbuffer **en, struct pbuffer **tlist_st, struct pbuffer **tlist_en, int *grant_start, int *grant_size);
void tcont_init(TCONT* self, SCHED* sched, int linerate, int countlimit, int bytelimit, int latency);
TCONT* tcont_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency);
void tcont_destroy(TCONT* obj);
void tcont_count(struct pbuffer **st, struct pbuffer **en);
int  tcont_gen(TCONT* self);
void tcont_control(TCONT* self, int slot, struct pbuffer **st, struct pbuffer **en);
void tcont_put(TCONT* self, packet* p);
void dba_init(DBA* self, SCHED* sched, TCONT* tcont);
DBA* dba_create(SCHED* sched, TCONT* tcont);
int  dba_gen(DBA* self);