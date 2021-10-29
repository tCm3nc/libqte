#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

int main() {
   unsigned char* p = (void*) malloc(10);
   if (p) {
       p[5] = (unsigned char) 0x41;
       free(p);
       free(p);
       unsigned char* q = malloc(10);
       assert(p == q);
   }
   return 0;
}
