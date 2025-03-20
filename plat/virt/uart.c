#include <stdlib.h>

#include "uart.h"

volatile struct UART_DR *uartdr = (struct UART_DR *) 0x9000000;
volatile struct UART_FR *uartfr = (struct UART_FR *) 0x9000018;


void wait_unbusy(void)
{
    while(uartfr->fr_busy != 0)
    {
        continue;
    }
}

//Transmit 8 bits to Uart
void putc(uint8_t c)
{
    if (c == '\n')
        putc('\r');
    uartdr->dr_data = c;
}

//Receive 8 bits from Uart
uint8_t getc(void)
{
    wait_unbusy();
    while (uartfr->fr_rxfe == 1)
    {
        continue;
    }
    return uartdr->dr_data;
}