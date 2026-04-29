#include <stdint.h>

#include "generic_aarch64_macros.h"
#include "mmu.h"
#include "log.h"
#include "picolibc.h"
#include "serial.h"
#include "tar.h"
#include "uart.h"

#include "bootstrap.h"

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
VmInfos setup_vm_bootstrap_data(void)
{
    VmInfos ret;

    INFO("<VM BOOTSTRAP> Unpacking data from VM bootstrap tar");

    if (detect_tar_at(BOOTSTRAP_TAR_LOC)) {
        
        INFO("<VM BOOTSTRAP> unpacking VM image");
        ret.vm_image_off = align_on_size(VM_DEST, 4*KB);
        ret.vm_image_size = extract_tarfile_to(BOOTSTRAP_TAR_LOC, 
            ret.vm_image_off, 
            (uint8_t *) "Image");


        INFO("<VM BOOTSTRAP> unpacking VM dtb");
        ret.vm_dtb_off = align_on_size(VM_DEST + ret.vm_image_size, 4*KB);
        ret.vm_dtb_size = extract_tarfile_to(BOOTSTRAP_TAR_LOC, 
            ret.vm_dtb_off,
            (uint8_t *) "VM.dtb");
    }

    return ret;
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

    VmInfos vm_data = setup_vm_bootstrap_data();
    mmu_init(&vm_data);

    SUCCESS("<BOOTSTRAP> BOOTSTRAPING COMPLETE");
}