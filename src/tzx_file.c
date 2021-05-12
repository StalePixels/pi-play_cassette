#include "tzx_file.h"

char *tzx_filename;
FILE *tzx_filehandle = NULL;
int32_t tzx_filesize;
uint8_t tzx_errno;

int8_t *tzx_data;
uint8_t tzx_version_major, tzx_version_minor;
