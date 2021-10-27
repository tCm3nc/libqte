#include "libqte.h"

// Malloc interceptor
bool __libqte_malloc_initialised;

// function prototype of the original malloc and free implementation for GLIBC
void* (*__orig_libc_malloc)(size_t);
void (*__orig_libc_free)(void*);

void __libqte_init_malloc(void) {
  if (__libqte_malloc_initialised)
    return;

  __orig_libc_malloc = dlsym(RTLD_NEXT, "malloc");
  __orig_libc_free = dlsym(RTLD_NEXT, "free");

  __libqte_malloc_initialised = 1;
  LOG("malloc initialised");
}

void* __libqte_malloc(size_t size) {
  if (!__libqte_malloc_initialised) {
    __libqte_init_malloc();
  }

  void* p = __orig_libc_malloc(size);
  if (!p) {
    LOG("Unable to allocate memory.");
    return NULL;
  }

  return p;
}

void __libqte_free(void* ptr) {
  if (!ptr) {
    LOG("Freeing a null pointer?");
    return;
  }
  __orig_libc_free(ptr);
}