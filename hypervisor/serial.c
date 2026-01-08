#include <stdint.h>

#include "uart.h"
#include "serial.h"

#include "log.h"

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

//print a unsigned integer on the serial
void putint(uint64_t nb)
{
    uint64_t count = 1;
    uint64_t saved = nb;
    if(nb == 0)
        putc('0');
    else
    {
        while(nb >= 10)
        {
            nb /= 10;
            count *= 10;
        }
        while(count > 0)
        {
            putc((saved/count) + 48);
            saved -= (saved/count)*count;
            count /= 10;
        }
    }
}

void puthex(uint64_t nb, uint8_t padding_mode)
{
    char hex_digits[] = "0123456789ABCDEF";
    char buffer[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    int digit_count = 0;
    int i;

    puts("0x");

    if (nb == 0) {
        if (padding_mode == 1) {
            for (i = 0; i < 16; i++) {
                putc('0');
            }
        } else {
            putc('0');
        }
        return;
    }

    while (nb > 0) {
        buffer[digit_count++] = hex_digits[nb & 0xF];
        nb >>= 4;
    }

    if (padding_mode == 1) {
        for (i = digit_count; i < 16; i++) {
            putc('0');
        }
    }

    while (digit_count > 0) {
        putc(buffer[--digit_count]);
    }
}