/*******************************************************************************
Copyright (c) 2019-2020, Andrea Fioraldi, Copyright (c) 2021-222 Melvin Mammen


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef LIBQTE_H
#define LIBQTE_H

#define _GNU_SOURCE   // this is required as RTLD_NEXT is not defined otherwise
#include <dlfcn.h>    // for dlsym, RTLD_NEXT
#include <errno.h>    // for EINVAL, ENOMEM etc.
#include <stdbool.h>  // for bool type
#include <stdint.h>   // for uint*_t types
#include <stdio.h>    // for fprintf
#include <stdlib.h>   // for the stdlib functions
#include <string.h>   // for memcpy, memset, memalign etc.
#include <unistd.h>   // for sysconf
#include <wchar.h>    // for wcs_* family of functions

#ifdef DEBUG
#define LOG(x...)                                                     \
  do {                                                                \
    fprintf(stderr, "[%s:%s:%d]:\t\t", __FILE__, __func__, __LINE__); \
    fprintf(stderr, x);                                               \
    fprintf(stderr, "\n");                                            \
    fflush(stderr);                                                   \
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

#define ASSERT_DLSYM(name)                                                    \
  ({                                                                          \
    void* a = (void*)dlsym(RTLD_NEXT, #name);                                 \
    if (!a) {                                                                 \
      fprintf(stderr, "FATAL ERROR: failed dlsym of " #name " in libqte!\n"); \
      abort();                                                                \
    }                                                                         \
    a;                                                                        \
  })

#define QTE_GRANULE_SIZE 16

void __libqte_init_hooks(void);
void __libqte_init_malloc(void);

void* __libqte_malloc(size_t size);
void __libqte_free(void* ptr);
void* __libqte_calloc(size_t nmemb, size_t size);
void* __libqte_realloc(void* ptr, size_t size);
int __libqte_posix_memalign(void** ptr, size_t alignment, size_t size);
void* __libqte_memalign(size_t alignment, size_t size);
void* __libqte_aligned_alloc(size_t alignment, size_t size);

void __libqte_debug(void);

void* __libqte_memcpy(void* dest, const void* src, size_t n);
void* __libqte_memmove(void* dest, const void* src, size_t n);
void* __libqte_memset(void* s, int c, size_t n);
void* __libqte_memchr(const void* s, int c, size_t n);
void* __libqte_memrchr(const void* s, int c, size_t n);
size_t __libqte_strlen(const char* s);
size_t __libqte_strnlen(const char* s, size_t len);
int __libqte_strcmp(const char* str1, const char* str2);
int __libqte_strncmp(const char* str1, const char* str2, size_t len);
int __libqte_memcmp(const void* mem1, const void* mem2, size_t len);
int __libqte_bcmp(const void* mem1, const void* mem2, size_t len);
char* __libqte_strstr(const char* haystack, const char* needle);
char* __libqte_strcasestr(const char* haystack, const char* needle);
void* __libqte_memmem(const void* haystack,
                      size_t haystack_len,
                      const void* needle,
                      size_t needle_len);
char* __libqte_strchr(const char* s, int c);
char* __libqte_strrchr(const char* s, int c);
size_t __libqte_wcslen(const wchar_t* s);
wchar_t* __libqte_wcscpy(wchar_t* d, const wchar_t* s);
int __libqte_wcscmp(const wchar_t* s1, const wchar_t* s2);
int __libqte_strcasecmp(const char* str1, const char* str2);
int __libqte_strncasecmp(const char* str1, const char* str2, size_t len);

#endif  // LIBQTE_H