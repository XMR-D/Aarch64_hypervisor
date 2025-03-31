.section ".text"

#include "TTcnst.h"

.global bootstrap

//Create a L3_NB_ENTRY entry L3 table mapping addresses starting from START_ADDR
//X10 = START_ADDRESS + x (x start at 0 and increment by L3_PAGE_SIZE)
L3_create_table:
    //Init x8 with nb of entries to set
    ldr x8, =L3_NB_ENTRY
l3loop:
    cbz x8, l3exit                 //If X8 = 0 then exit
    lsl x11, x10, #48
    ldr x9, =L3entry               //X11 = X10 << 48
    orr x9, x11, x9                //X9 = L3entry | X11
    stur x9, [x12, #0]
    ldr x9, =L3_PAGE_SIZE          //X12[0] = L3entry | X11
    add x10, x10, x9               //X10 += L3_PAGE_SIZE
    sub x8, x8, #1                 //i -= 1
    b l3loop
l3exit:
    ret

//X13 used for indexing, initialized to 1
//X10 contain the start address
L2_create_table:
    ldr x13, =L2_NB_ENTRY
l2loop:
    cbz x13, l2exit
    //Store the value of x10 (fist entry of previous L3 table created) in the actual entry of the L2
    lsl x11, x10, #48
    ldr x15, =L2entry
    orr x14, x11, x15
    stur x14, [X18, #0]
    //call L3_create_table to create a new table
    bl L3_create_table
    sub x13, x13, #1 
    b l2loop
l2exit:
    ret

//Setup ttbr0_el2 et ttbr1_el2
//      set TCR_EL2.T2SZ afin d'éco de la place mémoire (détail: Aarch64 memory management guide | Size of virtual addresses) 
//remap le code en higher half (à l'addresse 0xFFFF_FFFF_FFFF_FFFF)
//activer la pagination
//jump to _entry
bootstrap:
    //Create pagination,

    //Create first L2 table for higherhalf
    ldr X16, =L2_0
    bl L2_create_table
    //here put =L2_0 in TTBREL2_0

    //Create second L2 table for lowerhalf
    ldr X16, =L2_1
    bl L2_create_table
    //here put =L2_0 in TTBREL2_1


    //then activate pagination
    
    ret    
