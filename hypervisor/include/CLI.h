#ifndef CLI_H

//Special characters
#define BACKSPACE_K   8
#define ENTER_K       13
#define SPACE_K       32
#define DEL_K         127

//Command Resolve Identifier (CRI)
#define HELP        0       //Help : print help
#define MEMDUMP     1       //Md : Dump memory starting from an offset using various granularity
#define MEMTEST     2       //MemTest : Test memory to see if writable/free
#define HOSTCHECK   3       //Hostcheck : Check Host Kernel integrity
#define HOSTINFO    4       //Hostinfo : Retreive host kernel informations
#define GUESTINFO   5       //Guestinfo : Retreive guest kernel informations
#define BOOT        6       //Boot : Start booting sequence of the hypervisor
#define UNKNOWN      999     //Unknown command

#define BUFF_LEN    4096

#define SUCC 0
#define ERR -1

//CLI_ARGS struct : Structure containing our parameters.
//(See README for detail on arguments format)
typedef struct cli_args{
    uint64_t arg1 : 64;
    uint64_t arg2 : 64;
    uint64_t arg3 : 64;
    uint64_t arg4 : 64;
    uint64_t arg5 : 64;
    uint64_t arg6 : 64;
    uint64_t arg7 : 64;
    uint64_t arg8 : 64;
}__attribute__((packed)) CLI_args;

void CLI(void);

#endif 