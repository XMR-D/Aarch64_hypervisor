#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "picolibc.h"

int hmain()
{
    puts("Hello Hypervisor!\n");
    while(1) {
        char c = getc();
        putc(c);
    }
    return 0;
}