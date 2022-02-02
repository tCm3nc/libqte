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

#include "include/libqte.h"
// contains the definitions of QTE hypercalls.
#include <stddef.h>
#include "qte.h"

char* (*__lq_libc_fgets)(char*, int, FILE*);
int (*__lq_libc_atoi)(const char*);
long (*__lq_libc_atol)(const char*);
long long (*__lq_libc_atoll)(const char*);

void __libqte_init_hooks(void) {
  __libqte_init_malloc();
  __lq_libc_fgets = ASSERT_DLSYM(fgets);
  __lq_libc_atoi = ASSERT_DLSYM(atoi);
  __lq_libc_atol = ASSERT_DLSYM(atol);
  __lq_libc_atoll = ASSERT_DLSYM(atoll);
}

void* malloc(size_t size) {
  void* p = __libqte_malloc(size);
  LOG("%p = malloc(%zu)", p, size);
  return p;
}

void free(void* ptr) {
  LOG("Freeing pointer %p", ptr);
  __libqte_free(ptr);
  return;
}

void* calloc(size_t nmemb, size_t size) {
  LOG("calling calloc with %zu, %zu", nmemb, size);
  void* p = __libqte_calloc(nmemb, size);
  LOG("%p = calloc(%zu, %zu)", p, nmemb, size);
  return p;
}

void* realloc(void* ptr, size_t size) {
  void* p = __libqte_realloc(ptr, size);
  LOG("%p = realloc(%p, %zu)", p, ptr, size);
  return p;
}

int posix_memalign(void** ptr, size_t alignment, size_t size) {
  LOG("posix_memalign_before(%p, %zu, %zu)", ptr, alignment, size);
  int v = __libqte_posix_memalign(ptr, alignment, size);
  LOG("%d = posix_memalign(%p, %zu, %zu)", v, ptr, alignment, size);
  return v;
}

void* memalign(size_t alignment, size_t size) {
  void* p = __libqte_memalign(alignment, size);
  LOG("%p = memalign(%zu, %zu)", p, alignment, size);
  return p;
}

void* aligned_alloc(size_t alignment, size_t size) {
  void* p = __libqte_aligned_alloc(alignment, size);
  LOG("%p = aligned_alloc(%zu, %zu)", p, alignment, size);
  return p;
}

void* valloc(size_t size) {
  void* p = __libqte_memalign(sysconf(_SC_PAGESIZE), size);
  LOG("%p = valloc(%zu)", p, size);
  return p;
}

void* pvalloc(size_t size) {
  size_t page_size = sysconf(_SC_PAGESIZE);
  size = (size & (page_size - 1)) + page_size;
  void* p = __libqte_memalign(page_size, size);
  LOG("%p = pvalloc(%zu)", p, size);
  return p;
}

char* fgets(char* s, int size, FILE* stream) {
  s = (char*)QTE_STORE(s, size);
#ifndef __ANDROID__
  stream = (FILE*)QTE_LOAD(stream, sizeof(FILE));
#endif
  void* p = __lq_libc_fgets(s, size, stream);
  LOG("%p = fgets(%p, %d, %p)", p, s, size, stream);
  return p;
}

int memcmp(const void* s1, const void* s2, size_t n) {
  s1 = (void*)QTE_LOAD((void*)s1, n);
  s1 = (void*)QTE_LOAD((void*)s2, n);
  int cmp = __libqte_memcmp(s1, s2, n);
  LOG("%d = memcmp(%p, %p, %zu)", cmp, s1, s2, n);
  return cmp;
}

void* memcpy(void* dest, const void* src, size_t n) {
  LOG("memcpy(%p, %p, %zu)", dest, src, n);
  src = (void*)QTE_LOAD((void*)src, n);
  dest = (void*)QTE_STORE((void*)dest, n);
  void* p = __libqte_memcpy(dest, src, n);
  LOG("%p = memcpy(%p, %p, %zu)", p, dest, src, n);
  return p;
}

void* mempcpy(void* dest, const void* src, size_t n) {
  src = (void*)QTE_LOAD((void*)src, n);
  dest = (void*)QTE_STORE((void*)dest, n);
  void* p = (uint8_t*)__libqte_memcpy(dest, src, n) + n;
  LOG("%p = mempcpy(%p, %p, %zu)", p, dest, src, n);
  return p;
}

void* memmove(void* dest, const void* src, size_t n) {
  src = (void*)QTE_LOAD((void*)src, n);
  dest = (void*)QTE_STORE((void*)dest, n);
  void* p = __libqte_memmove(dest, src, n);
  LOG("%p = memmove(%p, %p, %zu)", p, dest, src, n);
  return p;
}

void* memset(void* s, int c, size_t n) {
  s = (void*)QTE_STORE((void*)s, n);
  void* r = __libqte_memset(s, c, n);
  LOG("%p = memset(%p, %d, %zu)", r, s, c, n);
  return r;
}

