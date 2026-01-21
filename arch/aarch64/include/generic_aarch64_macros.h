#ifndef GENERIC_AARCH64_MACROS_H
#define GENERIC_AARCH64_MACROS_H

#define GB 1073741824UL
#define MB 1048576UL
#define KB 1024UL

/* define the start of the hypervisor binary that is launched by Qemu */
#define FLASHBIN_START 0x40080000

/* define where the bootstrap tar will be placed */
#define BOOTSTRAP_TAR_LOC (FLASHBIN_START+25*MB)

/* define the spot where the VM Image will be placed (at 3GB)  */
#define VM_DEST 0xC0001000

#endif /* !GENERIC_AARCH64_MACROS_H */