#ifndef SERIAL_H

//Puts: Print a String
void puts(volatile char *str);

//Putint: print an integer in decimal format
void putint(uint64_t nb);

//Puthx: print an integer in hexadecimal format
void puthex(uint64_t nb, uint8_t padding_mode);
#endif