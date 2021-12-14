#include <stdlib.h>
#include <string.h>

int main(int argc, char** argp) {
  unsigned char* p = malloc(89);  // aligned to 96.
  p[0] = 'A';
  char f = p[96];  // This wont cause an error.

  f = p[101];  // This wont either! (96 + 16 = 112) So for every n byte
               // allocation, it seems n+16 bytes wont get caught?!

  char c = p[112];  // This will cause a problem
  free(p);
  return c;
}