#ifndef LIBQTE_H
#define LIBQTE_H

#define _GNU_SOURCE
#include <dlfcn.h>  // for dlsym
// #include <fcntl.h>
#include <stdbool.h>  // for bool type
#include <stdio.h>    // for fprintf
#include <stdlib.h>
#include <string.h>  // for memcpy, memset, memalign etc.

#ifdef DEBUG
#define LOG(x...)                                     \
  do {                                                \
    fprintf(stderr, "[%s:%d] :", __func__, __LINE__); \
    fprintf(stderr, x);                               \
    fprintf(stderr, "\n");                            \
  } while (0)
#else
#define LOG(x...) (void)(0);
#endif

void __libqte_init_hooks(void);
void __libqte_init_malloc(void);

void* __libqte_malloc(size_t size);
void __libqte_free(void* ptr);

#endif  // LIBQTE_H