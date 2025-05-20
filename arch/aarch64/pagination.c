#include <stdint.h>


#include "uart.h"
#include "serial.h"
#include "log.h"

#include "pagination.h"


//Starting from max address block on 2Gib = 0x7ffff000
uint64_t cur_phy_addr = 0x7ffff000;

//Tables will be above the text section and under the stack (0x7fff0000), 
// devices are mapped way lower (around 0xa00000 (see dtb dump))
uint64_t l3_addr = 0x40000000;

void pagination_settings()
{
    puts("Physicall memory size (in octect): ");
    putint(PHYS_MEM_SIZE);
    putc('\n');
    puts("Defined page size (in octect): ");
    putint(PAGESIZE);
    putc('\n');
    puts("Number of paging levels: 2 \n");
    puts("Level 2 block size (in octect): ");
    putint(L2_BLOCK_SIZE);
    putc('\n');
    puts("Level 3 block size (in octect): ");
    putint(L3_BLOCK_SIZE);
    putc('\n');
    puts("Number of entries in L2 tables: ");
    putint(L2_NB_ENTRY);
    putc('\n');
    puts("number of entries in L3 tables: ");
    putint(L3_NB_ENTRY);
    putc('\n');
}

//Populate a L3 table
//for 512 L3 entry do the following:
// - Starting from cur_l3_addr
// - Using block_attribute, create and setup a l3 entry
// - populate the table
//Return the starting addr of the table
void L3_create_table(uint8_t AP, uint8_t NS)
{
    uint64_t *entry_ptr = (uint64_t *) l3_addr;
    uint64_t descr;

    for (uint64_t i = 0; i < L3_NB_ENTRY; i++) 
    {   
        descr = 0;
        L3entry *entry = (L3entry *) &descr;

        //Setup upper attributes
        entry->Ignored = 0;
        entry->SoftwareUsed = 0;
        entry->XN = PT_NX;
        entry->PXN = PT_nPNX;
        entry->ContHint = PT_nHINT;
        entry->Ignored2 = 0b0;

        //Setup lower attributes
        entry->nG = PT_G;
        entry->AF = PT_AF;
        entry->SH = PT_ISH;
        entry->AP = AP;
        entry->NS = NS;
        entry->AttrIndx = PT_MEM;
        //Setup our L3 entry 
        //made from the address of the block, the upper and lower attribute 
        //+ define it as a page descrp not a table descr
        entry->baseaddr = (cur_phy_addr >> 12);
        entry->entrytype = PT_BLOCK;

        *entry_ptr = *(uint64_t *) entry;
        entry_ptr++;
        cur_phy_addr -= L3_BLOCK_SIZE;
    }
    return;
}


//Populate a L2 table
//For 512 L2 entry do the following:
// - Create a L3_table (call L3_create table)
// - Setup a corresponding L2 entry
// - Populate L2_high
//When 512 entry was setup do the same again with L2_low
//Then set up TTBR0_EL3 and TTBR1_EL3 with corresponding tables
void mmu_init()
{


    INFO("Pagination initializated set up with the following parameters:");
    pagination_settings();

    INFO("Pagination tables initialization started.");
    INFO("Creating tables.");
    uint64_t descr;

    for(uint64_t i = 0; i < L2_NB_ENTRY; i++)
    {
        //For precomputed number of L2 entries do the following:
        //Create l3_table at cur_l3_addr
        L3_create_table(PT_KERNEL, PT_SEC);
        
        descr = 0;
        //Setup L2 entry
        L2entry* entry = (L2entry *) &descr;
        entry->baseaddr = l3_addr >> 12;
        entry->NST = PT_SEC;
        entry->APT = 0b00;                          //For APT XNT and PXNT do not override setup in subsequent levels 
        entry->XNT = 0b00;
        entry->PXNT = 0b00;
        entry->entrytype = PT_TABLE;

        //Populate L2 table with the created entry
        L2_high[i] = *entry;

        //increment to next L3 table (contiguous repartitions)
        l3_addr += (L3_NB_ENTRY * L3_BLOCK_SIZE);

    }
    for(uint64_t i = 0; i < L2_NB_ENTRY; i++)
    {
        
        //Create l3_table at cur_l3_addr
        L3_create_table(PT_USER, PT_nSEC);

        descr = 0;
        //Setup L2 entry    2124414976
        L2entry* entry = (L2entry *) &descr;
        entry->baseaddr = l3_addr >> 12;
        entry->NST = PT_nSEC;
        entry->APT = 0b00;
        entry->XNT = 0b00;
        entry->PXNT = 0b00;
        entry->entrytype = PT_TABLE;
        L2_low[i] = *entry;

        //increment to next table (contiguous repartitions)
        l3_addr += (L3_NB_ENTRY * L3_BLOCK_SIZE);

    }

    INFO("Table created, initializing ttbr0_el2 and ttbr1_el2.");

    //lower half (user space)
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((uint64_t)&L2_low));

    //higher half (kernel space)
    asm volatile ("msr ttbr1_el1, %0" : : "r" ((uint64_t)&L2_high));
    
    INFO("Translation table base registers initialized, enabling MMU via SCTLR_EL1 register");

    /*asm volatile (
        "mrs x9, sctlr_el1\n" //read system control register
        "orr x9, x9, #1\n"    //flip bit 0 (MMU enable)
        "msr sctlr_el1, x9\n" //write back value
        "isb"               //isb (Instruction synchronization barrier) (flush the CPU piepline and refetch instructions with now MMU enabled)
    );*/

    SUCCESS("Paging Init finished successfully.");
    return;
}