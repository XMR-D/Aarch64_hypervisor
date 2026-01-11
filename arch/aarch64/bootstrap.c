#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "log.h"


#include "tar.h"
#include "hyp_mmu_setting.h"

/* 
    This routine will set the early settings for the hypervisor
    configuration register.

    - blocking stage2 translations (VM bit = 0)
    - specifying el2 host as hypervisor (E2H bit = 0)
    - setting SWIO at 0 because can be Res1 according to doc (SWIO bit = 1)
    - specify that lowers EL will be Aarch64 (RW bit = 1)

    later on this register will be updated to enable different trapping mechanisms
    
*/
static uint64_t early_set_hcr_el2(void)
{
    uint64_t hcr_el2_val = 0;

    hcr_el2_val = ((uint64_t) 0 << 34) | //E2H = 0
              ((uint64_t) 1 << 31) | //RW = 1
              ((uint64_t) 1 << 1);   // SWIO = 1
    // VM = 0 (by default when hcr_el2 is init at 0)

    asm volatile ("msr hcr_el2, %0" : : "r" (hcr_el2_val));
    
    return hcr_el2_val;
}

static uint64_t get_current_el(void)
{
    register uint64_t current_el;
    asm volatile ("mrs x0, CurrentEL" : "=r" (current_el));
    return current_el;
}

/* bootstrap the hypervisor by setting up a proper early environment */
void bootstrap_main(void) 
{
    INFO("<BOOTSTRAP> HYPERVISOR BOOTSTRAPING");
    puts("Current exception level : ");

    putint(get_current_el() >> 2);
    putc('\n');

    
    INFO("Setting up HCR_EL2 with folowing value:");
    puthex(early_set_hcr_el2(), 1);
    putc('\n');

    hyp_mmu_init();

    /* 
        TODO: CALL tar_extract_file_to with proper args to :
        1st : debug if tar is correctly detected
        2nd : check in memory if VM_IMAGE is correctly copied
    */

    /*
        3rd and next : implem dtb library to create dtb for VM
    */

    SUCCESS("<BOOTSTRAP> BOOTSTRAPING COMPLETE");
}