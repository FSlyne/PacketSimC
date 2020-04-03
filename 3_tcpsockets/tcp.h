#include "packet.h"
#include "app.h"


typedef struct {
    SCHED* sched;
    STORE* store;
    ASTORE* astore;
    void (* out0)(void* , rawdata*);
    void (* out1)(void* , packet*);
    void *typex0;
    void *typex1;
    unsigned mask;
} TSOCKET;

// http://blog.olkie.com/2013/11/05/online-c-function-prototype-header-generator-tool/
tcpseg* tcpseg_create(rawdata* rd);
void tcpseg_destroy(tcpseg* obj);
void tcpseg_init(tcpseg* self, rawdata* rd);
TSOCKET* tsocket_create(SCHED* sched);
void tsocket_destroy(TSOCKET* self);
void  tsocket_gen(TSOCKET* self);
void tsocket_init(TSOCKET* self, SCHED* sched, STORE* store, ASTORE* astore);
void tsocket_put0(TSOCKET* self, rawdata* rd);
void tsocket_put1(TSOCKET* self, packet* p);
unsigned int tsocket_select(TSOCKET* self);