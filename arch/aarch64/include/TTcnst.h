#ifndef TTCNST_H

#include <stdint.h>

extern volatile STACK_END;
extern volatile PHYS_MEM_SIZE;

/*From ARM developper guide on virtual memory management and OSdev*/

#define PAGESIZE    4096

// granularity
#define PT_PAGE     0b11        // 4k granule
#define PT_BLOCK    0b01        // 2M granule
// accessibility
#define PT_KERNEL   (0<<6)      // privileged, supervisor EL1 access only
#define PT_USER     (1<<6)      // unprivileged, EL0 access allowed
#define PT_RW       (0<<7)      // read-write
#define PT_RO       (1<<7)      // read-only
#define PT_AF       (1<<10)     // accessed flag
#define PT_NX       (1UL<<54)   // no execute
// shareability
#define PT_OSH      (2<<8)      // outter shareable
#define PT_ISH      (3<<8)      // inner shareable
// defined in MAIR register
#define PT_MEM      (0<<2)      // normal memory
#define PT_DEV      (1<<2)      // device MMIO
#define PT_NC       (2<<2)      // non-cachable

#define TTBR_CNP    1

#define L2entry  PT_PAGE | PT_AF | PT_KERNEL | PT_ISH | PT_MEM
#define L3entry  PT_BLOCK | PT_AF | PT_KERNEL | PT_ISH | PT_MEM | PT_RW | PT_RO

//L2 block size = 2MB | L3 block size = 4KB
#define L2_BLOCK_SIZE 0x200000
#define L3_BLOCK_SIZE PAGESIZE

//Define the number of entries in L2 table and L3 tables
#define L2_NB_ENTRY (PHYS_MEM_SIZE/L2_BLOCK_SIZE)
#define L3_NB_ENTRY (L2_BLOCK_SIZE/L3_BLOCK_SIZE)

//Table that will go in ttbr0_EL2
uint64_t L2_0[L2_NB_ENTRY];

//Table that will go in ttbr1_EL2
uint64_t L2_1[L2_NB_ENTRY];

#define START_ADDR 0x0

#endif