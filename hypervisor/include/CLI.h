#ifndef CLI_H

//Special characters
#define BACKSPACE   8
#define ENTER       13
#define SPACE       32
#define DEL         127

//Command Resolve Identifier (CRI)
#define HELP        0       //Help : print help
#define MEMDUMP     1       //Md : Dump memory starting from an offset using various granularity
#define MEMTEST     2       //MemTest : Test memory to see if writable/free
#define HOSTCHECK   3       //Hostcheck : Check Host Kernel integrity
#define HOSTINFO    4       //Hostinfo : Retreive host kernel informations
#define GUESTINFO   5       //Guestinfo : Retreive guest kernel informations
#define BOOT        6       //Boot : Start booting sequence of the hypervisor

#define BUFF_LEN    4096

void CLI(void);

#endif 