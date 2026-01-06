#ifndef CMD_H
#define CMD_H

uint64_t Unknown(void);

uint64_t Help(void);

uint64_t Memdump(void);

//Check memory chunks to see if available or not
uint64_t Memtest(void);

//Check memory chunks to see if available or not
uint64_t Hostcheck(void);

//Get info from host kernel iso
uint64_t Hostinfo(void);

//Get info from Guest kernel iso
uint64_t Guestinfo(void);

//Start the booting sequence of the host kernel, once started, boot the the guest kernel.
uint64_t Boot(void);

#endif /* !CMD_H */