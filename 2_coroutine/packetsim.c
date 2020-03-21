#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "scheduler.h"
#include "packetgen.h"
#include "pbuffer.h"
#include "queue.h"
#include "dba.h"
#include "aqm.h"
#include "socket.h"
//#include "message.h"

/* 
 * A lightweight Discrete Event Simulator developed in C
 * Copyright 2020 Frank Slyne, Marco Ruffini. Trinity College Dublin.
 * Released under MIT licence.
 *
 * To Do: Nanosecond clock, interrupts, signals, FSM, debug flags
 */


int main() {
   
    int scenario = 5;
    if (scenario == 1) { // (PKT+DIST, PKT+DIST) -> NULL BOX -> SINK
         printf("(PKT+DIST, PKT+DIST) -> NULL BOX -> SINK\n");
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3,1);  // from, to, flow_id
         PKT* pkt2=pkt_create(sched,2,3,2);
         PKT* pkt3=pkt_create(sched,3,3,3);
         DIST* distfunc=dist_create(100,1000); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         BOX* box=box_create(sched);
         
         pkt1->out=(void *)box_put; pkt1->typex=box; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pkt2->out=(void *)box_put; pkt2->typex=box; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
         pkt3->out=(void *)box_put; pkt3->typex=box; pkt3->arrivalfn=dist_exec; pkt3->arrivalfntype=distfunc;
         box->out=(void *)sink_put; box->typex=sink;
         
         spawn(sched, pkt_gen, pkt1, 0);
         spawn(sched, pkt_gen, pkt2, 0);
         spawn(sched, pkt_gen, pkt3, 0);
         
         sched_run(sched);   
         pkt_stats(pkt1);
         pkt_stats(pkt2);
         pkt_stats(pkt3);
         sink_stats(sink);         
    } else if (scenario == 2) { // Single queue. PKT+DIST -> QUEUE-> SINK
         printf("Single queue. PKT+DIST -> QUEUE-> SINK\n");
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,10, 1); // from, to, flow_id
         DIST* distfunc=dist_create(30,100); // Transmission (Mbps), Mean Packet size (Bytes), Poisson, exponentially distributed
         SINK* sink=sink_create(sched);
         QUEUE* queue=queue_create(sched,10, 121, 0, 1); // Mbps, Packet Count limit, Packet Byte limit, latency (usec)

         pkt1->out=(void *)queue_put; pkt1->typex=queue; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         queue->out=(void *)sink_put; queue->typex=sink;
   
         spawn(sched, pkt_gen, pkt1, 0);
         spawn(sched, queue_gen, queue, 0);

         sched_run(sched);
         
         pkt_stats(pkt1);
         sink_stats(sink);  
    } else if (scenario == 3) { // Single queue. PKT+DIST -> QUEUE-> SINK
         printf("Single queue. PKT+DIST -> QUEUE-> SINK\n");
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,10, 1); // from, to, flow_id
         PKT* pkt2=pkt_create(sched,2,10, 2); // from, to, flow_id
         PKT* pkt3=pkt_create(sched,3,10, 3); // from, to, flow_id
         PKT* pkt4=pkt_create(sched,4,10, 4); // from, to, flow_id
         PKT* pkt5=pkt_create(sched,5,10, 5); // from, to, flow_id
         DIST* distfunc=dist_create(10,100); // Transmission (Mbps), Mean Packet size (Bytes), Poisson, exponentially distributed
         SINK* sink=sink_create(sched);
         QUEUE* queue=queue_create(sched,20, 10, 0, 0); // Mbps, Packet Count limit, Packet Byte limit, latency (usec)

         pkt1->out=(void *)queue_put; pkt1->typex=queue; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pkt2->out=(void *)queue_put; pkt2->typex=queue; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
         pkt3->out=(void *)queue_put; pkt3->typex=queue; pkt3->arrivalfn=dist_exec; pkt3->arrivalfntype=distfunc;
         pkt4->out=(void *)queue_put; pkt4->typex=queue; pkt4->arrivalfn=dist_exec; pkt4->arrivalfntype=distfunc;
         pkt5->out=(void *)queue_put; pkt5->typex=queue; pkt5->arrivalfn=dist_exec; pkt5->arrivalfntype=distfunc;
         queue->out=(void *)sink_put; queue->typex=sink;
   
         spawn(sched, pkt_gen, pkt1, 0);
         spawn(sched, pkt_gen, pkt2, 0);
         spawn(sched, pkt_gen, pkt3, 0);
         spawn(sched, pkt_gen, pkt4, 0);
         spawn(sched, pkt_gen, pkt5, 0);
         spawn(sched, queue_gen, queue, 0);
    
         sched_run(sched);
         
         pkt_stats(pkt1);
         pkt_stats(pkt2);
         pkt_stats(pkt3);
         pkt_stats(pkt4);
         pkt_stats(pkt5);
         sink_stats(sink);
    }   else if (scenario == 4) { // (PKT+DIST, PKT+DIST)  -> WRED QUEUE -> SINK
         printf("(PKT+DIST, PKT+DIST)  -> WRED QUEUE -> SINK\n");
         SCHED* sched=sched_create(1); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         PKT* pkt2=pkt_create(sched,2,3, 2); // from, to, flow_id
         DIST* distfunc=dist_create(10,1000); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         WRED* wred=wred_create(sched,20, 0, 0, 0); // Mbps
    
         pkt1->out=(void *)wred_put; pkt1->typex=wred; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pkt2->out=(void *)wred_put; pkt2->typex=wred; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
         wred->out=(void *)sink_put; wred->typex=sink;
         
         spawn(sched, pkt_gen, pkt1, 0);
         spawn(sched, pkt_gen, pkt2, 0);
         spawn(sched, wred_gen, wred, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         pkt_stats(pkt2);
         sink_stats(sink);
    }  else if (scenario == 5) { //  PKT+DIST -> TRTCM  -> SINK
         printf("PKT+DIST -> TRTCM  -> SINK\n");
         int linerate=200;
         int cir=linerate*1000000/2;
         int pir=cir*2;
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         DIST* distfunc=dist_create(linerate,1000); // Transmission (Mbps), Mean Packet size (Bytes)
         SINK* sink=sink_create(sched);
         TRTCM* trtcm=trtcm_create(sched, pir, 128000, cir, 64000); // PIR, PBS, CIR, CBS
         pkt1->out=(void *)trtcm_put; pkt1->typex=trtcm; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         trtcm->out=(void *)sink_put; trtcm->typex=sink;
         spawn(sched, pkt_gen, pkt1, 0);
         sched_run(sched);
         pkt_stats(pkt1);
         sink_stats(sink);
    } else if (scenario == 6) { // PKT+DIST -> TCONT + DBA  -> SINK
         printf("PKT+DIST -> TCONT + DBA  -> SINK\n");
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 1); // from, to, flow_id
         DIST* distfunc=dist_create(40,100); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
         TCONT* tcont=tcont_create(sched,100, 0, 0, 0); // Mbps, Packet Count limit, Packet Byte limit, latency (usec)
         DBA* dba=dba_create(sched,tcont);
         
         pkt1->out=(void *)tcont_put; pkt1->typex=tcont; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         tcont->out=(void *)sink_put; tcont->typex=sink;
                  
         spawn(sched, pkt_gen, pkt1, 0);
         spawn(sched, dba_gen, dba, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         sink_stats(sink);      
    } else if (scenario == 7) { // (PKT+DIST)  -> PIE  -> SINK
         printf("(PKT+DIST)  -> PIE  -> SINK\n");
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 0); // from, to, flow_id
         DIST* distfunc=dist_create(15,100); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
    
         PIE* pie=pie_create(sched,10, 0, 0);
      
         pkt1->out=(void *)pie_put; pkt1->typex=pie; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pie->out=(void *)sink_put; pie->typex=sink;
                  
         spawn(sched, pkt_gen, pkt1, 0);
         spawn(sched, pie_gen, pie, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         sink_stats(sink);
     }  else if (scenario == 8) { // (PKT+DIST, PKT+DIST)  -> DUALQ  -> SINK
         printf("(PKT+DIST, PKT+DIST)  -> DUALQ  -> SINK\n");
         SCHED* sched=sched_create(10); // seconds
         PKT* pkt1=pkt_create(sched,1,3, 0); // from, to, flow_id
         PKT* pkt2=pkt_create(sched,2,3, 1); // from, to, flow_id
         DIST* distfunc=dist_create(10,100); // Transmission (Mbps), Mean Packet size (Bytes)
     //    QUEUE* queue1=queue_create(sched);
         SINK* sink=sink_create(sched);
    
         DUALQ* dualq=dualq_create(sched,20, 0, 0);
      
         pkt1->out=(void *)dualq_put; pkt1->typex=dualq; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
         pkt2->out=(void *)dualq_put; pkt2->typex=dualq; pkt2->arrivalfn=dist_exec; pkt2->arrivalfntype=distfunc;
         dualq->out=(void *)sink_put; dualq->typex=sink;
                  
         spawn(sched, pkt_gen, pkt1, 0);
         spawn(sched, pkt_gen, pkt2, 0);
         spawn(sched, dualq_gen, dualq, 0);
         
         sched_run(sched);
         
         pkt_stats(pkt1);
         pkt_stats(pkt2);
         sink_stats(sink);   
    } else if (scenario == 9) { // (PKT+DIST)  -> socket looped -> SINK
        printf("(PKT+DIST)  -> socket looped -> SINK\n");
        SCHED* sched=sched_create(10); // seconds
        SOCKET* socket=socket_create(sched);
        PKT* pkt1=pkt_create(sched,1,3, 0); // from, to, flow_id
        DIST* distfunc=dist_create(10,100); // Transmission (Mbps), Mean Packet size (Bytes)
        SINK* sink=sink_create(sched);
        
        pkt1->out=(void *)socket_put0; pkt1->typex=socket; pkt1->arrivalfn=dist_exec; pkt1->arrivalfntype=distfunc;
        socket->out[1]=(void *)socket_put1; socket->typex[1]=socket;
        socket->out[0]=(void *)sink_put; socket->typex[0]=sink;
        
        spawn(sched, pkt_gen, pkt1, 0);
        spawn(sched, socket_gen, socket, 0);
 
        sched_run(sched);
        pkt_stats(pkt1);       
        
        sink_stats(sink);  
    }

}



