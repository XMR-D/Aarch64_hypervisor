#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "picolibc.h"
#include "CLI.h"
#include "cmd.h"

#include "log.h"

//Command buffer and size
uint8_t input[INPUT_LEN] = {0};
uint64_t curr_len = 0;

//Current index
uint8_t curr = 0;

void ResetInput()
{
    for (uint64_t i = 0; i < curr_len; i++) {
        input[i] = 0;
    }
    curr_len = 0;
}

//CmdResolve: Resolve the cmd buffer checking format of arguments and setup Cli_arg struct
uint64_t CmdHandler()
{
    //TODO: Here parse arguments if necessary
    if (strcmp(input, (const uint8_t *) "help") == 0) {
        return Help();
    }
    else if (strcmp(input, (const uint8_t *) "boot") == 0) {
        return Boot();
    }
    else if (strcmp(input, (const uint8_t *) "memdump") == 0) {
        return Memdump();
    }
    else if (strcmp(input, (const uint8_t *) "memtest") == 0) {
        return Memtest();
    }
    else if (strcmp(input, (const uint8_t *) "hostcheck") == 0) {
        return Hostcheck();
    }
    else if (strcmp(input, (const uint8_t *) "hostinfo") == 0) {
        return Hostinfo();
    }
    else if (strcmp(input, (const uint8_t *) "guestinfo") == 0) {
        return Guestinfo();
    }
    else {
        puts("Command '");
        puts((volatile char *) input);
        puts("'");
        return Unknown();
    }
}

//CLI: main function that print a CLI, read the input, tokenize the args
//and parse the command passed
void CLI(void)
{
    uint8_t ch = 0;
    int curr = 0;
    
    //Reset cmd buffer and cmd args
    
    puts("EL2 > ");
    while(ch != ENTER_K)
    {
        ch = getc();
        //If printable and cmd buffer not saturated print it
        if ((is_printable(ch) || ch == SPACE_K) && curr_len < (INPUT_LEN-1)) {
            input[curr] = ch;
            curr++;
            curr_len++;
            putc(ch);
        }
        //Else do different things depending on input 
        else switch(ch) {
            case DEL_K:
                if (curr > 0) {
                    curr--;
                    input[curr] = 0;
                    curr_len--;
                    puts("\b \b");
                }
                break;
            case ENTER_K:
                putc('\n');
                break;
            default:
                break;
        }
    }
    
    CmdHandler();
    ResetInput();
}
