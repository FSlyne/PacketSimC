#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "scheduler.h"
#include "packetgen.h"
#include "pbuffer.h"
#include "dba.h"
#include "aqm.h"


int main() {
   
    int scenario = 6;
    if (scenario == 1) { // Single queue
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         DIST* distfunc=dist_create(10,100); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         QUEUE* queue=queue_create(sched,10, 0, 128000, 100); // Mbps, Packet Count limit, Packet Byte limit, latency (usec)

         pkt1->out=queue_put; pkt1->typex=queue; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         queue->out=sink_put; queue->typex=sink;
         
         sched_reg(sched, pkt1, pkt_gen, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         sink_stats(sink);
    } else if (scenario == 2) { // Simple NULL Box         
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3,1);  // from, to, flow_id
         PKT* pkt2=pkt_create(sched,2,3,1);
         DIST* distfunc=dist_create(1,1000); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         BOX* box=box_create(sched);
         
         pkt1->out=box_put; pkt1->typex=box; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pkt2->out=box_put; pkt2->typex=box; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
         box->out=sink_put; box->typex=sink;
         
         sched_reg(sched, pkt1, pkt_gen, 0);
         sched_reg(sched, pkt2, pkt_gen, 0);
         
         sched_run(sched);   
         pkt_stats(pkt1);
         pkt_stats(pkt2);
         sink_stats(sink);         
    } else if (scenario == 3) { // WRED queue
         SCHED* sched=sched_create(1); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         PKT* pkt2=pkt_create(sched,2,3, 2); // from, to, flow_id
         DIST* distfunc=dist_create(10,1000); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         WRED* wred=wred_create(sched,17); // Mbps
   
         pkt1->out=wred_put; pkt1->typex=wred; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pkt2->out=wred_put; pkt2->typex=wred; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
         wred->queue->out=sink_put; wred->queue->typex=sink;
         
         sched_reg(sched, pkt1, pkt_gen, 0);
         sched_reg(sched, pkt2, pkt_gen, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         pkt_stats(pkt2);
         sink_stats(sink);
    } else if (scenario == 4) { // TRTCM
         int linerate=100;
         int cir=linerate*1000000/2;
         int pir=cir*2;
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         DIST* distfunc=dist_create(linerate,100); // Transmission (Mbps), Mean Packet size (Bytes)
         SINK* sink=sink_create(sched);
         TRTCM* trtcm=trtcm_create(sched, pir, 64000, cir, 128000); // PIR, PBS, CIR, CBS
         pkt1->out=trtcm_put; pkt1->typex=trtcm; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         trtcm->out=sink_put; trtcm->typex=sink;
         sched_reg(sched, pkt1, pkt_gen, 0);
         sched_run(sched);
         pkt_stats(pkt1);
         sink_stats(sink);
    } else if (scenario == 5) { // TCONT + DBA
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         DIST* distfunc=dist_create(40,100); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         TCONT* tcont=tcont_create(sched,100, 0, 0, 0); // Mbps, Packet Count limit, Packet Byte limit, latency (usec)
         DBA* dba=dba_create(sched,tcont);
         
         pkt1->out=tcont_put; pkt1->typex=tcont; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         tcont->out=sink_put; tcont->typex=sink;
                  
         sched_reg(sched, pkt1, pkt_gen, 0);
         sched_reg(sched, dba, dba_gen, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         sink_stats(sink);      
    } else if (scenario == 6) { // DualQ
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 0); // from, to, flow_id
         PKT* pkt2=pkt_create(sched,2,3, 1); // from, to, flow_id
         DIST* distfunc=dist_create(10,100); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);

         DUALQ* dualq=dualq_create(sched,10, 0, 0, 0);
      
         pkt1->out=dualq_put; pkt1->typex=dualq; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pkt2->out=dualq_put; pkt2->typex=dualq; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
         dualq->out=sink_put; dualq->typex=sink;
                  
         sched_reg(sched, pkt1, pkt_gen, 0);
         sched_reg(sched, pkt2, pkt_gen, 0);
         sched_reg(sched, dualq, dualq_update, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         pkt_stats(pkt2);
         sink_stats(sink);      
    }

}



