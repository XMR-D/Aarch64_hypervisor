#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

typedef struct vminfos {
    uint64_t vm_image_off;
    uint64_t vm_image_size;
    uint64_t vm_dtb_off;
    uint64_t vm_dtb_size;
} VmInfos;

#endif