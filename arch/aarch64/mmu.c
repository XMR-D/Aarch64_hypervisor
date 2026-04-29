#include <stdint.h>

#include "bootstrap.h"
#include "log.h"
#include "picolibc.h"
#include "serial.h"
#include "uart.h"

#include "mmu.h"

extern uint64_t HYP_END;

/* global that represent the current physical address being mapped */
static uint64_t curr_addr = 0;

static Page_entry init_page_entry(uint64_t addr, Mapping_Flags *flags) {
    Page_entry entry;

    // Champs par défaut ou ignorés
    entry.Ignored = 0;
    entry.SoftwareUsed = 0;
    entry.ContHint = PT_nHINT;
    entry.Ignored2 = 0;
    entry.entrytype = PT_VALID_PAGE_TABLE;
    entry.nG = PT_G;
    entry.AF = PT_AF;

    // Utilisation de la structure de flags
    entry.XN       = flags->xn;
    entry.PXN      = flags->pxn;
    entry.SH       = flags->sh;
    entry.AP       = flags->ap;
    entry.NS       = flags->ns;
    entry.AttrIndx = flags->attr_indx;

    // Adresse physique (PA)
    entry.baseaddr = (addr >> 12);

    return entry;
}

static Table_entry init_block_entry(uint64_t addr, Mapping_Flags *flags) {
    Table_entry entry;

    entry.entrytype = PT_VALID_PAGE_TABLE;
    entry.baseaddr  = (addr >> 12);
    entry.ignored2  = 0;
    entry.zero      = 0;
    entry.ignored   = 0;

    /* On peut choisir de passer des flags hiérarchiques ici si nécessaire */
    /* Pour un mapping standard, on les laisse souvent à 0 */
    entry.PXNT = 0;
    entry.XNT  = 0;
    entry.APT  = 0;
    entry.NST  = flags->ns; // On propage souvent le bit non-secure

    return entry;
}

/**
 * L3 (Page) table creation: populates entries with physical addresses and flags.
 */
static uint64_t l3_tables_create(Table_config * tc, Mapping_Flags * mf, uint64_t * l3_ts)
{
    uint64_t aligned_addr = align_on_size(*l3_ts, tc->nb_l3_entry * 8);
    Page_entry * curr_entry = (Page_entry *) aligned_addr;
    
    for (uint64_t i = 0; i < tc->nb_l3_entry; i++) {
        *curr_entry = init_page_entry(curr_addr, mf);
        curr_entry++;
        curr_addr += tc->page_size;
    }

    *l3_ts = align_on_size((uint64_t) curr_entry + GAP_PADDING, tc->nb_l3_entry * 8);
    return aligned_addr;
}

/**
 * L2 (Block) table creation: links L2 entries to generated L3 tables.
 */
static uint64_t l2_table_create(Table_config * tc, Mapping_Flags * mf, uint64_t * l2_ts, uint64_t * l3_ts)
{
    uint64_t aligned_addr = align_on_size(*l2_ts, tc->nb_l2_entry * 8);
    uint64_t l3_table_phys_addr = 0;
    Table_entry * curr_entry = (Table_entry *) aligned_addr;
    
    for (uint64_t i = 0; i < tc->nb_l2_entry; i++) {
        l3_table_phys_addr = l3_tables_create(tc, mf, l3_ts);
        *curr_entry = init_block_entry(l3_table_phys_addr, mf);
        curr_entry++;
    }
    
    *l2_ts = align_on_size((uint64_t) curr_entry + GAP_PADDING, tc->nb_l2_entry * 8);
    return aligned_addr;
}

/**
 * L1 (Top-level) table creation: entry point for the recursive mapping process.
 */
static void l1_tables_create(Table_config * tc, Mapping_Flags * mf, uint64_t * l1_ts, uint64_t * l2_ts, uint64_t * l3_ts)
{
    uint64_t l2_table_phys_addr = 0;
    Table_entry * curr_entry = (Table_entry *) *l1_ts;

    for (uint64_t i = 0; i < tc->nb_l1_entry; i++) {
        l2_table_phys_addr = l2_table_create(tc, mf, l2_ts, l3_ts);
        *curr_entry = init_block_entry(l2_table_phys_addr, mf);
        curr_entry++;
    }
}

