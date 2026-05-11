#include <stdint.h>

#include "log.h"
#include "picolibc.h"
#include "serial.h"
#include "uart.h"

#include "mmu.h"

extern uint64_t HYP_END;

/* TODO change the flags for the stage 2 profile so the VM does not explode*/
static Page_entry 
init_page_entry(uint8_t stage_level, uint64_t addr, Mapping_Flags *flags) 
{
    Page_entry entry = {0};
    entry.baseaddr = (addr >> 12) & 0xFFFFFFFFF; 
    entry.type = 0b11;
    entry.af = 1;

    switch (stage_level)
    {
        case STAGE_1:
            entry.uxn = flags->xn;
            entry.pxn = flags->pxn;
            entry.sh = flags->sh;
            entry.ap = flags->ap;
            entry.ns = flags->ns;
            entry.attr_indx = flags->attr_indx;       
            return entry;
        case STAGE_2:
            entry.ap = flags->ap;
            entry.attr_indx = flags->attr_indx;
            entry.sh = flags->sh;
            entry.uxn = flags->xn;
            entry.pxn = flags->pxn;
            return entry;
        default:
            return (Page_entry) {0};
    }
}

/* TODO change the flags for the stage 2 profile so the VM does not explode*/
static Table_entry 
init_block_entry(uint8_t stage_level, uint64_t addr, Mapping_Flags *flags) 
{
    Table_entry entry = {0};
    entry.type = 0b11;
    entry.baseaddr  = (addr >> 12) & 0xFFFFFFFFF;

    switch (stage_level)
    {
        case STAGE_1:
            /* If necessary, Change the flag here to adapt the access rights 
               for subsequents blocks if needed */
            entry.pxnt = 0;
            entry.xnt  = 0;
            entry.apt  = 0;

            /* Propagate non-secure bit for mapping */
            entry.nst  = flags->ns;

            return entry;
        case STAGE_2:
            /* For stage 2, no nstable bit */
            entry.pxnt = 0;
            entry.xnt  = 0;
            entry.apt  = 0;
            entry.nst  = 0;
            return entry;
        default:
            return (Table_entry) {0};
    }

}

/* l3_tables_create() : L3 (Page) table creation that populates entries with physical addresses and flags. */
static uint64_t 
l3_tables_create(Table_config * tc, Mapping_Flags * mf, uint64_t * l3_ts)
{
    uint64_t aligned_addr = align_on_size(*l3_ts, tc->nb_l3_entry * 8);
    Page_entry * curr_entry = (Page_entry *) aligned_addr;
    
    for (uint64_t i = 0; i < tc->nb_l3_entry; i++) {
        *curr_entry = init_page_entry(tc->stage_level, tc->curr_addr, mf);
        curr_entry++;
        tc->curr_addr += tc->page_size;
    }

    *l3_ts = align_on_size((uint64_t) curr_entry + GAP_PADDING, tc->nb_l3_entry * 8);
    return aligned_addr;
}

/* l2_table_create(): L2 (Block) table creation that links L2 entries to generated L3 tables.*/
static uint64_t 
l2_table_create(Table_config * tc, Mapping_Flags * mf, uint64_t * l2_ts, uint64_t * l3_ts)
{
    uint64_t aligned_addr = align_on_size(*l2_ts, tc->nb_l2_entry * 8);
    uint64_t l3_table_phys_addr = 0;
    Table_entry * curr_entry = (Table_entry *) aligned_addr;
    
    for (uint64_t i = 0; i < tc->nb_l2_entry; i++) {
        l3_table_phys_addr = l3_tables_create(tc, mf, l3_ts);
        *curr_entry = init_block_entry(tc->stage_level, l3_table_phys_addr, mf);
        curr_entry++;
    }
    
    *l2_ts = align_on_size((uint64_t) curr_entry + GAP_PADDING, tc->nb_l2_entry * 8);
    return aligned_addr;
}

/* l1_tables_create() : L1 (Top-level) table creation, it's the entry point for the recursive mapping process. */
static void 
l1_tables_create(Table_config * tc, Mapping_Flags * mf, uint64_t * l1_ts, uint64_t * l2_ts, uint64_t * l3_ts)
{
    uint64_t l2_table_phys_addr = 0;
    Table_entry * curr_entry = (Table_entry *) *l1_ts;

    for (uint64_t i = 0; i < tc->nb_l1_entry; i++) {
        l2_table_phys_addr = l2_table_create(tc, mf, l2_ts, l3_ts);
        *curr_entry = init_block_entry(tc->stage_level, l2_table_phys_addr, mf);
        curr_entry++;
    }
}

