#ifndef PICOLIBC_H
#define PICOLIBC_H

#include <stddef.h>
#include <stdint.h>

//Memory
void * memcpy(void *dest, const void *src, size_t n);
int64_t memcmp(const void *bl1, const void *bl2, size_t n);

//Strings
int64_t strcmp(uint8_t *a, uint8_t *b);
uint64_t strlen(uint8_t *a);

//Check if c is printable
uint8_t is_printable(uint8_t c);

//Check if given string of length size is a hexadecimal representation
//0 str is an hex nb
//1 str is not an hex nb
//2 error while checking
uint8_t is_hex(uint8_t * str, uint8_t size);

//Check if given string of length size is a decimal representation
//0 str is a dec nb
//1 str is not a dec nb
//2 error while checking
uint8_t is_dec(uint8_t * str, uint8_t size);

//Convert size bytes of a string into a number in given base
int64_t from_str(uint8_t * str, uint8_t size, int64_t base);

//Math
uint64_t pow(uint64_t a, uint64_t b);
uint64_t align_on_size(uint64_t addr, uint64_t size);

#endif /* !PICOLIBC_H*/