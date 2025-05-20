#ifndef CMD_H

void Help(void);

//Memdump: Dump nb_byte from memory starting at offset s_off in granularity gran (1, 2, 4, 8, 16, 32, 64)

//void Memdump(uint64_t s_off, uint64_t nb_byte, uint8_t gran);
void Memdump(void);

void Memtest(void);

void Hostcheck(void);

void Hostinfo(void);

void Guestinfo(void);

void Boot(void);

void Unknown(void);

#endif