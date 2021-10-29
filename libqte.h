#ifndef LIBQTE_H
#define LIBQTE_H

#define _GNU_SOURCE   // this is required as RTLD_NEXT is not defined otherwise
#include <dlfcn.h>    // for dlsym, RTLD_NEXT
#include <errno.h>    // for EINVAL, ENOMEM etc.
#include <stdbool.h>  // for bool type
#include <stdio.h>    // for fprintf
#include <stdlib.h>   // for the stdlib functions
#include <string.h>   // for memcpy, memset, memalign etc.

#ifdef DEBUG
#define LOG(x...)                                                     \
  do {                                                                \
    fprintf(stderr, "[%s:%s:%d]:\t\t", __FILE__, __func__, __LINE__); \
    fprintf(stderr, x);                                               \
    fprintf(stderr, "\n");                                            \
  } while (0)
#else
#define LOG(x...) (void)(0);
#endif

#ifdef DEBUG
#define debug_break() \
  do {                \
    __asm__("int3");  \
  } while (0)
#else
#define debug_break() \
  do {                \
  } while (0)
#endif

void __libqte_init_hooks(void);
void __libqte_init_malloc(void);

void* __libqte_malloc(size_t size);
void __libqte_free(void* ptr);
// void* __libqte_calloc(size_t nmemb, size_t size);
// void* __libqte_realloc(void* ptr, size_t size);
int __libqte_posix_memalign(void** ptr, size_t alignment, size_t size);
void* __libqte_memalign(size_t alignment, size_t size);
void* __libqte_aligned_alloc(size_t alignment, size_t size);

#endif  // LIBQTE_H