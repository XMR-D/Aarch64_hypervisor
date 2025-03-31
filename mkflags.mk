
#This is where we define the tools and everything that will be used for build stage
#ARCH: Target architecture chosen by user
ARCH?=aarch64
# PLAT: Platerform chosen by user (will be the directory where plateform depedent code will be)
PLAT?=virt
#CROSS_COMPILER: Cross compiler used to build on the chosen target
CROSS_COMPILER?=$(ARCH)-linux-gnu-
#CC: Compiler
CC=$(CROSS_COMPILER)gcc
#LD: Linker
LD=$(CROSS_COMPILER)ld
#AS: Assembler
AS=$(CROSS_COMPILER)as
#OBJCOPY: Objcopy in order for us to get img files
OBJCOPY=$(CROSS_COMPILER)objcopy
#OBJDUMP: Objdump in order for us to get dump files to analyse
OBJDUMP=$(CROSS_COMPILER)objdump

# MAIN: hypervisor dir where files not depending on arch or platform will be
MAIN?=hypervisor

#Languages flags
CFLAGS= -Wall -Wextra -Werror -ffreestanding -fno-stack-protector -fno-zero-initialized-in-bss -g -c
SFLAGS=
LDFLAGS=-nostdlib -T link.ld