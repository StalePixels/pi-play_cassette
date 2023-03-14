#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define EXIT_NO_FILENAME                            0x01
#define EXIT_FILENAME_NOT_FOUND                     0x02
#define EXIT_FILE_NOT_RECOGNISED                    0x03

#define EXIT_NO_SOUNDCARD                           0x10
#define EXIT_PARAMS_SOUNDCARD                       0x11

void shutdown(uint8_t error_number, const char *additional_message);