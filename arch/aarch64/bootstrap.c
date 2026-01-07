#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "log.h"

#include "hyp_mmu_setting.h"

void bootstrap_main(void) 
{
    INFO("<BOOTSTRAP> HYPERVISOR BOOTSTRAP");
    puts("Current exception level : ");

    register uint64_t current_el;
    asm volatile ("mrs x0, CurrentEL" : "=r" (current_el));
    putint(current_el >> 2);
    putc('\n');

    hyp_mmu_init();
}