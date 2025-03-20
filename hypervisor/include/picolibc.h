#ifndef PICOLIBC_H

#include <stddef.h>
#include <stdint.h>

//Memory
void * memcpy(void *dest, const void *src, size_t n);
int64_t memcmp(const void *bl1, const void *bl2, size_t n);

//Strings
int64_t strcmp(const uint8_t *a, const uint8_t *b);
uint64_t strlen(const uint8_t *a);

//Math
uint64_t pow(uint64_t a, uint64_t b);
uint64_t div(int num, int denom);

#endif