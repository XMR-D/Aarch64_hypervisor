#include <stdint.h> 

#include "cmd.h"
#include "serial.h"

void Unknown(void)
{
    puts(" : Unknown command (see help to know usable commands)");
    return;
}

void Help(void)
{
    puts("  help : help\n\n");
    puts("       Display this help.\n");
    puts("\n");
    return;
}

void Memdump(void)
{
    puts("To be done.\n");
}

void Memtest(void)
{
    puts("To be done.\n");

}

void Memcheck(void)
{
    puts("To be done.\n");

}

void Hostinfo(void)
{
    puts("To be done.\n");

}

void Guestinfo(void)
{
    puts("To be done.\n");

}

void Boot(void)
{
    puts("To be done.\n");

}
