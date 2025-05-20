#include <stdint.h>

#include "uart.h"
#include "serial.h"
#include "log.h"
#include "picolibc.h"
#include "CLI.h"

int hmain()
{
    INFO("Starting Hypervisor logic");
    while(1) {
        CLI();
    }
    return 0;
}