#ifndef HYP_MMU_SETTING_H
#define HYP_MMU_SETTING_H

#include <stdint.h>

/* define size of the physical memory seen by the hypervisor*/
#define GB 1073741824UL
#define PHYS_MEM_SIZE 4*GB
#define PAGE_SIZE 4096
#define TOTAL_NB_L1_ENTRY PHYS_MEM_SIZE / PAGE_SIZE

/* define a small padding to guarantee the tables
 (ttbr0 and ttbr1) will be separtated */
#define GAP_PADDING 8

extern uint64_t HYP_STACK_END;

// Granularity
#define PT_TABLE    0b11        // 4k granule
#define PT_BLOCK    0b01

// Translation globality
#define PT_G   0b0          //global translation regime (region available to all processes)
#define PT_nG  0b1          //non-global translation regime (process-specific), relates to current ASID (see Global and process-specific translation table entries Armv7)

// Accessibility
#define PT_KERNEL   0b00      // privileged, access from EL1 only
#define PT_USER     0b01      // unprivileged, access from EL0 allowed

#define PT_RW       0b0      // read-write
#define PT_RO       0b1      // read-only

#define PT_AF       0b1      // accessibility flag (indicate first access since last set to 0 (should be init to 1))

#define PT_NX       0b1      // no execute (code that exec outside PL1 will generate a permission fault if set)
#define PT_nNX      0b0

#define PT_PNX      0b1      // No privilege execute (code that exec at PL1 will generate a permission fault if set)
#define PT_nPNX     0b0

//Contiguous hint
#define PT_HINT     0b1     //Contiguous hint, if set to 1 indicate that 16 adjacent translation table entries point to a contiguous address range.
#define PT_nHINT    0b0

// Shareability
#define PT_OSH      0b10      // outter shareable
#define PT_ISH      0b11      // inner shareable

// Security
#define PT_SEC      0b0       //The address related to, will be in secure physicall address space
#define PT_nSEC     0b1       //The address related to, will NOT be in secure physicall address space 

// defined in MAIR register
#define PT_MEM      0b00      // normal memory
#define PT_DEV      0b01      // device MMIO
#define PT_NC       0b10      // non-cachable

#define TTBR_CNP    1

typedef struct l1_entry {
    uint64_t entrytype: 2;       //Type of entry (here for block should be 0b11)
    uint64_t AttrIndx: 3;
    uint64_t NS: 1; 
    uint64_t AP: 2; 
    uint64_t SH: 2; 
    uint64_t AF: 1; 
    uint64_t nG: 1; 
    uint64_t baseaddr: 28;       //Base adress of a physical block
    uint64_t Ignored2: 12;       //SBZP: Should be zero or preserved
    uint64_t ContHint: 1;
    uint64_t PXN: 1;
    uint64_t XN: 1;
    uint64_t SoftwareUsed: 4;
    uint64_t Ignored: 5;
} __attribute__((packed)) L1_entry;

/*
//see osdev long descr levl 1/2 (table)
typedef struct l2entry {
    uint64_t entrytype: 2;       //Type of entry (here for page should be 0b11)
    uint64_t ignored2: 10;      //Ignored bits
    uint64_t baseaddr: 28;      //Base address of a L3 table
    uint64_t zero: 12;          //SBZP = Should be zero or ignored
    uint64_t ignored: 7;         //Ignored bits
    uint64_t PXNT: 1;            //PXN limit for subsequent levels
    uint64_t XNT: 1;             //XNT XN limit for subsequent lookups
    uint64_t APT: 2;             //APT Access permissions limit for next level lookup
    uint64_t NST: 1;             //NS table, for secure memory accesses, determines type of next level, Otherwise ignored
} __attribute__((packed)) L2entry;


//see osdev long descr levl 1/2 (table)
typedef struct l1entry {
    uint64_t entrytype: 2;       //Type of entry (here for page should be 0b11)
    uint64_t ignored2: 10;      //Ignored bits
    uint64_t baseaddr: 28;      //Base address of a L3 table
    uint64_t zero: 12;          //SBZP = Should be zero or ignored
    uint64_t ignored: 7;         //Ignored bits
    uint64_t PXNT: 1;            //PXN limit for subsequent levels
    uint64_t XNT: 1;             //XNT XN limit for subsequent lookups
    uint64_t APT: 2;             //APT Access permissions limit for next level lookup
    uint64_t NST: 1;             //NS table, for secure memory accesses, determines type of next level, Otherwise ignored
} __attribute__((packed)) L1entry;
*/

#endif