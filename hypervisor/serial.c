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

//homemade puthex
//1 = no zeros
void puthex(uint64_t nb, uint8_t padding_mode)
{
    uint64_t temp = 0;
    char arr[17] = "0000000000000000\0";
    uint8_t padded = 1;
    int8_t i = 0;


    if (nb == 0)
    {
        puts(arr);
        return;
    }

    if (nb <= 15)
        putc('0');


    while(nb > 0)
    {
        temp = nb % 16;
        if(temp < 10)
        {
            arr[i] = temp + 48;
            i += 1;
        }
        else
        {
            arr[i] = temp + 87;
            i += 1;
        }
        nb /= 16;
    }

    for(i = 15; i >= 0; i--)
    {
        if (padding_mode && padded && arr[i] == '0')
            continue;
        else if (padding_mode == 1)
        {
            putc(arr[i]);
            padded--;
        }
        else
        {
            putc(arr[i]);
        }
    }
}