void* memchr(const void* s, int c, size_t n) {
  void* p = __libqte_memchr(s, c, n);
  if (p == NULL) {
    s = (void*)QTE_LOAD((void*)s, n);
  } else {
    s = (void*)QTE_LOAD((void*)s, p - s);
  }
  LOG("%p = memchr(%p, %d, %zu)", p, s, c, n);
  return p;
}

void* memrchr(const void* s, int c, size_t n) {
  s = (void*)QTE_LOAD((void*)s, n);
  void* p = __libqte_memrchr(s, c, n);
  LOG("%p = memrchr(%p, %d, %zu)", p, s, c, n);
  return p;
}

void* memmem(const void* haystack,
             size_t haystacklen,
             const void* needle,
             size_t needlelen) {
  haystack = (void*)QTE_LOAD((void*)haystack, haystacklen);
  needle = (void*)QTE_LOAD((void*)needle, needlelen);
  void* p = __libqte_memmem(haystack, haystacklen, needle, needlelen);
  LOG("%p = memmem(%p, %zu, %p, %zu)", p, haystack, haystacklen, needle,
      needlelen);
  return p;
}

#ifndef __BIONIC__
void bzero(void* s, size_t n) {
  s = (void*)(void*)QTE_STORE((void*)s, n);
  __libqte_memset(s, 0, n);
}
#endif

void explicit_bzero(void* s, size_t n) {
  s = (void*)QTE_STORE((void*)s, n);
  LOG("explicit_bzero(%p, %zu)", s, n);
  __libqte_memset(s, 0, n);
}

int bcmp(const void* s1, const void* s2, size_t n) {
  s1 = (void*)QTE_LOAD((void*)s1, n);
  s2 = (void*)QTE_LOAD((void*)s2, n);
  int r = __libqte_bcmp(s1, s2, n);
  LOG("%d = bcmp(%p, %p, %zu)", r, s1, s2, n);
  return r;
}

char* strchr(const char* s, int c) {
  size_t l = __libqte_strlen(s);
  s = (char*)QTE_LOAD((void*)s, l + 1);
  void* p = __libqte_strchr(s, c);
  LOG("%p = strchr(%p, %d)", p, s, c);
  return p;
}

char* strrchr(const char* s, int c) {
  size_t l = __libqte_strlen(s);
  s = (char*)QTE_LOAD((void*)s, l + 1);
  void* p = __libqte_strrchr(s, c);
  LOG("%p strrchr(%p, %d)", p, s, c);
  return p;
}

int strcasecmp(const char* s1, const char* s2) {
  size_t l1 = __libqte_strlen(s1);
  s1 = (char*)QTE_LOAD((void*)s1, l1 + 1);
  size_t l2 = __libqte_strlen(s2);
  s2 = (char*)QTE_LOAD((void*)s2, l2 + 1);
  int r = __libqte_strcasecmp(s1, s2);
  LOG("%d = strcasecmp(%p, %p)", r, s1, s2);
  return r;
}

int strncasecmp(const char* s1, const char* s2, size_t n) {
  size_t l1 = __libqte_strlen(s1);
  s1 = (char*)QTE_LOAD((void*)s1, l1 + 1);
  size_t l2 = __libqte_strlen(s2);
  s2 = (char*)QTE_LOAD((void*)s2, l2 + 1);
  int r = __libqte_strncasecmp(s1, s2, n);
  LOG("%d = strncasecmp(%p, %p, %zu)", r, s1, s2, n);
  return r;
}

char* strcat(char* dest, const char* src) {
  size_t l1 = __libqte_strlen(src);
  src = (char*)QTE_LOAD((void*)src, l1 + 1);
  size_t l2 = __libqte_strlen(dest);
  dest = (char*)QTE_LOAD((void*)dest, l2 + 1);
  __libqte_memcpy(dest + l1, src, l2);
  dest[l1 + l2] = 0;
  void* p = dest;
  LOG("%p = strcat(%p, %p)", p, dest, src);
  return p;
}

int strcmp(const char* s1, const char* s2) {
  size_t l1 = __libqte_strlen(s1);
  s1 = (char*)QTE_LOAD((void*)s1, l1 + 1);
  size_t l2 = __libqte_strlen(s2);
  s2 = (char*)QTE_LOAD((void*)s2, l2 + 1);
  int r = __libqte_strcmp(s1, s2);
  LOG("%d = strcmp(%p, %p)", r, s1, s2);
  return r;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  size_t l1 = __libqte_strnlen(s1, n);
  s1 = (char*)QTE_LOAD((void*)s1, l1);
  size_t l2 = __libqte_strnlen(s2, n);
  s2 = (char*)QTE_LOAD((void*)s2, l2);
  int r = __libqte_strncmp(s1, s2, n);
  LOG("%d = strncmp(%p, %p, %zu)", r, s1, s2, n);
  return r;
}

char* strcpy(char* dest, const char* src) {
  size_t l = __libqte_strlen(src) + 1;
  src = (char*)QTE_LOAD((void*)src, l);
  dest = (char*)QTE_STORE((void*)dest, l);
  void* p = __libqte_memcpy(dest, src, l);
  LOG("%p = strcpy(%p, %p)", p, dest, src);
  return p;
}

