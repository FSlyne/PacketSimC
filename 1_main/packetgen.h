#include "scheduler.h"
#include "packet.h"

typedef struct {
   int mean_pkt_size;
   int linerate;
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
    int (*arrivalfn)();
    void *arrivalfntype;
} PKT;


typedef struct {
    int status;
    long pkt_rcvd[10];
    long bytes_rcvd[10];
    long delay_cumul[10];
    SCHED* sched;
} SINK;

void dist_init(DIST* self, int linerate, int mean_pkt_size);
DIST* dist_create(int linerate, int mean_pkt_size);
int dist_exec(DIST* self);
void pkt_init(PKT* self, SCHED* sched, int source, int dest, int flow_id);
PKT* pkt_create(SCHED* sched, int source, int dest, int flow_id);
int  pkt_gen(PKT* self);
void pkt_stats(PKT* self);
void sink_init(SINK* self, SCHED* sched);
SINK* sink_create(SCHED* sched);
void sink_put(SINK* self, packet *p);
void sink_stats(SINK* self);