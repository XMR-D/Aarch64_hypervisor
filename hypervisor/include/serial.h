#ifndef SERIAL_H

//Transmit a string
void puts(volatile char *str);
void putint(uint64_t nb);
void puthex(uint64_t nb, uint8_t padding_mode);


#endif