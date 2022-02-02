#ifndef QTE_H
#define QTE_H

#include <stdint.h>  // for uint*_t types
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

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

// Following QASAN's example of how to call the fake syscall.

#define QTE_FAKESYS_NR 0x1337
#define QTE_GRANULE_SIZE 16
#define BLOCK_ALLOCATION_ALIGN (1 << 16)

#define QTE_ENABLED (0)
#define QTE_DISABLED (1)

enum action_t {
  QTE_ACTION_ALLOC,
  QTE_ACTION_DEALLOC,
  QTE_ACTION_DEBUG,
  QTE_ACTION_SWAP_STATE,
  QTE_ACTION_UNTAG,
  QTE_ACTION_CHECK_LOAD,
  QTE_ACTION_CHECK_STORE,
};

// #define QTE_CALL0(action) syscall(QTE_FAKESYS_NR, action, NULL, NULL, NULL)
// #define QTE_CALL1(action, arg1) \
//   syscall(QTE_FAKESYS_NR, action, arg1, NULL, NULL)
// #define QTE_CALL3(action, arg1, arg2, arg3) \
//   syscall(QTE_FAKESYS_NR, action, arg1, arg2, arg3)

static inline long QTE_CALL0(enum action_t action) {
  return syscall(QTE_FAKESYS_NR, action);
}
static inline long QTE_CALL1(enum action_t action, void* arg1) {
  return syscall(QTE_FAKESYS_NR, action, arg1);
}
static inline long QTE_CALL2(enum action_t action, void* arg1, void* arg2) {
  return syscall(QTE_FAKESYS_NR, action, arg1, arg2, NULL);
}

static inline void* QTE_ALLOC(void* start, void* end) {
  void* ptr_tagged = (void*)QTE_CALL2(QTE_ACTION_ALLOC, start, end);
  return ptr_tagged;
}

static inline void* QTE_FREE(void* ptr) {
  void* ptr_untagged = (void*)QTE_CALL1(QTE_ACTION_DEALLOC, ptr);
  return ptr_untagged;
}

static inline uint8_t QTE_SWAP(uint8_t state) {
  uint8_t ret = (uint8_t)QTE_CALL1(QTE_ACTION_SWAP_STATE, (void*)(long)state);
  return ret;
}

static inline void* QTE_UNTAG(void* ptr) {
  void* ptr_untagged = (void*)QTE_CALL1(QTE_ACTION_UNTAG, ptr);
  return ptr_untagged;
}

static inline void* QTE_LOAD(void* ptr, unsigned int len) {
  void* ptr_untagged =
      (void*)QTE_CALL2(QTE_ACTION_CHECK_LOAD, ptr, (void*)(long)len);
  return ptr_untagged;
}

static inline void* QTE_STORE(void* ptr, unsigned int len) {
  void* ptr_untagged =
      (void*)QTE_CALL2(QTE_ACTION_CHECK_STORE, ptr, (void*)(long)len);
  return ptr_untagged;
}

static inline void QTE_DEBUG() {
  QTE_CALL0(QTE_ACTION_DEBUG);
}

#endif