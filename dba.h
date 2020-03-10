#include "packet.h"
#include "scheduler.h"

typedef struct {
    int status;
    int linerate;
    int latency;
    int bytesize;
    int countsize;
    int countlimit;
    int bytelimit;
    SCHED* sched;
    void (* out)(void* , int);
    void *typex;
    struct pbuffer *st_in;
    struct pbuffer *en_in;
    struct pbuffer *st_out;
    struct pbuffer *en_out;  
    packet *p;
    int needs;
    int tries;
} TCONT;

typedef struct {
    int status;
    SCHED* sched;
    TCONT* tcont;
} DBA;


void tcont_init(TCONT* self, SCHED* sched, int linerate, int countlimit, int bytelimit, int latency);
TCONT* tcont_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency);
void tcont_destroy(TCONT* obj);
void tcont_count(struct pbuffer **st, struct pbuffer **en);
int  tcont_gen(TCONT* self);
void tcont_control(TCONT* self, int slot);
void tcont_put(TCONT* self, packet* p);
void dba_init(DBA* self, SCHED* sched, TCONT* tcont);
DBA* dba_create(SCHED* sched, TCONT* tcont);
int  dba_gen(DBA* self);