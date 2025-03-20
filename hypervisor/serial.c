#include <stdint.h>

#include "uart.h"
#include "serial.h"

//Transmit a string
void puts(volatile char *str)
{
    for (uint32_t i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
            putc('\r');
        putc(str[i]);
    }
}