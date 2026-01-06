#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "log.h"
#include "tar.h"

#include "hyp_mmu_setting.h"

uint64_t align_on_upperpage(uint64_t addr) {
    return (addr + 0xFFF) & ~0xFFFULL;
}

L1_entry init_entry(uint64_t addr) {

    L1_entry entry;

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
    entry.AP = 1;
    /* disable unpriviledge access, R/W for privilege access*/
    entry.NS = 0b00;
    entry.AttrIndx = PT_MEM;
    
    entry.baseaddr = ((addr >> 12) << 12);

    return entry;
}


void hyp_mmu_init(void)
{
    INFO("<hypervisor> Starting hypervisor mapping...");

    /* highest address on 4GB, reformat that to make it modular from actual PHYSICAL SPACE */
    uint64_t curr_addr = 0;

    //ttbr0 addr is starting right after the stack with a small gap
    uint64_t ttbr0_addr = align_on_upperpage(HYP_STACK_END + GAP_PADDING);
    L1_entry * curr_entry = (L1_entry *) ttbr0_addr;

    uint64_t lowerhalf_nb_entries = HYP_STACK_END / PAGE_SIZE;

    for (uint64_t i = 0; i <= lowerhalf_nb_entries; i++) {
        *curr_entry = init_entry(curr_addr);
        curr_entry++;
        curr_addr += PAGE_SIZE;
    }

    uint64_t ttbr1_addr = align_on_upperpage(HYP_STACK_END + 
        (lowerhalf_nb_entries * 8) + GAP_PADDING);
        
    curr_entry = (L1_entry *) ttbr1_addr; 

    uint64_t remaining_entries = TOTAL_NB_L1_ENTRY - lowerhalf_nb_entries;
    
    for (uint64_t i = 0; i <= remaining_entries; i++) {
        *curr_entry = init_entry(curr_addr);
        curr_entry++;
        curr_addr += PAGE_SIZE;
    }
    

    INFO("<hypervisor> Stage1 translation table populated, entries in table: ");
    putint(TOTAL_NB_L1_ENTRY);
    putc('\n');

    /* lower half */
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((uint64_t) HYP_STACK_END));

    /* higher half */
    asm volatile ("msr ttbr1_el1, %0" : : "r" ((uint64_t) ttbr1_addr));

    INFO("Stage 1 translation table system registers initialized");
    
    asm volatile (
        "mrs x9, sctlr_el1\n" //read system control register
        "orr x9, x9, #1\n"    //flip bit 0 (MMU enable)
        "msr sctlr_el1, x9\n" //write back value
        "isb"               //isb (Instruction synchronization barrier) (flush the CPU piepline and refetch instructions with now MMU enabled)
    );

    SUCCESS("MMU enabled, Stage 1 translations initialized successfully.");

    return;
}
