#ifndef HYP_MMU_SETTING_H
#define HYP_MMU_SETTING_H

#include <stdint.h>

/* define size of the physical memory seen by the hypervisor*/
#define GB 1073741824UL
#define MB 1048576UL
#define KB 1024UL

#define PHYS_MEM_SIZE (4*GB)

#define L1_GRAN (1 * GB)
#define L2_GRAN (2 * MB)
#define L3_GRAN (4 * KB)

#define NB_L1_ENTRY (PHYS_MEM_SIZE / L1_GRAN)
#define NB_L2_ENTRY (L1_GRAN / L2_GRAN)
#define NB_L3_ENTRY (L2_GRAN / L3_GRAN)

#define PAGE_SIZE (4*KB)


/* define a small padding to guarantee the tables
 (ttbr0 and ttbr1) will be separtated */
#define GAP_PADDING 8

// entrytype
/*  
    bit0 indicate if the entry is valid, 
    bit1 indicate :
        if the page is invalid on L3
        that the entry is a table addr on L1/L2
*/
#define PT_VALID_PAGE_TABLE   0b11

// Translation globality
#define PT_G   0b0          //global translation regime (region available to all processes)
#define PT_nG  0b1          //non-global translation regime (process-specific), relates to current ASID (see Global and process-specific translation table entries Armv7)

// Accessibility
#define PT_KERNEL   0b00      // privileged access only
#define PT_USER     0b01      // unprivileged access allowed

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

// inner and outer cacheability, this field represent a MAIR entry index
#define PT_MEM      0b000      // normal memory
#define PT_DEV      0b001      // device MMIO
#define PT_NC       0b010      // non-cachable

#define TTBR_CNP    1

typedef struct page_entry {
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
} __attribute__((packed)) Page_entry;


//see osdev long descr levl 1/2 (table)
typedef struct table_entry {
    uint64_t entrytype: 2;       //Type of entry (here for page should be 0b11)
    uint64_t ignored2: 10;      //Ignored bits
    uint64_t baseaddr: 28;      //Base address of a L3 table
    uint64_t zero: 12;          //SBZP = Should be zero or ignored
    uint64_t ignored: 7;         //Ignored bits
    uint64_t PXNT: 1;            //PXN limit for subsequent levels
    uint64_t XNT: 1;             //XNT XN limit for subsequent lookups
    uint64_t APT: 2;             //APT Access permissions limit for next level lookup
    uint64_t NST: 1;             //NS table, for secure memory accesses, determines type of next level, Otherwise ignored
} __attribute__((packed)) Table_entry;


void hyp_mmu_init(void);

#endif