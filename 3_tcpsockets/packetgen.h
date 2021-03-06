#include "scheduler.h"
#include "packet.h"

typedef struct {
   int mean_pkt_size;
   int linerate;
   SCHED* sched;
} DIST;

typedef struct {
    int pktcnt;
    int status;
    SCHED* sched;
    int source;
    int dest;
    int flow_id;
    void (* out )(void* , packet*);
    void *typex;
    long (*arrivalfn)();
    void *arrivalfntype;
    //jmp_buf flag;
} PKT;


typedef struct {
    int status;
    long pkt_rcvd[10];
    long bytes_rcvd[10];
    long delay_cumul[10];
    SCHED* sched;
} SINK;

void dist_init(DIST* self, SCHED* sched, int linerate, int mean_pkt_size);
DIST* dist_create(SCHED* sched, int linerate, int mean_pkt_size);
long dist_exec(DIST* self);
void pkt_init(PKT* self, SCHED* sched, int source, int dest, int flow_id);
PKT* pkt_create(SCHED* sched, int source, int dest, int flow_id);
void  pkt_gen(int pid, PKT* self);
void pkt_stats(PKT* self);
void sink_init(SINK* self, SCHED* sched);
SINK* sink_create(SCHED* sched);
void sink_put(SINK* self, packet *p);
void sink_stats(SINK* self);