#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "log.h"
#include "tar.h"

#include "hyp_mmu_setting.h"

extern uint64_t HYP_END;

/* global that represent the current physicall address being mapped */
uint64_t curr_addr = 0;

uint64_t l1_table_start = 0;
uint64_t l2_tables_start = 0;
uint64_t l3_tables_start = 0;


// Align to 'size' (must be power of 2)
uint64_t align_on_tablesize(uint64_t addr, uint64_t size) {
    uint64_t mask = size - 1;
    return (addr + mask) & ~mask;
}

static Page_entry init_page_entry(uint64_t addr) {

    Page_entry entry;

    entry.Ignored = 0;
    entry.SoftwareUsed = 0;
    entry.XN = PT_NX;
    entry.PXN = PT_nPNX;
    entry.ContHint = PT_nHINT;
    entry.Ignored2 = 0b0;

    //Setup lower attributes
    entry.nG = PT_G;
    entry.AF = PT_AF;
    entry.SH = PT_ISH;

    /* initial mapping consider all page accessible (opre device mapping)*/
    entry.AP = 0b01;
    /* disable unpriviledge access, R/W for privilege access*/
    entry.NS = 0b0;
    entry.AttrIndx = PT_MEM;
    entry.entrytype = PT_VALID_PAGE_TABLE;
    entry.baseaddr = (addr >> 12);
    return entry;
}

static Table_entry init_block_entry(uint64_t addr) {

    Table_entry entry;

    entry.entrytype = PT_VALID_PAGE_TABLE;
    entry.ignored2 = 0;
    entry.baseaddr = (addr >> 12);
    entry.zero = 0;
    entry.ignored = 0;

    /* will be overriden by smaller levels*/
    entry.PXNT = 0;
    entry.XNT = 0;
    entry.APT = 0;
    entry.NST = 0;

    return entry;
}

/* 
   Create the l3 table for one l2 entry 
   the new l3 table is created at addr

   addr here is already aligned to the table size
*/
static uint64_t hyp_l3_tables_create(void)
{
    //ttbr0 addr is starting right after the stack with a small gap
    uint64_t aligned_addr = align_on_tablesize(l3_tables_start, NB_L3_ENTRY*8);
    Page_entry * curr_entry = (Page_entry *) aligned_addr;
    
    for (uint64_t i = 0; i < NB_L3_ENTRY; i++) {
        *curr_entry = init_page_entry(curr_addr);
        curr_entry++;
        curr_addr += PAGE_SIZE;
    }

    l3_tables_start = align_on_tablesize((uint64_t) curr_entry + GAP_PADDING, NB_L3_ENTRY*8);

    return aligned_addr;
}

/* 
   Create the l2 table for one l1 entry 
   the new l2 table is created at addr
*/
static uint64_t hyp_l2_table_create(void)
{
    //ttbr0 addr is starting right after the stack with a small gap
    uint64_t aligned_addr = align_on_tablesize(l2_tables_start, NB_L2_ENTRY*8);

    uint64_t l3_table_off = 0;
    Table_entry * curr_entry = (Table_entry *) aligned_addr;
    
    for (uint64_t i = 0; i < NB_L2_ENTRY; i++) {
        l3_table_off = hyp_l3_tables_create();
        *curr_entry = init_block_entry(l3_table_off);
        curr_entry++;
    }
    
    l2_tables_start = align_on_tablesize((uint64_t) curr_entry + GAP_PADDING, NB_L2_ENTRY*8);

    return aligned_addr;
}