char* strncpy(char* dest, const char* src, size_t n) {
  size_t l = __libqte_strnlen(src, n);
  void* p = NULL;
  if (l < n) {
    src = (char*)QTE_LOAD((void*)src, l + 1);
    p = __libqte_memcpy(dest, src, l + 1);
  } else {
    src = (char*)QTE_LOAD((void*)src, n);
    p = __libqte_memcpy(dest, src, n);
  }
  LOG("%p = strncpy(%p, %p, %zu)", p, dest, src, n);
  return p;
}

char* stpcpy(char* dest, const char* src) {
  size_t l = __libqte_strlen(src) + 1;
  src = (char*)QTE_LOAD((void*)src, l);
  dest = (char*)QTE_STORE((void*)dest, l);
  char* p = __libqte_memcpy(dest, src, l) + (l - 1);
  LOG("%p = stpcpy(%p, %p)", p, dest, src);
  return p;
}

char* strdup(const char* s) {
  size_t l = __libqte_strlen(s);
  void* p = __libqte_malloc(l + 1);
  s = (char*)QTE_LOAD((void*)s, l + 1);
  __libqte_memcpy(p, s, l + 1);
  LOG("%p = strdup(%p)", p, s);
  return p;
}

size_t strlen(const char* s) {
  size_t l = __libqte_strlen(s);
  s = (char*)QTE_LOAD((void*)s, l + 1);
  LOG("%zu = strlen(%p)", l, s);
  return l;
}

size_t strnlen(const char* s, size_t n) {
  size_t l = __libqte_strnlen(s, n);
  s = (char*)QTE_LOAD((void*)s, n);
  LOG("%zu = strnlen(%p, %zu)", l, s, n);
  return l;
}

char* strstr(const char* haystack, const char* needle) {
  size_t l = __libqte_strlen(haystack) + 1;
  haystack = (char*)QTE_LOAD((void*)haystack, l);
  l = __libqte_strlen(needle) + 1;
  needle = (char*)QTE_LOAD((void*)needle, l);
  void* p = __libqte_strstr(haystack, needle);
  LOG("%p = strstr(%p, %p)", p, haystack, needle);
  return p;
}

char* strcasestr(const char* haystack, const char* needle) {
  size_t l = __libqte_strlen(haystack) + 1;
  haystack = (char*)QTE_LOAD((void*)haystack, l);
  l = __libqte_strlen(needle) + 1;
  needle = (char*)QTE_LOAD((void*)needle, l);
  void* p = __libqte_strcasestr(haystack, needle);
  LOG("%p = strcasestr(%p, %p)", p, haystack, needle);
  return p;
}

int atoi(const char* nptr) {
  size_t l = __libqte_strlen(nptr) + 1;
  nptr = (char*)QTE_LOAD((void*)nptr, l);
  int r = __lq_libc_atoi(nptr);
  LOG("%d = atoi(%p)", r, nptr);
  return r;
}

long atol(const char* nptr) {
  size_t l = __libqte_strlen(nptr) + 1;
  nptr = (char*)QTE_LOAD((void*)nptr, l);
  long r = __lq_libc_atol(nptr);
  LOG("%ld = atol(%p)", r, nptr);
  return r;
}

long long atoll(const char* nptr) {
  size_t l = __libqte_strlen(nptr) + 1;
  nptr = (char*)QTE_LOAD((void*)nptr, l);
  long long r = __lq_libc_atoll(nptr);
  LOG("%lld = atoll(%p)", r, nptr);
  return r;
}

size_t wcslen(const wchar_t* s) {
  size_t r = __libqte_wcslen(s);
  s = (wchar_t*)QTE_LOAD((void*)s, sizeof(wchar_t) * (r + 1));
  LOG("%zu = wcslen(%p)", r, (void*)s);
  return r;
}

wchar_t* wcscpy(wchar_t* dest, const wchar_t* src) {
  size_t l = __libqte_wcslen(src) + 1;
  src = (wchar_t*)QTE_LOAD((void*)src, l * sizeof(wchar_t));
  dest = (wchar_t*)QTE_STORE((void*)dest, l * sizeof(wchar_t));
  void* p = __libqte_wcscpy(dest, src);
  LOG("%p = wcscpy(%p, %p)", p, dest, src);
  return p;
}

int wcscmp(const wchar_t* s1, const wchar_t* s2) {
  size_t l1 = __libqte_wcslen(s1);
  s1 = (wchar_t*)QTE_LOAD((void*)s1, sizeof(wchar_t) * (l1 + 1));
  size_t l2 = __libqte_wcslen(s2);
  s2 = (wchar_t*)QTE_LOAD((void*)s2, sizeof(wchar_t) * (l2 + 1));
  int r = __libqte_wcscmp(s1, s2);
  LOG("%d = wcscmp(%p, %p)", r, s1, s2);
  return r;
}
