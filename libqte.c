#include "libqte.h"

void _libqte_init() {
#ifdef DEBUG
  log_debug = getenv("QTE_DEBUG") != NULL;
#endif
}

int __libc_start_main(int*(main)(int, char**, char**),
                      int argc,
                      char** ubp_av,
                      void (*init)(void),
                      void (*fini)(void),
                      void (*rtld_fini)(void),
                      void(*stack_end)) {
  void* original_main = dlsym(RTLD_NEXT, "__libc_start_main");
  // time to initialise our runtime.
  _libqte_init();
  return original_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}
