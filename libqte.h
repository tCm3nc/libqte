#ifndef LIBQTE_H
#define LIBQTE_H

#define _GNU_SOURCE
#include <dlfcn.h>  // for dlsym
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
int log_debug;
#endif

void __libqte_init_hooks(void);

#endif