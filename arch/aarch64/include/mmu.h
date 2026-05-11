#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#include "generic_aarch64_macros.h"

/* 
    define a small padding to guarantee the tables
    (ttbr0 and ttbr1) will be separtated. 
*/
#define GAP_PADDING 8

/* 
    define the magic for the Translation stage selector used in
    init_page_entry and init_block_entry 
*/
#define STAGE_1 0
#define STAGE_2 1

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



typedef union {
    struct {
        uint64_t type          : 2;   // [1:0]   0b01 pour Block (L1-L2), 0b11 pour Page (L3)
        uint64_t attr_indx     : 3;   // [4:2]   Index vers registre MAIR_ELx
        uint64_t ns            : 1;   // [5]     Non-Secure bit
        uint64_t ap            : 2;   // [7:6]   Access Permissions (EL0/EL1)
        uint64_t sh            : 2;   // [9:8]   Shareability
        uint64_t af            : 1;   // [10]    Access Flag
        uint64_t ng            : 1;   // [11]    not-Global (utilisé pour l'ASID)
        uint64_t baseaddr      : 36;  // [47:12] Output Address (OA)
        uint64_t reserved      : 4;   // [51:48] RES0 (sauf si FEAT_LPA2 est actif)
        uint64_t cont          : 1;   // [52]    Contiguous hint
        uint64_t pxn           : 1;   // [53]    Privileged Execute-Never
        uint64_t uxn           : 1;   // [54]    User Execute-Never (XN)
        uint64_t software      : 4;   // [58:55] Ignored by hardware
        uint64_t pbha          : 4;   // [62:59] Page-Based Hardware Attributes
        uint64_t xnx           : 1;   // [63]    Extended Execute-Never
    } __attribute__((packed));
    uint64_t raw;
} Page_entry;

typedef union {
    struct {
        uint64_t type          : 2;   // [1:0]   0b11 pour un Table Descriptor
        uint64_t ignored       : 10;  // [11:2]  Ignored
        uint64_t baseaddr      : 36;  // [47:12] Adresse physique de la table L(n+1)
        uint64_t reserved      : 11;  // [58:48] RES0
        uint64_t pxnt          : 1;   // [59]    Limite PXN pour les niveaux suivants
        uint64_t xnt           : 1;   // [60]    Limite XN pour les niveaux suivants
        uint64_t apt           : 2;   // [62:61] Limite Permissions pour les niveaux suivants
        uint64_t nst           : 1;   // [63]    Sécurité pour les niveaux suivants
    } __attribute__((packed));
    uint64_t raw;
} Table_entry;

typedef struct table_config {
    uint64_t phys_mem_size;
    uint64_t l1_gran;
    uint64_t l2_gran;
    uint64_t l3_gran;

    uint64_t nb_l1_entry;
    uint64_t nb_l2_entry;    
    uint64_t nb_l3_entry;

    uint64_t page_size;
    uint64_t curr_addr;

    uint8_t stage_level;

} Table_config;

typedef struct {
    uint8_t  attr_indx; // Index dans le MAIR (PT_MEM, PT_DEV...)
    uint8_t  ap;        // Permissions d'accès (PT_USER, PT_PRIV...)
    uint8_t  sh;        // Partageabilité (PT_ISH, PT_OSH...)
    uint8_t  xn;        // Execute-Never (PT_nNX, PT_NX...)
    uint8_t  pxn;       // Privileged Execute-Never
    uint8_t  ns;        // Non-Secure bit
} Mapping_Flags;


void mmu_init();

#endif