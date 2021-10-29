#define _GNU_SOURCE
#include <stdint.h>  // for `uintptr_t` type
#include "libqte.h"

// Malloc interceptor
bool __libqte_malloc_initialised;

// function prototype of the original malloc and free implementation for GLIBC
void* (*__orig_libc_malloc)(size_t);
void (*__orig_libc_free)(void*);
int (*__orig_libc_posix_memalign)(void*, size_t, size_t);

void __libqte_init_malloc(void) {
  if (__libqte_malloc_initialised)
    return;

  __orig_libc_malloc = dlsym(RTLD_NEXT, "malloc");
  __orig_libc_free = dlsym(RTLD_NEXT, "free");
  __orig_libc_posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");

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

void* __libqte_calloc(size_t nmemb, size_t size) {
  size_t total_size = nmemb * size;

  void* p = __libqte_malloc(total_size);

  if (!p)
    return NULL;
  // FIXME: could this be the problem?
  __builtin_memset(p, 0, total_size);
  return p;
};

void* __libqte_realloc(void* ptr, size_t size) {
  void* p = __libqte_malloc(size);
  if (!p)
    return NULL;

  if (!ptr)
    return p;
  // FIXME: could this be the problem?
  __builtin_memcpy(p, ptr, size);
  // free the old pointer
  __libqte_free(ptr);
  return p;
};

/*
 * for functions - memalign, pvalloc, valloc, aligned_alloc, posix_memalign :
 * the allocated memory is not zeroed.
 */

// allocate memory of `size` bytes, and place the pointer in `ptr`.
// address of pointer must be aligned to `alignment`
int __libqte_posix_memalign(void** ptr, size_t alignment, size_t size) {
  // if alignment is not a power of 2 or not aligned to sizeof pointer on this
  // arch, return invalid value for alignment.

  if ((alignment % 2) || (alignment % sizeof(void*)))
    return EINVAL;

  // Empty size? Easy peasy..
  if (size == 0) {
    *ptr = NULL;
    return 0;
  }

  // pass to original implementation
  void* p = NULL;
  int ret = __orig_libc_posix_memalign(&p, alignment, size);
  *ptr = p;
  return 0;
}

void* __libqte_memalign(size_t alignment, size_t size) {
  void* p = NULL;
  __libqte_posix_memalign(&p, alignment, size);
  return p;
}

void* __libqte_aligned_alloc(size_t alignment, size_t size) {
  // same function as memalign, with requirement that `size` be a multiple of
  // `alignment`.
  void* p = NULL;
  // check if the size is aligned. a/b != 0
  if (size % alignment)
    return NULL;

  __libqte_posix_memalign(&p, alignment, size);
  return p;
}

// valloc(size_t size) - allocate `size` bytes and returns ptr. the adderss will
// be a multiple of the page size. equivalent to memalign(sysconf(_SC_PAGESIZE),
// size)
// pvalloc(size_t size) - similar to valloc, but rounds the size of allocation
// up to the next multiple of the system page size.