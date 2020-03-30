#include "packet.h"

typedef struct {
   packet* p;
} tcpseg;

struct tcpbuffer
{
    tcpseg *s;
    int key;
    struct tcpbuffer *next;
    struct tcpbuffer *prev;
};

typedef struct {
    SCHED* sched;
    jmp_buf flag;
    int myclock;
    struct tcpbuffer *st;
    struct tcpbuffer *en;  
} TSTORE;

typedef struct {
    SCHED* sched;
    STORE* store;
    TSTORE* tstore;
    void (* out0)(void* , packet*);
     void (* out1)(void* , tcpseg*);
    void *typex0;
    void *typex1;
    unsigned mask;
    struct tcpbuffer *st;
    struct tcpbuffer *en;  
} TSOCKET;

// http://blog.olkie.com/2013/11/05/online-c-function-prototype-header-generator-tool/
tcpseg* tcpseg_create(packet* p);
void tcpseg_destroy(tcpseg* obj);
void tcpseg_init(tcpseg* self, packet* p);
TSOCKET* tsocket_create(SCHED* sched);
void tsocket_destroy(TSOCKET* self);
void  tsocket_gen(TSOCKET* self);
void tsocket_init(TSOCKET* self, SCHED* sched, STORE* store, TSTORE* tstore);
void tsocket_put0(TSOCKET* self, packet* p);
void tsocket_put1(TSOCKET* self, tcpseg* s);
unsigned int tsocket_select(TSOCKET* self);
void tstore_clear(TSTORE* self);
int tstore_count(TSTORE* self);
TSTORE* tstore_create(SCHED* sched);
void tstore_destroy(TSTORE* obj);
void tstore_init(TSTORE* self, SCHED* sched);
void tstore_insert_raw(struct tcpbuffer **st, struct tcpbuffer **en, tcpseg* s, int key);
void tstore_insert(TSTORE* self, tcpseg* s, int key);
void tstore_lpush(TSTORE* self, tcpseg* s, int key);
void tstore_read(TSTORE* self, tcpseg **s, int *key);
void tstore_rpop_block(TSTORE* self, tcpseg **s, int *key);
void tstore_rpop_raw(struct tcpbuffer **st, struct tcpbuffer **en, tcpseg **s, int *key);
void tstore_rpop(TSTORE* self, tcpseg **s, int *key);
