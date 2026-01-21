#include <stdint.h>

#include "picolibc.h"
#include "uart.h"
#include "serial.h"
#include "log.h"
#include "hyp_mmu_setting.h"
#include "generic_aarch64_macros.h"
#include "tar.h"


/* 
    NOTE: All size and specific VM data locations are computed using macros
         defined in generic_aarch64_macros.h 
*/

/* 
    This routine will set the early settings for the hypervisor
    configuration register.

    - blocking stage2 translations (VM bit = 0)
    - specifying el2 host as hypervisor (E2H bit = 0)
    - setting SWIO at 0 because can be Res1 according to doc (SWIO bit = 1)
    - specify that lowers EL will be Aarch64 (RW bit = 1)

    later on this register will be updated to enable different trapping mechanisms
    
*/
static 
uint64_t early_set_hcr_el2(void)
{
    uint64_t hcr_el2_val = 0;

    hcr_el2_val = ((uint64_t) 0 << 34) | //E2H = 0
              ((uint64_t) 1 << 31) | //RW = 1
              ((uint64_t) 1 << 1);   // SWIO = 1
    // VM = 0 (by default when hcr_el2 is init at 0)

    asm volatile ("msr hcr_el2, %0" : : "r" (hcr_el2_val));
    
    return hcr_el2_val;
}

static
void setup_vm_bootstrap_data(void)
{
    uint64_t unpacked_data_size = 0;

    INFO("<VM BOOTSTRAP> Unpacking data from VM bootstrap tar");

    if (detect_tar_at(BOOTSTRAP_TAR_LOC)) {
        
        INFO("<VM BOOTSTRAP> unpacking VM image");
        unpacked_data_size += extract_tarfile_to(BOOTSTRAP_TAR_LOC, 
            align_on_size(VM_DEST, 4*KB), 
            (uint8_t *) "Image");
        
        INFO("<VM BOOTSTRAP> unpacking VM dtb");
        unpacked_data_size += extract_tarfile_to(BOOTSTRAP_TAR_LOC, 
            align_on_size(VM_DEST + unpacked_data_size, 4*KB),
            (uint8_t *) "VM.dtb");
    }

    puts("Total data unpacked from VM bootstrap tar : ");
    putint(unpacked_data_size);
    puts(".\n");
}

static 
uint64_t get_current_el(void)
{
    register uint64_t current_el;
    asm volatile ("mrs x0, CurrentEL" : "=r" (current_el));
    return current_el;
}

/* bootstrap the hypervisor by setting up a proper early environment */
void 
bootstrap_main(void) 
{
    INFO("<BOOTSTRAP> HYPERVISOR BOOTSTRAPING");
    puts("Current exception level : ");
    putint(get_current_el() >> 2);
    putc('\n');
    
    INFO("Setting up early HCR_EL2");
    early_set_hcr_el2();


    /* Init the tables */
    hyp_mmu_init();

    setup_vm_bootstrap_data();

    /* 3rd and next : implem dtb library to create dtb for VM */

    SUCCESS("<BOOTSTRAP> BOOTSTRAPING COMPLETE");
}