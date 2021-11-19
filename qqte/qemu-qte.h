#ifndef QEMU_QTE_H
#define QEMU_QTE_H

#include <stdint.h>  // for the uint*_t types
#include <stdlib.h>  // for rand()
#include "../include/qte.h"

uint32_t hash16(uint32_t input, uint32_t key);
uint16_t wyhash16(void);
uint16_t rand_range16(const uint16_t s);
uint16_t generate_tag(void);

void tag_allocation(void* start, void* end);
void init_qte_runtime(void);

#endif