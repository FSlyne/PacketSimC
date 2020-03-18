#include "packet.h"
#include "scheduler.h"

typedef struct {
    SCHED* sched;
    void *typex;
    void (* out)(void* , int);
    int m;
    int id;
} CHAN;

void channel_init(CHAN* self, SCHED* sced, int id);
CHAN* channel_create(SCHED* sched, int id);
int channel_write(CHAN* self);
void channel_read(CHAN* self, int m);