static void setup_hyp_mappings(void)
{
    INFO("<MMU> Creating Mapping for hypervisor. (Stage 1)");

    /* Hypervisor profile =========================== */

    Table_config hyp_cfg = {
        .phys_mem_size = 4*GB,
        .l1_gran = 1 * GB,
        .l2_gran = 2 * MB,
        .l3_gran = 4 * KB,
        .nb_l1_entry = hyp_cfg.phys_mem_size / hyp_cfg.l1_gran,
        .nb_l2_entry = hyp_cfg.l1_gran / hyp_cfg.l2_gran,
        .nb_l3_entry = hyp_cfg.l2_gran / hyp_cfg.l3_gran,
        .page_size = 4 * KB
    };

    Mapping_Flags hyp_flags = {
        .attr_indx = PT_MEM,
        .ap        = PT_USER,
        .sh        = PT_ISH,
        .xn        = PT_nNX,
        .pxn       = PT_nPNX,
        .ns        = 0
    };

    /* Mapping =========================== */

    uint64_t l1_alignement = hyp_cfg.nb_l1_entry * 8;

    if (hyp_cfg.nb_l1_entry < 8)
        l1_alignement = 64;

    /* Hold the value of the location where the tables needs to be created */
    uint64_t l1_ts = align_on_size(((uint64_t) &HYP_END) + GAP_PADDING, 
                                            l1_alignement);
    uint64_t l2_ts = align_on_size(
                l1_ts + (hyp_cfg.nb_l1_entry * 8) + GAP_PADDING, 
                hyp_cfg.nb_l2_entry * 8);

    uint64_t l3_ts = align_on_size(l1_ts 
        + ((hyp_cfg.nb_l1_entry * 8) + (hyp_cfg.nb_l1_entry * hyp_cfg.nb_l2_entry * 8)) + GAP_PADDING,
        hyp_cfg.nb_l3_entry*8); 

    l1_tables_create(&hyp_cfg, &hyp_flags, &l1_ts, &l2_ts, &l3_ts);

    INFO("<MMU> Setting up TTBR0_EL2 with following value:");
    puthex(l1_ts, 1);
    putc('\n');

    asm volatile ("msr ttbr0_el2, %0" : : "r" ((uint64_t) l1_ts));
    asm volatile ("isb");

    INFO("<MMU> Hypervisor mapping created.");
}

static void setup_vm_mappings(VmInfos *vm_data)
{
    puts("VMDATA\n");
    puthex(vm_data->vm_image_off, 1);
    putc('\n');
    puthex(vm_data->vm_image_size, 0);
    putc('\n');
    puthex(vm_data->vm_dtb_off, 1);
    putc('\n');
    puthex(vm_data->vm_dtb_size, 0);
    putc('\n');
    puts("END OF DUMP\n");
}

static void st1_trsl_setup(void)
{
    // For 4KB granule, 32-bit input (4GB), 3-level translation
    uint64_t tcr_el2 = ((uint64_t) 32 << 0) |   // T0SZ=32 (64-32=32  2^32=4GB)
                   ((uint64_t) 1 << 8) |        // IRGN0=1 (Inner WB Cacheable)
                   ((uint64_t) 1 << 10) |       // ORGN0=1 (Outer WB Cacheable)
                   ((uint64_t) 0b11 << 12) |    // SH0=3 (Inner Shareable)
                   ((uint64_t) 0b00 << 14) |    // TG0=0 (4KB granule)
                   ((uint64_t) 0b000 << 16) |   // PS flag : indicate physical space : 4GB
                   ((uint64_t) 0b1 << 23) |     // Res1
                   ((uint64_t) 0b1 << 31) ;     // Res1

    INFO("<MMU> Setting up TCR_EL2 with following value:");
    puthex(tcr_el2, 1);
    putc('\n');
    
    asm volatile("msr tcr_el2, %0" : : "r" (tcr_el2));
    asm volatile("isb");

    INFO("<MMU> Setting up MAIR_EL2 with following value:");
    //Normal inner/outer RW cacheable, write read allocate on attrindex 0
    uint64_t mair = 0xFFUL;
    puthex(mair, 1);
    putc('\n');

    asm volatile ("msr mair_el2, %0" : : "r" (mair));
    asm volatile ("isb");

    SUCCESS("<MMU> Stage 1 system registers initialized");
    INFO("<MMU> Invalidating TLB...");

    /* Invalidate tlb before enabling mmu */
    asm volatile("tlbi alle2");
    asm volatile("dsb sy");
    asm volatile("isb");

    INFO("<MMU> Enabling MMU...");

    uint64_t sctlr_el2_val = 0;

    asm volatile ("mrs x0, sctlr_el2" : "=r"(sctlr_el2_val));

    INFO("<MMU> Flipping WXN bit to prevent translation obliteration....");
    sctlr_el2_val &= ~(1UL << 19);
    sctlr_el2_val |= 1;

    puts("sctlr_el2 before mmu enable : ");
    puthex(sctlr_el2_val, 1);
    putc('\n');

    asm volatile ("msr sctlr_el2, %0\n" : : "r" (sctlr_el2_val));
    asm volatile ("isb"); /* isb (Instruction synchronization barrier) */
    /* (flush the CPU piepline and refetch instructions with now MMU enabled) */
    
    SUCCESS("<MMU> MMU enabled, Stage 1 translations initialized successfully.");

}

void mmu_init(VmInfos *vm_data)
{

    setup_hyp_mappings();
    setup_vm_mappings(vm_data);
    st1_trsl_setup();
    
    return;
}
