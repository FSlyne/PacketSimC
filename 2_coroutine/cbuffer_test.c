#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cbuffer.h"


int main() {
struct cbuffer *st = NULL, *en = NULL;
char *data[] = {"first" , "second", "third", "fourth" , "fifth", "sixth"};

//char value[BUFSIZE];
int size;

for (int i=0; i<6; i++) {
   size=strlen(data[i]);
   insert(&st,&en,data[i],size,8-i);
//   rpop(&st,&en, value,&size);
//   printf("%s\n",value);
}

//lpop(&st,&en, value,&size);

//printf("%s\n",value);

printstack(st);

}
