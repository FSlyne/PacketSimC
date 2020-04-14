#include "packet.h"
#include "app.h"

struct tcpbuffer
{
    tcpseg *s;
    int key;
    struct tcpbuffer *next;
    struct tcpbuffer *prev;
};

typedef struct {
    SCHED* sched;
    //jmp_buf flag;
    //int myclock;
    struct tcpbuffer *rdr;
    struct tcpbuffer *st;
    struct tcpbuffer *en;  
} TSTORE;

typedef struct {
    SCHED* sched;
    STORE* store;
    ASTORE* astore;
    TSTORE* txstore;
    TSTORE* rxstore;
    int rxwnd;
    int txwnd;
    int lastackrcvd;
    void (* out0)(void* , rawdata*);
    void (* out1)(void* , packet*);
    void *typex0;
    void *typex1;
    unsigned mask;
    float RTTm; // measured RTT; https://www.geeksforgeeks.org/tcp-timers/
    float RTTs; // smoothed RTT;
    float RTTd; // deviated RTT;
    long RTO; // retransmission Timeout
} TSOCKET;

// http://blog.olkie.com/2013/11/05/online-c-function-prototype-header-generator-tool/
void tstore_clear(TSTORE* self);
int tstore_count(TSTORE* self);
TSTORE* tstore_create(SCHED* sched);
void tstore_destroy(TSTORE* obj);
void tstore_init(TSTORE* self, SCHED* sched);
void tstore_insert_unique_raw(struct tcpbuffer **st, struct tcpbuffer **en,  tcpseg* s, int key);
void tstore_insert_unique(TSTORE* self,  tcpseg* s, int key);
void tstore_lpush(TSTORE* self, tcpseg* s, int key);
void tstore_read(TSTORE* self, tcpseg **s, int *key);
void tstore_rpop_block(int pid, TSTORE* self, tcpseg **s, int *key);
void tstore_rpop_raw(struct tcpbuffer **st, struct tcpbuffer **en, tcpseg **s, int *key);
void tstore_rpop(TSTORE* self, tcpseg **s, int *key);
void tstore_rget(TSTORE* self,  tcpseg **s, int *key);
void tstore_del_upto(TSTORE* self, int max);

void tseg_ack(TSOCKET* self, int acknum, long ctime);
tcpseg* tcpseg_copy(tcpseg* target);
tcpseg* tcpseg_create(rawdata* rd, int num, int type, long ctime, int wnd);
void tcpseg_destroy(tcpseg* obj);
void tcpseg_init(tcpseg* self, rawdata* rd, int num, int type, long ctime, int wnd);
TSOCKET* tsocket_create(SCHED* sched);
void tsocket_destroy(TSOCKET* self);
void  tsocket_gen(int pid, TSOCKET* self);
void tsocket_init(TSOCKET* self, SCHED* sched, STORE* store, ASTORE* astore, TSTORE* txstore,  TSTORE* rxstore);
void tsocket_put0(TSOCKET* self, rawdata* rd);
void tsocket_put1(TSOCKET* self, packet* p);
void tseg_transmit(int pid, TSOCKET* self);
void tseg_receive(int pid, TSOCKET* self);
unsigned int tsocket_select(int pid, TSOCKET* self);