#include <stdlib.h>
#include <string.h>

int main(int argc, char** argp) {
  // The purpose of this test is to check that addition and subtraction ops
  // can handle tagged pointer arithmetic.
  // This allocation means that [A]+0xffff + 1 should be a valid access.
  // This test is to ensure that QTE is instrumented properly for this case.

  unsigned char* r = malloc((1 << 16) * 5);
  unsigned char* s = &r[(1 << 16) * 5];
  s[0] = 'C';
  free(r);
}