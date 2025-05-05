#include <stdint.h>

#include "picolibc.h"

#include "serial.h"
//Memory

//memcpy: copy n bytes from src to dest assuming memory does not overlap
void * memcpy(void *dest, const void *src, size_t n)
{
    uint8_t * ds = (uint8_t *) dest;
    uint8_t * sr = (uint8_t *) src;

    for (size_t i = 0; i < n; i++) {
        *ds = *sr;
        ds++;
        sr++;
    }
    return dest;
}

//memcmp: compare n bytes of two memory area
int64_t memcmp(const void *bl1, const void *bl2, size_t n)
{
    uint8_t * block1 = (uint8_t *) bl1;
    uint8_t * block2 = (uint8_t *) bl2;

    for(size_t i = 0; i < n; i++) {
        int64_t sub = *block1 - *block2;
        if (sub != 0)
            return sub;
        block1++;
        block2++;
    }
    return 0;
}

//Strings

//strlen: length of a string
uint64_t strlen(const uint8_t *a)
{
    uint64_t ret = 0;
    while(a[ret] != 0)
        ret++;
    return ret;
}

//strcmp: compare two strings
int64_t strcmp(const uint8_t *a, const uint8_t *b)
{
    uint64_t la = strlen(a);
    uint64_t lb = strlen(b);
    uint64_t sub = la - lb;

    if(sub != 0)
        return sub;
    for(size_t i = 0; i < la; i++) {
        uint64_t subc = a[i] - b[i];
        if (subc != 0)
            return subc;
    }
    return 0;
}

//Math
uint64_t pow(uint64_t a, uint64_t b)
{
    uint64_t count = 1;

    while (b >= 1) {
        count *= a;
        b--;
    }
    return count;
}