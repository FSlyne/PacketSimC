#include "scheduler.h"
#include "packet.h"
#include "store.h"

typedef struct {
    int status;
    int linerate;
    int latency;
    int bytesize;
    int countsize;
    int countlimit;
    int bytelimit;
    int myclock;
    SCHED* sched;
    STORE* store;
    void (* out)(void* , packet*);
    void *typex;
    struct pbuffer *st;
    struct pbuffer *en;   
} WRED;

typedef struct {
   SCHED* sched;
   STORE* store;
   void *typex;
   void (* out)(void* , packet*);
   int linerate;
   int countlimit;
   int bytelimit;
   int countsize;
   int bytesize;
   int target;
   int tupdate;
   int tupdate_last;
   float alpha;
   float beta;
   float p;
   int cqdelay;
   int pqdelay;
   int myclock;
} PIE;

typedef struct {
   SCHED* sched;
   STORE* store;
   void *typex;
   void (* out)(void* , packet*);
   int linerate;
   int packets_rec;
   int packets_drop;
   int countlimit;
   int bytelimit;
   int byte_size;  //Current size of the queue in bytes
   int last_update;
   int llpktcount;
   int clpktcount;
   int packets_HPrec;
   int packets_HPdrop;
   int packets_LPrec;
   int packets_LPdrop;
   int maxqlen;
   //
   int MTU;
   int MAX_LINK_RATE;
   int MIN_LINK_RATE;
   int target; // ms, PI AQM Classic queue delay targets
   int tupdate; // ms, PI Classic queue sampling interval
   int tupdate_last;
   int alpha; // Hz^2, PI integral gain
   int beta; // Hz^2, PI proportional gain
   int p_Cmax;
   // Constants derived from PI2 AQM parameters
   int alpha_U; // PI integral gain per update interval
   int beta_U;  // PI prop.nal gain per update interval
   // DualQ Coupled framework parameters
   int k; // Coupling factor
   // scheduler weight or equival.t parameter (scheduler-dependent)
   int limit; // ms Dual buffer size
   // L4S ramp AQM parameters
   int minTh; // us L4S min marking threshold in time units
   int range; // us Range of L4S ramp in time units
   int Th_len; // Min L4S marking threshold in bytes
   // Constants derived from L4S AQM parameters
   int p_Lmax; // Max L4S marking prob
   int floor; // MIN_LINK_RATE is in Mb/s
   int maxTh; // L4S min marking threshold in time units
   int p;
   int cqdelay;
   int lqdelay;
   int prevq; 
   int curq;
   int vtime;
   float p_C; float p_CL;
   float p_L;
} DUALQ;


void pie_init(PIE* self, SCHED* sched, STORE* store, int linerate, int countlimit, int bytelimit);
PIE* pie_create(SCHED* sched, int linerate, int countlimit, int bytelimit);
void pie_gen(PIE* self);
void pie_put(PIE* self, packet* p);
void wred_init(WRED* self, SCHED* sched, STORE* store, int linerate, int countlimit, int bytelimit, int latency);
WRED* wred_create(SCHED* sched, int linerate, int countlimit, int bytelimit, int latency);
void wred_destroy(WRED* self);
void  wred_gen(WRED* self);
void wred_put(WRED* self, packet* p);
void dualq_init(DUALQ* self, SCHED* sched, STORE* store, int linerate, int countlimit, int bytelimit);
DUALQ* dualq_create(SCHED* sched, int linerate, int countlimit, int bytelimit);
int dualq_laqm(DUALQ* self);
void dualq_gen(DUALQ* self);
void dualq_put(DUALQ* self, packet* p);
int dualq_update(DUALQ* self);