#ifndef TAR_H
#define TAR_H

typedef struct tar_posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
                                /* 500 */
} Tar_posix_header;


typedef struct tarinfos 
{
    uint64_t total_tar_size;
    uint64_t nb_files_in_tar;
    uint64_t start_offset;
    uint64_t end_offset;
} TarInfos;


void extract_tarfile_to(uint64_t tar_off, uint64_t dest_off, uint8_t * name);

#endif