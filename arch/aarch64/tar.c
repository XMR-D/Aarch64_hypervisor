#include <stdint.h> 

#include "tar.h"

#define BLOCKSIZE 512

/* 
    Function that check for the end of a tar file
    marked by a 512 byte block full of zero.
*/
static uint8_t is_end_of_tar(uint8_t * c)
{
    uint64_t i;

    for (i = 0; i < BLOCKSIZE; i++) {
        if (*c != 0) {
            return 0;
        }
    }
    return 1;
}

/*
    Method to extract the tar size from the 
    tar archive placed at off in memory

    Basically starts and check for each block, if it's full of 0
    it's the end of tar meaning we reach the end and mst return the
    size. size is increased by blocksize eachtime.

    off must reprensent the start of a tar otherwise UB.
*/
uint64_t compute_tar_size(uint64_t off)
{
    uint8_t * curr = (uint8_t *) &off;
    uint64_t size = 0;

    while (!is_end_of_tar(curr)) {
        size += BLOCKSIZE;
        curr += BLOCKSIZE;
    }
    return size;
}
