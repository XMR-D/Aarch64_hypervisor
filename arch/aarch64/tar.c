#include <stdint.h> 

#include "log.h"
#include "uart.h"
#include "serial.h"
#include "picolibc.h"

#include "tar.h"

#define BLOCKSIZE 512

/* 
    Function that check for the end of a tar file
    marked by a 512 byte block full of zero.
*/
static uint8_t 
is_end_of_tar(void * off)
{
    uint8_t * curr = (uint8_t * ) off;
    uint64_t i;

    for (i = 0; i < BLOCKSIZE; i++) {
        if (*curr != 0) {
            return 0;
        }
    }
    return 1;
}

/* 
    Function that check if current offset is a posix_header or not

    Note: To do it, compute the checksum starting at off, and compare it
    to the one extracted using the function.
*/
static uint8_t 
is_posix_header(void * off)
{
    Tar_posix_header * block = (Tar_posix_header *) off;
    uint8_t * cur = (uint8_t *) off;

    uint8_t * bcheck = (uint8_t *) block->chksum;

    /* posix header checksum is expressed in octal due to legacy format from 1988 */
    uint64_t extracted_checksum = from_str(bcheck, strlen(bcheck), 8);

    uint64_t computed_checksum = 0;

    for (uint64_t i = 0; i < BLOCKSIZE; i++) {

        /* if on checksum field replace the value by spaces in the computation */
        if (i >= 0x94 && i <= 0x9b)
            computed_checksum += 0x20;
        else 
            computed_checksum += *cur;
        
        cur++; 
    }

    return extracted_checksum == computed_checksum;
}

static void
log_tar_infos(TarInfos tar_infos)
{
    puts("Tar detected at : ");
    puthex(tar_infos.start_offset, 1);
    puts(" !\n");

    puts("Total size of tar archive (in octects) : ");
    putint(tar_infos.total_tar_size);
    putc('\n');

    puts("Number of files in tar file : ");
    putint(tar_infos.nb_files_in_tar);
    putc('\n');

    puts("Tar is ending at : ");
    puthex(tar_infos.end_offset, 1);
    putc('\n');
}


static uint64_t 
detect_tar_at(uint64_t off)
{
    uint8_t * tar_ptr = (uint8_t *) off;
    TarInfos tar_infos;

    /* 
       Check if the offset correspond to a valid tar file 
       by checking if the given offset is describing a header block
       every valid tar files start with a header block.
    */
    if ( (*tar_ptr == 0) || !is_posix_header(tar_ptr)) {
        WARN("tar not found.");
        puts("No tar file found at: ");
        puthex((uint64_t) tar_ptr, 1);
        putc('\n');
        return 0;
    }

    tar_infos.total_tar_size = 0;
    tar_infos.nb_files_in_tar = 0;
    tar_infos.start_offset = off;
    tar_infos.end_offset = off;

    while (!is_end_of_tar(tar_ptr)) {

        if (is_posix_header(tar_ptr)) {
            tar_infos.nb_files_in_tar += 1;
        }
        tar_infos.end_offset += BLOCKSIZE;
        tar_infos.total_tar_size += BLOCKSIZE;

        tar_ptr += BLOCKSIZE;
    }
    /* do not forget to add the end of tar sections to the size of the tar*/
    tar_infos.end_offset += BLOCKSIZE*2;

    log_tar_infos(tar_infos);

    return 1;

}

void 
extract_tarfile_to(uint64_t tar_off, uint64_t dest_off, uint8_t * name)
{
    uint8_t * dest_ptr = (uint8_t *) dest_off;
    uint8_t * tar_ptr = (uint8_t *) tar_off;

    uint8_t filefound = 0;

    uint8_t * src_ptr;
    uint64_t filesize = 0;

    if (!detect_tar_at(tar_off)) {
        return;
    }

    while (!filefound) {
        if(is_end_of_tar(tar_ptr)) {

            WARN("File not found in tar."); 
            puts("No file named : ");
            puts((volatile char *) name);
            puts(" found. Nothing to extract. exiting...\n");
            return;
        }

        if (is_posix_header(tar_ptr)) {

            Tar_posix_header * tar_file = (Tar_posix_header *) tar_ptr;
            if (strcmp(name, (uint8_t *) tar_file->name) == 0) {
                puts("Requested file found in tar.\n");
                filefound = 1;
                src_ptr = (uint8_t *) (tar_ptr + BLOCKSIZE);
                filesize = from_str((uint8_t *)tar_file->size, strlen((uint8_t*)  tar_file->size), 8);
            }
        }
    }

    memcpy(dest_ptr, src_ptr, filesize);

    SUCCESS("File extracted and copied.");
    puts("Name of the file extracted : ");
    puts((volatile char *) name);
    puts(", Copied to :");
    puthex((uint64_t) dest_ptr, 1);
    putc('\n');
}
