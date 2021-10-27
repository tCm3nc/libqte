#include "libqte.h"

void _libqte_init() {
  // initialise hooks
  __libqte_init_hooks();
}

int __libc_start_main(int*(main)(int, char**, char**),
                      int argc,
                      char** ubp_av,
                      void (*init)(void),
                      void (*fini)(void),
                      void (*rtld_fini)(void),
                      void(*stack_end)) {
  typeof(&__libc_start_main) original_main =
      dlsym(RTLD_NEXT, "__libc_start_main");
  // time to initialise our runtime.
  _libqte_init();
  return original_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}
