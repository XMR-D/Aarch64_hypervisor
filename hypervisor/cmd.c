#include <stdint.h> 

#include "cmd.h"
#include "serial.h"
#include "CLI.h"

uint64_t Unknown(void)
{
    puts(" : Unknown command (type 'help' to see usable commands)\n");
    return 0;
}

uint64_t Help(void)
{
    puts("help\n\n");
    puts("       Display this help.\n");
    puts("\n\n");
    puts("memdump [start_addr] [nb_b] [gran]\n\n");
    puts("       Dump [nb_b] bytes from memory starting from offset [start_addr] following granularity [gran]\n");
    puts("\n\n");
    puts("memtest [start_addr] [nb_b]\n\n");
    puts("       Test [nb_b] of memory to check if it is writeable starting from [start_addr]\n");
    puts("\n\n");
    puts("hostcheck\n\n");
    puts("       Check for host kernel integrity.\n");
    puts("\n\n");
    puts("hostinfo\n\n");
    puts("       Dump information on host kernel.\n");
    puts("\n\n");
    puts("guestinfo\n\n");
    puts("       Dump information on guest kernel.\n");
    puts("\n\n");
    puts("boot\n\n");
    puts("       Start boot sequence.\n");
    puts("\n\n");
    return 0;
}

/*void Memdump(uint64_t s_off, uint64_t nb_byte, uint8_t gran)
{

}*/

uint64_t Memdump(void)
{
    puts("Memdump: To be done.\n");
    return 0;
}
//Check memory chunks to see if available or not
uint64_t Memtest(void)
{
    puts("Memtest: To be done.\n");
    return 0;
}

//Check memory chunks to see if available or not
uint64_t Hostcheck(void)
{
    puts("Hostcheck: To be done.\n");
    return 0;
}

//Get info from host kernel iso
uint64_t Hostinfo(void)
{
    puts("Hostinfo: To be done.\n");
    return 0;
}

//Get info from Guest kernel iso
uint64_t Guestinfo(void)
{
    puts("GuestInfo: To be done.\n");
    return 0;
}

//Start the booting sequence of the host kernel, once started, boot the the guest kernel.
uint64_t Boot(void)
{
    puts("Boot: To be done.\n");
    return 0;
}
