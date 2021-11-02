#include <assert.h>  // for assert
#include <stdint.h>  // for `uintptr_t` type

#include "../include/libqte.h"

bool is_aligned(void* addr) {
  // tests if address is not aligned, return the inverse of test.
  return !((uintptr_t)addr & (QTE_GRANULE_SIZE - 1));
}

int main() {
  // test that malloc returns allocations aligned to the tag granule size (16).
  void* p = malloc(16);
  assert((is_aligned(p) == true));
  free(p);
  // short allocation
  p = malloc(1);
  assert((is_aligned(p) == true));
  free(p);
  // 0 byte allocation
  p = malloc(0);
  assert((is_aligned(p)) == true);
  free(p);
  // massive allocation
  p = malloc(0xffffffff);
  assert((is_aligned(p)) == true);
  free(p);
}