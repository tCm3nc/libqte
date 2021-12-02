#ifndef QTE_H
#define QTE_H

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

enum action_t {
  QTE_ACTION_ALLOC,
  QTE_ACTION_DEALLOC,
  QTE_ACTION_DEBUG,
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

static inline void QTE_DEBUG() {
  QTE_CALL0(QTE_ACTION_DEBUG);
}

#endif