#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "picolibc.h"
//Memory

//memcpy: copy n bytes from src to dest assuming memory does not overlap
void * memcpy(void *dest, const void *src, size_t n)
{
    uint8_t * ds = (uint8_t *) dest;
    uint8_t * sr = (uint8_t *) src;

    for (size_t i = 0; i < n; i++) {
        *ds = *sr;
        ds++;
        sr++;
    }
    return dest;
}

//memcmp: compare n bytes of two memory area
int64_t memcmp(const void *bl1, const void *bl2, size_t n)
{
    uint8_t * block1 = (uint8_t *) bl1;
    uint8_t * block2 = (uint8_t *) bl2;

    for(size_t i = 0; i < n; i++) {
        int64_t sub = *block1 - *block2;
        if (sub != 0)
            return sub;
        block1++;
        block2++;
    }
    return 0;
}

//Strings

//strlen: length of a string
uint64_t strlen(const uint8_t *a)
{
    uint64_t ret = 0;
    while(a[ret] != 0)
        ret++;
    return ret;
}

//strcmp: compare two strings
int64_t strcmp(const uint8_t *a, const uint8_t *b)
{
    uint64_t la = strlen(a);
    uint64_t lb = strlen(b);
    uint64_t sub = la - lb;

    if(sub != 0)
        return sub;
    for(size_t i = 0; i < la; i++) {
        uint64_t subc = a[i] - b[i];
        if (subc != 0)
            return subc;
    }
    return 0;
}

//Remtrailling: Remove trailing whitespace
void rem_trail(uint8_t *a)
{
    uint64_t len = strlen(a)-1;

    while(len > 0 && *(a+len) == ' ')
    {
        puts("removing a char\n");
        *(a+len) = 0;
        len--;
    }
}

//Check if a uint8_t is printable
uint8_t is_printable(uint8_t c)
{
    return (c >= 33 && c <= 126);
}

//Check if a uint8_t is (abcdef)
uint8_t is_hexchar(uint8_t c)
{
    return ((c >= 97 && c <= 102) || (c >= 65 && c <= 70) || (c >= 48 && c <= 57));
}

//Check if a uint8_t is alphabetic
uint8_t is_alphabetic(uint8_t c)
{
    return ((c >= 97 && c <= 122) || (c >= 65 && c <= 90));
}

//translate c to lower case
uint8_t to_lower(uint8_t c)
{
    return c + 32;
}

//Translate a character to it's decimal value, non valid char return negative value
int8_t char_to_num(uint8_t c)
{
    //not a digit (abcdef)
    if (c > 57)
    {
        switch(to_lower(c))
        {
            case 'a':
                return 10;
            case 'b':
                return 11;
            case 'c':
                return 12;
            case 'd':
                return 13;
            case 'e':
                return 14;
            case 'f':
                return 15;
            default:
                puts("!> Invalid Character found in operations (hint : it's either out of base (>16) or non alphanumeric)\n");  
                return -1;
        }
    }
    else
        return c - 48;
}

//Check if given string of length size is a hexadecimal representation
//0 str is an hex nb
//1 str is not an hex nb
//2 error while checking
uint8_t is_hex(uint8_t * str, uint8_t size)
{
    uint8_t * curs = str;

    //Check for the hex format
    if ((size < 3) || (*curs != '0' && *(curs+1) != 'x'))
        return 1;

    uint8_t i = 2;

    for (;i < size; i++)
    {
        if(!is_hexchar(*curs))
            return 1;
        i++;
        curs++;
    }

    return 0;
}

//Check if given string of length size is a decimal representation
//0 str is an dec nb
//1 str is not an dec nb
//2 error while checking
uint8_t is_dec(uint8_t * str, uint8_t size)
{
    uint8_t * curs = str;

    //Check for the hex format if detected not an dec
    if (*curs == '0' && *(curs+1) == 'x')
        return 1;

    uint8_t i = 2;

    for (;i < size; i++)
    {
        if(is_alphabetic(*curs))
            return 1;
        i++;
        curs++;
    }

    return 0;
}


//Translate a given string representation of an integer in base base into an integer
//result: value of the integer
//-1: error found
int64_t from_str(uint8_t * str, uint8_t size, int64_t base)
{
    putint(size);
    putc('\n');
    putint(base);
    putc('\n');
    uint8_t * curs = str;
    uint64_t result = 0;
    uint8_t i = 0;

    int8_t num = 0;

    //If invalid size or invalid base error
    if (base > 16) {
        puts("!> Invalid base requested (hint: operations used an unsupported base)\n");
        return -1;
    }

    //Skip potential 0b or 0x
    if (base == 16 || base == 2)
        i += 2;

    while (size > 0)
    {
        num = char_to_num(curs[i]);

        //curs[i] is invalid
        if (num < 0 || (num >= base)) {
            puts("!> Argument format error: Invalid character found (hint: too high for given base or purely invalid): '");
            putc(curs[i]);
            puts("'.\n");
            return -1;
        }

        result += num*pow(base, size);
        size--;
        i++;
        curs++;
    }

    return result;
}

//Math
uint64_t pow(uint64_t a, uint64_t b)
{
    uint64_t count = 1;

    while (b >= 1) {
        count *= a;
        b--;
    }
    return count;
}