#include "tzx_file.h"

char *tzx_filename;             // Path to file data
FILE *tzx_filehandle = NULL;    // FILE pointer
int32_t tzx_filesize;           // overall size on disk
int32_t tzx_datasize;           // actual data portion of the file

int8_t *tzx_data;
uint8_t tzx_version_major, tzx_version_minor;
