.section ".text"

#include "TTcnst.h"

.global bootstrap

L0:
    .zero 8

L1:
    .zero 4096

L2:
    .zero 4096

//loop through L2 512 times to fill appropriates addresses
//loop through L3 512 times to fill appropriates addresses
//init i in x9 and page offset in x15

Setup_pages:
    //setup L1 before setting up other L1 is a one entry table
    add x9, xzr, xzr
    add x15, xzr, xzr
loop:
    subs xzr, x9, #512
    b.gt exit
    //do the work here to setup L2 and L3 => research on which values goes there
    
    add x9, x9, #1

exit:
    ret

//Setup ttbr0_el2 et ttbr1_el2
//      set TCR_EL2.T2SZ afin d'éco de la place mémoire (détail: Aarch64 memory management guide | Size of virtual addresses) 
//remap le code en higher half (à l'addresse 0xFFFF_FFFF_FFFF_FFFF)
//activer la pagination
//jump to _entry
bootstrap:
    bl Setup_pages
    
