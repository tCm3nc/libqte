#include "../include/libqte.h"  // for LOG
#include "assert.h"             // for assert
#include "errno.h"              // for errno
#include "stdlib.h"
#include "string.h"  // for strerror

// verify that overloaded posix_memalign does indeed return addresses aligned to
// requested values.

int do_test() {
  void* p = NULL;
  size_t alignment = 256;
  size_t size = 14;
  int r = posix_memalign(&p, alignment, size);
  if (r != 0) {
    LOG("Unable to allocate memory reason : %s", strerror(r));
    return 1;
  }
  if (p == NULL) {
    LOG("Unable to allocate memory, ptr is NULL!");
    return 2;
  }

  LOG("posix_memalign(%p, %zu, %zu) = %d", p, alignment, size, r);

  if ((size_t)p % alignment) {
    LOG("Address is not at an aligned boundary.");
    return 3;
  }
  assert((size_t)p % alignment == 0);
  free(p);
  return 0;
}

int main(int argc, char** argv) {
  do_test();
  return 0;
}