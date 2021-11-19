#include "qemu-qte.h"

// Fast psuedorandom generator,
// reference:
// https://lemire.me/blog/2019/07/03/a-fast-16-bit-random-number-generator/

// Seed for psuedorandom number generator
uint16_t wyhash16_x;

uint32_t hash16(uint32_t input, uint32_t key) {
  uint32_t hash = input * key;
  return ((hash >> 16) ^ hash) & 0xFFFF;
}

uint16_t wyhash16() {
  wyhash16_x += 0xfc15;
  return hash16(wyhash16_x, 0x2ab);
}

uint16_t rand_range16(const uint16_t s) {
  uint16_t x = wyhash16();
  uint32_t m = (uint32_t)x * (uint32_t)s;
  uint16_t l = (uint16_t)m;
  if (l < s) {
    uint16_t t = -s % s;
    while (l < t) {
      x = wyhash16();
      m = (uint32_t)x * (uint32_t)s;
      l = (uint16_t)m;
    }
  }
  return m >> 16;
}

uint16_t generate_tag() {
  // Generate a random tag that may or may not be used.
  uint16_t tag = rand_range16(0xffff);
  LOG("Generated tag : %x", tag);
  return tag;
}

void tag_allocation(void* start, void* end) {
  // Tag an allocation by generating a 16 bit tag.
  uint16_t tag = generate_tag();
  (void)tag;
  size_t increment = 0;
  (void)increment;
  // while (start < end) {
  //   // For every TAG granule size, tag the allocation.
  // }
}

void init_qte_runtime() {
  // Initialise the seed using the output from random, clamped to 16 bits.
  // This may be quite terrible.
  wyhash16_x = (uint16_t)(random());
  LOG("QTE runtime initialised");
}