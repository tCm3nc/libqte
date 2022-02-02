#define _GNU_SOURCE
#include <pthread.h>   // for pthread_spinlock_t
#include <stdalign.h>  // for alignof
#include <stddef.h>    // for max_align_t
#include "include/libqte.h"
#include "qte.h"  // for macros related to talking to the QTE qemu.

// A collection of functions that manage the internal malloc hooks.

bool __libqte_malloc_initialised;

// function prototype of the original malloc and free implementation for GLIBC
void* (*__orig_libc_malloc)(size_t);
void (*__orig_libc_free)(void*);

const size_t alloc_align = alignof(max_align_t);

#ifdef __GLIBC__
// Since symbols are solved lazily at runtime through dl's `dl_runtime_resolve`
// that requires heap allocations, if we intercept `calloc`, this results in a
// infinite recursive loop.
// The solution is to have a temporary allocation zone, which will be used for
// local allocations until dl is able to resolve the symbol.
// for reference:
// https://stackoverflow.com/questions/6083337/overriding-malloc-using-the-ld-preload-mechanism

#define ALLOC_ZONE_SIZE 4096
static size_t __alloc_zone_idx;
static unsigned char __alloc_zone[ALLOC_ZONE_SIZE];
#endif

struct chunk_begin {
  size_t requested_size;
  void* aligned_orig;  // NULL if not aligned
  struct chunk_begin* next;
  struct chunk_begin* prev;
};

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
    // Use the temporary allocation zone to allocate some memory for now.
    void* t = &__alloc_zone[__alloc_zone_idx];
    if (size & (alloc_align - 1)) {
      // align the size and bump to zones.
      __alloc_zone_idx += (size & ~(alloc_align - 1)) + alloc_align;
    } else {
      // bump zone by aligned size
      __alloc_zone_idx += size;
    }
    return t;
  }
  // align the size to the tag granule size.
  size_t aligned_size = size;
  if (size & (QTE_GRANULE_SIZE - 1)) {
    aligned_size = size + (QTE_GRANULE_SIZE - (size & QTE_GRANULE_SIZE - 1) &
                           (QTE_GRANULE_SIZE - 1));
  }

  // TODO: disable QTE for this thread. Otherwise it results in a recursive
  // loop where the backend malloc calls realloc, and that calls further libc
  // functions which result in QTE instrumentation picking it up and bailing on
  // load-widening instances inside libc's optimised implementations.
  uint8_t state = QTE_SWAP(QTE_DISABLED);
  struct chunk_begin* p =
      __orig_libc_malloc(sizeof(struct chunk_begin) + aligned_size);
  // Now renable
  QTE_SWAP(state);

  if (!p) {
    LOG("Unable to allocate memory.");
    return NULL;
  }
  p->requested_size = size;
  p->aligned_orig = NULL;
  p->next = p->prev = NULL;

  // Inform QTE about this allocation.
  // NOTE: We only inform QTE of the requested size.
  void* tagged_ptr = QTE_ALLOC(&p[1], (char*)&p[1] + size);
  LOG("Original alloc(%zu) %p, Tagged alloc(%zu) : %p", size, p, aligned_size,
      tagged_ptr);

  return tagged_ptr;
}

void __libqte_free(void* ptr) {
  if (!ptr) {
    LOG("Freeing a null pointer?");
    return;
  }
  // If the pointer is from our temporary allocation region, just return
  if ((ptr >= (void*)__alloc_zone) &&
      (ptr <= ((void*)__alloc_zone + ALLOC_ZONE_SIZE))) {
    LOG("free(%p) on an internal pre-malloc allocation. no-op", ptr);
    return;
  }
  struct chunk_begin* p = ptr;
  p -= 1;
  // size_t n = p->requested_size;
  // TODO: finish checking!
  // QTE_STORE
  void* untagged_ptr = QTE_FREE(&p[1]);
  LOG("untagged ptr : %p, tagged ptr : %p", untagged_ptr, ptr);
  // TODO: disable QTE for this thread. See note in `__libqte_malloc` for why.
  uint8_t state = QTE_SWAP(QTE_DISABLED);
  // Free the allocation as well as the `struct chunk_begin` header.
  struct chunk_begin* q = untagged_ptr;
  q -= 1;
  __orig_libc_free(q);
  // re-enable after backend free.
  QTE_SWAP(state);
}

void* __libqte_calloc(size_t nmemb, size_t size) {
  size_t total_size = nmemb * size;
  if (!__libqte_malloc_initialised) {
    if (total_size < ALLOC_ZONE_SIZE) {
      // Use temporary memory while `dl_runtime_resolve` tries to find symbols
      void* t = &__alloc_zone[__alloc_zone_idx];
      // TODO: what about aligned bump?
      __alloc_zone_idx += total_size;
      return t;
    } else {
      // total_size looks like it may be an issue. Don't really have a suitable
      // way to deal with it right now.
      return NULL;
    }
  }
  void* p = __libqte_malloc(total_size);
  if (!p)
    return NULL;
  __builtin_memset(p, 0, total_size);
  return p;
};

void* __libqte_realloc(void* ptr, size_t size) {
  void* p = __libqte_malloc(size);
  if (!p)
    return NULL;

  if (!ptr)
    return p;
  // disable QTE for this thread as the following access will throw an invalid
  // access error. (`chunk_begin` hasn't been tagged in QTE land.)
  // TODO: verify that this behaviour is as intended.
  uint8_t state = QTE_SWAP(QTE_DISABLED);
  // Still need to translate the pointer though!
  uintptr_t untagged_ptr = (uintptr_t)QTE_UNTAG(ptr);
  LOG("realloc(%p -> %p)", (void*)(long)ptr, (void*)(long)untagged_ptr);
  // ptr = untagged_ptr;
  size_t n = ((struct chunk_begin*)untagged_ptr)[-1].requested_size;
  if (size < n)
    n = size;
  // re-enable QTE
  QTE_SWAP(state);
  __builtin_memcpy(p, ptr, n);
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

  size_t rem = size % alignment;
  size_t len = size;
  if (rem)
    len += rem;

  // TODO: Disable QTE for this thread. See note in `__libqte_malloc` for why.
  uint8_t state = QTE_SWAP(QTE_DISABLED);
  char* orig = __orig_libc_malloc(sizeof(struct chunk_begin) + len);
  QTE_SWAP(state);

  if (!orig)
    return ENOMEM;

  char* data = orig + sizeof(struct chunk_begin);
  data += alignment - ((uintptr_t)data % alignment);

  struct chunk_begin* p = (struct chunk_begin*)data - 1;
  p->requested_size = len;
  p->aligned_orig = orig;

  void* tagged_ptr = QTE_ALLOC(data, data + len);
  *ptr = tagged_ptr;
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

// Dynamically raising a trap instruction to QEMU to enable debugging.
void __libqte_debug() {
  QTE_DEBUG();
}