ENTRY(_start)

OUTPUT_FORMAT(elf64-littleaarch64)
OUTPUT_ARCH(aarch64)
OUTPUT(hypervisor.bin)

FLASH_START = 0x000100000;
RAM_START = 0x40000000;
TEXT_START = 0x40080000;

SECTIONS
{
    . = FLASH_START;
    . = RAM_START;
    . = TEXT_START;
    .text : {
        *(.text*)
    }
    .rodata : {
        *(.rodata*)
    }
    .data : {
        *(.data*)
    }
    .bss : {
        *(.bss COMMON)
    }
    /* Align next tables to 0x1000 */
    . = ALIGN(0x1000);
    TTBR0_START = .;
    . += 8192*8;
    TTBR0_END = .;
    . = ALIGN(0x1000);
    TTBR1_START = .;
    . += 8192*8;
    TTBR1_END = .;
    . = ALIGN(8);

    /* Setup stack at 0x7fff0000, this will be important at the begining but later on each process will have their own stack */
    . = 0x7fff0000;
    STACK_TOP = .;
}

/*
Ou est ce que je suis chargé ? (le binaire) (Lire la datasheet de la board)
  => Soit chargé par un bootfirmware (qqun ma chargé) (Qemu toute plateforme)
  => Soit je suis le premier bout de code.

TTBRs not mandatory but usefull to get quick pagination and Classic C memory model.

TODO: Need to adapt this linker script to be adaptable to specific arch and plat choice
*/