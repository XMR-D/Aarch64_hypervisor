.section ".text"

.global bootstrap

//Setup ttbr0_el2 et ttbr1_el2
//      set TCR_EL2.T2SZ afin d'éco de la place mémoire (détail: Aarch64 memory management guide | Size of virtual addresses) 
//remap le code en higher half (à l'addresse 0xFFFF_FFFF_FFFF_FFFF)
//activer la pagination
//jump to _entry
bootstrap:
    //Create paginations tables
    bl tables_init
    ret    
