.section ".text"

.global bootstrap

//Setup ttbr0_el2 and ttbr1_el2
//      set TCR_EL2.T2SZ to spare some memory space
//remap EL2 code to higher half (à l'addresse 0xFFFF_FFFF)
//activer la pagination
//return to _start logic
bootstrap:
    //Create paginations tables
    stp x29, x30, [sp, #-16]!
    mov x29, sp

    bl hyp_mmu_init
    //Here call other methods to init modules if needed before accessing the hmain
    //bl hyp_remap
    ldp x29, x30, [sp], #16
    ret
