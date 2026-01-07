ENTRY(_start)

OUTPUT_FORMAT(elf64-littleaarch64)
OUTPUT_ARCH(aarch64)
OUTPUT(hypervisor.bin)

FLASH_START = 0x000100000;
RAM_START = 0x40000000;
TEXT_START = 0x40080000;

/* Note about systems dev :

Upon pagination activation swap TEXT_START address to higherhalf (at 0xffffffff)

Whos is loading the binary I'm currently doing ? (Read the data sheet or whatever info of the env) 
    => loaded by a bootfirmware (qqun ma chargé) (Qemu all plat)
    => the binary is the first to be loaded nothing before.

TTBRs not mandatory but usefull to get quick pagination and Classic C memory model.
    => allows us to do quick pagination and higherhalf remap before jumping into logic => this is much needeed

*/

bin_start = 0;
bin_end = 0;

SECTIONS
{
    . = FLASH_START;
    . = RAM_START;
    . = TEXT_START;

    . = bin_start; /* binary start before text section */

    .text : {
        *(.text*)
    }
    .rodata : {
        *(.rodata*)
    }
    .data : {
        *(.data*)
    }

    . = bin_end; /* binary end before the bss section, bss is full of zeroes not need to pack it */

    .bss : {
        *(.bss COMMON)
    }
    
    /* Setup stack at 0x7fff0000, this will be important at the begining but later 
    on each process will have their own stack, this represent the hypervisor stack */
    . = 0x7fff0000;
    STACK_TOP = .;
    . += 8M;
    STACK_END = .;
}

HYP_END = STACK_END;