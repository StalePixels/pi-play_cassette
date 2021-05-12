#include "shutdown.h"
#include "tzx_file.h"
#include "tzx_load.h"

void shutdown(uint8_t error_number, const char *additional_message) {
    switch (error_number) {
        case EXIT_NO_FILENAME:
            fprintf(stderr, "\nERROR: Playback filename required!\n");
            exit(EXIT_FAILURE);
        case EXIT_NO_SOUNDCARD:
            fprintf(stderr, "\nERROR: Soundcard open error: %s\n", additional_message);
            exit(EXIT_FAILURE);
        case EXIT_PARAMS_SOUNDCARD:
            fprintf(stderr, "\nERROR: Soundcard config error: %s\n", additional_message);
            exit(EXIT_FAILURE);
            
        case ERR_TZX_FILE_OPEN_ERROR:
            fprintf(stderr, "\nERROR: Could not open datafile: %s\n", additional_message);
            exit(EXIT_FAILURE);
        
    }
    
    exit(EXIT_SUCCESS);
}