/* 
   Create the l1 table 
   the new l1 table is created at addr
*/
static void hyp_l1_tables_create(void)
{

    /* 
        Every tables needs to be aligned to their size but if the size is smaller than
        64 bytes, then it must be aligned to 64 
    */
    uint64_t l1_alignement = NB_L1_ENTRY*8;
    if (NB_L1_ENTRY < 8)
        l1_alignement = 64;

    /* Compute the starting point of each sets of tables */
    l1_table_start = align_on_tablesize(((uint64_t) &HYP_END) + GAP_PADDING, 
                                            l1_alignement);

    l2_tables_start = align_on_tablesize(l1_table_start + 
                (NB_L1_ENTRY * 8) + GAP_PADDING, NB_L2_ENTRY*8);

    l3_tables_start = align_on_tablesize(l1_table_start + 
                ((NB_L1_ENTRY * 8) + (NB_L1_ENTRY * NB_L2_ENTRY * 8)) + GAP_PADDING, NB_L3_ENTRY*8); 


    uint64_t l2_table_off = 0;
    Table_entry * curr_entry = (Table_entry *) l1_table_start;

    /* for each entry in the L1 table create a l2 table and place it in the entry*/
    for (uint64_t i = 0; i < NB_L1_ENTRY; i++) {
           
        /*
            puts("creating a L1 entry at : ");
            puthex((uint64_t) curr_entry, 0);
            putc('\n');
        */

        l2_table_off = hyp_l2_table_create();
        *curr_entry = init_block_entry(l2_table_off);
        curr_entry++;
    }

    /* DO NOT UPDATE l1_table_start as it will be used to be placed within TTBR0_EL2 */
}

void hyp_mmu_init(void)
{
    INFO("<MMU> Starting hypervisor mapping...");
    
    hyp_l1_tables_create();
    
    INFO("<MMU> Stage1 translation tables populated");

    puts("entries in L1: ");
    putint(NB_L1_ENTRY);
    putc('\n');

    puts("entries in L2: ");
    putint(NB_L2_ENTRY);
    putc('\n');

    puts("entries in L3: ");
    putint(NB_L3_ENTRY);
    putc('\n');

    puts("ttbr0_el2 = ");
    puthex((uint64_t) l1_table_start, 0);
    putc('\n');

    INFO("<MMU> Setting up HCR_EL2...");

    // Disable Stage 2 translation (VM=0) for now
    uint64_t hcr_el2_zeroed = 0x2;
    asm volatile ("msr hcr_el2, %0" : : "r" (hcr_el2_zeroed));
    asm volatile ("isb");

    INFO("<MMU> Setting up TCR_EL2...");

    // For 4KB granule, 32-bit input (4GB), 3-level translation
    uint64_t tcr_el2 = (32 << 0) |   // T0SZ=25 (64-32=32 bit input = 4GB)
                   (0 << 6) |    // EPD0=0 (for tlb translation use ttbr0)
                   (1 << 8) |    // IRGN0=1 (Inner WB Cacheable)
                   (1 << 10) |   // ORGN0=1 (Outer WB Cacheable)
                   (3 << 12) |   // SH0=3 (Inner Shareable)
                   (0 << 14);    // TG0=0 (4KB granule)

    asm volatile("msr tcr_el2, %0" : : "r"(tcr_el2));
    asm volatile("isb");

    INFO("<MMU> Setting up MAIR_EL2...");
    uint64_t mair = (0xBBUL << 0) |    //Normal inner/outer RW cacheable, write-through
                    (0x04UL << 8);     //Device nGnRnE (no gathering, no reordering, no early write acknowledgement)
    
    asm volatile ("msr mair_el2, %0" : : "r" (mair));
    asm volatile ("isb");

    INFO("<MMU> Setting up TTBR0_EL2...");

    asm volatile ("msr ttbr0_el2, %0" : : "r" ((uint64_t) l1_table_start));
    asm volatile ("isb");

    SUCCESS("<MMU> Stage 1 system registers initialized");
    INFO("<MMU> Invalidating TLB...");

    /* Invalidate tlb before enabling mmu */
    asm volatile("tlbi alle2");
    asm volatile("dsb sy");
    asm volatile("isb");

    INFO("<MMU> Enabling MMU...");
    asm volatile (
        "mrs x9, sctlr_el2\n" //read system control register
        "orr x9, x9, #1\n"    //flip bit 0 (MMU² enable)
        "msr sctlr_el2, x9\n" //write back value
        "isb"               //isb (Instruction synchronization barrier) (flush the CPU piepline and refetch instructions with now MMU enabled)
    );
    
    SUCCESS("<MMU> MMU enabled, Stage 1 translations initialized successfully.");

    return;
}
