#include "include/libqte.h"

void __libqte_init_hooks(void) {
  __libqte_init_malloc();
}

void* malloc(size_t size) {
  void* p = __libqte_malloc(size);
  LOG("malloc(%zu) = %p", size, p);
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
  LOG("calloc(%zu, %zu) = %p", nmemb, size, p);
  return p;
}

void* realloc(void* ptr, size_t size) {
  void* p = __libqte_realloc(ptr, size);
  LOG("realloc(%p, %zu) = %p", ptr, size, p);
  return p;
}

int posix_memalign(void** ptr, size_t alignment, size_t size) {
  LOG("posix_memalign_before(%p, %zu, %zu)", ptr, alignment, size);
  int v = __libqte_posix_memalign(ptr, alignment, size);
  LOG("posix_memalign(%p, %zu, %zu) = %d", ptr, alignment, size, v);
  return v;
}

void* memalign(size_t alignment, size_t size) {
  void* p = __libqte_memalign(alignment, size);
  LOG("memalign(%zu, %zu) = %p", alignment, size, p);
  return p;
}

void* aligned_alloc(size_t alignment, size_t size) {
  void* p = __libqte_aligned_alloc(alignment, size);
  LOG("aligned_alloc(%zu, %zu) = %p", alignment, size, p);
  return p;
}

// TODO: still have fgets, memcmp, memcpy, mempcpy, memmove, memset, memchr,
// memrchr, memmem, bcmp, explicit_bzero, strchr, strrchr, strcasecmp,
// strncasecmp, strcat, strcmp, strncmp, strcpy, strncpy, stpcpy, strdup,
// strlen, strnlen, strstr, strcasestr, atoi, atol, atoll, wcslen, wcscpy,
// wscscmp to implement.