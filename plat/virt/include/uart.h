#ifndef UART_H
#include <stdint.h>

//at offset 0x0
struct UART_DR
{
    uint8_t dr_reserved : 4;
    uint8_t dr_oe : 1;
    uint8_t dr_be : 1;
    uint8_t dr_pe : 1;
    uint8_t dr_fe : 1;
    uint8_t dr_data : 8;
}__attribute__((packed));

//at 0x018
struct UART_FR
{
    uint8_t fr_reserved : 7;
    uint8_t fr_ri : 1;
    uint8_t fr_txfe : 1;
    uint8_t fr_rxff : 1;
    uint8_t fr_txff : 1;
    uint8_t fr_rxfe : 1;
    uint8_t fr_busy : 1;
    uint8_t fr_dcd : 1;
    uint8_t fr_dsr : 1;
    uint8_t fr_cts : 1;
}__attribute__((packed));

void putc(uint8_t c);
uint8_t getc(void);
void wait_unbusy(void);

#endif