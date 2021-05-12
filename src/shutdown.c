#include "shutdown.h"
#include "tzx_file.h"
#include "tzx_load.h"

void shutdown(uint8_t error_number, const char *additional_message) {
    // First, some cleanup
    
    if(tzx_data)
    {
        free(tzx_data);
    }
    
    if(tzx_filehandle) {
        fclose(tzx_filehandle);
    }
    
    switch (error_number) {
        case EXIT_NO_FILENAME:
            fprintf(stderr, "\nERROR: Playback filename required!\n");
            exit(EXIT_NO_FILENAME);
            break;
            
        case EXIT_NO_SOUNDCARD:
            fprintf(stderr, "\nERROR: Soundcard open error: %s\n", additional_message);
            exit(EXIT_NO_SOUNDCARD);
            break;
            
        case EXIT_PARAMS_SOUNDCARD:
            fprintf(stderr, "\nERROR: Soundcard config error: %s\n", additional_message);
            exit(EXIT_PARAMS_SOUNDCARD);
            break;
            
        case ERR_TZX_FILE_SIZE_ERROR:
            fprintf(stderr, "\nERROR: Could not calculate size of datafile: %s\n", additional_message);
            exit(ERR_TZX_FILE_SIZE_ERROR);
            break;
            
        case ERR_TZX_FILE_OPEN_ERROR:
            fprintf(stderr, "\nERROR: Could not open datafile: %s\n", additional_message);
            exit(ERR_TZX_FILE_OPEN_ERROR);
            break;
            
        case ERR_TZX_FILE_NO_MEMORY:
            fprintf(stderr, "\nERROR: Could not allocate memory for datafile: %s\n", additional_message);
            exit(ERR_TZX_FILE_NO_MEMORY);
            break;
            
        case ERR_TZX_NOT_TZX_FILE:
            fprintf(stderr, "\nERROR: Datafile appears not to be a TZX: %s\n", additional_message);
            exit(ERR_TZX_NOT_TZX_FILE);
            break;
            
        case ERR_TZX_BETAS_NOT_SUPPORTED:
            fprintf(stderr, "\nERROR: TZX appears an unsupported beta: %s\n", additional_message);
            exit(ERR_TZX_BETAS_NOT_SUPPORTED);
            break;
            
        case ERR_TZX_IS_ACTUALLY_TAP:
            fprintf(stderr, "\nERROR: Datafile is actually a TAP: %s\n", additional_message);
            exit(ERR_TZX_IS_ACTUALLY_TAP);
            break;
    }
    
    exit(EXIT_SUCCESS);
}