
#ifndef PACKET_H
#define PACKET_H

typedef struct {
   int id;
   int create_time;  // time at which packet was created
   int enqueue_time;
   int source;
   int dest;
   int size;
   int flow_id;
} packet;

struct pbuffer
{
    packet *p; 
    int key;
    struct pbuffer *next;
    struct pbuffer *prev;
};


void packet_init(packet* self, int id, int t, int source, int dest, int flow_id, int size);
packet* packet_create(int id, int t, int source, int dest, int flow_id, int size);
packet* packet_create_noinit();
void packet_copy(packet* from_p, packet* to_p);
void packet_destroy(packet* obj);

#endif // PACKET_H