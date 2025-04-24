#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "picolibc.h"

int hmain()
{
    puts("Hello Hypervisor (with virtual memory)!\n");
    while(1) {
        char c = getc();
        putc(c);
    }
    return 0;
}