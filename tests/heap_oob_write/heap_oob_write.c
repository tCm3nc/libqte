#include <stdlib.h>
#include <string.h>

int main(int argc, char** argp) {
  unsigned char* p = (unsigned char*)malloc(89);
  p[19] = 'A';
  p[90] = 'A';  // FIXME: this is a valid *aligned* access!
  p[97] = 'A';  // NOTE: This isnt. malloc(89) returns a 96 byte buffer.
  free(p);
  return 0;
}