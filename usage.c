#include "usage.h"
#include "shutdown.h"

void usage(uint8_t exit_reason) {
    printf("\n tzx-pi             GPIO controlled TZX decoder (initially for NextPi)");
    printf("\n--------           ====================================================\n");
    printf("\n OPTION             EXPLINATION\n");
    printf("\n  -0               Don't send audio to soundcard");
    printf("\n  -d               Enable debug output");
    printf("\n  -l               Verbose Logging (inc sample data)");
    printf("\n  -t datafile.tzx  Read \"datafile.tzx\" for audiodata");

    printf("\n\n           Xalior, Stale Pixels - 2021, 2022 - some rights reserved ;)\n");
    
    // Were we invoked with a reason to exit?
    shutdown(exit_reason, NULL);
}