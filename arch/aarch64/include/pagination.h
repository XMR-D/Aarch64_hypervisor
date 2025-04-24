#ifndef PAGINATION_H

#include <stdint.h>

extern volatile uint64_t STACK_END;
#define PHYS_MEM_SIZE 0x80000000

/*From ARM developper guide on virtual memory management and OSdev*/
#define PAGESIZE 4096

//L2 block size = 2MB | L3 block size = 4KB
#define L2_BLOCK_SIZE 0x200000
#define L3_BLOCK_SIZE PAGESIZE

//Define the number of entries in L2 table and L3 tables
#define L2_NB_ENTRY ((PHYS_MEM_SIZE/2)/L2_BLOCK_SIZE)
#define L3_NB_ENTRY (L2_BLOCK_SIZE/L3_BLOCK_SIZE)



// granularity
#define PT_PAGE     0b11        // 4k granule
#define PT_BLOCK    0b01
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




uint64_t block_attributes = PT_BLOCK | PT_AF | PT_KERNEL | PT_ISH | PT_MEM | PT_RW | PT_RO;
uint64_t table_attributes = PT_PAGE | PT_AF | PT_KERNEL | PT_ISH | PT_MEM;

typedef struct l3entry {
    uint16_t UPAT: 12;           //Upper page attributes
    uint16_t ignored: 12;        //SBZP: Should be zero or preserved
    uint32_t baseaddr: 28;       //Base adress of a physical block
    uint16_t LPAT: 10;           //Lower page attributes
    uint8_t entrytype: 2;        //Type of entry (here for block should be 0b11)
} L3entry;

//see osdev long descr levl 1/2 (table)
typedef struct l2entry {
    uint8_t NST: 1;             //NS table, for secure memory accesses, determines type of next level, Otherwise ignored
    uint8_t APT: 2;             //APT Access permissions limit for next level lookup
    uint8_t XNT: 1;             //XNT XN limit for subsequent lookups
    uint8_t PXNT: 1;            //PXN limit for subsequent levels
    uint8_t ignored: 7;         //Ignored bits
    uint16_t zero: 12;          //SBZP = Should be zero or ignored
    uint32_t baseaddr: 28;      //Base address of a L3 table
    uint16_t ignored2: 10;      //Ignored bits
    uint8_t entrytype: 2;       //Type of entry (here for page should be 0b11)
} L2entry;

//Table that will go in ttbr0_EL2
L2entry L2_high[L2_NB_ENTRY];
//Table that will go in ttbr1_EL2
L2entry L2_low[L2_NB_ENTRY];


#endif