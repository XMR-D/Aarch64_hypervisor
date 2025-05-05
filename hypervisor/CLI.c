#include <stdint.h>

#include "uart.h"
#include "serial.h"

#include "CLI.h"
#include "cmd.h"

uint8_t cmd[BUFF_LEN] = "";
uint8_t curs = 0;

//Check if a uint8_t is printable
uint8_t is_printable(uint8_t c)
{
    return (c >= 33 && c <= 126);
}

void Reset(void)
{
    for(uint8_t j = 0; j <= curs ; j++)
        cmd[j] = 0;
    curs = 0;

}

//CmdResolve will resolve the cmd buffer checking format of arguments and setup Arg struct
uint64_t CmdResolve()
{
    //PARSE COMMAND
    return 0;
}

void CmdHandler(uint64_t CRI)
{
    switch (CRI) {
        case HELP:
            Help();
            break;
        case MEMDUMP:
            Memdump();
            break;
        case MEMTEST:
            Memtest();
            break;
        case HOSTCHECK:
            Memcheck();
            break;
        case HOSTINFO:
            Hostinfo();
            break;
        case GUESTINFO:
            Guestinfo();
            break;
        case BOOT:
            Boot();
            break;
        default:
            puts("Command '");
            puts((volatile char *) cmd);
            puts("' ");
            Unknown();
            break;
    }
}
//Polling function to call in kmain to retreive character
void CLI(void)
{
    uint8_t input = 0;
    puts("EL2 > ");
    while(input != ENTER)
    {
        input = getc();
        //If printable print it
        if (is_printable(input)) {
            cmd[curs] = input;
            curs++;
            putc(input);
        }
        //Else do different things depending on input 
        else switch(input) {
            case DEL:
                if (curs > 0) {
                    cmd[curs] = 0;
                    curs--;
                    puts("\b \b");
                }
                break;
            case SPACE:
                cmd[curs] = ' ';
                curs++;
                break;
            case ENTER:
                putc('\n');
                break;
            default:
                break;
        }
    }
    uint64_t CRI = CmdResolve();
    CmdHandler(CRI);
    Reset();
}