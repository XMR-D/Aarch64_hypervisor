#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "log.h"

#include "pagination.h"


//Starting from max address on 4Gib = 0xffffffff
uint64_t cur_phy_addr = 0xffffffff;

//change this to where I want my l3 tables
uint64_t cur_l3_addr = 0x40000000;

//Populate a L3 table
//for 512 L3 entry do the following:
// - Starting from cur_l3_addr
// - Using block_attribute, create and setup a l3 entry
// - populate the table
//Return the starting addr of the table

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

uint64_t L3_create_table()
{
    uint64_t *entry_ptr = (uint64_t *) cur_l3_addr;
    for (uint64_t i = 0; i < L3_NB_ENTRY; i++) 
    {   
        uint64_t attr = block_attributes;

        L3entry *entry = (L3entry *) &attr;


        //TODO REVIEW HOW ATTRIBUTES ARE HANDLE
        entry->UPAT = block_attributes;
        entry->baseaddr = cur_phy_addr;
        entry->LPAT = block_attributes;
        entry->entrytype = PT_PAGE;

        *entry_ptr = *(uint64_t *) entry;
        entry_ptr++;
        cur_phy_addr -= L3_BLOCK_SIZE;
    }
    return cur_l3_addr;
}


//Populate a L2 table
//For 512 L2 entry do the following:
// - Create a L3_table (call L3_create table)
// - Setup a corresponding L2 entry
// - Populate L2_high
//When 512 entry was setup do the same again with L2_low
//Then set up TTBR0_EL3 and TTBR1_EL3 with corresponding tables
void tables_init()
{
    INFO("Pagination initializated set up with the following parameters:");
    pagination_settings();

    INFO("Pagination tables initialization started.");
    WARN("Creating hiherhalf L2 table.");
    for(uint64_t i = 0; i < L2_NB_ENTRY; i++)
    {
        //Create l3_table at cur_l3_addr
        L3_create_table();

        uint64_t attr = table_attributes;
        L2entry* entry = (L2entry *) &attr;
        entry->baseaddr = cur_l3_addr;
        
        L2_high[i] = *entry;

        //increment to next table (contiguous repartitions)
        cur_l3_addr += (L3_NB_ENTRY * L3_BLOCK_SIZE);

    }
    WARN("Creating lowerhalf L2 table.");

    for(uint64_t i = 0; i < L2_NB_ENTRY; i++)
    {
        
        //Create l3_table at cur_l3_addr
        L3_create_table();


        uint64_t attr = table_attributes;
        L2entry* entry = (L2entry *) &attr;
        entry->baseaddr = cur_l3_addr;
        
        L2_low[i] = *entry;

        //increment to next table (contiguous repartitions)
        cur_l3_addr += (L3_NB_ENTRY * L3_BLOCK_SIZE);

    }
    SUCC("Paging Init finished successfully.");
    return;
}