#include <stdint.h> 

#include "cmd.h"
#include "serial.h"

void Unknown(void)
{
    puts(" : Unknown command (type 'help' to see usable commands)\n");
    return;
}

void Help(void)
{
    puts("  help : help\n\n");
    puts("       Display this help.\n");
    puts("\n");
    return;
}

/*void Memdump(uint64_t s_off, uint64_t nb_byte, uint8_t gran)
{

}*/

void Memdump(void)
{
    puts("To be done.\n");
}
//Check memory chunks to see if available or not
void Memtest(void)
{
    puts("To be done.\n");
}

//Check memory chunks to see if available or not
void Hostcheck(void)
{
    puts("To be done.\n");
}

//Get info from host kernel iso
void Hostinfo(void)
{
    puts("To be done.\n");
}

//Get info from Guest kernel iso
void Guestinfo(void)
{
    puts("To be done.\n");
}

//Start the booting sequence of the host kernel, once started, boot the the guest kernel.
void Boot(void)
{
    puts("To be done.\n");
}
