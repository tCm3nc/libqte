#include <stdlib.h>
#include <string.h>

int main(int argc, char** argp) {
  // Becomes a 96 byte allocation after alignment to QTE_GRANULE_SIZE.
  unsigned char* p = malloc(89);
  p[0] = 'A';      // Valid memory write.
  char c = p[89];  // This is technically an OOB read, but this results in an
                   // intra-allocation read. (every allocation is aligned to 16
                   // bytes.) So QTE wont pick this up unfortunately.
  char f = p[96];  // One byte read outside of the aligned allocated region.
  f = p[-1];       // Negative index based access.
  free(p);
  return c;
}