static uint64_t 
setup_hyp_mappings(void)
{
    INFO("<MMU> (Stage 1) Creating Mapping for hypervisor.");

    /* Hypervisor profile =========================== */

    Table_config hyp_cfg = {
        .phys_mem_size = 4 * GB,
        .l1_gran = 1 * GB,
        .l2_gran = 2 * MB,
        .l3_gran = 4 * KB,
        .nb_l1_entry = hyp_cfg.phys_mem_size / hyp_cfg.l1_gran,
        .nb_l2_entry = hyp_cfg.l1_gran / hyp_cfg.l2_gran,
        .nb_l3_entry = hyp_cfg.l2_gran / hyp_cfg.l3_gran,
        .page_size = 4 * KB,
        .curr_addr = 0,
        .stage_level = STAGE_1
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

    asm volatile ("msr ttbr0_el2, %0" : : "r" ((uint64_t) l1_ts));
    asm volatile ("isb");

    INFO("<MMU> (Stage 1) Hypervisor mapping created.");
    return l3_ts;
}

static void 
st1_trsl_setup(void)
{
    // For 4KB granule, 32-bit input (4GB), 3-level translation
    uint64_t tcr_el2 = ((uint64_t) 32 << 0) |   // T0SZ=32 (64-32=32  2^32=4GB)
                   ((uint64_t) 1 << 8)      |   // IRGN0=1 (Inner WB Cacheable)
                   ((uint64_t) 1 << 10)     |   // ORGN0=1 (Outer WB Cacheable)
                   ((uint64_t) 0b11 << 12)  |   // SH0=3 (Inner Shareable)
                   ((uint64_t) 0b00 << 14)  |   // TG0=0 (4KB granule)
                   ((uint64_t) 0b000 << 16) |   // PS flag : indicate physical space : 4GB
                   ((uint64_t) 0b1 << 23)   |   // Res1
                   ((uint64_t) 0b1 << 31);      // Res1

    INFO("<MMU> (Stage 1) Setting up TCR_EL2 with following value:");
    puthex(tcr_el2, 1);
    putc('\n');

    asm volatile("msr tcr_el2, %0" : : "r" (tcr_el2));
    asm volatile("isb");

    INFO("<MMU> (Stage 1) Setting up MAIR_EL2 with following value:");
    /* Normal inner/outer RW cacheable, write read allocate on attrindex 0 */
    uint64_t mair = 0xFFUL;
    puthex(mair, 1);
    putc('\n');

    asm volatile ("msr mair_el2, %0" : : "r" (mair));
    asm volatile ("isb");

    SUCCESS("<MMU> (Stage 1) System registers initialized");
    INFO("<MMU> Invalidating TLB...");

    /* Invalidate tlb before enabling mmu */
    asm volatile("tlbi alle2");
    asm volatile("dsb sy");
    asm volatile("isb");

    INFO("<MMU> (Stage 1) Enabling MMU...");

    uint64_t sctlr_el2_val = 0;

    asm volatile ("mrs x0, sctlr_el2" : "=r"(sctlr_el2_val));

    INFO("<MMU> (Stage 1) Flipping WXN bit to prevent translation obliteration....");
    sctlr_el2_val &= ~(1UL << 19);
    sctlr_el2_val |= 1;

    puts("sctlr_el2 before mmu enable : ");
    puthex(sctlr_el2_val, 1);
    putc('\n');

    /*  isb (Instruction synchronization barrier)
        (flush the CPU piepline and refetch instructions with now MMU enabled) 
    */
    asm volatile ("msr sctlr_el2, %0\n" : : "r" (sctlr_el2_val));
    asm volatile ("isb");
    
    SUCCESS("<MMU> (Stage 1) MMU enabled, translations initialized successfully.");
}

static void 
setup_vm_mappings(uint64_t vm_tables_loc)
{
    INFO("<MMU> (Stage 2) Creating Mapping for VM.");

    /* VM profile (stage 2) =========================== */

    Table_config vm_cfg = {
        .phys_mem_size = 2*GB,
        .l1_gran = 1 * GB,
        .l2_gran = 2 * MB,
        .l3_gran = 4 * KB,
        .nb_l1_entry = vm_cfg.phys_mem_size / vm_cfg.l1_gran,
        .nb_l2_entry = vm_cfg.l1_gran / vm_cfg.l2_gran,
        .nb_l3_entry = vm_cfg.l2_gran / vm_cfg.l3_gran,
        .page_size = 4 * KB,
        .curr_addr = 2 * GB,
        .stage_level = STAGE_2
    };

    Mapping_Flags vm_flags = {
        .attr_indx = PT_MEM,
        .ap        = PT_USER,
        .sh        = PT_ISH,
        .xn        = PT_nNX,
        .pxn       = PT_nPNX,
        .ns        = 0
    };

    /* Mapping =========================== */
    uint64_t l1_alignement = vm_cfg.nb_l1_entry * 8;

    if (vm_cfg.nb_l1_entry < 8)
        l1_alignement = 64;

    /* Hold the value of the location where the tables needs to be created */
    uint64_t l1_ts = align_on_size(vm_tables_loc + GAP_PADDING, 
                                            l1_alignement);

    uint64_t l2_ts = align_on_size( l1_ts 
        + (vm_cfg.nb_l1_entry * 8) + GAP_PADDING, 
                vm_cfg.nb_l2_entry * 8);

    uint64_t l3_ts = align_on_size(l1_ts 
        + ((vm_cfg.nb_l1_entry * 8) + (vm_cfg.nb_l1_entry * vm_cfg.nb_l2_entry * 8)) + GAP_PADDING,
        vm_cfg.nb_l3_entry*8); 

    /* TODO: Think about how to adapt the subsequent code so that it handle stage 2 translation table 
       entry format entry must follow the VTTBR format for EL1 translation regime 
    */
    l1_tables_create(&vm_cfg, &vm_flags, &l1_ts, &l2_ts, &l3_ts);

}

static void 
st2_trsl_setup(uint64_t s2_l1_addr)
{
    // T0SZ = 32 for 4GB (64 - 31 = 33)
    // SL0 = 0 (Start at Level 1)
    uint64_t vtcr_el2 = ((uint64_t) 32 << 0) |   
                        ((uint64_t) 0 << 6)  |   
                        ((uint64_t) 1 << 8)  |   
                        ((uint64_t) 1 << 10) |   
                        ((uint64_t) 3 << 12) |   
                        ((uint64_t) 0 << 14) |   
                        ((uint64_t) 0 << 16) |   
                        ((uint64_t) 1 << 31);    


    INFO("<MMU> (Stage 2) Setting up VTCR_EL2 with following value:");
    puts("vtcr_el2 before mmu enable : ");
    puthex(vtcr_el2, 1);
    putc('\n');
    asm volatile("msr vtcr_el2, %0" : : "r" (vtcr_el2));

    // Ajout d'un VMID (ex: 1) pour éviter les conflits de TLB
    uint64_t vttbr = (1ULL << 48) | (s2_l1_addr & 0xFFFFFFFFFFFFULL);
    asm volatile("msr vttbr_el2, %0" : : "r" (vttbr));
    
    asm volatile("isb");

    INFO("<MMU> (Stage 2) Configuration complete.");

}

void 
mmu_init()
{
    /* 
        Setup the hypervisor mappings, by doing so we return the starting address were the 
        VM mapping should start , that way we get a dynamic location selector for the vm mappings
        
        Note : Since it's a tiny project we did not check if starting at vm_ts, the mappings 
        would be able to fit the remaining space. If this was used as an industrial hypervisor
        that should be checked and a appropriate space should be selected.

        Also, an industrial implementation should have his own memory allocator for that whole mmu section
        that would guarantee a safe use of memory.
    */
    uint64_t vm_ts = setup_hyp_mappings();
    setup_vm_mappings(vm_ts);

    /*  Setup the translations, st1 correspond to stage 1 (hypervisor mappings TTBR0) 
        and st2 correspond to stage 2 (Linux Kernel mappings placed in VTTBR0) */
    st1_trsl_setup();
    st2_trsl_setup(vm_ts);
}
