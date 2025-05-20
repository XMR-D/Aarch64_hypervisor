#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "picolibc.h"
#include "CLI.h"
#include "cmd.h"

#include "log.h"

//Command buffer and size
uint8_t input[BUFF_LEN] = "";
uint64_t input_len = 0;

//Dummy pointer used to point to the command typed
uint8_t * cmd;
//Current index
uint8_t curr = 0;

//CLI argument
CLI_args cmd_args;


//TODO : FIX HELP/5 situation
//TODO : DEBUG FROM_HEX handling hexvalues

void Dump_args(void)
{
    WARN("Dumping command arguments after parsing");
    putint(cmd_args.arg1);
    putc('\n');
    putint(cmd_args.arg2);
    putc('\n');
    putint(cmd_args.arg3);
    putc('\n');
    putint(cmd_args.arg4);
    putc('\n');
    putint(cmd_args.arg5);
    putc('\n');
    putint(cmd_args.arg6);
    putc('\n');
    putint(cmd_args.arg7);
    putc('\n');
    putint(cmd_args.arg8);
    putc('\n');
}


//Reset: Set cmd buffer to 0, reset arguments structure
void Reset(void)
{
    cmd_args.arg1 = 0;
    cmd_args.arg2 = 0;
    cmd_args.arg3 = 0;
    cmd_args.arg4 = 0;
    cmd_args.arg5 = 0;
    cmd_args.arg6 = 0;
    cmd_args.arg7 = 0;
    cmd_args.arg8 = 0;

    for(uint8_t j = 0; j <= curr ; j++)
        input[j] = 0;
    curr = 0;

    input_len = 0;

}

void fill_args(uint64_t val, uint8_t index)
{
    switch(index)
    {
        case 1:
            cmd_args.arg1 = val;
            break;
        case 2:
            cmd_args.arg2 = val;
            break;
        case 3:
            cmd_args.arg3 = val;
            break;
        case 4:
            cmd_args.arg4 = val;
            break;
        case 5:
            cmd_args.arg5 = val;
            break;
        case 6:
            cmd_args.arg6 = val;
            break;
        case 7:
            cmd_args.arg7 = val;
            break;
        case 8:
            cmd_args.arg8 = val;
            break;
    }
}

//CmdTokenize: Tokenize cmd and fill the args structure
//PARSE_SUCC: cmd tokenized successfully
//PARSE_ERR: cmd tokenization failed (Wrong argument format)
uint64_t CmdParse(uint8_t * start)
{
    uint8_t j = 0;

    //arggindex indicate which field of the struct to fill
    uint8_t argindex = 1;
    int64_t value = 0;
    uint8_t * curs = start;
    uint8_t * tok = curs;

    while(input_len > 0)
    {
        tok = curs;

        //No token left
        if (*tok == '\0')
            break;

        //TOKENIZE
       for (j = 0; curs[j] != ' '; j++)
       {
            if(curs[j] == 0)
                break;
       }
        //remove the ' ' or do nothing if at the end of the str
        curs[j] = 0;

        //PARSE
        //token is an hex number
        if (is_hex(tok, j) == 0) {
            value = from_str(tok, j, 16);
        }
        //token is an decimal number
        else if (is_dec(tok, j) == 0) {
            value = from_str(tok, j, 10);
        }
        //token is not a supported integer representation
        else 
        {
            Reset();
            return ERR;
        }

        //if value is negative an error has been encountered
        if (value < 0) {
            Reset();
            return ERR;
        }

        if (argindex > 8) {
            puts("!> Error while handling argument structure. Too much argument (hint: Max argument number : 8)\n");
            Reset();
            return ERR;
        }

        fill_args(value, argindex);
        //skip to next token

        curs += j+1;
        input_len -= j+1;
        argindex++;
    }

    //Remove that after successfull debug
    Dump_args();

    return SUCC;
}

//CmdResolve: Resolve the cmd buffer checking format of arguments and setup Cli_arg struct
uint64_t CmdResolve()
{
    uint8_t * start_arg = input;
    uint8_t * command = input;
    int8_t error = SUCC;
    uint8_t i = 0;

    rem_trail((uint8_t *) input);



    //TODO: DEBUG PARSER INNER FUNCTIONS

    //Skip first word that would be tokenized as the command requested
    for (i = 0; input[i] != ' '; i++) {
        input_len--;
        if (input[i] == 0)
            break;
    }

    //remove first ' '
    start_arg += i;
    *start_arg = 0;
    start_arg++;

    //Input len recomputation
    input_len = strlen(start_arg);

    //If after extracting first word I have more than a char => argument => parse
    if (input_len >= 1) {
        //start of arg is just after first ' '
        error = CmdParse(start_arg);
        
        if (error == ERR) {
            puts("!> Error while parsing command (see above).\n");
            return ERR;
        }
    }

    cmd = command;

    if (strcmp(command, (const uint8_t *) "help") == 0)
        return HELP;
    else if (strcmp(command, (const uint8_t *) "boot") == 0)
        return BOOT;
    else if (strcmp(command, (const uint8_t *) "memdump") == 0)
        return MEMDUMP;
    else if (strcmp(command, (const uint8_t *) "memtest") == 0)
        return MEMTEST;
    else if (strcmp(command, (const uint8_t *) "hostcheck") == 0)
        return HOSTCHECK;
    else if (strcmp(command, (const uint8_t *) "hostinfo") == 0)
        return HOSTINFO;
    else if (strcmp(command, (const uint8_t *) "guestinfo") == 0)
        return GUESTINFO;
    else
        return UNKNOWN;
}

//CmdHandler: Handle the CRI generated by Cmdresolve and execute appropriate logic
void CmdHandler(int64_t CRI)
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
            Hostcheck();
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
            puts("'");
            Unknown();
            break;
    }
}

//CLI: Polling function that implement a Command Line interface
void CLI(void)
{
    //Reset cmd buffer and cmd args
    Reset();

    uint8_t ch = 0;
    puts("EL2 > ");
    while(ch != ENTER_K)
    {
        ch = getc();
        //If printable and cmd buffer not saturated print it
        if ((is_printable(ch) || ch == SPACE_K) && input_len < (BUFF_LEN-1)) {
            input[curr] = ch;
            curr++;
            input_len++;
            putc(ch);
        }
        //Else do different things depending on input 
        else switch(ch) {
            case DEL_K:
                if (curr > 0) {
                    curr--;
                    input[curr] = 0;
                    input_len--;
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
    
    int64_t CRI = CmdResolve();

    //if no error, handle the command
    if (CRI != ERR) {
        CmdHandler(CRI);
        return;
    }

    puts("!> Error while resolving command (see above).\n");
    return;
}