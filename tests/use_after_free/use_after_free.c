#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
  unsigned char* p = (unsigned char*)malloc(5);
  //
  const char a[] = "AAAA";
  const char b[] = "B";
  memcpy(p, &a, 4);
  free(p);
  memcpy(p, &b, 1);
  return 0;
}
